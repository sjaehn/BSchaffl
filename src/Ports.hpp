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
	MSG_FILTER_NOTE	= 4,
	MSG_FILTER_KEYPR= 5,
	MSG_FILTER_CC	= 6,
	MSG_FILTER_PROG	= 7,
	MSG_FILTER_CHPR	= 8,
	MSG_FILTER_PITCH= 9,
	MSG_FILTER_SYS	= 10,
	QUANT_RANGE	= 11,
	QUANT_MAP	= 12,
	QUANT_POS	= 13,
	USR_LATENCY	= 14,
	USR_LATENCY_FR	= 15,
	STEP_POS	= 16,
	STEP_LEV	= 31,
	LATENCY		= 47,
	NR_CONTROLLERS	= 48
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
