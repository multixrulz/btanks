#ifndef BTANKS_CONTROL_PICKER_H__
#define BTANKS_CONTROL_PICKER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "container.h"
#include <string>
#include "export_btanks.h"

class Chooser;
class ControlPicker : public Container {
public: 
	ControlPicker(const int w, const std::string &font, const std::string &label, const std::string &config_key, const std::string &def, const std::string &variant);
	void save(); 
	void reload();
private: 
	std::string _config_key, _default;
	std::vector<std::string> _values;
	Chooser *_controls;
};

#endif

