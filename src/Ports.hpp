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
	MIDI_CH_FILTER	= 4,
	MSG_FILTER_NOTE	= 20,
	MSG_FILTER_KEYPR= 21,
	MSG_FILTER_CC	= 22,
	MSG_FILTER_PROG	= 23,
	MSG_FILTER_CHPR	= 24,
	MSG_FILTER_PITCH= 25,
	MSG_FILTER_SYS	= 26,
	QUANT_RANGE	= 27,
	QUANT_MAP	= 28,
	QUANT_POS	= 29,
	USR_LATENCY	= 30,
	USR_LATENCY_FR	= 31,
	STEP_POS	= 32,
	STEP_LEV	= 47,
	LATENCY		= 63,
	NR_CONTROLLERS	= 64
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
