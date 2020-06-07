/* B.Schaffl
 * MIDI Pattern Delay Plugin
 *
 * Copyright (C) 2018 - 2020 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "BSchaffl.hpp"

#include <cstdio>
#include <string>
#include <stdexcept>
#include <algorithm>

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))

BSchaffl::BSchaffl (double samplerate, const LV2_Feature* const* features) :
	map(NULL),
	rate(samplerate), bpm(120.0f), speed(1), bar (0), barBeat (0),
	beatsPerBar (4), beatUnit (4),  positionSeq (0.0), latencySeq (0.0),
	refFrame(0),
	uiOn (false),
	midiData (),
	input(NULL), output(NULL),
	controllerPtrs {nullptr}, controllers {0.0f},
	stepPositions {0.0},
	message ()

{
	// Init array members
	std::fill (stepAutoPositions, stepAutoPositions + MAXSTEPS - 1, true);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (m, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);

	recalculateAutoPositions ();
}

BSchaffl::~BSchaffl () {}

void BSchaffl::connect_port(uint32_t port, void *data)
{
	switch (port) {
	case INPUT:
		input = (LV2_Atom_Sequence*) data;
		break;
	case OUTPUT:
		output = (LV2_Atom_Sequence*) data;
		break;
	default:
		if ((port >= CONTROLLERS) && (port < CONTROLLERS + NR_CONTROLLERS))
		{
			controllerPtrs[port - CONTROLLERS] = (float*) data;
		}
	}
}

void BSchaffl::run (uint32_t n_samples)
{

	// Check ports
	if ((!input) || (!output)) return;
	for (int i = 0; i < NR_CONTROLLERS; ++i)
	{
		if (!controllerPtrs[i]) return;
	}

	// Update controller values
	for (int i = 0; i < LATENCY; ++i)
	{
		const float newValue = controllerLimits[i].validate (*controllerPtrs[i]);

		if (newValue != controllers[i])
		{
			controllers[i] = newValue;

			if (i == SWING) recalculateAutoPositions();

			else if (i == NR_OF_STEPS) recalculateAutoPositions();

			else if ((i >= STEP_POS) && (i < STEP_POS + MAXSTEPS - 1))
			{
				const int step = i - STEP_POS;
				if (newValue == 0.0f)
				{
					if (!stepAutoPositions[step])
					{
						stepAutoPositions[step] = true;
						recalculateAutoPositions();
					}
				}

				else if (stepPositions[step] != newValue)
				{
					stepAutoPositions[step] = false;
					stepPositions[step] = newValue;
					recalculateAutoPositions();
				}
			}
		}
	}

	// Re-calculate latency
	latencySeq = 0;
	const int nrSteps = controllers[NR_OF_STEPS];
	for (int i = 0; i < nrSteps - 1; ++i)
	{
		const double inStartSeq = double (i) / double (nrSteps);
		const double outStartSeq = (i == 0 ? 0.0 : stepPositions[i - 1]);
		const double diffSeq = inStartSeq - outStartSeq;
		if (diffSeq > latencySeq) latencySeq = diffSeq;
	}

	// Report latency
	const float latencyFr = floor (getFrameFromSequence (latencySeq));
	*controllerPtrs[LATENCY] = latencyFr;

	// Prepare forge buffer and initialize atom sequence
	const uint32_t space = output->atom.size;
	lv2_atom_forge_set_buffer(&forge, (uint8_t*) output, space);
	lv2_atom_forge_sequence_head(&forge, &frame, 0);

	uint32_t last_t =0;

	// Process audio data
	LV2_ATOM_SEQUENCE_FOREACH (input, ev)
	{
		if ((ev->body.type == uris.atom_Object) || (ev->body.type == uris.atom_Blank))
		{
			bool scheduleUpdatePosition = false;

			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

			// GUI on
			if (obj->body.otype == uris.bschafflr_uiOn) uiOn = true;

			// GUI off
			else if (obj->body.otype == uris.bschafflr_uiOff) uiOn = false;

			// Update time/position data
			else if (obj->body.otype == uris.time_Position)
			{
				LV2_Atom *oBbeat = NULL, *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL, *oBar = NULL;

				lv2_atom_object_get
				(
					obj,
					uris.time_barBeat, &oBbeat,
					uris.time_beatsPerMinute,  &oBpm,
					uris.time_beatsPerBar,  &oBpb,
					uris.time_beatUnit,  &oBu,
					uris.time_bar,  &oBar,
					uris.time_speed, &oSpeed,
					NULL);

				// BPM changed?
				if (oBpm && (oBpm->type == uris.atom_Float) && (((LV2_Atom_Float*)oBpm)->body != bpm))
				{
					bpm = ((LV2_Atom_Float*)oBpm)->body;
					scheduleUpdatePosition = true;
					if (bpm < 1.0) message.setMessage (JACK_STOP_MSG);
					else message.deleteMessage (JACK_STOP_MSG);
				}

				// Beats per bar changed?
				if (oBpb && (oBpb->type == uris.atom_Float) && (((LV2_Atom_Float*)oBpb)->body > 0) && (((LV2_Atom_Float*)oBpb)->body != beatsPerBar))
				{
					beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;
					scheduleUpdatePosition = true;
				}

				// BeatUnit changed?
				if (oBu && (oBu->type == uris.atom_Int) && (((LV2_Atom_Int*)oBu)->body > 0) && (((LV2_Atom_Int*)oBu)->body != beatUnit))
				{
					beatUnit = ((LV2_Atom_Int*)oBu)->body;
					scheduleUpdatePosition = true;
				}

				// Speed changed?
				if (oSpeed && (oSpeed->type == uris.atom_Float) && (((LV2_Atom_Float*)oSpeed)->body != speed))
				{
					speed = ((LV2_Atom_Float*)oSpeed)->body;
					if (speed == 0.0) message.setMessage (JACK_STOP_MSG);
					else message.deleteMessage (JACK_STOP_MSG);
				}

				// Bar position changed
				if (oBar && (oBar->type == uris.atom_Long) && (bar != (((LV2_Atom_Long*)oBar)->body)))
				{
					bar = ((LV2_Atom_Long*)oBar)->body;
					scheduleUpdatePosition = true;
				}

				// Beat position changed ?
				if (oBbeat && (oBbeat->type == uris.atom_Float) && (((LV2_Atom_Float*)oBbeat)->body != barBeat))
				{
					barBeat = ((LV2_Atom_Float*)oBbeat)->body;
					scheduleUpdatePosition = true;
				}

				if (scheduleUpdatePosition)
				{
					// Hard set new position if new data received
					//float barsequencepos = ((LV2_Atom_Float*)oBbeat)->body * sequencesperbar / beatsPerBar; // Position within a bar (0..sequencesperbar)
					//position = MODFL (barsequencepos);
					positionSeq = getSequenceFromBeats (barBeat + beatsPerBar * bar);
					refFrame = ev->time.frames;
				}
			}
		}

		// Read incoming MIDI events
		else if (ev->body.type == uris.midi_Event)
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			MidiData midi = {{0, 0, 0}, 0, 0};
			midi.size = LIM (ev->body.size, 0, 3);
			memcpy (midi.msg, msg, midi.size);

			// Calculate position within sequence
			const double inputSeq = positionSeq + getSequenceFromFrame (ev->time.frames - refFrame);
			const float inputSeqPos = MODFL (inputSeq);

			// Calulate step
			const int nrOfSteps = controllers[NR_OF_STEPS];
			const int step = double (nrOfSteps) * inputSeqPos;

			// Calulate fractional position within step
			const float iprev = float (step) / float (nrOfSteps);
			const float inext = (step >= nrOfSteps - 1 ? 1.0f : float (step + 1) / float (nrOfSteps));
			const float ifrac = (iprev != inext ? (inputSeqPos - iprev) / (inext - iprev) : 0.0f);

			// Calculate output position
			const float oprev = (step == 0.0 ? 0.0f : stepPositions[step - 1]);
			const float onext = (step >= nrOfSteps - 1 ? 1.0f : stepPositions[step]);
			const float outputSeqPos = oprev + ifrac * (onext - oprev);
			midi.position = inputSeq + latencySeq + outputSeqPos - inputSeqPos;

			// Level MIDI NOTE_ON and NOTE_OFF
			if (((midi.msg[0] & 0xF0) == 0x80) || ((midi.msg[0] & 0xF0) == 0x90))
			{
				midi.msg[3] = float (midi.msg[3]) * controllers[STEP_LEV + step];
			}

			// Store MIDI data
			midiData.push_back (midi);
			fprintf
			(
				stderr,
				"BSchaffl.lv2 @ %1.10f: Schedule MIDI signal %i (%i,%i) to %1.10f\n",
				inputSeq,
				midi.msg[0],
				midi.msg[1],
				midi.msg[2],
				midi.position
			);
		}

		play (last_t, ev->time.frames);
		last_t = ev->time.frames;
	}

	// Play remaining samples
	if (last_t < n_samples) play (last_t, n_samples);

	// Update position in case of no new barBeat submitted on next call
	double relSeq = getSequenceFromFrame (n_samples - refFrame);
	positionSeq += relSeq;
	refFrame = 0;

	// Send collected data to GUI
	if (uiOn)
	{
		if (message.isScheduled ()) notifyMessageToGui();
		notifyStatusToGui();
	}

	// Close off sequence
	lv2_atom_forge_pop(&forge, &frame);
}

void BSchaffl::play (uint32_t start, uint32_t end)
{
	const double startSeq = positionSeq + getSequenceFromFrame (start - refFrame);
	const double endSeq = positionSeq + getSequenceFromFrame (end - refFrame);

	while ((!midiData.empty()) && midiData.front().position <= endSeq)
	{
		// Calculate frame
		int64_t frame = start;
		double seq = midiData.front().position;
		if (seq >= startSeq)
		{
			frame = start + getFrameFromSequence (seq - startSeq);
			//frame = LIM (frame, 0, end);
		}

		// Send MIDI
		LV2_Atom midiatom;
		midiatom.type = uris.midi_Event;
		midiatom.size = midiData.front().size;
		lv2_atom_forge_frame_time (&forge, frame);
		lv2_atom_forge_raw (&forge, &midiatom, sizeof (LV2_Atom));
		lv2_atom_forge_raw (&forge, &midiData.front().msg, midiatom.size);
		lv2_atom_forge_pad (&forge, sizeof (LV2_Atom) + midiatom.size);

		fprintf
		(
			stderr, "BSchaffl.lv2 @ %1.10f - %1.10f: Send MIDI %i (%i, %i) at %1.10f (frame %li (%i - %i))\n",
			startSeq,
			endSeq,
			midiData.front().msg[0],
			midiData.front().msg[1],
			midiData.front().msg[2],
			midiData.front().position,
			frame,
			start,
			end
		);

		// Remove sent data
		midiData.pop_front();
	}
}

double BSchaffl::getSequenceFromBeats (const double beats)
{
	if ((controllers[SEQ_LEN_VALUE] == 0.0) || (bpm == 0.0) || (beatsPerBar == 0.0)) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return (bpm ? beats / (controllers[SEQ_LEN_VALUE] * (bpm / 60.0)) : 0.0);
		case BEATS:	return beats / controllers[SEQ_LEN_VALUE];
		case BARS:	return (beatsPerBar ? beats / (controllers[SEQ_LEN_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

double BSchaffl::getBeatsFromSequence (const double sequence)
{
	if (controllers[SEQ_LEN_VALUE] == 0.0) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return (bpm ? sequence * (controllers[SEQ_LEN_VALUE] * (bpm / 60.0)) : 0.0);
		case BEATS:	return sequence * controllers[SEQ_LEN_VALUE];
		case BARS:	return (beatsPerBar ? sequence * (controllers[SEQ_LEN_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

double BSchaffl::getSequenceFromFrame (const int64_t frames)
{
	if ((controllers[SEQ_LEN_VALUE] == 0.0) || (rate == 0) || (bpm == 0.0) || (beatsPerBar == 0.0)) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return frames * (1.0 / rate) / controllers[SEQ_LEN_VALUE] ;
		case BEATS:	return (bpm ? frames * (speed / (rate / (bpm / 60))) / controllers[SEQ_LEN_VALUE] : 0.0);
		case BARS:	return (bpm && beatsPerBar ? frames * (speed / (rate / (bpm / 60))) / (controllers[SEQ_LEN_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

int64_t BSchaffl::getFrameFromSequence (const double sequence)
{
	if ((controllers[SEQ_LEN_VALUE] == 0.0) || (rate == 0) || (bpm == 0.0) || (beatsPerBar == 0.0) || (speed == 0.0)) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return sequence / ((1.0 / rate) / controllers[SEQ_LEN_VALUE]);
		case BEATS:	return (bpm ? sequence / ((speed / (rate / (bpm / 60))) / controllers[SEQ_LEN_VALUE]) : 0.0);
		case BARS:	return (bpm && beatsPerBar ? sequence / ((speed / (rate / (bpm / 60))) / (controllers[SEQ_LEN_VALUE] * beatsPerBar)) : 0.0);
		default:	return 0.0;
	}
}

void BSchaffl::recalculateAutoPositions ()
{
	int nrMarkers = controllers[NR_OF_STEPS] - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		//fprintf(stderr, "%f ", controllers[STEP_POS + i]);
		if (stepAutoPositions[i])
		{
			if ((i == nrMarkers - 1) || (!stepAutoPositions[i + 1]))
			{
				double s = 2.0 * controllers[SWING] / (controllers[SWING] + 1.0);
				double anc = (start == 0 ? 0 : stepPositions[start - 1]);
				double suc = (i == nrMarkers - 1 ? 1 : stepPositions[i + 1]);
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - s : s) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - s : s);
					anc += f * step;
					stepPositions[j] = anc;
				}
			}
		}
		else start = i + 1;
	}
	//fprintf(stderr, "%i\n", nrMarkers);
}

void BSchaffl::notifyStatusToGui ()
{
	// Calculate step
	int outStep = 0;
	const double seqPos = fmod (positionSeq, 1.0);
	while ((outStep < controllers[NR_OF_STEPS] - 1) && (seqPos > stepPositions[outStep])) ++outStep;

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time (&forge, 0);
	lv2_atom_forge_object (&forge, &frame, 0, uris.bschafflr_statusEvent);
	lv2_atom_forge_key (&forge, uris.bschafflr_step);
	lv2_atom_forge_int (&forge, outStep);
	lv2_atom_forge_pop (&forge, &frame);
}

void BSchaffl::notifyMessageToGui ()
{
	uint32_t messageNr = message.loadMessage ();

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, uris.notify_messageEvent);
	lv2_atom_forge_key(&forge, uris.notify_message);
	lv2_atom_forge_int(&forge, messageNr);
	lv2_atom_forge_pop(&forge, &frame);
}

LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BSchaffl* instance;
	try {instance = new BSchaffl(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BSchaffl.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	if (!instance)
	{
		fprintf(stderr, "BSchaffl.lv2: Plugin instantiation failed.\n");
		return NULL;
	}

	if (!instance->map)
	{
		fprintf(stderr, "BSchaffl.lv2: Host does not support urid:map.\n");
		delete (instance);
		return NULL;
	}

	return (LV2_Handle)instance;
}

void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BSchaffl* inst = (BSchaffl*) instance;
	inst->connect_port (port, data);
}

void run (LV2_Handle instance, uint32_t n_samples)
{
	BSchaffl* inst = (BSchaffl*) instance;
	inst->run (n_samples);
}

void cleanup (LV2_Handle instance)
{
	BSchaffl* inst = (BSchaffl*) instance;
	delete inst;
}

const LV2_Descriptor descriptor =
{
	BSCHAFFL_URI,
	instantiate,
	connect_port,
	NULL, //activate,
	run,
	NULL, //deactivate,
	cleanup,
	NULL //extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index)
	{
		case 0: return &descriptor;
		default: return NULL;
	}
}
