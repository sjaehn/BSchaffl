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
	AMP_SWING	= 2,
	AMP_RANDOM	= 3,
	SWING		= 4,
	SWING_RANDOM	= 5,
	NR_OF_STEPS	= 6,
	MIDI_CH_FILTER	= 7,
	MSG_FILTER_NOTE	= 23,
	MSG_FILTER_KEYPR= 24,
	MSG_FILTER_CC	= 25,
	MSG_FILTER_PROG	= 26,
	MSG_FILTER_CHPR	= 27,
	MSG_FILTER_PITCH= 28,
	MSG_FILTER_SYS	= 29,
	QUANT_RANGE	= 30,
	QUANT_MAP	= 31,
	QUANT_POS	= 32,
	TIME_COMPENS	= 33,
	USR_LATENCY	= 34,
	USR_LATENCY_FR	= 35,
	STEP_POS	= 36,
	STEP_LEV	= 51,
	LATENCY		= 67,
	NR_CONTROLLERS	= 68
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
