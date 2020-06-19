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

#ifndef UPDOWNBUTTON_HPP_
#define UPDOWNBUTTON_HPP_

#include "BWidgets/ToggleButton.hpp"

class UpDownButton : public BWidgets::ToggleButton
{
public:
	UpDownButton () : UpDownButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}

	UpDownButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0) :
			ToggleButton (x, y, width, height, name, defaultValue) {}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override
	{
		ToggleButton::onButtonPressed (event);
		Widget::cbfunction_[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
	}

	virtual void onButtonReleased (BEvents::PointerEvent* event) override
	{
		ToggleButton::onButtonReleased (event);
		Widget::cbfunction_[BEvents::EventType::BUTTON_RELEASE_EVENT] (event);
	}

protected:
	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		if ((area.getWidth() >= 6) && (area.getHeight() >= 6))
		{
			// Draw super class widget elements first
			Widget::draw (area);

			cairo_t* cr = cairo_create (widgetSurface_);
			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX(), area.getY(), area.getWidth(), area.getHeight());
				cairo_clip (cr);

				double x0 = getXOffset ();
				double y0 = getYOffset ();
				double w = getEffectiveWidth ();
				double h = getEffectiveHeight ();
				BColors::Color butColor = *bgColors.getColor (value == 1 ? BColors::ACTIVE : BColors::NORMAL);

				if (value)
				{
					cairo_move_to (cr, x0 + 0.2 * w, y0 + 0.65 * h);
					cairo_line_to (cr, x0 + 0.5 * w, y0 + 0.35 * h);
					cairo_line_to (cr, x0 + 0.8 * w, y0 + 0.65 * h);
				}
				else
				{
					cairo_move_to (cr, x0 + 0.2 * w, y0 + 0.35 * h);
					cairo_line_to (cr, x0 + 0.5 * w, y0 + 0.65 * h);
					cairo_line_to (cr, x0 + 0.8 * w, y0 + 0.35 * h);
				}

				cairo_set_line_width (cr, 2);
				cairo_set_source_rgba (cr, CAIRO_RGBA (butColor));
				cairo_stroke (cr);

				cairo_destroy (cr);
			}
		}
	}
};

#endif /* UPDOWNBUTTON_HPP_ */
