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

#include "alarm.h"
#include "resource_manager.h"
#include "config.h"
#include "object.h"
#include "item.h"
#include "ai/waypoints.h"

class Car : public Object, public ai::Waypoints {
public: 
	Car();

	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void emit(const std::string &event, Object * emitter);
	
	virtual void onObstacle(const int idx);
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		//ai::Waypoints::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		//ai::Waypoints::deserialize(s);
	}
private: 
	virtual void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;
};

void Car::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability >= 0.2) {
		base = 0.2;
		base_value = 0.8;
		return;
	}
}
/*
const int Car::getPenalty(const int map_im, const int obj_im) const {
	return (map_im >= 20 || obj_im >= 20)?5000:0;
}
*/

void Car::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>::empty, v2<float>::empty);
	} else if (event == "collision") {
		if (emitter != NULL && emitter->speed > 0) {
			Item * item = dynamic_cast<Item *>(emitter);
			//no items.
			if (item == NULL) {
				GET_CONFIG_VALUE("objects.car.damage", int, d, 5);
				emitter->addDamage(this, d);
				emitter->addEffect("stunned", 0.1f);
				emit("death", emitter);
			}
		}
	}
	Object::emit(event, emitter);
}

void Car::onObstacle(const int idx) {
	if ((idx % 21) == 1) { //approx once per 5 second
		playRandomSound("klaxon", false);
	}
}

void Car::onSpawn() {
	//obstacle_filter.insert("car");
	//obstacle_filter.insert("civilian");
	//obstacle_filter.insert("trooper");
	//obstacle_filter.insert("player");
	_avoid_obstacles = true;

	ai::Waypoints::onSpawn(this);
	//GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
	play("hold", true);
	
	disown(); 
}

Car::Car() : Object("car") {}

void Car::calculate(const float dt) {
	ai::Waypoints::calculate(this, dt);

	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.05f);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}


void Car::tick(const float dt) {
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
	}
}


Object * Car::clone() const {
	return new Car(*this);
}


REGISTER_OBJECT("car", Car, ());
