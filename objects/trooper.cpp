
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

#include "object.h"
#include "resource_manager.h"
#include "alarm.h"
#include "config.h"
#include "trooper.h"

void Trooper::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability > 0.2f) {
		base_value = 0.2f;
		base = 0;
		penalty = 0;
	}
}

#include "player_manager.h"

void Trooper::tick(const float dt) {
	setDirection(_velocity.getDirection8() - 1);
	Object::tick(dt);
	
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold" && state != "fire") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (state == "hold") {
			cancelAll();
			play("run", true);
		}		
	}
	
	if (!_object.empty() && _fire.tick(dt) && _state.fire && !_variants.has("nukeman")) {
		_fire.reset();
		if (disable_ai || validateFire(0)) {
			if (getState() != "fire")
				playNow("fire");
			spawn(_object, _object, v2<float>(), _direction);
		}
	}
	if (_state.alt_fire && _variants.has("nukeman")) {
		//MUGGAGAGAGAGAG!!!
		Object *o = spawn("nuke-explosion", "nuke-explosion");
		emit("death", o); //fix frags calculation to remove this player manager call: 
		PlayerManager->onPlayerDeath(this, o);
	}
}

const bool Trooper::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "missiles" && type == "nuke" && _variants.has("player")) {
		_variants.add("nukeman");
		hp = max_hp = 999;
		init("nukeman");
		need_sync = true;
		return true;
	}
	return false;
}

void Trooper::onSpawn() {
	if (_variants.has("player")) {
		speed *= 1.75f;
		hp = max_hp *= 2;
	}
	
	if (_variants.has("disembark")) {
		playSound("disembark", false);
	}
	if (_object.empty()) {
		//nothing to do
	} else if (_object == "thrower-missile") {
		GET_CONFIG_VALUE("objects.thrower.fire-rate", float, fr, 3);
		_fire.set(fr);
	} else if (_object == "machinegunner-bullet") {
		GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
		_fire.set(fr);
	} else throw_ex(("unsupported weapon %s", _object.c_str()));
	
	play("hold", true);
	_pose = "run";
}

void Trooper::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse(human-death)", "dead-" + animation, v2<float>(), v2<float>());
	} else if (event == "collision" && emitter != NULL && emitter->classname == "vehicle" && !_variants.has("nukeman")) {
		if (_velocity.same_sign(getRelativePosition(emitter)) &&
			attachVehicle(emitter))
			return;
	}
	Object::emit(event, emitter);
}

const bool Trooper::validateFire(const int idx) {
	return true;
}

Object* Trooper::clone() const  {
	return new Trooper(*this);
}
