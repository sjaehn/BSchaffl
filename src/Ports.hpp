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
	INPUT			= 0,
	OUTPUT			= 1,

	CONTROLLERS		= 2,
	SEQ_LEN_VALUE		= 0,
	SEQ_LEN_BASE		= 1,
	AMP_SWING		= 2,
	AMP_RANDOM		= 3,
	AMP_PROCESS		= 4,
	SWING			= 5,
	SWING_RANDOM		= 6,
	SWING_PROCESS		= 7,
	NR_OF_STEPS		= 8,
	AMP_MODE		= 9,
	MIDI_CH_FILTER		= 10,
	MSG_FILTER_NOTE		= 26,
	MSG_FILTER_KEYPR	= 27,
	MSG_FILTER_CC		= 28,
	MSG_FILTER_PROG		= 29,
	MSG_FILTER_CHPR		= 30,
	MSG_FILTER_PITCH	= 31,
	MSG_FILTER_SYS		= 32,
	NOTE_POSITION_STR	= 33,
	NOTE_VALUE_STR		= 34,
	NOTE_OVERLAP		= 35,
	NOTE_OFF_AMP		= 36,
	QUANT_RANGE		= 37,
	QUANT_MAP		= 38,
	QUANT_POS		= 39,
	TIME_COMPENS		= 40,
	USR_LATENCY		= 41,
	USR_LATENCY_FR		= 42,
	STEP_POS		= 43,
	STEP_LEV		= 58,
	LATENCY			= 74,
	NR_CONTROLLERS		= 75
};

enum BSchafflSeqLenBaseIndex
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
};

#endif /* PORTS_HPP_ */
