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
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include "BUtilities/stof.hpp"
#include "SharedData.hpp"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))

static SharedData sharedData[4] = {SharedData(), SharedData(), SharedData(), SharedData()};

BSchaffl::BSchaffl (double samplerate, const LV2_Feature* const* features) :
	map(NULL),
	rate(samplerate), bpm(120.0f), speed(1), bar (0), barBeat (0),
	beatsPerBar (4), beatUnit (4),  positionSeq (0.0),
	latencySeq (0.0), latencyFr (0),
	refFrame(0),
	uiOn (false),
	actStep (-1),
	midiData (),
	input(NULL), output(NULL),
	sharedDataNr (0),
	controllerPtrs {nullptr}, controllers {0.0f},
	stepPositions {0.0},
	shape {Shape<MAXNODES>()},
	message (),
	notify_shape (true),
	notify_controllers {false}

{
	// Init array members
	std::fill (stepAutoPositions, stepAutoPositions + MAXSTEPS - 1, true);
	std::fill (stepRnds, stepRnds + MAXSTEPS - 1, 1.0);

	shape.setDefaultShape ();

	// Init random engine
	srand (time (0));

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

BSchaffl::~BSchaffl ()
{
	if ((sharedDataNr > 0) && (sharedDataNr <= 4)) sharedData[sharedDataNr - 1].unlink (this);
}

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

		float newValue;

		// Sync with control ports
		if (sharedDataNr == 0) newValue = controllerLimits[i].validate (*controllerPtrs[i]);

		// Otherwise sync with globally shared data
		else if (sharedDataNr <= 4) newValue = controllerLimits[i].validate (sharedData[sharedDataNr - 1].get (i));

		if (newValue != controllers[i])
		{
			controllers[i] = newValue;
			/*if ((sharedDataNr >= 1) && (sharedDataNr <= 4))*/ notify_controllers[i] = true;

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

	recalculateLatency();
	*controllerPtrs[LATENCY] = latencyFr;
	if (latencyFr > 192000) message.setMessage (LATENCY_MAX_MSG);
	else message.deleteMessage (LATENCY_MAX_MSG);

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
			if (obj->body.otype == uris.bschaffl_uiOn) uiOn = true;

			// GUI off
			else if (obj->body.otype == uris.bschaffl_uiOff) uiOn = false;

			// Linked / unlinked to shared data
			else if (obj->body.otype == uris.bschaffl_sharedDataLinkEvent)
			{
				LV2_Atom *oNr = NULL;

				lv2_atom_object_get
				(
					obj,
					uris.bschaffl_sharedDataNr, &oNr,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int))
				{
					const int nr = ((LV2_Atom_Int*)oNr)->body;

					if ((nr >= 0) && (nr <= 4))
					{
						if (sharedDataNr != 0) sharedData[sharedDataNr - 1].unlink (this);
						if ((nr != 0) && sharedData[nr - 1].empty())
						{
							for (int i = 0; i < NR_CONTROLLERS; ++i) sharedData[nr - 1].set (i, controllers[i]);
						}
						if (nr != 0) sharedData[nr - 1].link (this);
						sharedDataNr = nr;
					}
				}
			}

			// Controller changed
			else if ((obj->body.otype == uris.bschaffl_controllerEvent) && (sharedDataNr != 0))
			{
				LV2_Atom *oNr = NULL, *oVal = NULL;

				lv2_atom_object_get
				(
					obj,
					uris.bschaffl_controllerNr, &oNr,
					uris.bschaffl_controllerValue, &oVal,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int) && oVal && (oVal->type == uris.atom_Float))
				{
					const int nr =  ((LV2_Atom_Int*)oNr)->body;

					if ((nr >= 0) && (nr < NR_CONTROLLERS))
					{
						const float val = controllerLimits[nr].validate(((LV2_Atom_Float*)oVal)->body);
						controllers[nr] = val;
						sharedData[sharedDataNr - 1].set (nr, val);

						if (nr == SWING) recalculateAutoPositions();

						else if (nr == NR_OF_STEPS) recalculateAutoPositions();

						else if ((nr >= STEP_POS) && (nr < STEP_POS + MAXSTEPS - 1))
						{
							const int step = nr - STEP_POS;
							if (val == 0.0f)
							{
								if (!stepAutoPositions[step])
								{
									stepAutoPositions[step] = true;
									recalculateAutoPositions();
								}
							}

							else if (stepPositions[step] != val)
							{
								stepAutoPositions[step] = false;
								stepPositions[step] = val;
								recalculateAutoPositions();
							}
						}
					}
				}
			}

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
					//fprintf(stderr, "BSchaffl.lv2: Change position %f -> ", positionSeq);
					positionSeq = getSequenceFromBeats (barBeat + beatsPerBar * bar);
					//fprintf(stderr, "%f\n", positionSeq);
					refFrame = ev->time.frames;
				}

				recalculateLatency();
			}

			// Shape changed notifications
			else if (obj->body.otype == uris.bschaffl_shapeEvent)
			{
				LV2_Atom *oData = NULL;
				lv2_atom_object_get (obj,
						     uris.bschaffl_shapeData, &oData,
						     NULL);

				if (oData && (oData->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oData;
					if (vec->body.child_type == uris.atom_Float)
					{
						shape.clearShape ();
						const uint32_t vecSize = (uint32_t) ((oData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float)));
						float* data = (float*) (&vec->body + 1);
						for (unsigned int i = 0; (i < vecSize) && (i < MAXNODES); ++i)
						{
							Node node;
							node.nodeType = NodeType (int (data[i * 7]));
							node.point.x = data[i * 7 + 1];
							node.point.y = data[i * 7 + 2];
							node.handle1.x = data[i * 7 + 3];
							node.handle1.y = data[i * 7 + 4];
							node.handle2.x = data[i * 7 + 5];
							node.handle2.y = data[i * 7 + 6];
							shape.appendRawNode (node);
						}
						shape.validateShape();
					}
				}
			}
		}

		// Read incoming MIDI events
		else if (ev->body.type == uris.midi_Event)
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			MidiData midi = {{0, 0, 0}, 0, 0, 0, 0, false};
			midi.size = LIM (ev->body.size, 0, 3);
			memcpy (midi.msg, msg, midi.size);

			// Filter channel and message
			bool process = filterMsg (midi.msg[0]) && (controllers[MIDI_CH_FILTER + (midi.msg[0] & 0x0F)] != 0.0f);

			// Calculate position within sequence
			const double inputSeq =	positionSeq +
						getSequenceFromFrame (ev->time.frames - refFrame, speed) +
						(controllers[TIME_COMPENS] != 1.0f ? latencySeq : 0);
			const float inputSeqPos = MODFL (inputSeq);

			// Calulate step
			const int nrOfSteps = controllers[NR_OF_STEPS];
			const int step = LIM (double (nrOfSteps) * inputSeqPos, 0, nrOfSteps - 1);

			// Update step
			if ((actStep < 0) || (step != actStep))
			{
				// Sequence just started: Re-randomize sequence
				if (step < actStep)
				{
					for (int i = 0; i < nrOfSteps; ++i) randomizeStep (i);
				}

				actStep = step;
			}

			// Calulate fractional position within step
			const float iprev = float (step) / float (nrOfSteps);
			const float inext = (step >= nrOfSteps - 1 ? 1.0f : float (step + 1) / float (nrOfSteps));
			const float ifrac = (iprev != inext ? (inputSeqPos - iprev) / (inext - iprev) : 0.0f);

			// Calculate output position
			const float qrange = (controllers[QUANT_POS] != 0.0f ? controllers[QUANT_RANGE] : 0.0);
			const float oprev = getStepStart (step);
			const float onext = getStepEnd (step);
			const float ofrac = (ifrac <= qrange ? 0.0f : (1.0f - ifrac < qrange ? 1.0f : ifrac));
			const float outputSeqPos = oprev + ofrac * (onext - oprev);
			const float origin = inputSeq + latencySeq;
			const float shift = (outputSeqPos - inputSeqPos) * controllers[SWING_PROCESS];
			midi.shiftSeq = (process? shift : 0);

			// Garbage collection
			// Removes data from queue which are scheduled to a time point later than the latest possible time point
			const float maxProcSeq = origin + onext - inputSeqPos;
			const float maxSeq = (origin > maxProcSeq ? origin : maxProcSeq);
			clearMidiData (maxSeq);

			// MIDI NOTE_ON and NOTE_OFF
			if
			(
				(
					((midi.msg[0] & 0xF0) == 0x80) ||
					((midi.msg[0] & 0xF0) == 0x90)
				) &&
				process
			)
			{
				// Map step (smart quantization)
				const float mrange = (controllers[QUANT_MAP] != 0.0f ? controllers[QUANT_RANGE] : 0.0);
				const int map =
				(
					(midi.msg[0] & 0xF0) == 0x80 ?
					(ifrac < mrange ? ((step + nrOfSteps - 1) % nrOfSteps) : step) :
					(1.0 - ifrac < mrange ? ((step + 1) % nrOfSteps) : step)
				);

				// Calculate and set amp
				float aswing = ((map % 2) == 0 ? controllers[AMP_SWING] : 1.0 / controllers[AMP_SWING]);
				aswing = LIM (aswing, 0, 1);
				const float rnd = 1.0f + controllers[AMP_RANDOM] * (2.0f * float (rand()) / float (RAND_MAX) - 1.0f);
				float amp =
				(
					controllers[AMP_MODE] == 0.0f ?
					controllers[STEP_LEV + map] * rnd * aswing :
					LIM (shape.getMapValue (inputSeqPos), 0.0, 1.0) * rnd
				);

				// NOTE_OFF
				if ((midi.msg[0] & 0xF0) == 0x80)
				{
					const int nr =
					(
						(controllers[NOTE_OFF_AMP] == 0) || controllers[NOTE_POSITION_STR] || controllers[NOTE_VALUE_STR] ?
						getNoteOnMsg (midi.msg[0] & 0x0F, midi.msg[1]) :
						-1
					);
					const float s1 = (nr >= 0 ? midiData[nr].shiftSeq : shift);

					// Use note on amp ? Look for corresponding NOTE_ON
					if (controllers[NOTE_OFF_AMP] == 0)
					{
						if (nr >= 0) amp = midiData[nr].amp;
					}

					// Calculate shift
					midi.positionSeq = origin +
					(
						controllers[NOTE_POSITION_STR] ?
						(controllers[NOTE_VALUE_STR] ? 0 : shift - s1) :
						(controllers[NOTE_VALUE_STR] ? s1 : shift)
					);

					// Handle overlapping notes
					if
					(
						(nr >= 0) &&
						(controllers[NOTE_POSITION_STR] || controllers[NOTE_VALUE_STR]) &&
						(controllers[NOTE_OVERLAP] == OVERLAP_MERGE)
					)
					{
						const int noteOffNr = getNoteOffMsg (midi.msg[0] & 0x0F, midi.msg[1], nr);

						if (noteOffNr >= 0)
						{
							// Nested notes: Ignore inner NOTE_OFF
							 if (midi.positionSeq <= midiData[noteOffNr].positionSeq)
							{
								midi.msg[0] = 0;
							}

							// Overlapping notes: Remove inner NOTE_OFF
							else
							{
								/*fprintf
								(
									stderr,
									"BSchaffl.lv2: Remove MIDI signal from #%i %i (%i,%i) to %f (shift = %f, latency = %f)\n",
									noteOffNr,
									midiData[noteOffNr].msg[0],
									midiData[noteOffNr].msg[1],
									midiData[noteOffNr].msg[2],
									midiData[noteOffNr].positionSeq,
									midiData[noteOffNr].shiftSeq,
									latencySeq
								);*/

								midiData.erase (&midiData.iterator[noteOffNr]);
							}
						}

					}
				}

				// NOTE_ON ?
				else
				{
					//Store amp
					midi.amp = amp;

					// Calculate shift
					if (controllers[NOTE_POSITION_STR]) midi.positionSeq = origin;
					else midi.positionSeq = origin + shift;

					// Handle overlapping notes
					if (controllers[NOTE_POSITION_STR] || controllers[NOTE_VALUE_STR])
					{
						// Split: Insert or move NOTE_OFF just before new NOTE_ON
						if (controllers[NOTE_OVERLAP] == OVERLAP_SPLIT)
						{
							const int noteOnNr = getNoteOnMsg (midi.msg[0] & 0x0F, midi.msg[1]);
							if (noteOnNr >= 0)
							{
								const int noteOffNr = getNoteOffMsg (midi.msg[0] & 0x0F, midi.msg[1], noteOnNr);
								const MidiData noteOff =
								(
									noteOffNr >= 0 ?
									MidiData
									{
										{0x80 || (midi.msg[0] & 0x0F), midi.msg[1], midiData[noteOffNr].msg[2]},
										midiData[noteOffNr].size,
										midi.positionSeq,
										midi.shiftSeq,
										midiData[noteOffNr].amp,
										midiData[noteOffNr].inactive
									} :
									MidiData
									{
										{0x80 || (midi.msg[0] & 0x0F), midi.msg[1], midi.msg[2]},
										midi.size,
										midi.positionSeq,
										midi.shiftSeq,
										midi.amp,
										midi.inactive
									}
								);

								if ((noteOffNr >= 0) && (midi.positionSeq < midiData[noteOffNr].positionSeq))
								{
									// Remome old NOTE_OFF
									/*fprintf
									(
										stderr,
										"BSchaffl.lv2: Remove MIDI signal from #%i %i (%i,%i) to %f (shift = %f, latency = %f)\n",
										noteOffNr,
										midiData[noteOffNr].msg[0],
										midiData[noteOffNr].msg[1],
										midiData[noteOffNr].msg[2],
										midiData[noteOffNr].positionSeq,
										midiData[noteOffNr].shiftSeq,
										latencySeq
									);*/

									midiData.erase (&midiData.iterator[noteOffNr]);

									// Insert NOTE_OFF
									queueMidiData (noteOff);
								}

							}
						}

						// Merge: Ignore new NOTE_ON
						else if (controllers[NOTE_OVERLAP] == OVERLAP_MERGE)
						{
							const int nr = getNoteOnMsg (midi.msg[0] & 0x0F, midi.msg[1]);
							if (nr >= 0)
							{
								const int noteOffNr = getNoteOffMsg (midi.msg[0] & 0x0F, midi.msg[1], nr);
								if ((noteOffNr < 0) || (midi.positionSeq < midiData[noteOffNr].positionSeq))
								{
									midi.msg[0] = 0;
								}
							}
						}
					}
				}

				// Apply amp
				const float proc = controllers[AMP_PROCESS];
				const float invel = float (midi.msg[2]);
				const float outvel = invel + (invel * amp - invel) * proc;
				midi.msg[2] = LIM (outvel, 0, 127);
			}

			else midi.positionSeq = origin + midi.shiftSeq;

			// Store MIDI data
			if (midi.msg[0]) queueMidiData (midi);
		}

		play (last_t, ev->time.frames);
		last_t = ev->time.frames;
	}

	// Play remaining samples
	if (last_t < n_samples) play (last_t, n_samples);

	// Update position in case of no new barBeat submitted on next call
	double relSeq = getSequenceFromFrame (n_samples - refFrame, speed);
	positionSeq += relSeq;
	refFrame = 0;

	// Send collected data to GUI
	if (uiOn)
	{
		if (message.isScheduled ()) notifyMessageToGui();
		notifyStatusToGui();
		for (int i = 0; i < NR_CONTROLLERS; ++i) if (notify_controllers[i]) notifyControllerToGui (i);
		if (notify_shape) notifyShapeToGui();
	}

	// Close off sequence
	lv2_atom_forge_pop(&forge, &frame);
}

void BSchaffl::play (uint32_t start, uint32_t end)
{
	const double startSeq = positionSeq + getSequenceFromFrame (start - refFrame, speed) + (controllers[TIME_COMPENS] != 1.0f ? latencySeq : 0);
	const double endSeq = positionSeq + getSequenceFromFrame (end - refFrame, speed) + (controllers[TIME_COMPENS] != 1.0f ? latencySeq : 0);

	// Send midi data to output
	for (unsigned int i = 0; i < midiData.size; ++i)
	{
		if ((midiData[i].positionSeq <= endSeq) && (!midiData[i].inactive))
		{
			// Calculate frame
			int64_t frame = start;
			double seq = midiData[i].positionSeq;
			if (seq >= startSeq)
			{
				frame = start + getFrameFromSequence (seq - startSeq, speed);
				//frame = LIM (frame, 0, end);
			}

			// Send MIDI
			LV2_Atom midiatom;
			midiatom.type = uris.midi_Event;
			midiatom.size = midiData[i].size;
			lv2_atom_forge_frame_time (&forge, frame);
			lv2_atom_forge_raw (&forge, &midiatom, sizeof (LV2_Atom));
			lv2_atom_forge_raw (&forge, &midiData[i].msg, midiatom.size);
			lv2_atom_forge_pad (&forge, sizeof (LV2_Atom) + midiatom.size);

			midiData[i].inactive = true;

			/*fprintf
			(
				stderr, "BSchaffl.lv2 @ %f: Send Midi #%i %i (%i, %i) (frame = %li)  \n",
				seq,
				i,
				midiData[i].msg[0],
				midiData[i].msg[1],
				midiData[i].msg[2],
				frame
		 	);*/
		}
	}

	// Remove sent data
	for (MidiData** it = midiData.begin(); (it < midiData.end()) && ((**it).positionSeq <= endSeq); )
	{
		if ((**it).inactive)
		{
			// NOTE_OFF ?
			if (((**it).msg[0] & 0xF0) == 0x80)
			{
				// Erase corresponding NOTE_ON
				uint8_t ch = (**it).msg[0] & 0x0F;
				uint8_t note = (**it).msg[1];
				for (MidiData** nit = midiData.begin(); nit < it; )
				{
					if
					(
						(((**nit).msg[0] & 0xF0) == 0x90) &&
						(((**nit).msg[0] & 0x0F) == ch) &&
						((**nit).msg[1]  == note)
					)
					{
						nit = midiData.erase (nit);
						--it;
					}

					else ++nit;
				}

				// Erase NOTE_OFF and go to next it
				it = midiData.erase (it);
			}

			// NOTE_ON ? Next it
			else if (((**it).msg[0] & 0xF0) == 0x90) ++it;

			// Otherwise erase and go to next it
			else it = midiData.erase (it);

		}

		// Otherwise next it
		else ++it;
	}
}

void BSchaffl::randomizeStep (const int step)
{
	const int nrOfSteps = controllers[NR_OF_STEPS];
	if ((step < nrOfSteps - 1) && (step >= 0))
	{
		// Randomize 0.0 .. 2.0
		float rnd = 1.0f + controllers[SWING_RANDOM] * (2.0f * float (rand()) / float (RAND_MAX) - 1.0f);

		// Prevent overlap with antecessor
		if ((step >= 1) && (rnd < stepRnds[step - 1] - 1.0f)) rnd = stepRnds[step - 1] - 1.0f;

		stepRnds[step] = rnd;
	}
}

double BSchaffl::getStepStart (const int step)
{
	const int nrOfSteps = controllers[NR_OF_STEPS];
	const int s = step % nrOfSteps;
	if (s == 0) return 0.0;
	else
	{
		const float rnd = stepRnds[s - 1];
		return
		(
			rnd < 1.0f ?
			(
				s - 1 > 0 ?
				stepPositions[s - 1] - (1.0f - rnd) * (stepPositions[s - 1] - stepPositions[s - 2]) :
				stepPositions[s - 1] - (1.0f - rnd) * stepPositions[s - 1]
			) :
			(
				s - 1 < nrOfSteps - 2 ?
				stepPositions[s - 1] + (rnd - 1.0f) * (stepPositions[s] - stepPositions[s - 1]) :
				stepPositions[s - 1] + (rnd - 1.0f) * (1.0f - stepPositions[s - 1])
			)
		);
	}
}

double BSchaffl::getStepEnd (const int step)
{
	const int nrOfSteps = controllers[NR_OF_STEPS];
	const int s = step % nrOfSteps;
	if (s == nrOfSteps - 1) return 1.0;
	else return getStepStart (s + 1);
}

int BSchaffl::getNoteOnMsg (const uint8_t ch, const uint8_t note, int start) const
{
	for (int i = ((start >= 0) && (start < int (midiData.size)) ? start : midiData.size - 1); i >= 0; --i)
	{
		if
		(
			((midiData[i].msg[0] & 0xF0) == 0x90) &&
			((midiData[i].msg[0] & 0x0F) == ch) &&
			(midiData[i].msg[1] == note)
		) return i;
	}

	return -1;
}

int BSchaffl::getNoteOffMsg (const uint8_t ch, const uint8_t note, int start) const
{
	for (int i = ((start >= 0) && (start < int (midiData.size)) ? start : 0); i < int (midiData.size); ++i)
	{
		if
		(
			((midiData[i].msg[0] & 0xF0) == 0x80) &&
			((midiData[i].msg[0] & 0x0F) == ch) &&
			(midiData[i].msg[1] == note)
		) return i;
	}

	return -1;
}

void BSchaffl::clearMidiData (const float maxSeq)
{
	while ((!midiData.empty()) && (midiData.back().positionSeq > maxSeq))
	{
		/*fprintf
		(
			stderr,
			"BSchaffl.lv2 @: Remove MIDI signal %i (%i,%i) from %f (maxSeq = %f)\n",
			midiData.back().msg[0],
			midiData.back().msg[1],
			midiData.back().msg[2],
			midiData.back().positionSeq,
			maxSeq
		);*/

		midiData.pop_back();
	}
}

void BSchaffl::queueMidiData (const MidiData& midi)
{
	for (MidiData** m = midiData.end(); m >= midiData.begin(); --m)
	{
		if ((m == midiData.begin()) || ((**(m - 1)).positionSeq <= midi.positionSeq))
		{
			midiData.insert (m, midi);
			break;
		}
	}

	/*fprintf
	(
		stderr,
		"BSchaffl.lv2: Schedule MIDI signal #%li %i (%i,%i) to %f (shift = %f, latency = %f)\n",
		midiData.size,
		midi.msg[0],
		midi.msg[1],
		midi.msg[2],
		midi.positionSeq,
		midi.shiftSeq,
		latencySeq
	);*/
}

double BSchaffl::getSequenceFromBeats (const double beats)
{
	if (controllers[SEQ_LEN_VALUE] == 0.0) return 0.0;

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

double BSchaffl::getSequenceFromFrame (const int64_t frames, float speed)
{
	if (controllers[SEQ_LEN_VALUE] == 0.0) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return (rate ? frames * (1.0 / rate) / controllers[SEQ_LEN_VALUE] : 0.0);
		case BEATS:	return (rate ? frames * speed * bpm / (60.0 * rate * controllers[SEQ_LEN_VALUE]) : 0.0);
		case BARS:	return (rate && beatsPerBar ? frames * speed * bpm / (60.0 * rate * controllers[SEQ_LEN_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

int64_t BSchaffl::getFrameFromSequence (const double sequence, float speed)
{
	if (controllers[SEQ_LEN_VALUE] == 0.0) return 0.0;

	switch (int (controllers[SEQ_LEN_BASE]))
	{
		case SECONDS: 	return sequence * rate * controllers[SEQ_LEN_VALUE];
		case BEATS:	return (speed && bpm ? 60 * sequence * rate * controllers[SEQ_LEN_VALUE] / (speed * bpm) : 0.0);
		case BARS:	return (speed && bpm ? 60.0 * sequence * beatsPerBar * rate * controllers[SEQ_LEN_VALUE] / (speed * bpm) : 0.0);
		default:	return 0.0;
	}
}

bool BSchaffl::filterMsg (const uint8_t msg)
{
	uint8_t msgGroup = msg / 16;
	return
	(
		msgGroup < 8 ?
		false :
		(
			msgGroup == 8 ?
			(controllers[MSG_FILTER_NOTE] != 0.0f) :
			(controllers[MSG_FILTER_NOTE + msgGroup - 9] != 0.0f)
		)
	);
}

void BSchaffl::recalculateLatency ()
{
	if (controllers[USR_LATENCY] != 0.0f)
	{
		latencyFr = controllers[USR_LATENCY_FR];
		latencySeq = getSequenceFromFrame (latencyFr);
	}

	else
	{
		double qLatencySeq = (controllers[QUANT_POS] != 0.0f ? controllers[QUANT_RANGE] / controllers[NR_OF_STEPS] : 0.0);

		latencySeq = 0;
		const int nrSteps = controllers[NR_OF_STEPS];
		for (int i = 0; i < nrSteps - 1; ++i)
		{
			const double inStartSeq = double (i) / double (nrSteps);
			const double outStartSeq = (i == 0 ? 0.0 : stepPositions[i - 1]);
			const double rndSeq = (i == 0 ? 0.0 : controllers[SWING_RANDOM] * (stepPositions[i - 1] - (i == 1 ? 0.0 : stepPositions[i - 2])));
			const double diffSeq = inStartSeq - (outStartSeq - rndSeq);
			if (diffSeq > latencySeq) latencySeq = diffSeq;
		}

		latencySeq += qLatencySeq;
		latencyFr = getFrameFromSequence (latencySeq);
	}
}

void BSchaffl::recalculateAutoPositions ()
{
	int nrMarkers = controllers[NR_OF_STEPS] - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
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
}

void BSchaffl::notifyControllerToGui (const int nr)
{
	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time (&forge, 0);
	lv2_atom_forge_object (&forge, &frame, 0, uris.bschaffl_controllerEvent);
	lv2_atom_forge_key (&forge, uris.bschaffl_controllerNr);
	lv2_atom_forge_int (&forge, nr);
	lv2_atom_forge_key (&forge, uris.bschaffl_controllerValue);
	lv2_atom_forge_float (&forge, controllers[nr]);
	lv2_atom_forge_pop (&forge, &frame);
	notify_controllers[nr] = false;
}

void BSchaffl::notifyStatusToGui ()
{
	// Calculate step
	int outStep = 0;
	const double seqPos = fmod (positionSeq - (controllers[TIME_COMPENS] == 1.0f ? latencySeq : 0), 1.0);
	while ((outStep < controllers[NR_OF_STEPS] - 1) && (seqPos > getStepEnd (outStep))) ++outStep;

	// Calculate latency in ms
	const float latencyMs = float (latencyFr) * 1000.0f / rate;

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time (&forge, 0);
	lv2_atom_forge_object (&forge, &frame, 0, uris.bschaffl_statusEvent);
	lv2_atom_forge_key (&forge, uris.bschaffl_step);
	lv2_atom_forge_int (&forge, outStep);
	lv2_atom_forge_key (&forge, uris.bschaffl_latency);
	lv2_atom_forge_float (&forge, latencyMs);
	lv2_atom_forge_key (&forge, uris.bschaffl_rate);
	lv2_atom_forge_double (&forge, rate);
	lv2_atom_forge_pop (&forge, &frame);
}

void BSchaffl::notifyShapeToGui ()
{
	size_t size = shape.size ();

	// Load shapeBuffer
	float shapeBuffer[MAXNODES * 7];
	for (unsigned int i = 0; i < size; ++i)
	{
		Node node = shape.getRawNode (i);
		shapeBuffer[i * 7] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, uris.bschaffl_shapeEvent);
	lv2_atom_forge_key(&forge, uris.bschaffl_shapeData);
	lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&forge, &frame);

	notify_shape = false;
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

LV2_State_Status BSchaffl::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{

	char shapesDataString[0x8010] = "Shape data:\n";

	for (unsigned int nd = 0; nd < shape.size (); ++nd)
	{
		char valueString[160];
		Node node = shape.getNode (nd);
		snprintf
		(
			valueString,
			126,
			"typ:%d; ptx:%f; pty:%f; h1x:%f; h1y:%f; h2x:%f; h2y:%f",
			int (node.nodeType),
			node.point.x,
			node.point.y,
			node.handle1.x,
			node.handle1.y,
			node.handle2.x,
			node.handle2.y
		);
		if (nd < shape.size ()) strcat (valueString, ";\n");
		else strcat (valueString, "\n");
		strcat (shapesDataString, valueString);
	}

	store (handle, uris.bschaffl_shapeData, shapesDataString, strlen (shapesDataString) + 1, uris.atom_String, LV2_STATE_IS_POD);

	return LV2_STATE_SUCCESS;
}

LV2_State_Status BSchaffl::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	size_t   size;
	uint32_t type;
	uint32_t valflags;

	const void* shapesData = retrieve (handle, uris.bschaffl_shapeData, &size, &type, &valflags);
	if (shapesData && (type == uris.atom_String))
	{
		// Clear old shape first
		shape.clearShape();

		// Parse retrieved data
		std::string shapesDataString = (char*) shapesData;
		const std::string keywords[7] = {"typ:", "ptx:", "pty:", "h1x:", "h1y:", "h2x:", "h2y:"};
		while (!shapesDataString.empty())
		{
			// Look for next "typ:"
			size_t strPos = shapesDataString.find ("typ:");
			size_t nextPos = 0;
			if (strPos == std::string::npos) break;			// No "typ:" found => end
			if (strPos + 4 > shapesDataString.length()) break;	// Nothing more after id => end
			shapesDataString.erase (0, strPos + 4);

			int typ;
			try {typ = BUtilities::stof (shapesDataString, &nextPos);}
			catch  (const std::exception& e)
			{
				fprintf (stderr, "BSchaffl.lv2: Restore shape incomplete. Can't parse shape node type from \"%s...\"", shapesDataString.substr (0, 63).c_str());
				break;
			}

			if (nextPos > 0) shapesDataString.erase (0, nextPos);

			// Look for shape data
			Node node = {(NodeType) typ, {0, 0}, {0, 0}, {0, 0}};
			for (int i = 1; i < 7; ++i)
			{
				strPos = shapesDataString.find (keywords[i]);
				if (strPos == std::string::npos) continue;	// Keyword not found => next keyword
				if (strPos + 4 >= shapesDataString.length())	// Nothing more after keyword => end
				{
					shapesDataString ="";
					break;
				}
				if (strPos > 0) shapesDataString.erase (0, strPos + 4);
				float val;
				try {val = BUtilities::stof (shapesDataString, &nextPos);}
				catch  (const std::exception& e)
				{
					fprintf (stderr, "BSchaffl.lv2: Restore shape incomplete. Can't parse %s from \"%s...\"",
							 keywords[i].substr(0,3).c_str(), shapesDataString.substr (0, 63).c_str());
					break;
				}

				if (nextPos > 0) shapesDataString.erase (0, nextPos);
				switch (i)
				{
					case 1: node.point.x = val;
						break;
					case 2:	node.point.y = val;
						break;
					case 3:	node.handle1.x = val;
						break;
					case 4:	node.handle1.y = val;
						break;
					case 5:	node.handle2.x = val;
						break;
					case 6:	node.handle2.y = val;
						break;
					default:break;
				}
			}

			// Set data
			shape.appendNode (node);
		}

		// Validate all shapes
		if ((shape.size () < 2) || (!shape.validateShape ())) shape.setDefaultShape ();

		// Force GUI notification
		notify_shape = true;
	}

	return LV2_STATE_SUCCESS;
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

static LV2_State_Status state_save(LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BSchaffl* inst = (BSchaffl*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_save (store, handle, flags, features);
}

static LV2_State_Status state_restore(LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BSchaffl* inst = (BSchaffl*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_restore (retrieve, handle, flags, features);
}

void cleanup (LV2_Handle instance)
{
	BSchaffl* inst = (BSchaffl*) instance;
	delete inst;
}

static const void* extension_data(const char* uri)
{
	static const LV2_State_Interface state  = {state_save, state_restore};
	if (!strcmp(uri, LV2_STATE__interface)) return &state;
	return NULL;
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
	extension_data
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
