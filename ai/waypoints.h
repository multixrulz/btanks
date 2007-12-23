#ifndef BTANKS_AI_WAYPOINTS_H__
#define BTANKS_AI_WAYPOINTS_H__

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

#include <string>
//#include <set>
#include "alarm.h"
#include "export_btanks.h"
#include "old_school.h"

class Object;
namespace mrt {
	class Serializator;
}

namespace ai {

class BTANKSAPI Waypoints : private ai::OldSchool {
public: 
	Waypoints();
	virtual void onSpawn(const Object *object);
	void calculate(Object *object, const float dt);

	virtual void onObstacle(const Object *o) = 0;

	virtual ~Waypoints() {}

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

protected:
	//std::set<std::string> obstacle_filter;
	bool _no_waypoints;
	bool _avoid_obstacles;
	bool _stop_on_obstacle;
	
private: 
	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
};

}

#endif
