
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

#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "launcher.h"
#include "config.h"


Launcher::Launcher(const std::string &classname) 
: Object(classname), _fire(false) {
}

Object * Launcher::clone() const {
	return new Launcher(*this);
}

void Launcher::onSpawn() {
	if (registered_name.substr(0, 6) == "static")
		disown();

	Object *_smoke = spawnGrouped("single-pose", "launcher-smoke", v3<float>::empty, Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	add("smoke", _smoke);
	add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v3<float>::empty, Centered));
	add("alt-mod", spawnGrouped("alt-missiles-on-launcher", "guided-missiles-on-launcher", v3<float>(2,2,0), Centered));
	
	GET_CONFIG_VALUE("objects.launcher.fire-rate", float, fr, 0.3);
	_fire.set(fr);
}


void Launcher::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		World->detachVehicle(this);		
		
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;

		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


void Launcher::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limitRotation(dt, rt, true, false);
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Launcher::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
		groupEmit("mod", "hold");
	}

	if (_velocity.is0()) {	
		cancelRepeatable();
		play("hold", true);
		groupEmit("mod", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			//play("start", false);
			play("move", true);
			groupEmit("mod", "move");
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		groupEmit("mod", "launch");
	}
	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		groupEmit("alt-mod", "launch");
	}
}

const bool Launcher::take(const BaseObject *obj, const std::string &type) {
	if (BaseObject::take(obj, type)) 
		return true;

	if (obj->classname == "mod" && type == "machinegunner") {
		LOG_DEBUG(("taking mod: %s", type.c_str()));
		remove("mod");
		add("mod", spawnGrouped("machinegunner-on-launcher", "machinegunner-on-launcher", v3<float>::empty, Centered));
		return true;
	}
	const bool primary_mod = (obj->classname == "missiles" && (type != "smoke" && type != "stun" && type != "nuke"));
	if (primary_mod && get("mod")->classname != "missiles-on-vehicle") {
		LOG_DEBUG(("restoring default mod."));
		remove("mod");
		add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v3<float>::empty, Centered));
	}
	if (primary_mod) {
		if (get("mod")->take(obj, type))
			return true;
	} else {
		if (get("alt-mod")->take(obj, type))
			return true;		
	}
	
	return false;
}

void Launcher::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}

void Launcher::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
}

REGISTER_OBJECT("launcher", Launcher, ("player"));
REGISTER_OBJECT("static-launcher", Launcher, ("vehicle"));
