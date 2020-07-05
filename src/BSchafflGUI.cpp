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
	Window (1020, 480, "B.Schaffl", parentWindow, true),
	controller (NULL), write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),

	mContainer (0, 0, 1020, 480, "main"),

	helpButton (950, 60, 24, 24, "halobutton", "Help"),
	//ytButton (50, 80, 24, 24, "halobutton", "Tutorial"),

	toolbox (380, 330, 620, 30, "scontainer"),
	toolIcon (4, 2 , 40, 26, "widget", pluginPath + "inc/tools.png"),
	convertToShapeIcon (50, 5, 50, 20, "widget", pluginPath + "inc/convert_to_linear_shape.png"),
	convertToStepsIcon (50, 5, 50, 20, "widget", pluginPath + "inc/convert_to_steps.png"),
	convertToShapeButton (48, 3, 54, 24, "halobutton", "Convert step sliders to shape"),
	convertToStepsButton (48, 3, 54, 24, "halobutton", "Convert shape to step sliders"),
	markersToolbox (120, 5, 50, 20, "widget", pluginPath + "inc/markers_tb.png"),
	shapeToolbox (190, 5, 140, 20, "widget", pluginPath + "inc/shape_tools_tb.png"),
	editToolbox (360, 5, 80, 20, "widget", pluginPath + "inc/edit_tb.png"),
	historyToolbox (460, 5, 80, 20, "widget", pluginPath + "inc/history_tb.png"),
	gridToolbox (560, 5, 50, 20, "widget", pluginPath + "inc/grid_tb.png"),
	markersAutoButton (118, 3, 24, 24, "halobutton", "All markers: Auto"),
	markersManualButton (148, 3, 24, 24, "halobutton", "All markers: Manual"),
	shapeToolButtons
	{
		HaloToggleButton (188, 3, 24, 24, "halobutton", "Select"),
		HaloToggleButton (218, 3, 24, 24, "halobutton", "Point node"),
		HaloToggleButton (248, 3, 24, 24, "halobutton", "Auto Bezier node"),
		HaloToggleButton (278, 3, 24, 24, "halobutton", "Symmetric Bezier node"),
		HaloToggleButton (308, 3, 24, 24, "halobutton", "Asymmetric Bezier node")
	},
	editToolButtons
	{
		HaloButton (358, 3, 24, 24, "halobutton", "Cut"),
		HaloButton (388, 3, 24, 24, "halobutton", "Copy"),
		HaloButton (418, 3, 24, 24, "halobutton", "Paste")
	},
	historyToolButtons
	{
		HaloButton (458, 3, 24, 24, "halobutton", "Reset"),
		HaloButton (488, 3, 24, 24, "halobutton", "Undo"),
		HaloButton (518, 3, 24, 24, "halobutton", "Redo")
	},
	gridShowButton (558, 3, 24, 24, "halobutton", "Show grid"),
	gridSnapButton (588, 3, 24, 24, "halobutton", "Snap to grid"),

	convertToShapeMessage
	(
		560, 100, 300, 150, "menu",
		"Shape-controlled amp mode",
		"Do you want to convert the amp data from the sliders-controlled mode to this shape ?",
		std::vector<std::string>{"OK", "Cancel"}
	),
        convertToShapeToLinearButton (20, 70, 12, 12, "slider", 0.0),
        convertToShapeToLinearText (60, 70, 200, 20, "menu/text", "Convert sliders to linear fading"),
        convertToShapeToConstButton (20, 90, 12, 12, "slider", 0.0),
        convertToShapeToConstText (60, 90, 200, 20, "menu/text", "Convert sliders to steps"),

	convertToStepsMessage
	(
		560, 100, 300, 110, "menu",
		"Sliders-controlled amp mode",
		"Do you want to convert the amp data from the shape-controlled mode to sliders ?",
		std::vector<std::string>{"OK", "Cancel"}
	),

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
		20, 90, 340, 390, "selectmenu",
		std::list<std::pair<Widget*, Widget*>>
		({
			{&midiChFilterIcon, &midiChFilterContainer},
			{&midiMsgFilterIcon, &midiMsgFilterContainer},
			{&smartQuantizationIcon, &smartQuantizationContainer},
			{&userLatencyIcon, &userLatencyContainer}
		})
	),

	sContainer (380, 90, 620, 240, "scontainer"),

	modeSwitch (426, 71, 28, 14, "slider", 0.0),
	seqLenValueListbox
	(
		380, 380, 50, 20, 0, -220, 50, 220, "menu",
		BItems::ItemList
		({
			{0.125, "1/8"}, {0.25, "1/4"}, {0.333333, "1/3"}, {0.5, "1/2"},
			{0.666667, "2/3"}, {0.75, "3/4"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"},
			{6, "6"}, {8, "8"}, {12, "12"}, {16, "16"}
		}), 1.0
	),
	seqLenBaseListbox
	(
		440, 380, 90, 20, 0, -80, 90, 80, "menu",
		BItems::ItemList
		({{0, "Second(s)"}, {1, "Beat(s)"}, {2, "Bar(s)"}}), 2.0
	),
	ampSwingControl
	(
		565, 372, 120, 28, "slider", 1.0, 0.0078125, 128.0, 0.0, "%3.1f",
		[] (const double val, const double min, const double max)
		{return (val >= 1.0 ? 1.0 - 0.5 / LIMIT (val, min, max) : 0.5 * LIMIT (val, min, max));},
		[] (const double frac, const double min, const double max)
		{return (frac >= 0.5 ? 0.5 / (1.0 - LIMIT (frac, 0, 1)) : 2 * LIMIT (frac, 0, 1));}
	),
	ampRandomControl (720, 372, 120, 28, "slider", 0.0, 0.0, 1.0, 0.0, "%1.2f"),
	ampProcessControl (875, 372, 120, 28, "procslider", 1.0, -1.0, 2.0, 0.0, "%1.2f"),
	swingControl (565, 422, 120, 28, "slider", 1.0, 1.0 / 3.0, 3.0, 0.0),
	swingRandomControl (720, 422, 120, 28, "slider", 0.0, 0.0, 1.0, 0.0, "%1.2f"),
	swingProcessControl (875, 422, 120, 28, "procslider", 0.0, 0.0, 1.0, 0.0, "%1.2f"),
	nrStepsControl (380, 422, 155, 28, "slider", 1.0, 1.0, MAXSTEPS, 1.0, "%2.0f"),
	stepControlContainer (40, 40, 580, 130, "widget", 0.0),
	shapeWidget (40, 40, 580, 130, "shape"),
	markerListBox (12, -68, 86, 66, "listbox", BItems::ItemList ({"Auto", "Manual"})),

	latencyValue (0, 0, 0, 0, "widget", 0),
	latencyDisplay (900, 10, 120, 10, "smlabel", ""),
	controllers{nullptr},
	messageLabel (480, 63, 465, 20, "hilabel", ""),
	inIcon (4, 14, 32, 12, "widget", pluginPath + "inc/in.png"),
	ampIcon (4, 105, 32, 12, "widget", pluginPath + "inc/amp.png"),
	delIcon (4, 185, 32, 12, "widget", pluginPath + "inc/del.png"),
	outIcon (4, 214, 32, 12, "widget", pluginPath + "inc/out.png"),

	clipboard (),

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
		midiChFilterSwitches[i] = BWidgets::HSwitch (10 + 80.0 * int (i / 4), 56 + int (i % 4) * 20, 28, 14, "slider", 1);
		midiChFilterLabels[i] = BWidgets::Label (44 + 80.0 * int (i / 4), 53  + int (i % 4) * 20, 36, 20, "lflabel", "#" + std::to_string (i + 1));
	}

	// Link widgets to controllers
	controllers[SEQ_LEN_VALUE] = &seqLenValueListbox;
	controllers[SEQ_LEN_BASE] = &seqLenBaseListbox;
	controllers[AMP_SWING] = &ampSwingControl;
	controllers[AMP_RANDOM] = &ampRandomControl;
	controllers[AMP_PROCESS] = &ampProcessControl;
	controllers[SWING] = &swingControl;
	controllers[SWING_RANDOM] = &swingRandomControl;
	controllers[SWING_PROCESS] = &swingProcessControl;
	controllers[NR_OF_STEPS] = &nrStepsControl;
	controllers[AMP_MODE] = &modeSwitch;
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
	convertToShapeButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::convertButtonClickedCallback);
	convertToStepsButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::convertButtonClickedCallback);
	convertToShapeToLinearButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::lightButtonClickedCallback);
	convertToShapeToConstButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::lightButtonClickedCallback);
	markersAutoButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::markersAutoClickedCallback);
	markersManualButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::markersAutoClickedCallback);
	for (HaloToggleButton& s: shapeToolButtons) s.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::shapeToolClickedCallback);
	for (HaloButton& e: editToolButtons) e.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::editToolClickedCallback);
	for (HaloButton& h: historyToolButtons) h.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::historyToolClickedCallback);
	gridShowButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::gridToolClickedCallback);
	gridSnapButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BSchafflGUI::gridToolClickedCallback);
	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpButtonClickedCallback);
	//ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytButtonClickedCallback);
	shapeWidget.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, shapeChangedCallback);
	stepControlContainer.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, valueChangedCallback);

	// Configure widgets
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	userLatencySlider.hide();
	userLatencyUnitListbox.hide();
	nrStepsControl.setScrollable (true);
	nrStepsControl.getDisplayLabel ()->setState (BColors::ACTIVE);
	swingControl.getDisplayLabel ()->setState (BColors::ACTIVE);
	markerListBox.setOversize (true);

	shapeWidget.hide();
	shapeWidget.setMergeable (BEvents::POINTER_DRAG_EVENT, false);
	shapeWidget.setTool (ToolType::POINT_NODE_TOOL);
	shapeWidget.setDefaultShape ();
	shapeWidget.setScaleParameters (0.05, 0, 1.1);
	shapeWidget.setMajorXSteps (1.0);
	shapeWidget.setMinorXSteps (1.0/16.0);
	shapeWidget.setLowerLimit (0.0);
	shapeWidget.setHigherLimit (1.0);

	shapeToolButtons[1].setValue (1.0);
	gridSnapButton.setValue (1.0);
	shapeToolbox.hide();
	editToolbox.hide();
	//historyToolbox.hide();
	gridToolbox.hide();
	for (HaloToggleButton& s : shapeToolButtons) s.hide();
	for (HaloButton& e : editToolButtons) e.hide();
	//for (HaloButton& h : historyToolButtons) h.hide();
	gridShowButton.hide();
	gridSnapButton.hide();
	convertToShapeIcon.hide();

	convertToShapeMessage.setOversize (true);

	//Initialialize and configure stepControllers
	double sw = sContainer.getEffectiveWidth() - 40;
	double sx = sContainer.getXOffset() + 40;
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepControl[i] = BWidgets::VSlider ((i + 0.5) * sw / MAXSTEPS + sx - 7, 20, 14, 80, "slider", 1.0, 0.0, 1.0, 0.01);
		stepControl[i].setHardChangeable (false);
		stepControl[i].setScrollable (true);
		stepControl[i].setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BSchafflGUI::valueChangedCallback);
		stepControl[i].applyTheme (theme, "slider");
		stepControlContainer.add (stepControl[i]);

		stepControlLabel[i] = BWidgets::Label ((i + 0.5) * sw / MAXSTEPS + sx - 14, 0, 28, 20, "mlabel", "1.00");
		stepControlLabel[i].applyTheme (theme, "mlabel");
		stepControlLabel[i].setState (BColors::ACTIVE);
		stepControlContainer.add (stepControlLabel[i]);

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
	sContainer.add (stepControlContainer);
	sContainer.add (shapeWidget);

	mContainer.add (selectMenu);
	mContainer.add (sContainer);
	mContainer.add (helpButton);
	//mContainer.add (ytButton);

	toolbox.add (toolIcon);
	toolbox.add (convertToShapeIcon);
	toolbox.add (convertToStepsIcon);
	toolbox.add (convertToShapeButton);
	toolbox.add (convertToStepsButton);
	toolbox.add (markersToolbox);
	toolbox.add (shapeToolbox);
	toolbox.add (editToolbox);
	toolbox.add (historyToolbox);
	toolbox.add (gridToolbox);
	toolbox.add (markersAutoButton);
	toolbox.add (markersManualButton);
	for (HaloToggleButton& s : shapeToolButtons) toolbox.add (s);
	for (HaloButton& e : editToolButtons) toolbox.add (e);
	for (HaloButton& h : historyToolButtons) toolbox.add (h);
	toolbox.add (gridShowButton);
	toolbox.add (gridSnapButton);
	mContainer.add (toolbox);

	mContainer.add (modeSwitch);
	mContainer.add (seqLenValueListbox);
	mContainer.add (seqLenBaseListbox);
	mContainer.add (ampSwingControl);
	mContainer.add (ampRandomControl);
	mContainer.add (ampProcessControl);
	mContainer.add (swingControl);
	mContainer.add (swingRandomControl);
	mContainer.add (swingProcessControl);
	mContainer.add (nrStepsControl);
	mContainer.add (latencyValue);
	mContainer.add (latencyDisplay);
	mContainer.add (messageLabel);

	convertToShapeMessage.add (convertToShapeToLinearButton);
        convertToShapeMessage.add (convertToShapeToLinearText);
        convertToShapeMessage.add (convertToShapeToConstButton);
        convertToShapeMessage.add (convertToShapeToConstText);

	add (mContainer);

	std::array<double, MAXSTEPS> s;
	s.fill (1.0);
	sliderHistory.setDefault (s);
	sliderHistory.clear();

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
			if (obj->body.otype == uris.bschaffl_statusEvent)
			{
				const LV2_Atom *oPos = NULL, *oLat = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bschaffl_step, &oPos,
					uris.bschaffl_latency, &oLat,
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

			// Shape notification
			else if (obj->body.otype == uris.bschaffl_shapeEvent)
			{
				LV2_Atom *oData = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bschaffl_shapeData, &oData,
					NULL
				);

				if (oData && (oData->type == uris.atom_Vector))
				{

					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oData;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float)));
						float* data = (float*) (&vec->body + 1);

						shapeWidget.setValueEnabled (false);
						shapeWidget.clearShape ();
						for (int i = 0; (i < size) && (i < MAXNODES); ++i)
						{
							Node node;
							node.nodeType = NodeType (int (data[i * 7]));
							node.point.x = data[i * 7 + 1];
							node.point.y = data[i * 7 + 2];
							node.handle1.x = data[i * 7 + 3];
							node.handle1.y = data[i * 7 + 4];
							node.handle2.x = data[i * 7 + 5];
							node.handle2.y = data[i * 7 + 6];
							shapeWidget.appendRawNode (node);
						}
						shapeWidget.validateShape();
						shapeWidget.pushToSnapshots ();
						shapeWidget.update ();
						shapeWidget.setValueEnabled (true);
					}
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
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1020 * sz, 480 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 1020, 480, sz);

	RESIZE (helpButton, 950, 60, 24, 24, sz);

	RESIZE (toolbox, 380, 330, 620, 40, sz);
	RESIZE (toolIcon, 4, 2, 40, 26, sz);
	RESIZE (convertToShapeIcon, 50, 5, 50, 20, sz);
	RESIZE (convertToStepsIcon, 50, 5, 50, 20, sz);
	RESIZE (convertToShapeButton, 48, 3, 54, 24, sz);
	RESIZE (convertToStepsButton, 48, 3, 54, 24, sz);
	RESIZE (markersToolbox, 120, 5, 50, 20, sz);
	RESIZE (shapeToolbox, 190, 5, 140, 20, sz);
	RESIZE (editToolbox, 360, 5, 80, 20, sz);
	RESIZE (historyToolbox, 460, 5, 80, 20, sz);
	RESIZE (gridToolbox, 560, 5, 50, 20, sz);
	RESIZE (markersAutoButton, 118, 3, 24, 24, sz);
	RESIZE (markersManualButton, 148, 3, 24, 24, sz);
	for (size_t i = 0; i < shapeToolButtons.size(); ++i) {RESIZE (shapeToolButtons[i], 188 + i * 30, 3, 24, 24, sz);}
	for (size_t i = 0; i < editToolButtons.size(); ++i) {RESIZE (editToolButtons[i], 358 + i * 30, 3, 24, 24, sz);}
	for (size_t i = 0; i < historyToolButtons.size(); ++i) {RESIZE (historyToolButtons[i], 458 + i * 30, 3, 24, 24, sz);}
	RESIZE (gridShowButton, 588, 3, 24, 24, sz);
	RESIZE (gridSnapButton, 588, 3, 24, 24, sz);

	RESIZE (midiChFilterIcon, 0, 0, 300, 20, sz);
	RESIZE (midiChFilterContainer, 0, 0, 340, 140, sz);
	RESIZE (midiChFilterText, 10, 10, 320, 50, sz);
	RESIZE (midiChFilterAllSwitch, 10, 36, 28, 14, sz);
	RESIZE (midiChFilterAllLabel, 44, 33, 120, 20, sz);
	for (unsigned int i = 0; i < midiChFilterSwitches.size(); ++i)
	{
		RESIZE (midiChFilterSwitches[i], 10 + 80.0 * int (i / 4), 56 + int (i % 4) * 20, 28, 14, sz);
		RESIZE (midiChFilterLabels[i], 44 + 80.0 * int (i / 4), 53  + int (i % 4) * 20, 36, 20, sz);
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

	RESIZE (selectMenu, 20, 90, 340, 390, sz);

	RESIZE (sContainer, 380, 90, 620, 240, sz);

	RESIZE (modeSwitch, 426, 71, 28, 14, sz);
	RESIZE (seqLenValueListbox, 380, 380, 50, 20, sz);
	seqLenValueListbox.resizeListBox (BUtilities::Point (50 * sz, 220 * sz));
	seqLenValueListbox.moveListBox (BUtilities::Point (0, -220 * sz));
	seqLenValueListbox.resizeListBoxItems (BUtilities::Point (50 * sz, 20 * sz));
	RESIZE (seqLenBaseListbox, 440, 380, 90, 20, sz);
	seqLenBaseListbox.resizeListBox (BUtilities::Point (90 * sz, 80 * sz));
	seqLenBaseListbox.moveListBox (BUtilities::Point (0, -80 * sz));
	seqLenBaseListbox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (ampSwingControl, 565, 372, 120, 28, sz);
	RESIZE (ampRandomControl, 720, 372, 120, 28, sz);
	RESIZE (ampProcessControl, 875, 372, 120, 28, sz);
	RESIZE (swingControl, 565, 422, 120, 28, sz);
	RESIZE (swingRandomControl, 720, 422, 120, 28, sz);
	RESIZE (swingProcessControl, 875, 422, 120, 28, sz);
	RESIZE (nrStepsControl, 380, 422, 155, 28, sz);
	RESIZE (stepControlContainer, 40, 40, 580, 130, sz);
	RESIZE (shapeWidget, 40, 40, 580, 130, sz);
	RESIZE (markerListBox, 12, -68, 86, 66, sz);
	markerListBox.resizeItems (BUtilities::Point (80 * sz, 20 * sz));

	RESIZE (messageLabel, 480, 63, 465, 20, sz);
	RESIZE (latencyDisplay, 900, 10, 120, 10, sz);

	RESIZE (inIcon, 4, 14, 32, 12, sz);
	RESIZE (ampIcon, 4, 105, 32, 12, sz);
	RESIZE (delIcon, 4, 185, 32, 12, sz);
	RESIZE (outIcon, 4, 214, 32, 12, sz);

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

	toolbox.applyTheme (theme);
	toolIcon.applyTheme (theme);
	convertToShapeIcon.applyTheme (theme);
	convertToStepsIcon.applyTheme (theme);
	convertToShapeButton.applyTheme (theme);
	convertToStepsButton.applyTheme (theme);
	markersToolbox.applyTheme (theme);
	shapeToolbox.applyTheme (theme);
	editToolbox.applyTheme (theme);
	historyToolbox.applyTheme (theme);
	gridToolbox.applyTheme (theme);
	markersAutoButton.applyTheme (theme);
	markersManualButton.applyTheme (theme);
	for (HaloToggleButton& s : shapeToolButtons) s.applyTheme (theme);
	for (HaloButton& e : editToolButtons) e.applyTheme (theme);
	for (HaloButton& h : historyToolButtons) h.applyTheme (theme);
	gridShowButton.applyTheme (theme);
	gridSnapButton.applyTheme (theme);

	convertToShapeMessage.applyTheme (theme);
        convertToShapeToLinearButton.applyTheme (theme);
        convertToShapeToLinearText.applyTheme (theme);
        convertToShapeToConstButton.applyTheme (theme);
        convertToShapeToConstText.applyTheme (theme);

	convertToStepsMessage.applyTheme (theme);

	modeSwitch.applyTheme (theme);
	seqLenValueListbox.applyTheme (theme);
	seqLenBaseListbox.applyTheme (theme);
	ampSwingControl.applyTheme (theme);
	ampRandomControl.applyTheme (theme);
	ampProcessControl.applyTheme (theme);
	swingControl.applyTheme (theme);
	swingRandomControl.applyTheme (theme);
	swingProcessControl.applyTheme (theme);
	stepControlContainer.applyTheme (theme);
	shapeWidget.applyTheme (theme);
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

	sz = (getWidth() / 1020 > getHeight() / 480 ? getHeight() / 480 : getWidth() / 1020);
	resizeGUI ();
}

void BSchafflGUI::onCloseRequest (BEvents::WidgetEvent* event)
{
	if (event)
	{
		Widget* requestWidget = event->getRequestWidget ();
		if (requestWidget)
		{
			if (requestWidget == (Widget*) &convertToShapeMessage)
			{
				if (convertToShapeMessage.getValue() == 1.0)
				{
					if (convertToShapeToLinearButton.getValue() != 0.0)
					{
						int nrSteps = nrStepsControl.getValue();
						shapeWidget.setValueEnabled (false);
						shapeWidget.clearShape ();

						shapeWidget.appendRawNode
						(
							Node
							(
								END_NODE,
								{0.0, (getStepValue (0) + getStepValue (nrSteps - 1)) /2},
								{0.0, 0.0},
								{0.0, 0.0}
							)
						);

						for (int i = 0; i < nrSteps; ++i)
						{
							shapeWidget.appendRawNode
							(
								Node
								(
									POINT_NODE,
									{(double (i) + 0.5) / double (nrSteps), getStepValue (i)},
									{0.0, 0.0},
									{0.0, 0.0}
								)
							);
						}

						shapeWidget.appendRawNode
						(
							Node
							(
								END_NODE,
								{1.0, (getStepValue (0) + getStepValue (nrSteps - 1)) /2},
								{0.0, 0.0},
								{0.0, 0.0}
							)
						);

						shapeWidget.validateShape();
						shapeWidget.pushToSnapshots ();
						shapeWidget.update ();
						shapeWidget.setValueEnabled (true);
						shapeWidget.setValue (1.0);
					}

					else if (convertToShapeToConstButton.getValue() != 0.0)
					{
						int nrSteps = nrStepsControl.getValue();
						shapeWidget.setValueEnabled (false);
						shapeWidget.clearShape ();

						shapeWidget.appendRawNode (Node (END_NODE, {0.0, (getStepValue (0))}, {0.0, 0.0}, {0.0, 0.0}));

						for (int i = 0; i < nrSteps; ++i)
						{
							if (i != 0)
							{
								shapeWidget.appendRawNode
								(
									Node
									(
										POINT_NODE,
										{double (i) / double (nrSteps), getStepValue (i)},
										{0.0, 0.0},
										{0.0, 0.0}
									)
								);
							}

							shapeWidget.appendRawNode
							(
								Node
								(
									POINT_NODE,
									{double (i + 1) / double (nrSteps) - 1.0 / double (MAPRES), getStepValue (i)},
									{0.0, 0.0},
									{0.0, 0.0}
								)
							);
						}

						shapeWidget.appendRawNode (Node (END_NODE, {1.0, (getStepValue (0))}, {0.0, 0.0}, {0.0, 0.0}));

						shapeWidget.validateShape();
						shapeWidget.pushToSnapshots ();
						shapeWidget.update ();
						shapeWidget.setValueEnabled (true);
						shapeWidget.setValue (1.0);
					}
				}


				if (requestWidget->getMainWindow()) release (requestWidget);
			}

			else if (requestWidget == (Widget*) &convertToStepsMessage)
			{
				if (convertToStepsMessage.getValue() == 1.0)
				{
					// Convert to sliders
					int nrSteps = nrStepsControl.getValue();
					for (int i = 0; i < nrSteps; ++i)
					{
						stepControl[i].setValue (shapeWidget.getMapValue ((double (i) + 0.5) / double (nrSteps)));
					}
				}

				if (requestWidget->getMainWindow()) release (requestWidget);
			}

			else Window::onCloseRequest (event);
		}
	}
}

double BSchafflGUI::getStepValue (const int stepNr) const
{
	const float aswing = ((stepNr % 2) == 0 ? ampSwingControl.getValue() : 1.0 / ampSwingControl.getValue());
	return stepControl[stepNr].getValue() * LIM (aswing, 0, 1);
}

void BSchafflGUI::sendUiStatus (const bool on)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, (on ? uris.bschaffl_uiOn : uris.bschaffl_uiOff));
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSchafflGUI::sendShape ()
{
	size_t size = shapeWidget.size ();

	uint8_t obj_buf[4096];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	// Load shapeBuffer
	float shapeBuffer[MAXNODES * 7];
	for (unsigned int i = 0; i < size; ++i)
	{
		Node node = shapeWidget.getRawNode (i);
		shapeBuffer[i * 7 + 0] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object (&forge, &frame, 0, uris.bschaffl_shapeEvent);
	lv2_atom_forge_key(&forge, uris.bschaffl_shapeData);
	lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&forge, &frame);
	write_function (controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
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

	const double sw = sContainer.getWidth() - 40 * sz;
	const double sx = 40 * sz;
	const double sccw = stepControlContainer.getWidth();
	const double oddf = (ampSwingControl.getValue() >= 1.0 ? 1.0 : ampSwingControl.getValue());
	const double evenf = (ampSwingControl.getValue() >= 1.0 ? 1.0 / ampSwingControl.getValue() : 1.0);

	for (int i = 0; i < MAXSTEPS; ++i)
	{
		if (i < nrStepsi)
		{
			stepControl[i].resize (14 * sz, (14 + LIMIT (96 * ((i % 2) == 0 ? oddf : evenf), 0, 96 )) * sz);
			stepControl[i].moveTo ((i + 0.5) * sccw / nrStepsi - 7 * sz, 130 * sz - stepControl[i].getHeight());
			stepControl[i].show();

			if (i < nrStepsi - 1) markerWidgets[i].resize (10 * sz, 16 * sz);

			stepControlLabel[i].moveTo ((i + 0.5) * sccw / nrStepsi - 14 * sz, 0);
			stepControlLabel[i].resize (28 * sz, 20 * sz);
			stepControlLabel[i].show();

			inputStepLabel[i].moveTo (sx + (i + 0.1) * sw / nrStepsi, 10 * sz);
			inputStepLabel[i].resize (0.8 * (sw / nrStepsi), 20 * sz);
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
			outputStepLabel[i].moveTo (sx + outx + 0.1 * outw, 210 * sz);
			outputStepLabel[i].resize (0.8 * outw, 20 * sz);
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
			markerWidgets[i].moveTo (markerWidgets[i].getValue() * sw + sx - 5 * sz, 184 * sz);
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

				else if (controllerNr == AMP_MODE)
				{
					if (value == 0.0f)
					{
						ui->stepControlContainer.show();
						ui->shapeWidget.hide();
						ui->shapeToolbox.hide();
						ui->editToolbox.hide();
						//ui->historyToolbox.hide();
						ui->gridToolbox.hide();
						for (HaloToggleButton& s : ui->shapeToolButtons) s.hide();
						for (HaloButton& e : ui->editToolButtons) e.hide();
						//for (HaloButton& h : ui->historyToolButtons) h.hide();
						ui->gridShowButton.hide();
						ui->gridSnapButton.hide();
						ui->ampSwingControl.setState (BColors::NORMAL);
						ui->convertToShapeIcon.hide();
						ui->convertToStepsIcon.show();
						ui->convertToShapeButton.hide();
						ui->convertToStepsButton.show();
						if (ui->convertToShapeMessage.getParent()) ui->release (&ui->convertToShapeMessage);
					}
					else
					{
						ui->stepControlContainer.hide();
						ui->shapeWidget.show();
						ui->shapeToolbox.show();
						ui->editToolbox.show();
						//ui->historyToolbox.show();
						ui->gridToolbox.show();
						for (HaloToggleButton& s : ui->shapeToolButtons) s.show();
						for (HaloButton& e : ui->editToolButtons) e.show();
						//for (HaloButton& h : ui->historyToolButtons) h.show();
						ui->gridShowButton.show();
						ui->gridSnapButton.show();
						ui->ampSwingControl.setState (BColors::INACTIVE);
						ui->convertToShapeIcon.show();
						ui->convertToStepsIcon.hide();
						ui->convertToShapeButton.show();
						ui->convertToStepsButton.hide();
						if (ui->convertToStepsMessage.getParent()) ui->release (&ui->convertToStepsMessage);
					}

					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
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
					int stepNr = controllerNr - STEP_LEV;
					ui->stepControlLabel[stepNr].setText (BUtilities::to_string (value, "%1.2f"));
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);

					std::array<double, MAXSTEPS> u = ui->sliderHistory.undo();
					for (int i = 0; i < MAXSTEPS; ++i)
					{
						if ((i != stepNr) && (float (u[i]) != float (ui->stepControl[i].getValue())))
						{
							ui->sliderHistory.redo();
							break;
						}
					}
					ui->stepControlContainer.setValue (1.0);
				}

				else if (controllerNr == AMP_SWING)
				{
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
					ui->rearrange_controllers();
				}

				else if (controllerNr == AMP_PROCESS)
				{
					if ((value >= 0.0) && (value <= 1.0)) ui->ampProcessControl.setState (BColors::NORMAL);
					else ui->ampProcessControl.setState (BColors::ACTIVE);
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
				}

				else if (controllerNr == SWING)
				{
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
					ui->setAutoMarkers();
					ui->rearrange_controllers();
					ui->redrawSContainer();
				}

				else if (controllerNr == NR_OF_STEPS)
				{
					ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
					ui->shapeWidget.setMinorXSteps (1.0 / value);
					ui->setAutoMarkers();
					ui->rearrange_controllers();
					ui->redrawSContainer();
				}

				else ui->write_function (ui->controller, CONTROLLERS + controllerNr, sizeof (float), 0, &value);
			}

			else if (widget == &ui->stepControlContainer)
			{
				if (value != 0.0)
				{
					std::array<double, MAXSTEPS> steps;
					for (int i = 0; i < MAXSTEPS; ++i) steps[i] = ui->stepControl[i].getValue();
					ui->sliderHistory.push (steps);
					ui->stepControlContainer.setValue (0.0);
				}
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

void BSchafflGUI::shapeChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	ShapeWidget* widget = (ShapeWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	if (value != 1.0) return;
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	ui->sendShape ();
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
	HaloButton* hb = (HaloButton*) vev->getWidget();
	if (!hb) return;
	BSchafflGUI* ui = (BSchafflGUI*)hb->getMainWindow();
	if (!ui) return;

	if (hb == &ui->markersAutoButton)
	{
		for (Marker& m : ui->markerWidgets) m.setHasValue (false);
	}

	else if (hb == &ui->markersManualButton)
	{
		for (Marker& m : ui->markerWidgets) m.setHasValue (true);
	}

	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
}

void BSchafflGUI::shapeToolClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	HaloToggleButton* widget = (HaloToggleButton*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify tool
	int widgetNr = 0;
	if (value)
	{
		for (int i = 1; i < NR_TOOLS; ++i)
		{
			if (widget == &ui->shapeToolButtons[i - 1])
			{
				widgetNr = i;
				break;
			}
		}
	}

	ui->shapeWidget.setTool (ToolType (widgetNr));

	// Allow only one button pressed
	for (HaloToggleButton& s : ui->shapeToolButtons)
	{
		if (&s != widget) s.setValue (0.0);
	}
}

void BSchafflGUI::editToolClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	HaloButton* widget = (HaloButton*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	if (value != 1.0) return;
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify editButtons
	int widgetNr = -1;
	for (size_t i = 0; i < ui->editToolButtons.size(); ++i)
	{
		if (widget == &ui->editToolButtons[i])
		{
			widgetNr = i;
			break;
		}
	}

	// Action
	switch (widgetNr)
	{
		case 0:		ui->clipboard = ui->shapeWidget.cutSelection();
				break;

		case 1:		ui->clipboard = ui->shapeWidget.copySelection();
				break;

		case 2:		ui->shapeWidget.pasteSelection (ui->clipboard);
				break;

		default:	break;
	}
}

void BSchafflGUI::historyToolClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	HaloButton* widget = (HaloButton*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	if (value != 1.0) return;
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify editButtons
	int widgetNr = -1;
	for (size_t i = 0; i < ui->historyToolButtons.size(); ++i)
	{
		if (widget == &ui->historyToolButtons[i])
		{
			widgetNr = i;
			break;
		}
	}

	// Action
	if (ui->modeSwitch.getValue() == 0.0)
	{
		std::array<double, MAXSTEPS> values;

		switch (widgetNr)
		{
			case 0:		for (int i = 0; i < MAXSTEPS; ++i)
					{
						values[i] = 1.0;
						ui->stepControl[i].setValueable (false);
						ui->stepControl[i].setValue(1.0);
						ui->stepControl[i].setValueable (true);
						ui->stepControlLabel[i].setText (BUtilities::to_string (value, "%1.2f"));
						ui->write_function (ui->controller, CONTROLLERS + STEP_LEV + i, sizeof (float), 0, &value);
					}
					ui->sliderHistory.push (values);
					break;

			case 1:		values = ui->sliderHistory.undo();
					for (int i = 0; i < MAXSTEPS; ++i)
					{
						float value = values[i];
						ui->stepControl[i].setValueable (false);
						ui->stepControl[i].setValue(value);
						ui->stepControl[i].setValueable (true);
						ui->stepControlLabel[i].setText (BUtilities::to_string (value, "%1.2f"));
						ui->write_function (ui->controller, CONTROLLERS + STEP_LEV + i, sizeof (float), 0, &value);
					}
					break;

			case 2:		values = ui->sliderHistory.redo();
					for (int i = 0; i < MAXSTEPS; ++i)
					{
						float value = values[i];
						ui->stepControl[i].setValueable (false);
						ui->stepControl[i].setValue(value);
						ui->stepControl[i].setValueable (true);
						ui->stepControlLabel[i].setText (BUtilities::to_string (value, "%1.2f"));
						ui->write_function (ui->controller, CONTROLLERS + STEP_LEV + i, sizeof (float), 0, &value);
					}
					break;

			default:	break;
		}
	}

	else
	{
		switch (widgetNr)
		{
			case 0:		ui->shapeWidget.reset();
					break;

			case 1:		ui->shapeWidget.undo();
					break;

			case 2:		ui->shapeWidget.redo();
					break;

			default:	break;
		}
	}
}

void BSchafflGUI::gridToolClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	HaloToggleButton* widget = (HaloToggleButton*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	if (widget == &ui->gridShowButton)
	{
		if (value) ui->shapeWidget.showGrid();
		else ui->shapeWidget.hideGrid();
		ui->shapeWidget.setSnap (false);
		ui->gridSnapButton.setValue (0.0);
	}

	else if (widget == &ui->gridSnapButton)
	{
		if (value)
		{
			ui->shapeWidget.showGrid();
			ui->shapeWidget.setSnap (true);
		}
		else
		{
			ui->shapeWidget.hideGrid();
			ui->shapeWidget.setSnap (false);
		}
		ui->gridShowButton.setValue (0.0);
	}
}

void BSchafflGUI::convertButtonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	HaloButton* widget = (HaloButton*) event->getWidget ();
	if (!widget) return;
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	if (widget == &ui->convertToShapeButton)
	{
		if (!ui->convertToShapeMessage.getParent())
		{
			ui->add (ui->convertToShapeMessage);
			ui->convertToShapeToLinearButton.setValue (1.0);
		}
	}

	else if (widget == &ui->convertToStepsButton)
	{
		if (!ui->convertToStepsMessage.getParent()) ui->add (ui->convertToStepsMessage);
	}
}

void BSchafflGUI::lightButtonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	LightButton* widget = (LightButton*) event->getWidget ();
	if (!widget) return;
	double value = widget->getValue();
	if (!value) return;
	BWidgets::Widget* parent = widget->getParent();
	if (!parent) return;
	std::vector<BWidgets::Widget*> children = parent->getChildren();
	BSchafflGUI* ui = (BSchafflGUI*) widget->getMainWindow();
	if (!ui) return;

	// Un-toggle all other LightButtons
	for (BWidgets::Widget* w : children)
	{
		LightButton* l = dynamic_cast<LightButton*> (w);
		if ((l) && (l != widget)) l->setValue (0.0);
	}
}

void BSchafflGUI::helpButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " HELP_URL);}
//void BSchafflGUI::ytButtonClickedCallback (BEvents::Event* event) {system(OPEN_CMD " " YT_URL);}

void BSchafflGUI::redrawSContainer ()
{
	double width = sContainer.getWidth ();
	double x0 = sContainer.getXOffset();
	double y0 = sContainer.getYOffset();
	double height = sContainer.getHeight ();

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
	cairo_move_to (cr, 40 * sz - x0, -y0);
	cairo_rel_line_to (cr, 0, 240 * sz);

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		cairo_move_to (cr, double (i + 1) / double (nrSteps) * (width - 40 * sz) + 40 * sz - x0, -y0);
		cairo_rel_line_to (cr, 0, 170 * sz);
		cairo_line_to (cr, markerWidgets[i].getValue() * (width - 40 * sz) + 40 * sz - x0, 180 * sz - y0);
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
	if ((screenWidth < 1060) || (screenHeight < 520)) sz = 0.66;

	if (resize) resize->ui_resize(resize->handle, 1020 * sz, 480 * sz);

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
