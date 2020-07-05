/* B.Shapr
 * Beat / envelope shaper LV2 plugin
 *
 * Copyright (C) 2019 by Sven Jähnichen
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

#ifndef SNAPSHOTS_HPP_
#define SNAPSHOTS_HPP_

#include <cstddef>
#include <array>
#include <iostream>

template <class T, size_t sz>
class Snapshots
{
protected:
        std::array<T, sz> store_;
        size_t pos_ = 0;
        size_t horizon_ = 0;
        size_t size_ = 0;
        T default_ = T ();

public:
        void clear ()
        {
                store_.fill (default_);
                pos_ = 0;
                horizon_ = 0;
                size_ = 0;
        }

        void setDefault (const T& value)
        {
                default_ = value;
        }

        void push (const T& value)
        {
                horizon_ = ((pos_ + 1) % sz);
                store_[horizon_] = value;
                pos_ = horizon_;
                size_ = (size_ < sz ? size_ + 1 : sz);
        }

        T undo ()
        {
                pos_ = (size_ == 0 ? 0 : (((size_ < sz) && (pos_ == 0)) ? 0 : ((horizon_ == ((pos_ + sz - 1) % sz)) ? pos_ : ((pos_ + sz - 1) % sz))));
                return store_[pos_];
        }

        T redo ()
        {
                pos_ = (size_ == 0 ? 0 : ((horizon_ == pos_) ? pos_ : ((pos_ + 1) % sz)));
                return store_[pos_];
        }
};


#endif /* SNAPSHOTS_HPP_ */
