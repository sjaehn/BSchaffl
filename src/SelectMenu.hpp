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

class SelectMenu : public BWidgets::Widget
{
public:
        SelectMenu () : SelectMenu (0, 0, 0, 0, "widget", std::list<std::pair<Widget*, Widget*>>{}) {}

        SelectMenu (const double x, const double y, const double width, const double height, const std::string& name,
                    const std::list<std::pair<Widget*, Widget*>>& menuList) :
                Widget (x, y, width, height, name),
                menuList_ (),
                activePair_ (nullptr)
        {
                for (std::pair<Widget*, Widget*> const& p : menuList)
                {
                        if (p.first && p.second)
                        {
                                add (*p.first);
                                add (*p.second);
                                menuList_.push_back (p);
                                p.first->setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, SelectMenu::buttonPressedCallback);
                        }
                }

                activate (nullptr);
        }

        virtual void update () override
        {
                Widget::update();
                activate (activePair_);
        }

protected:

        void activate (std::pair<Widget*, Widget*>* newActivePair)
        {
                double y = 0.0;
                activePair_ = newActivePair;

                for (std::pair<Widget*, Widget*> const& p : menuList_)
                {
                        if (p.first && p.second)
                        {
                                p.first->moveTo (0, y);
                                y += p.first->getHeight() + 10.0;

                                if (newActivePair == &p)
                                {
                                        p.second->moveTo (0, y - 10.0);
                                        y += p.second->getHeight();
                                        p.second->show();
                                }

                                else p.second->hide();
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

                for (std::pair<Widget*, Widget*>& p : s->menuList_)
                {
                        if (p.first && p.second && (p.first == w))
                        {
                                if (s->activePair_ != &p) s->activate (&p);
                                else s->activate (nullptr);
                                break;
                        }
                }
        }

        std::list<std::pair<Widget*, Widget*>> menuList_;
        std::pair<Widget*, Widget*>* activePair_;
};

#endif /* SELECTMENU_HPP_ */
