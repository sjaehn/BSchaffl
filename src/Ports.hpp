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
	AMP_PROCESS	= 4,
	SWING		= 5,
	SWING_RANDOM	= 6,
	SWING_PROCESS	= 7,
	NR_OF_STEPS	= 8,
	MIDI_CH_FILTER	= 9,
	MSG_FILTER_NOTE	= 25,
	MSG_FILTER_KEYPR= 26,
	MSG_FILTER_CC	= 27,
	MSG_FILTER_PROG	= 28,
	MSG_FILTER_CHPR	= 29,
	MSG_FILTER_PITCH= 30,
	MSG_FILTER_SYS	= 31,
	QUANT_RANGE	= 32,
	QUANT_MAP	= 33,
	QUANT_POS	= 34,
	TIME_COMPENS	= 35,
	USR_LATENCY	= 36,
	USR_LATENCY_FR	= 37,
	STEP_POS	= 38,
	STEP_LEV	= 53,
	LATENCY		= 69,
	NR_CONTROLLERS	= 70
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
