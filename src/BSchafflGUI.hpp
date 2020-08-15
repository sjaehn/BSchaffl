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

 #ifndef BSCHAFFLGUI_HPP_
 #define BSCHAFFLGUI_HPP_

#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include "BWidgets/Widget.hpp"
#include "BWidgets/Window.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "BWidgets/HSwitch.hpp"
#include "BWidgets/VSlider.hpp"
#include "BWidgets/VSliderValue.hpp"
#include "BWidgets/HSliderValue.hpp"
#include "BWidgets/DialValue.hpp"
#include "BWidgets/ListBox.hpp"
#include "BWidgets/ToggleButton.hpp"
#include "BWidgets/TextButton.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "BWidgets/ImageIcon.hpp"
#include "BWidgets/Text.hpp"
#include "BWidgets/MessageBox.hpp"
#include "Marker.hpp"
#include "LightButton.hpp"
#include "SwingHSlider.hpp"
#include "HaloButton.hpp"
#include "HaloToggleButton.hpp"
#include "SelectMenu.hpp"
#include "ShapeWidget.hpp"

#include "definitions.hpp"
#include "Ports.hpp"
#include "Urids.hpp"

#ifndef MESSAGENR_
#define MESSAGENR_
enum MessageNr
{
	NO_MSG		= 0,
	JACK_STOP_MSG	= 1,
        LATENCY_MAX_MSG = 2,
	MAX_MSG		= 2
};
#endif /* MESSAGENR_ */

#define BG_FILE "inc/surface.png"
#define HELP_URL "https://github.com/sjaehn/BSchaffl/blob/master/README.md"
//#define YT_URL " https://youtu.be/PuzoxiAs-h8"
#define OPEN_CMD "xdg-open"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))
#define RESIZE(widget, x, y, w, h, sz) (widget).moveTo ((x) * (sz), (y) * (sz)); (widget).resize ((w) * (sz), (h) * (sz));

const std::string messageStrings[MAX_MSG + 1] =
{
	"",
	"*** Jack transport off or halted. ***",
        "*** Latency exceeds maximum of 192000 frames. ***"
};

const std::array<std::string, NR_MIDI_MSG_FILTERS> midiMsgGroupTexts = {{"Note", "Key pressure", "Control change", "Program change", "Channel pressure", "Pitch bend", "System exclusive"}};

class BSchafflGUI : public BWidgets::Window
{
public:
	BSchafflGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow);
	~BSchafflGUI ();
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	virtual void onConfigureRequest (BEvents::ExposeEvent* event) override;
        virtual void onCloseRequest (BEvents::WidgetEvent* event) override;
	void applyTheme (BStyles::Theme& theme) override;
        void sendUiStatus (const bool on);
        void sendShape ();

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
        double getStepValue (const int stepNr) const;
	void resizeGUI ();
        void setMarker (const int markerNr, double value);
        void setAutoMarkers ();
	void rearrange_controllers ();
        void redrawSContainer ();
	static void valueChangedCallback (BEvents::Event* event);
        static void shapeChangedCallback (BEvents::Event* event);
        static void markerClickedCallback (BEvents::Event* event);
	static void markerDraggedCallback (BEvents::Event* event);
        static void listBoxChangedCallback (BEvents::Event* event);
        static void markersAutoClickedCallback (BEvents::Event* event);
        static void shapeToolClickedCallback (BEvents::Event* event);
        static void editToolClickedCallback (BEvents::Event* event);
        static void historyToolClickedCallback (BEvents::Event* event);
        static void gridToolClickedCallback (BEvents::Event* event);
        static void convertButtonClickedCallback (BEvents::Event* event);
        static void lightButtonClickedCallback (BEvents::Event* event);
        static void helpButtonClickedCallback (BEvents::Event* event);
	//static void ytButtonClickedCallback (BEvents::Event* event);

        std::string pluginPath;

	BWidgets::Widget mContainer;

        HaloButton helpButton;
        //HaloButton ytButton;

        BWidgets::Widget toolbox;
        BWidgets::ImageIcon toolIcon;
        BWidgets::ImageIcon convertToShapeIcon;
        BWidgets::ImageIcon convertToStepsIcon;
        HaloButton convertToShapeButton;
        HaloButton convertToStepsButton;
        BWidgets::ImageIcon markersToolbox;
        BWidgets::ImageIcon shapeToolbox;
        BWidgets::ImageIcon editToolbox;
        BWidgets::ImageIcon historyToolbox;
        BWidgets::ImageIcon gridToolbox;
        HaloButton markersAutoButton;
        HaloButton markersManualButton;
        std::array<HaloToggleButton, 5> shapeToolButtons;
        std::array<HaloButton, 3> editToolButtons;
        std::array<HaloButton, 3> historyToolButtons;
        HaloToggleButton gridShowButton;
        HaloToggleButton gridSnapButton;

        BWidgets::MessageBox convertToShapeMessage;
        LightButton convertToShapeToLinearButton;
        BWidgets::Text convertToShapeToLinearText;
        LightButton convertToShapeToConstButton;
        BWidgets::Text convertToShapeToConstText;

        BWidgets::MessageBox convertToStepsMessage;

        BWidgets::ImageIcon midiChFilterIcon;
        BWidgets::Widget midiChFilterContainer;
        BWidgets::Text midiChFilterText;
        BWidgets::HSwitch midiChFilterAllSwitch;
        BWidgets::Label midiChFilterAllLabel;
        std::array<BWidgets::HSwitch, NR_MIDI_CHS> midiChFilterSwitches;
        std::array<BWidgets::Label, NR_MIDI_CHS> midiChFilterLabels;

        BWidgets::ImageIcon midiMsgFilterIcon;
        BWidgets::Widget midiMsgFilterContainer;
        BWidgets::Text midiMsgFilterText;
        BWidgets::HSwitch midiMsgFilterAllSwitch;
        BWidgets::Label midiMsgFilterAllLabel;
        std::array<BWidgets::HSwitch, NR_MIDI_MSG_FILTERS> midiMsgFilterSwitches;
        std::array<BWidgets::Label, NR_MIDI_MSG_FILTERS> midiMsgFilterLabels;

        BWidgets::ImageIcon midiNoteOptionsIcon;
        BWidgets::Widget midiNoteOptionsContainer;
        BWidgets::Text midiNoteText;
        BWidgets::HSwitch midiNotePositionSwitch;
        BWidgets::Label midiNotePositionLabel;
        BWidgets::HSwitch midiNoteValueSwitch;
        BWidgets::Label midiNoteValueLabel;
        BWidgets::PopupListBox midiNoteOverlapListbox;
        BWidgets::Label midiNoteOverlapLabel;
        BWidgets::Text midiNoteText2;
        BWidgets::Label midiNoteOffAmpLabel;
        BWidgets::PopupListBox midiNoteOffAmpListbox;

        BWidgets::ImageIcon smartQuantizationIcon;
        BWidgets::Widget smartQuantizationContainer;
        BWidgets::HSliderValue smartQuantizationRangeSlider;
        BWidgets::HSwitch smartQuantizationMappingSwitch;
        BWidgets::HSwitch smartQuantizationPositioningSwitch;
        BWidgets::Text smartQuantizationText1;
        BWidgets::Label smartQuantizationRangeLabel;
        BWidgets::Text smartQuantizationText2;
        BWidgets::Label smartQuantizationMappingLabel;
        BWidgets::Label smartQuantizationPositionLabel;

        BWidgets::ImageIcon userLatencyIcon;
        BWidgets::Widget userLatencyContainer;
        BWidgets::Text timeCompensText;
        BWidgets::Label timeCompensLabel;
        BWidgets::HSwitch timeCompensSwitch;
        BWidgets::Text userLatencyText;
        BWidgets::HSwitch userLatencySwitch;
        BWidgets::Label userLatencyLabel;
        BWidgets::RangeWidget userLatencyValue;
        BWidgets::HSliderValue userLatencySlider;
        BWidgets::PopupListBox userLatencyUnitListbox;

        SelectMenu selectMenu;

        BWidgets::DrawingSurface sContainer;
        BWidgets::HSwitch modeSwitch;
	BWidgets::PopupListBox seqLenValueListbox;
        BWidgets::PopupListBox seqLenBaseListbox;
        SwingHSlider ampSwingControl;
        BWidgets::HSliderValue ampRandomControl;
        BWidgets::HSliderValue ampProcessControl;
        SwingHSlider swingControl;
        BWidgets::HSliderValue swingRandomControl;
        BWidgets::HSliderValue swingProcessControl;
	BWidgets::HSliderValue nrStepsControl;
        BWidgets::ValueWidget stepControlContainer;
        ShapeWidget shapeWidget;
	std::array<BWidgets::VSlider, MAXSTEPS> stepControl;
        std::array<BWidgets::Label, MAXSTEPS> stepControlLabel;
	std::array<Marker, MAXSTEPS - 1> markerWidgets;
        BWidgets::ListBox markerListBox;
        BWidgets::ValueWidget latencyValue;     // Dummy
        BWidgets::Label latencyDisplay;
        std::array<BWidgets::ValueWidget*, NR_CONTROLLERS> controllers;
        BWidgets::Label messageLabel;
        BWidgets::ImageIcon inIcon;
        BWidgets::ImageIcon ampIcon;
        BWidgets::ImageIcon delIcon;
        BWidgets::ImageIcon outIcon;
        std::array<BWidgets::Label, MAXSTEPS> inputStepLabel;
        std::array<BWidgets::Label, MAXSTEPS> outputStepLabel;

        std::vector<Node> clipboard;
        Snapshots<std::array<double, MAXSTEPS>, MAXUNDO> sliderHistory;

        double sz;
	cairo_surface_t* bgImageSurface;

	LV2_Atom_Forge forge;
	BSchafflURIs uris;
	LV2_URID_Map* map;
        double rate;

	// Definition of styles
	BColors::ColorSet fgColors = {{{0.75, 0.75, 0.75, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.2, 0.2, 0.2, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
        BColors::ColorSet procColors = {{{0.75, 0.75, 0.75, 1.0}, {0.75, 0.0, 0.0, 1.0}, {0.2, 0.2, 0.2, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{BColors::lightgrey, BColors::white, BColors::darkgrey, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
        BColors::ColorSet procBgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.15, 0.15, 0.15, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};

	BStyles::Border border = {{BColors::darkgrey, 1.0}, 0.0, 2.0, 0.0};
        BStyles::Border menuborder = {{BColors::darkgrey, 1.0}, 0.0, 0.0, 0.0};
        BStyles::Border stepborder = {{BColors::darkgrey, 1.0}, 0.0, 2.0, 3.0};
        BStyles::Border labelBorder = BStyles::Border (BStyles::noLine, 0.0, 4.0);
        BStyles::Border focusborder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.5), 2.0));
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.75));
        BStyles::Fill actBg = BStyles::Fill (BColors::lightgrey);
	BStyles::Border screenBorder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.75), 4.0));
	BStyles::Font defaultFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
        BStyles::Font mdFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 10.0,
                                              BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
        BStyles::Font smFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 8.0,
                                              BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
      	BStyles::Font lfFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
      						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
      	BStyles::Font txFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
      						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_TOP);

        BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
						       {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
						    {"border", STYLEPTR (&BStyles::noBorder)},
						    {"textcolors", STYLEPTR (&txColors)},
						    {"font", STYLEPTR (&defaultFont)}}};
        BStyles::StyleSet mlStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                                 {"textcolors", STYLEPTR (&txColors)},
                                                 {"font", STYLEPTR (&mdFont)}}};
        BStyles::StyleSet smStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                                 {"textcolors", STYLEPTR (&txColors)},
                                                 {"font", STYLEPTR (&smFont)}}};
        BStyles::StyleSet lfStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                              {"border", STYLEPTR (&BStyles::noBorder)},
                                              {"textcolors", STYLEPTR (&txColors)},
                                              {"font", STYLEPTR (&lfFont)}}};
        BStyles::StyleSet txStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                              {"border", STYLEPTR (&BStyles::noBorder)},
                                              {"textcolors", STYLEPTR (&txColors)},
                                              {"font", STYLEPTR (&txFont)}}};

        BStyles::StyleSet focusStyles = {"labels", {{"background", STYLEPTR (&screenBg)},
                        			    {"border", STYLEPTR (&focusborder)},
                        			    {"textcolors", STYLEPTR (&txColors)},
                        			    {"font", STYLEPTR (&defaultFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"B.Schaffl", 	{{"background", STYLEPTR (&BStyles::blackFill)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"main", 	{{"background", STYLEPTR (&widgetBg)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"widget", 	{{"uses", STYLEPTR (&defaultStyles)}}},
                {"screen", 	{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"scontainer", 	{{"background", STYLEPTR (&BStyles::noFill)},
				 {"border", STYLEPTR (&border)}}},
                {"listbox",	{{"border", STYLEPTR (&menuborder)},
 				 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"listbox/item",{{"uses", STYLEPTR (&defaultStyles)},
 				 {"border", STYLEPTR (&labelBorder)},
 				 {"textcolors", STYLEPTR (&BColors::whites)},
 				 {"font", STYLEPTR (&defaultFont)}}},
 		{"listbox/button",{{"border", STYLEPTR (&BColors::darks)},
 				 {"background", STYLEPTR (&BStyles::blackFill)},
 			 	 {"bgcolors", STYLEPTR (&BColors::darks)}}},
 		{"menu",	{{"border", STYLEPTR (&menuborder)},
 				 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"menu/item",	{{"background", STYLEPTR (&BStyles::blackFill)},
 			 	 {"border", STYLEPTR (&BStyles::noBorder)},
 				 {"textcolors", STYLEPTR (&BColors::whites)},
 				 {"font", STYLEPTR (&lfFont)}}},
 		{"menu/button",	{{"border", STYLEPTR (&menuborder)},
 				 {"background", STYLEPTR (&BStyles::blackFill)},
 				 {"bgcolors", STYLEPTR (&bgColors)}}},
 		{"menu/listbox",{{"border", STYLEPTR (&menuborder)},
 				 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"menu/listbox/item",{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&BStyles::noBorder)},
 				 {"textcolors", STYLEPTR (&BColors::whites)},
 				 {"font", STYLEPTR (&lfFont)}}},
 		{"menu/listbox/button",{{"border", STYLEPTR (&BStyles::noBorder)},
 				 {"background", STYLEPTR (&BStyles::blackFill)},
 				 {"bgcolors", STYLEPTR (&bgColors)}}},
                {"shape",	{{"background", STYLEPTR (&BStyles::noFill)},
                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                 {"fgcolors", STYLEPTR (&fgColors)},
                                 {"symbolcolors", STYLEPTR (&fgColors)},
                                 {"font", STYLEPTR (&smFont)},
                                 {"bgcolors", STYLEPTR (&bgColors)}}},
                {"shape/focus", {{"background", STYLEPTR (&screenBg)},
                                 {"border", STYLEPTR (&screenBorder)},
                                 {"font", STYLEPTR (&smFont)},
                                 {"textcolors", STYLEPTR (&txColors)}}},
		{"selectmenu",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"bgcolors", STYLEPTR (&fgColors)}}},
                {"halobutton", 	{{"uses", STYLEPTR (&defaultStyles)},
 				 {"fgcolors", STYLEPTR (&bgColors)}}},
 		{"halobutton/focus", {{"uses", STYLEPTR (&focusStyles)}}},
		{"slider",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"slider/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"procslider",{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&procColors)},
				 {"bgcolors", STYLEPTR (&procBgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"procslider/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"label",	{{"uses", STYLEPTR (&labelStyles)}}},
                {"mlabel",	{{"uses", STYLEPTR (&mlStyles)}}},
                {"lflabel",	{{"uses", STYLEPTR (&lfStyles)}}},
                {"text",	{{"uses", STYLEPTR (&txStyles)}}},
                {"steplabel",	{{"uses", STYLEPTR (&smStyles)},
				 {"border", STYLEPTR (&stepborder)}}},
                {"actsteplabel",{{"uses", STYLEPTR (&smStyles)},
                                 {"background", STYLEPTR (&actBg)},
                                 {"textcolors", STYLEPTR (&BColors::darks)}}},
                {"smlabel",	{{"uses", STYLEPTR (&smStyles)}}},
		{"hilabel",	{{"uses", STYLEPTR (&labelStyles)},
				 {"textcolors", STYLEPTR (&BColors::whites)}}},
	});
};

#endif /* BSCHAFFLGUI_HPP_ */
