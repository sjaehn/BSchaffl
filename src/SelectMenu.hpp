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

#ifndef SELECTMENU_HPP_
#define SELECTMENU_HPP_

#include "BWidgets/Widget.hpp"
#include <list>
#include <utility>
#include "UpDownButton.hpp"

class SelectMenu : public BWidgets::Widget
{
public:
        SelectMenu () : SelectMenu (0, 0, 0, 0, "widget", std::list<std::pair<Widget*, Widget*>>{}) {}

        SelectMenu (const double x, const double y, const double width, const double height, const std::string& name,
                    const std::list<std::pair<Widget*, Widget*>>& menuList) :
                Widget (x, y, width, height, name),
                menuList_ (),
                activeItem_ (nullptr)
        {
                for (std::pair<Widget*, Widget*> const& p : menuList)
                {
                        if (p.first && p.second)
                        {
                                menuList_.push_back
                                (       MenuItem
                                        {
                                                UpDownButton (0, 0, p.first->getHeight(), p.first->getHeight(), name),
                                                p.first,
                                                p.second
                                        }
                                );
                                add (menuList_.back().button);
                                add (*p.first);
                                add (*p.second);
                                menuList_.back().button.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, SelectMenu::buttonPressedCallback);
                                menuList_.back().title->setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, SelectMenu::buttonPressedCallback);
                        }
                }

                activate (nullptr);
        }

        virtual void update () override
        {
                Widget::update();
                activate (activeItem_);
        }

        virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}

        virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
        {
        	Widget::applyTheme (theme, name);
                for (MenuItem& mi : menuList_) mi.button.applyTheme (theme, name);
        }

protected:

        struct MenuItem
        {
                UpDownButton button;
                Widget* title;
                Widget* content;
        };

        void activate (MenuItem* newActiveItem)
        {
                double y = 0.0;
                activeItem_ = newActiveItem;

                for (MenuItem& mi : menuList_)
                {
                        if (mi.title && mi.content)
                        {
                                mi.button.moveTo (0, y);
                                mi.button.setValue (1.0);
                                mi.title->moveTo (1.25 * mi.title->getHeight(), y);
                                y += mi.title->getHeight() + 10.0;

                                if (newActiveItem == &mi)
                                {
                                        mi.content->moveTo (0, y - 10.0);
                                        y += mi.content->getHeight();
                                        mi.content->raiseToTop();
                                        mi.content->show();
                                }

                                else
                                {
                                        mi.button.setValue (0.0);
                                        mi.content->hide();
                                }
                        }
                }
        }

        static void buttonPressedCallback (BEvents::Event* event)
        {
                if (!event) return;
        	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
        	if (pev->getButton() != BDevices::LEFT_BUTTON) return;
        	Widget* w = event->getWidget();
        	if (!w) return;
        	SelectMenu* s = (SelectMenu*)w->getParent();
        	if (!s) return;

                for (MenuItem& mi : s->menuList_)
                {
                        if (mi.title && mi.content && ((mi.title == w) || (&mi.button == w)))
                        {
                                if (s->activeItem_ != &mi) s->activate (&mi);
                                else s->activate (nullptr);
                                break;
                        }
                }
        }

        std::list<MenuItem> menuList_;
        MenuItem* activeItem_;
};

#endif /* SELECTMENU_HPP_ */
