/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include "rt_config.h"
#include "mrt/serializator.h"

IMPLEMENT_SINGLETON(RTConfig, IRTConfig);

IRTConfig::IRTConfig() : server_mode(false), editor_mode(false), game_type(GameTypeDeathMatch), teams(0) {}

void IRTConfig::serialize(mrt::Serializator &s) const {
	s.add((int)game_type);
	s.add(teams);
}

#include "mrt/logger.h"

void IRTConfig::deserialize(const mrt::Serializator &s) {
	int t;
	s.get(t);
	LOG_DEBUG(("deserialized game type %d", t));
	game_type = (GameType)t;
	s.get(teams);
	LOG_DEBUG(("deserialized teams %d", teams));
}
