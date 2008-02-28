/*
 * blow_bottle_processor.h - sound generator for blown bottle noises
 *
 * Copyright (c) 2008 Danny McRae <khjklujn/at/users.sourceforge.net>
 * 
 * 
 * This file is part of Linux MultiMedia Studio - http://lmms.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */
#ifndef _BLOW_BOTTLE_PROCESSOR_H
#define _BLOW_BOTTLE_PROCESSOR_H

#include "BlowBotl.h"

#include "stk_processor.h"

#include "blow_bottle_model.h"

class blowBottleProcessor: public stkProcessor<blowBottleModel, BlowBotl>
{
public:
	blowBottleProcessor( sample_rate_t _sample_rate );
	~blowBottleProcessor( void );
	
	void setControls( blowBottleModel * _model );
};

#endif
