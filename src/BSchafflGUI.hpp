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
#include "Marker.hpp"
#include "LightButton.hpp"
#include "SwingHSlider.hpp"
#include "HaloButton.hpp"

#include "definitions.hpp"
#include "Ports.hpp"
#include "Urids.hpp"

#ifndef MESSAGENR_
#define MESSAGENR_
enum MessageNr
{
	NO_MSG		= 0,
	JACK_STOP_MSG	= 1,
	MAX_MSG		= 1
};
#endif /* MESSAGENR_ */

#define BG_FILE "inc/surface.png"
//#define HELP_URL "https://github.com/sjaehn/BSchaffl/blob/master/README.md"
//#define YT_URL " https://youtu.be/PuzoxiAs-h8"
//#define OPEN_CMD "xdg-open"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))
#define RESIZE(widget, x, y, w, h, sz) (widget).moveTo ((x) * (sz), (y) * (sz)); (widget).resize ((w) * (sz), (h) * (sz));

const std::string messageStrings[MAX_MSG + 1] =
{
	"",
	"*** Jack transport off or halted. ***"
};

class BSchafflGUI : public BWidgets::Window
{
public:
	BSchafflGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow);
	~BSchafflGUI ();
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	virtual void onConfigureRequest (BEvents::ExposeEvent* event) override;
	void applyTheme (BStyles::Theme& theme) override;
        void sendUiStatus (const bool on);

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
	void resizeGUI ();
        void setMarker (const int markerNr, double value);
        void setAutoMarkers ();
	void rearrange_controllers ();
        void redrawSContainer ();
	static void valueChangedCallback (BEvents::Event* event);
        static void markerClickedCallback (BEvents::Event* event);
	static void markerDraggedCallback (BEvents::Event* event);
        static void listBoxChangedCallback (BEvents::Event* event);
        static void markersAutoClickedCallback (BEvents::Event* event);
        //static void helpButtonClickedCallback (BEvents::Event* event);
	//static void ytButtonClickedCallback (BEvents::Event* event);

        std::string pluginPath;

	BWidgets::Widget mContainer;
        BWidgets::DrawingSurface sContainer;
        //HaloButton helpButton;
        //HaloButton ytButton;
	BWidgets::PopupListBox seqLenValueListbox;
        BWidgets::PopupListBox seqLenBaseListbox;
        SwingHSlider swingControl;
        BWidgets::TextButton markersAutoButton;
	BWidgets::HSliderValue nrStepsControl;
	std::array<BWidgets::VSliderValue, MAXSTEPS> stepControl;
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

	double sz;
	cairo_surface_t* bgImageSurface;

	LV2_Atom_Forge forge;
	BSchafflURIs uris;
	LV2_URID_Map* map;

	// Definition of styles
	BColors::ColorSet fgColors = {{{0.75, 0.75, 0.75, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.2, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
        BColors::ColorSet rdColors = {{{0.75, 0.0, 0.0, 1.0}, {1.0, 0.25, 0.25, 1.0}, {0.2, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{BColors::lightgrey, BColors::white, BColors::darkgrey, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};

	BStyles::Border border = {{BColors::darkgrey, 1.0}, 0.0, 2.0, 0.0};
        BStyles::Border actborder = {{BColors::white, 2.0}, 0.0, 1.0, 0.0};
        BStyles::Border stepborder = {{BColors::darkgrey, 1.0}, 0.0, 2.0, 3.0};
	BStyles::Border blindborder = {{{0.0, 0.0, 0.0, 0.0}, 1.0}, 0.0, 2.0, 0.0};
        BStyles::Border labelBorder = BStyles::Border (BStyles::noLine, 0.0, 4.0);
        BStyles::Border focusborder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.5), 2.0));
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.75));
        BStyles::Fill actBg = BStyles::Fill (BColors::lightgrey);
	BStyles::Border screenBorder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.75), 4.0));
	BStyles::Font defaultFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
        BStyles::Font smFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 8.0,
                                              BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);

        BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
						       {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
						    {"border", STYLEPTR (&BStyles::noBorder)},
						    {"textcolors", STYLEPTR (&txColors)},
						    {"font", STYLEPTR (&defaultFont)}}};
        BStyles::StyleSet smStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                                 {"textcolors", STYLEPTR (&txColors)},
                                                 {"font", STYLEPTR (&smFont)}}};

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
		{"mmonitor", 	{{"background", STYLEPTR (&BStyles::blackFill)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
 		{"smonitor", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&border)}}},
 		{"nbutton", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&border)}}},
 		{"abutton", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&actborder)}}},
		{"rcontainer", 	{{"background", STYLEPTR (&BStyles::noFill)},
				 {"border", STYLEPTR (&border)}}},
		{"scontainer", 	{{"background", STYLEPTR (&BStyles::noFill)},
				 {"border", STYLEPTR (&border)}}},
                {"listbox",	{{"border", STYLEPTR (&border)},
 				 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"listbox/item",{{"uses", STYLEPTR (&defaultStyles)},
 				 {"border", STYLEPTR (&labelBorder)},
 				 {"textcolors", STYLEPTR (&BColors::whites)},
 				 {"font", STYLEPTR (&defaultFont)}}},
 		{"listbox/button",{{"border", STYLEPTR (&BColors::darks)},
 				 {"background", STYLEPTR (&BStyles::blackFill)},
 			 	 {"bgcolors", STYLEPTR (&BColors::darks)}}},
		{"dial", 	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"redbutton", 	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&rdColors)},
				 {"bgcolors", STYLEPTR (&bgColors)}}},
                {"halobutton", 	{{"uses", STYLEPTR (&defaultStyles)},
 				 {"fgcolors", STYLEPTR (&bgColors)}}},
 		{"halobutton/focus", {{"uses", STYLEPTR (&focusStyles)}}},
		{"dial/focus", 	{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"slider",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"slider/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"switch",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)}}},
		{"switch/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"label",	{{"uses", STYLEPTR (&labelStyles)}}},
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
