#ifndef ___SDLX_TTF_H__
#define ___SDLX_TTF_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "SDL/SDL_ttf.h"
#include <string>
#include "export_sdlx.h"

namespace sdlx {
class Surface;
class SDLXAPI TTF {
public: 
	static void init();
	
	TTF();
	~TTF();
	
	void open(const std::string &fname, const int psize);
	void renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg);
	void close();
private:
	TTF(const TTF &);
	TTF & operator=(const TTF &);
	
	TTF_Font * _font;
};

}

#endif
