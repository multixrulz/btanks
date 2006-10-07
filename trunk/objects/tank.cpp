#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "tank.h"
#include "config.h"

REGISTER_OBJECT("tank", Tank, ());

Tank::Tank() 
: Object("player"), _fire(false) {
}

Tank::Tank(const std::string &animation) 
: Object("player"), _fire(false) {
	setup(animation);
}


void Tank::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>::empty, Centered);
	_smoke->impassability = 0;

	Object *_missiles = spawnGrouped("missiles-on-tank", "guided-missiles-on-tank", v3<float>::empty, Centered);
	_missiles->impassability = 0;

	add("missiles", _missiles);
	add("smoke", _smoke);
	
	GET_CONFIG_VALUE("objects.tank.fire-rate", float, fr, 0.5);
	_fire.set(fr);
}

Object * Tank::clone() const {
	return new Tank(*this);
}

void Tank::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}

const bool Tank::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects") {
		addEffect(type);
		return true;
	}
	if (get("missiles")->take(obj, type))
		return true;
	return BaseObject::take(obj, type);
}

void Tank::calculate(const float dt) {
	BaseObject::calculate(dt);
	GET_CONFIG_VALUE("objects.tank.rotation-time", float, rt, 0.05);
	limitRotation(dt, 8, rt, true, false);
}

void Tank::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
		groupEmit("missiles", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
			groupEmit("missiles", "move");
		}
	}


	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		//v3<float> v = _velocity.is0()?_direction:_velocity;
		//v.normalize();
		std::string bullet("bullet");
		if (isEffectActive("dirt")) {
			bullet = "dirt-bullet";
		}
		spawn(bullet, bullet, v3<float>::empty, _direction);
	}
	if (_state.alt_fire) {
		groupEmit("missiles", "launch");
	}
	
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

void Tank::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}
void Tank::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
}
