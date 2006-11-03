#ifndef __BTANKS_CREDITS_H__
#define __BTANKS_CREDITS_H__

/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "math/v3.h"

class Credits {
public:
	Credits();
	void render(const float dt, sdlx::Surface &surface);
	~Credits();
private: 
	unsigned int _w, _h;
	sdlx::Font _font, _medium_font;
	sdlx::Surface _surface;
	v3<float> _position;
	v3<float> _velocity;
};


#endif

