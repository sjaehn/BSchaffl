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

#include "BSchafflGUI.hpp"
#include "Ports.hpp"
#include "screen.h"


BSchafflGUI::BSchafflGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow) :
	Window (955, 470, "B.Schaffl", parentWindow, true),
	controller (NULL), write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),

	mContainer (0, 0, 955, 470, "main"),

	helpButton (885, 60, 24, 24, "halobutton", "Help"),
	//ytButton (50, 80, 24, 24, "halobutton", "Tutorial"),

	midiChFilterIcon (0, 0, 300, 20, "widget", pluginPath + "inc/midi_ch_filter.png"),
	midiChFilterContainer (0, 0, 340, 140, "screen"),
	midiChFilterText (10, 10, 320, 50, "text", "MIDI channels to be processed by the plugin."),
	midiChFilterAllSwitch (10, 36, 28, 14, "slider", 1),
	midiChFilterAllLabel (44, 33, 120, 20, "lflabel", "All"),

	midiMsgFilterIcon (0, 0, 300, 20, "widget", pluginPath + "inc/midi_msg_filter.png"),
	midiMsgFilterContainer (0, 0, 340, 200, "screen"),
	midiMsgFilterText (10, 10, 320, 20, "text", "MIDI messages to be processed by the plugin."),
	midiMsgFilterAllSwitch (10, 36, 28, 14, "slider", 1),
	midiMsgFilterAllLabel (50, 33, 120, 20, "lflabel", "All"),

	smartQuantizationIcon (0, 0, 300, 20, "widget", pluginPath + "inc/smart_quantization.png"),
	smartQuantizationContainer (0, 0, 340, 180, "screen"),
        smartQuantizationRangeSlider (10, 60, 110, 28, "slider", 0.25, 0.0, 0.5, 0.0, "%1.2f"),
        smartQuantizationMappingSwitch (10, 136, 28, 14, "slider", 1.0),
        smartQuantizationPositioningSwitch (10, 156, 28, 14, "slider", 0.0),
        smartQuantizationText1 (10, 10, 320, 50, "text", "Synchronizes not exactly fitting MIDI signals (e.g., notes) with the step pattern if the signal is within a range from the step start or end."),
        smartQuantizationRangeLabel (130, 70, 90, 20, "lflabel", "Range (steps)"),
	smartQuantizationText2 (10, 100, 320, 30, "text", "MIDI signals can be synchronized just by assignment to a step or by fitting into a step or both."),
        smartQuantizationMappingLabel (50, 133, 120, 20, "lflabel", "Assign to a step"),
        smartQuantizationPositionLabel (50, 153, 120, 20, "lflabel", "Fit into a step"),

	userLatencyIcon (0, 0, 300, 20, "widget", pluginPath + "inc/latency.png"),
	userLatencyContainer (0, 0, 340, 180, "screen"),
	timeCompensText (10, 10, 320, 30, "text", "Some hosts already compensate time/position by the plugin's latency. Otherwise the plugin can do it."),
	timeCompensLabel (50, 45, 230, 20, "lflabel", "Latency-compensate time/position"),
	timeCompensSwitch (10, 48, 28, 14, "slider", 0.0),
	userLatencyText (10, 80, 320, 30, "text", "The plugin itself calculates the latency by default. Alternatively, you may define a fixed latency."),
	userLatencySwitch (10, 118, 28, 14, "slider", 0.0),
	userLatencyLabel (50, 115, 180, 20, "lflabel", "User-defined latency"),
	userLatencyValue (0, 0, 0, 0, "widget", 0.0, 0.0, 192000, 1.0),
	userLatencySlider (10, 140, 160, 28, "slider", 0, 0, 192000, 1, "%6.0f"),
	userLatencyUnitListbox
	(
		180, 150, 90, 20, 0, 20, 90, 40, "menu",
		BItems::ItemList ({BItems::Item({1, "Frames"})}),
		1
	),

	selectMenu
	(
		20, 90, 340, 360, "selectmenu",
		std::list<std::pair<Widget*, Widget*>>
		({
			{&midiChFilterIcon, &midiChFilterContainer},
			{&midiMsgFilterIcon, &midiMsgFilterContainer},
			{&smartQuantizationIcon, &smartQuantizationContainer},
			{&userLatencyIcon, &userLatencyContainer}
		})
	),

	sContainer (380, 90, 555, 210, "scontainer"),

	seqLenValueListbox
	(
		380, 370, 50, 20, 0, -220, 50, 220, "menu",
		BItems::ItemList
		({
			{0.125, "1/8"}, {0.25, "1/4"}, {0.333333, "1/3"}, {0.5, "1/2"},
			{0.666667, "2/3"}, {0.75, "3/4"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"},
			{6, "6"}, {8, "8"}, {12, "12"}, {16, "16"}
		}), 1.0
	),
	seqLenBaseListbox
	(
		440, 370, 90, 20, 0, -80, 90, 80, "menu",
		BItems::ItemList
		({{0, "Second(s)"}, {1, "Beat(s)"}, {2, "Bar(s)"}}), 2.0
	),
	ampSwingControl
	(
		545, 312, 120, 28, "slider", 1.0, 0.0078125, 128.0, 0.0, "%3.1f",
		[] (const double val, const double min, const double max)
		{return (val >= 1.0 ? 1.0 - 0.5 / LIMIT (val, min, max) : 0.5 * LIMIT (val, min, max));},
		[] (const double frac, const double min, const double max)
		{return (frac >= 0.5 ? 0.5 / (1.0 - LIMIT (frac, 0, 1)) : 2 * LIMIT (frac, 0, 1));}
	),
	ampRandomControl (680, 312, 120, 28, "slider", 0.0, 0.0, 1.0, 0.0, "%1.2f"),
	swingControl (545, 362, 120, 28, "slider", 1.0, 1.0 / 3.0, 3.0, 0.0),
	swingRandomControl (680, 362, 120, 28, "slider", 0.0, 0.0, 1.0, 0.0, "%1.2f"),
	markersAutoButton (415, 320, 80, 20, "button", "Auto"),
	nrStepsControl (380, 412, 555, 28, "slider", 1.0, 1.0, MAXSTEPS, 1.0, "%2.0f"),
	markerListBox (12, -68, 86, 66, "listbox", BItems::ItemList ({"Auto", "Manual"})),

	latencyValue (0, 0, 0, 0, "widget", 0),
	latencyDisplay (835, 10, 120, 10, "smlabel", ""),
	controllers{nullptr},
	messageLabel (420, 63, 440, 20, "hilabel", ""),
	inIcon (4, 14, 32, 12, "widget", pluginPath + "inc/in.png"),
	ampIcon (4, 90, 32, 12, "widget", pluginPath + "inc/amp.png"),
	delIcon (4, 160, 32, 12, "widget", pluginPath + "inc/del.png"),
	outIcon (4, 184, 32, 12, "widget", pluginPath + "inc/out.png"),

	sz (1.0), bgImageSurface (nullptr),
	map (NULL)

{
	// Init
	for (unsigned int i = 0; i < midiMsgFilterSwitches.size(); ++i)
	{
		midiMsgFilterSwitches[i] = BWidgets::HSwitch (10, 56 + i * 20, 28, 14, "slider", 1);
		midiMsgFilterLabels[i] = BWidgets::Label (50, 53 + i * 20, 180, 20, "lflabel", midiMsgGroupTexts[i]);
	}

	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i)
	{
		midiChFilterSwitches[i] = BWidgets::HSwitch (10 + 70.0 * int (i / 4), 56 + int (i % 4) * 20, 28, 14, "slider", 1);
		midiChFilterLabels[i] = BWidgets::Label (44 + 70.0 * int (i / 4), 53  + int (i % 4) * 20, 36, 20, "lflabel", "#" + std::to_string (i + 1));
	}

	// Link widgets to controllers
	controllers[SEQ_LEN_VALUE] = &seqLenValueListbox;
	controllers[SEQ_LEN_BASE] = &seqLenBaseListbox;
	controllers[AMP_SWING] = &ampSwingControl;
	controllers[AMP_RANDOM] = &ampRandomControl;
	controllers[SWING] = &swingControl;
	controllers[SWING_RANDOM] = &swingRandomControl;
	controllers[NR_OF_STEPS] = &nrStepsControl;
	controllers[QUANT_RANGE] = &smartQuantizationRangeSlider;
        controllers[QUANT_MAP] = &smartQuantizationMappingSwitch;
        controllers[QUANT_POS] = &smartQuantizationPositioningSwitch;
	controllers[TIME_COMPENS] = &timeCompensSwitch;
	controllers[USR_LATENCY] = &userLatencySwitch;
	controllers[USR_LATENCY_FR] = &userLatencyValue;

	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i) controllers[MIDI_CH_FILTER + i] = &midiChFilterSwitches[i];
	for (unsigned int i = 0; i < midiMsgFilterSwitches.size(); ++i) controllers[MSG_FILTER_NOTE + i] = &midiMsgFilterSwitches[i];
	for (int i = 0; i < MAXSTEPS - 1; ++i) controllers[STEP_POS + i] = &markerWidgets[i];
	for (int i = 0; i < MAXSTEPS; ++i) controllers[STEP_LEV + i] = &stepControl[i];
	controllers[LATENCY] = &latencyValue;

	// Set callbacks
	for (BWidgets::ValueWidget* v : controllers) v->setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
	midiChFilterAllSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
	midiMsgFilterAllSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
	userLatencySlider.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
	markerListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::listBoxChangedCallback);
	markersAutoButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::markersAutoClickedCallback);
	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpButtonClickedCallback);
	//ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytButtonClickedCallback);

	// Configure widgets
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	userLatencySlider.hide();
	userLatencyUnitListbox.hide();
	swingRandomControl.hide();
	nrStepsControl.setScrollable (true);
	nrStepsControl.getDisplayLabel ()->setState (BColors::ACTIVE);
	swingControl.getDisplayLabel ()->setState (BColors::ACTIVE);
	markerListBox.setOversize (true);

	//Initialialize and configure stepControllers
	double sw = sContainer.getEffectiveWidth() - 40;
	double sx = sContainer.getXOffset() + 40;
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepControl[i] = BWidgets::VSlider ((i + 0.5) * sw / MAXSTEPS + sx - 7, 60, 14, 80, "slider", 1.0, 0.0, 1.0, 0.01);
		stepControl[i].setHardChangeable (false);
		stepControl[i].setScrollable (true);
		stepControl[i].setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
		stepControl[i].applyTheme (theme, "slider");
		sContainer.add (stepControl[i]);

		stepControlLabel[i] = BWidgets::Label ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 28, 20, "mlabel", "1.00");
		stepControlLabel[i].applyTheme (theme, "mlabel");
		stepControlLabel[i].setState (BColors::ACTIVE);
		sContainer.add (stepControlLabel[i]);

		inputStepLabel[i] = BWidgets::Label (sx + (i + 0.1) * sw / MAXSTEPS, 10, 0.8 * sw / MAXSTEPS, 20, "steplabel", "#" + std::to_string (i + 1));
		sContainer.add (inputStepLabel[i]);

		outputStepLabel[i] = BWidgets::Label (sx + (i + 0.1) * sw / MAXSTEPS, 180, 0.8 * sw / MAXSTEPS, 20, "steplabel", "#" + std::to_string (i + 1));
		sContainer.add (outputStepLabel[i]);
	}

	//Initialialize and configure markers
	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		markerWidgets[i] = Marker ((i + 1) * sw / MAXSTEPS + sx - 5, 154, 10, 16, "marker", (double(i) + 1.0) / MAXSTEPS, 0.0, 1.0, 0.0);
		markerWidgets[i].setHasValue (false);
		markerWidgets[i].setDraggable (true);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::markerClickedCallback);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BSchafflGUI::markerDraggedCallback);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
		markerWidgets[i].applyTheme (theme, "slider");
		sContainer.add (markerWidgets[i]);
	}

	setAutoMarkers ();
	rearrange_controllers ();
	redrawSContainer ();
	applyTheme (theme);

	// Pack widgets
	midiChFilterContainer.add (midiChFilterText);
	midiChFilterContainer.add (midiChFilterAllSwitch);
	midiChFilterContainer.add (midiChFilterAllLabel);
	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i)
	{
		midiChFilterContainer.add (midiChFilterSwitches[i]);
		midiChFilterContainer.add (midiChFilterLabels[i]);
	}

	midiMsgFilterContainer.add (midiMsgFilterText);
	midiMsgFilterContainer.add (midiMsgFilterAllSwitch);
	midiMsgFilterContainer.add (midiMsgFilterAllLabel);
	for (unsigned int i = 0; i < midiMsgFilterSwitches.size(); ++i)
	{
		midiMsgFilterContainer.add (midiMsgFilterSwitches[i]);
		midiMsgFilterContainer.add (midiMsgFilterLabels[i]);
	}

	smartQuantizationContainer.add (smartQuantizationRangeSlider);
        smartQuantizationContainer.add (smartQuantizationMappingSwitch);
        smartQuantizationContainer.add (smartQuantizationPositioningSwitch);
        smartQuantizationContainer.add (smartQuantizationText1);
        smartQuantizationContainer.add (smartQuantizationRangeLabel);
	smartQuantizationContainer.add (smartQuantizationText2);
        smartQuantizationContainer.add (smartQuantizationMappingLabel);
        smartQuantizationContainer.add (smartQuantizationPositionLabel);

	userLatencyContainer.add (timeCompensText);
	userLatencyContainer.add (timeCompensLabel);
	userLatencyContainer.add (timeCompensSwitch);
	userLatencyContainer.add (userLatencyText);
	userLatencyContainer.add (userLatencySwitch);
	userLatencyContainer.add (userLatencyLabel);
	userLatencyContainer.add (userLatencyValue);
	userLatencyContainer.add (userLatencySlider);
	userLatencyContainer.add (userLatencyUnitListbox);

	sContainer.add (inIcon);
	sContainer.add (ampIcon);
	sContainer.add (delIcon);
	sContainer.add (outIcon);

	mContainer.add (selectMenu);
	mContainer.add (sContainer);
	mContainer.add (helpButton);
	//mContainer.add (ytButton);
	mContainer.add (seqLenValueListbox);
	mContainer.add (seqLenBaseListbox);
	mContainer.add (ampSwingControl);
	mContainer.add (ampRandomControl);
	mContainer.add (swingControl);
	mContainer.add (swingRandomControl);
	mContainer.add (markersAutoButton);
	mContainer.add (nrStepsControl);
	mContainer.add (latencyValue);
	mContainer.add (latencyDisplay);
	mContainer.add (messageLabel);
	add (mContainer);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0) m = (LV2_URID_Map*) features[i]->data;
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);
}

BSchafflGUI::~BSchafflGUI() {}

void BSchafflGUI::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port_index == OUTPUT))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Status notification
			if (obj->body.otype == uris.bschafflr_statusEvent)
			{
				const LV2_Atom *oPos = NULL, *oLat = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bschafflr_step, &oPos,
					uris.bschafflr_latency, &oLat,
					0
				);
				if (oPos && (oPos->type == uris.atom_Int))
				{
					const int step = LIMIT (((LV2_Atom_Int*)oPos)->body, 0, MAXSTEPS - 1);
					if (inputStepLabel[step].getName() != "actsteplabel")
					{
						for (int i = 0; i < MAXSTEPS; ++i)
						{
							if (i == step)
							{
								inputStepLabel[i].rename ("actsteplabel");
								outputStepLabel[i].rename ("actsteplabel");
							}

							else
							{
								inputStepLabel[i].rename ("steplabel");
								outputStepLabel[i].rename ("steplabel");
							}

							inputStepLabel[i].applyTheme (theme);
							outputStepLabel[i].applyTheme (theme);
						}
					}
				}

				if (oLat && (oLat->type == uris.atom_Float))
				{
					const float latencyMs = ((LV2_Atom_Float*)oLat)->body;
					latencyDisplay.setText ((latencyMs > 0) ? "Latency: " + BUtilities::to_string (latencyMs, "%5.1f") + " ms" : "");
				}
			}

			// Message notification
			if (obj->body.otype == uris.notify_messageEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_message, &data, 0);
				if (data && (data->type == uris.atom_Int))
				{
					const int messageNr = ((LV2_Atom_Int*)data)->body;
					std::string msg = ((messageNr >= NO_MSG) && (messageNr <= MAX_MSG) ? messageStrings[messageNr] : "");
					messageLabel.setText (msg);
				}
			}
		}
	}

	// Scan controller ports
	else if ((format == 0) && (port_index >= CONTROLLERS) && (port_index < CONTROLLERS + NR_CONTROLLERS))
	{
		float* pval = (float*) buffer;
		const int controllerNr = port_index - CONTROLLERS;

		if (controllerNr == USR_LATENCY_FR) userLatencySlider.setValue (*pval);

		if ((controllerNr >= STEP_POS) && (controllerNr < STEP_POS + MAXSTEPS - 1))
		{
			setMarker (controllerNr - STEP_POS, *pval);
			setAutoMarkers ();
			rearrange_controllers ();
			redrawSContainer ();
		}

		else  controllers[controllerNr]->setValue (*pval);
	}
}

void BSchafflGUI::resizeGUI()
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12 * sz);
	lfFont.setFontSize (12 * sz);
	txFont.setFontSize (12 * sz);
	mdFont.setFontSize (10 * sz);
	smFont.setFontSize (8 * sz);

	// Resize Background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 955 * sz, 470 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 955, 470, sz);

	RESIZE (helpButton, 885, 60, 24, 24, sz);

	RESIZE (midiChFilterIcon, 0, 0, 300, 20, sz);
	RESIZE (midiChFilterContainer, 0, 0, 340, 140, sz);
	RESIZE (midiChFilterText, 10, 10, 320, 50, sz);
	RESIZE (midiChFilterAllSwitch, 10, 36, 28, 14, sz);
	RESIZE (midiChFilterAllLabel, 44, 33, 120, 20, sz);
	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i)
	{
		RESIZE (midiChFilterSwitches[i], 10 + 70.0 * int (i / 4), 56 + int (i % 4) * 20, 28, 14, sz);
		RESIZE (midiChFilterLabels[i], 44 + 70.0 * int (i / 4), 53  + int (i % 4) * 20, 36, 20, sz);
	}

	RESIZE (midiMsgFilterIcon, 0, 0, 300, 20, sz);
	RESIZE (midiMsgFilterContainer, 0, 0, 340, 200, sz);
	RESIZE (midiMsgFilterText, 10, 10, 320, 20, sz);
	RESIZE (midiMsgFilterAllSwitch, 10, 36, 28, 14, sz);
	RESIZE (midiMsgFilterAllLabel, 50, 33, 120, 20, sz);
	for (unsigned int i = 0; i < midiMsgFilterSwitches.size(); ++i)
	{
		RESIZE (midiMsgFilterSwitches[i], 10, 56 + i * 20, 28, 14, sz);
		RESIZE (midiMsgFilterLabels[i], 50, 53 + i * 20, 180, 20, sz);
	}

	RESIZE (smartQuantizationIcon, 0, 0, 300, 20, sz);
	RESIZE (smartQuantizationContainer, 0, 0, 340, 180, sz);
	RESIZE (smartQuantizationRangeSlider, 10, 60, 110, 28, sz);
	RESIZE (smartQuantizationMappingSwitch, 10, 136, 28, 14, sz);
	RESIZE (smartQuantizationPositioningSwitch, 10, 156, 28, 14, sz);
	RESIZE (smartQuantizationText1, 10, 10, 320, 50, sz);
	RESIZE (smartQuantizationRangeLabel, 130, 70, 90, 20, sz);
	RESIZE (smartQuantizationText2, 10, 100, 320, 30, sz);
	RESIZE (smartQuantizationMappingLabel, 50, 133, 120, 20, sz);
	RESIZE (smartQuantizationPositionLabel, 50, 153, 120, 20, sz);

	RESIZE (userLatencyIcon, 0, 0, 300, 20, sz);
	RESIZE (userLatencyContainer, 0, 0, 340, 180, sz);
	RESIZE (timeCompensText, 10, 10, 320, 30, sz);
	RESIZE (timeCompensLabel, 50, 45, 230, 20, sz);
	RESIZE (timeCompensSwitch, 10, 48, 28, 14, sz);
	RESIZE (userLatencyText, 10, 80, 320, 30, sz);
	RESIZE (userLatencySwitch, 10, 118, 28, 14, sz);
	RESIZE (userLatencyLabel, 50, 115, 180, 20, sz);
	RESIZE (userLatencySlider, 10, 140, 160, 28, sz);
	RESIZE (userLatencyUnitListbox, 180, 150, 90, 20, sz);
	userLatencyUnitListbox.resizeListBox (BUtilities::Point (90 * sz, 40 * sz));
	userLatencyUnitListbox.moveListBox (BUtilities::Point (0, 20 * sz));
	userLatencyUnitListbox.resizeListBoxItems (BUtilities::Point (40 * sz, 20 * sz));

	RESIZE (selectMenu, 20, 90, 340, 360, sz);

	RESIZE (sContainer, 380, 90, 555, 210, sz);

	RESIZE (seqLenValueListbox, 380, 370, 50, 20, sz);
	seqLenValueListbox.resizeListBox (BUtilities::Point (50 * sz, 220 * sz));
	seqLenValueListbox.moveListBox (BUtilities::Point (0, -220 * sz));
	seqLenValueListbox.resizeListBoxItems (BUtilities::Point (50 * sz, 20 * sz));
	RESIZE (seqLenBaseListbox, 440, 370, 90, 20, sz);
	seqLenBaseListbox.resizeListBox (BUtilities::Point (90 * sz, 80 * sz));
	seqLenBaseListbox.moveListBox (BUtilities::Point (0, -80 * sz));
	seqLenBaseListbox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (ampSwingControl, 545, 312, 120, 28, sz);
	RESIZE (ampRandomControl, 680, 312, 120, 28, sz);
	RESIZE (swingControl, 545, 362, 120, 28, sz);
	RESIZE (swingRandomControl, 680, 362, 120, 28, sz);
	RESIZE (markersAutoButton, 415, 320, 80, 20, sz);
	RESIZE (nrStepsControl, 380, 412, 555, 28, sz);
	RESIZE (markerListBox, 12, -68, 86, 66, sz);
	markerListBox.resizeItems (BUtilities::Point (80 * sz, 20 * sz));

	RESIZE (messageLabel, 420, 63, 440, 20, sz);
	RESIZE (latencyDisplay, 835, 10, 120, 10, sz);

	RESIZE (inIcon, 4, 14, 32, 12, sz);
	RESIZE (ampIcon, 4, 90, 32, 12, sz);
	RESIZE (delIcon, 4, 160, 32, 12, sz);
	RESIZE (outIcon, 4, 184, 32, 12, sz);

	// Update monitors
	rearrange_controllers ();
	redrawSContainer ();

	// Apply changes
	applyTheme (theme);
	show ();
}

void BSchafflGUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);

	midiChFilterIcon.applyTheme (theme);
        midiChFilterContainer.applyTheme (theme);
	midiChFilterText.applyTheme (theme);
	midiChFilterAllSwitch.applyTheme (theme);
	midiChFilterAllLabel.applyTheme (theme);
	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i)
	{
		midiChFilterSwitches[i].applyTheme (theme);
		midiChFilterLabels[i].applyTheme (theme);
	}

	midiMsgFilterIcon.applyTheme (theme);
        midiMsgFilterContainer.applyTheme (theme);
	midiMsgFilterText.applyTheme (theme);
	midiMsgFilterAllSwitch.applyTheme (theme);
	midiMsgFilterAllLabel.applyTheme (theme);
	for (unsigned int i = 0; i < midiMsgFilterSwitches.size(); ++i)
	{
		midiMsgFilterSwitches[i].applyTheme (theme);
		midiMsgFilterLabels[i].applyTheme (theme);
	}

	smartQuantizationIcon.applyTheme (theme);
	smartQuantizationContainer.applyTheme (theme);
        smartQuantizationRangeSlider.applyTheme (theme);
        smartQuantizationMappingSwitch.applyTheme (theme);
        smartQuantizationPositioningSwitch.applyTheme (theme);
        smartQuantizationText1.applyTheme (theme);
        smartQuantizationRangeLabel.applyTheme (theme);
	smartQuantizationText2.applyTheme (theme);
        smartQuantizationMappingLabel.applyTheme (theme);
        smartQuantizationPositionLabel.applyTheme (theme);

	userLatencyIcon.applyTheme (theme);
        userLatencyContainer.applyTheme (theme);
	timeCompensText.applyTheme (theme);
	timeCompensLabel.applyTheme (theme);
	timeCompensSwitch.applyTheme (theme);
	userLatencyText.applyTheme (theme);
	userLatencySwitch.applyTheme (theme);
	userLatencyLabel.applyTheme (theme);
	userLatencyValue.applyTheme (theme);
	userLatencySlider.applyTheme (theme);
	userLatencyUnitListbox.applyTheme (theme);

	selectMenu.applyTheme (theme);

	sContainer.applyTheme (theme);
	helpButton.applyTheme (theme);
	//ytButton.applyTheme (theme);

	seqLenValueListbox.applyTheme (theme);
	seqLenBaseListbox.applyTheme (theme);
	ampSwingControl.applyTheme (theme);
	ampRandomControl.applyTheme (theme);
	swingControl.applyTheme (theme);
	swingRandomControl.applyTheme (theme);
	markersAutoButton.applyTheme (theme);
	nrStepsControl.applyTheme (theme);
	latencyValue.applyTheme (theme);
	latencyDisplay.applyTheme (theme);
	messageLabel.applyTheme (theme);
	sContainer.applyTheme (theme);
	markerListBox.applyTheme (theme);
	inIcon.applyTheme (theme);
	ampIcon.applyTheme (theme);
	delIcon.applyTheme (theme);
	outIcon.applyTheme (theme);
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepControl[i].applyTheme (theme);
		stepControlLabel[i].applyTheme (theme);
		inputStepLabel[i].applyTheme (theme);
		outputStepLabel[i].applyTheme (theme);
	}
}

void BSchafflGUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (getWidth() / 955 > getHeight() / 470 ? getHeight() / 470 : getWidth() / 955);
	resizeGUI ();
}

void BSchafflGUI::sendUiStatus (const bool on)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, (on ? uris.bschafflr_uiOn : uris.bschafflr_uiOff));
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSchafflGUI::setMarker (const int markerNr, double value)
{
	if ((markerNr < 0) || (markerNr >= MAXSTEPS - 1)) return;

	// Value 0.0: Automatic
	if (value == 0.0)
	{
		markerWidgets[markerNr].setHasValue (false);
	}

	else
	{
		// Set value and switch off automatic
		value = LIMIT (value, MINMARKERVALUE, 1.0);
		markerWidgets[markerNr].setHasValue (true);
		markerWidgets[markerNr].setValue (value);

		// Validate ancessors
		for (int i = markerNr - 1; i >= 0; --i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() > value) markerWidgets[i].setValue (value);
				else break;
			}
		}

		// Validate successors
		for (int i = markerNr + 1; i < MAXSTEPS - 1; ++i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() < value) markerWidgets[i].setValue (value);
				else break;
			}
		}
	}
}

void BSchafflGUI::setAutoMarkers ()
{
	int nrMarkers = controllers[NR_OF_STEPS]->getValue() - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		if (!markerWidgets[i].hasValue())
		{
			if ((i == nrMarkers - 1) || (markerWidgets[i + 1].hasValue()))
			{
				double swing = 2.0 * swingControl.getValue() / (swingControl.getValue() + 1.0);
				double anc = (start == 0 ? 0 : markerWidgets[start - 1].getValue());
				double suc = (i == nrMarkers - 1 ? 1 : markerWidgets[i + 1].getValue());
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - swing : swing) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - swing : swing);
					anc += f * step;
					markerWidgets[j].setValue (anc);
				}
			}
		}
		else start = i + 1;
	}
}

void BSchafflGUI::rearrange_controllers ()
{
	const int nrStepsi = controllers[NR_OF_STEPS]->getValue();

	if ((nrStepsi < 1) || (nrStepsi > MAXSTEPS)) return;

	const double sw = sContainer.getEffectiveWidth() - 40 * sz;
	const double sx = sContainer.getXOffset() + 40 * sz;
	const double inw = sw / nrStepsi;
	const double oddf = (ampSwingControl.getValue() >= 1.0 ? 1.0 : ampSwingControl.getValue());
	const double evenf = (ampSwingControl.getValue() >= 1.0 ? 1.0 / ampSwingControl.getValue() : 1.0);

	for (int i = 0; i < MAXSTEPS; ++i)
	{
		if (i < nrStepsi)
		{
			stepControl[i].setHeight ((14 + LIMIT (66 * ((i % 2) == 0 ? oddf : evenf), 0, 66 )) * sz);
			stepControl[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 7 * sz, 140 * sz - stepControl[i].getHeight());
			stepControl[i].show();

			stepControlLabel[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 14 * sz, 40 * sz);
			stepControlLabel[i].show();

			inputStepLabel[i].moveTo (sx + (i + 0.1) * sw / nrStepsi, 10 * sz);
			inputStepLabel[i].setWidth (0.8 * inw);
			inputStepLabel[i].show();

			const double outx = (i == 0 ? 0.0 : markerWidgets[i - 1].getValue() * sw);
			const double outw =
			(
				i == 0 ?
				markerWidgets[i].getValue() * sw :
				(
					i < nrStepsi - 1 ?
					(markerWidgets[i].getValue() - markerWidgets[i - 1].getValue()) * sw :
					(1 - markerWidgets[i - 1].getValue()) * sw
				)
			);
			outputStepLabel[i].moveTo (sx + outx + 0.1 * outw, 180 * sz);
			outputStepLabel[i].setWidth (0.8 * outw);
			outputStepLabel[i].show();

		}
		else
		{
			stepControl[i].hide ();
			stepControlLabel[i].hide();
			inputStepLabel[i].hide();
			outputStepLabel[i].hide();
		}
	}

	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		if (i < nrStepsi - 1)
		{
			markerWidgets[i].moveTo (markerWidgets[i].getValue() * sw + sx - 5 * sz, 154 * sz);
			markerWidgets[i].show ();
		}
		else markerWidgets[i].hide ();
	}
}

void BSchafflGUI::valueChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
		const float value = widget->getValue();

		if (widget->getMainWindow ())
		{
			BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow ();

			// Get controller nr
			int controllerNr = -1;
			for (int i = 0; i < NR_CONTROLLERS; ++i)
			{
				if (widget == ui->controllers[i])
				{
					controllerNr = i;
					break;
				}
			}

			if (controllerNr >= 0)
			{
				if (controllerNr == LATENCY)
				{
					// Do nothing
				}

				else if ((controllerNr >= MIDI_CH_FILTER) && (controllerNr < MIDI_CH_FILTER + NR_MIDI_CHS))
				{
					int count = 0;
					for (BWidgets::HSwitch& s : ui->midiChFilterSwitches)
					{
						if (s.getValue() != 0.0) ++count;
					}

					if (count == 0)
					{
						ui->midiChFilterAllSwitch.setState (BColors::NORMAL);
						ui->midiChFilterAllLabel.setState (BColors::NORMAL);
						ui->midiChFilterAllSwitch.setValue (0.0);
					}

					else if (count == NR_MIDI_CHS)
					{
						ui->midiChFilterAllSwitch.setState (BColors::NORMAL);
						ui->midiChFilterAllLabel.setState (BColors::NORMAL);
						ui->midiChFilterAllSwitch.setValue (1.0);
					}

					else
					{
						ui->midiChFilterAllSwitch.setState (BColors::INACTIVE);
						ui->midiChFilterAllLabel.setState (BColors::INACTIVE);
					}

					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
				}

				else if ((controllerNr >= MSG_FILTER_NOTE) && (controllerNr < MSG_FILTER_NOTE + NR_MIDI_MSG_FILTERS))
				{
					int count = 0;
					for (BWidgets::HSwitch& s : ui->midiMsgFilterSwitches)
					{
						if (s.getValue() != 0.0) ++count;
					}

					if (count == 0)
					{
						ui->midiMsgFilterAllSwitch.setState (BColors::NORMAL);
						ui->midiMsgFilterAllLabel.setState (BColors::NORMAL);
						ui->midiMsgFilterAllSwitch.setValue (0.0);
					}

					else if (count == NR_MIDI_MSG_FILTERS)
					{
						ui->midiMsgFilterAllSwitch.setState (BColors::NORMAL);
						ui->midiMsgFilterAllLabel.setState (BColors::NORMAL);
						ui->midiMsgFilterAllSwitch.setValue (1.0);
					}

					else
					{
						ui->midiMsgFilterAllSwitch.setState (BColors::INACTIVE);
						ui->midiMsgFilterAllLabel.setState (BColors::INACTIVE);
					}

					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
				}

				else if (controllerNr == USR_LATENCY)
				{
					if (value == 0.0)
					{
						ui->userLatencySlider.hide();
						ui->userLatencyUnitListbox.hide();
					}
					else
					{
						ui->userLatencySlider.show();
						ui->userLatencyUnitListbox.show();
					}

					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
				}

				else if ((controllerNr >= STEP_POS) && (controllerNr < STEP_POS + MAXSTEPS - 1))
				{
					float pos = (((Marker*)widget)->hasValue() ? value : 0.0f);
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &pos);
				}

				else if ((controllerNr >= STEP_LEV) && (controllerNr < STEP_LEV + MAXSTEPS))
				{
					ui->stepControlLabel[controllerNr - STEP_LEV].setText (BUtilities::to_string (value, "%1.2f"));
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
				}

				else if (controllerNr == AMP_SWING)
				{
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
					ui->rearrange_controllers();
				}

				else if ((controllerNr == SWING) || (controllerNr == NR_OF_STEPS))
				{
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
					ui->setAutoMarkers();
					ui->rearrange_controllers();
					ui->redrawSContainer();
				}

				else ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
			}

			else if (widget == &ui->midiChFilterAllSwitch)
			{
				for (BWidgets::HSwitch& s : ui->midiChFilterSwitches) s.setValue (value);
			}

			else if (widget == &ui->midiMsgFilterAllSwitch)
			{
				for (BWidgets::HSwitch& s : ui->midiMsgFilterSwitches) s.setValue (value);
			}

			else if (widget == &ui->userLatencySlider)
			{
				ui->userLatencyValue.setValue (ui->userLatencySlider.getValue());
			}
		}
	}
}

void BSchafflGUI::markerClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::RIGHT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BSchafflGUI* ui = (BSchafflGUI*)marker->getMainWindow();
	if (!ui) return;

	int nrSteps = ui->controllers[NR_OF_STEPS]->getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			Marker* oldMarker = (Marker*) ui->markerListBox.getParent();
			ui->markerListBox.setValue (UNSELECTED);

			if (oldMarker && (oldMarker == marker))
			{
				if (ui->markerListBox.isVisible()) ui->markerListBox.hide();
				else ui->markerListBox.show ();
			}

			else if (oldMarker && (oldMarker != marker))
			{
				oldMarker->release (&ui->markerListBox);
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

			else
			{
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

		}
	}
}

void BSchafflGUI::markerDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::LEFT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BSchafflGUI* ui = (BSchafflGUI*)marker->getMainWindow();
	if (!ui) return;

	int nrSteps = ui->controllers[NR_OF_STEPS]->getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			double x0 = ui->sContainer.getXOffset() + 40 * ui->sz;
			double w = ui->sContainer. getEffectiveWidth() - 40 * ui->sz;
			double frac = (w > 0 ? (pev->getPosition().x + marker->getPosition().x - x0) / w : MINMARKERVALUE);
			frac = LIMIT (frac, MINMARKERVALUE, 1.0);

			// Limit to antecessors value
			for (int j = i - 1; j >= 0; --j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac < ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			// Limit to successors value
			for (int j = i + 1; j < nrSteps - 1; ++j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac > ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			ui->setMarker (i, frac);
			ui->setAutoMarkers();
			ui->rearrange_controllers();
			ui->redrawSContainer();
			break;
		}
	}
}

void BSchafflGUI::listBoxChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	BWidgets::ListBox* lb = (BWidgets::ListBox*) vev->getWidget();
	if (!lb) return;
	Marker* m = (Marker*) lb->getParent();
	if (!m) return;
	BSchafflGUI* ui = (BSchafflGUI*)m->getMainWindow();
	if (!ui) return;

	double value = vev->getValue();
	if (value == 1.0) m->setHasValue (false);
	else if (value == 2.0) m->setHasValue (true);
	else return;

	lb->hide();
	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
}

void BSchafflGUI::markersAutoClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	if (vev->getValue() == 0.0) return;
	BWidgets::TextButton* tb = (BWidgets::TextButton*) vev->getWidget();
	if (!tb) return;
	BSchafflGUI* ui = (BSchafflGUI*)tb->getMainWindow();
	if (!ui) return;

	for (Marker& m : ui->markerWidgets) m.setHasValue (false);

	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
}

void BSchafflGUI::helpButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " HELP_URL);}
//void BSchafflGUI::ytButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " YT_URL);}

void BSchafflGUI::redrawSContainer ()
{
	double width = sContainer.getEffectiveWidth ();
	double height = sContainer.getEffectiveHeight ();

	cairo_surface_clear (sContainer.getDrawingSurface ());
	cairo_t* cr = cairo_create (sContainer.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (BColors::black));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.0, 0.0, 0.0, 0.5);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	int nrSteps = controllers[NR_OF_STEPS]->getValue();

	// Step lines
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
	cairo_move_to (cr, 40 * sz, 0);
	cairo_rel_line_to (cr, 0, 210 * sz);

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		cairo_move_to (cr, double (i + 1) / double (nrSteps) * (width - 40 * sz) + 40 * sz, 0);
		cairo_rel_line_to (cr, 0, 140 * sz);
		cairo_line_to (cr, markerWidgets[i].getValue() * (width - 40 * sz) + 40 * sz, 150 * sz);
		cairo_rel_line_to (cr, 0, 60 * sz);
		cairo_stroke (cr);
	}

	cairo_destroy (cr);
	sContainer.update();
}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeWindow parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BSCHAFFL_URI) != 0)
	{
		std::cerr << "BSchaffl.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeWindow) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BSchaffl.lv2#GUI: No parent window.\n";

	// New instance
	BSchafflGUI* ui;
	try {ui = new BSchafflGUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BSchaffl.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 995) || (screenHeight < 490)) sz = 0.66;

	if (resize) resize->ui_resize(resize->handle, 955 * sz, 470 * sz);

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());

	ui->sendUiStatus (true);
	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BSchafflGUI* pluginGui = (BSchafflGUI*) ui;
	delete pluginGui;
}

void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BSchafflGUI* pluginGui = (BSchafflGUI*) ui;
	pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BSchafflGUI* pluginGui = (BSchafflGUI*) ui;
	pluginGui->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = { callIdle };

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

const LV2UI_Descriptor guiDescriptor = {
		BSCHAFFLGUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
