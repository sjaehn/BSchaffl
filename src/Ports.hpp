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

#ifndef PORTS_HPP_
#define PORTS_HPP_

enum BSchafflPortIndex
{
	INPUT		= 0,
	OUTPUT		= 1,

	CONTROLLERS	= 2,
	SEQ_LEN_VALUE	= 0,
	SEQ_LEN_BASE	= 1,
	SWING		= 2,
	NR_OF_STEPS	= 3,
	QUANT_RANGE	= 4,
	QUANT_MAP	= 5,
	QUANT_POS	= 6,
	USR_LATENCY	= 7,
	USR_LATENCY_FR	= 8,
	STEP_POS	= 9,
	STEP_LEV	= 24,
	LATENCY		= 40,
	NR_CONTROLLERS	= 41
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
