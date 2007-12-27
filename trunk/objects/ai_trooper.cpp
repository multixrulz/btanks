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

#include "trooper.h"
#include "ai/base.h"
#include "ai/herd.h"
#include "ai/old_school.h"
#include "config.h"
#include "registrar.h"
#include "mrt/random.h"
#include "special_owners.h"

class AITrooper : public Trooper, private ai::Herd, private ai::Base, ai::OldSchool {
public:
	AITrooper(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true), _target_dir(-1) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("fighting-vehicle");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");
			_targets.insert("helicopter");
			_targets.insert("monster");
			_targets.insert("watchtower");
	}
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		ai::Base::serialize(s);
		ai::OldSchool::serialize(s);
		s.add(_reaction);
		s.add(_target_dir);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		ai::Base::deserialize(s);
		ai::OldSchool::deserialize(s);
		s.get(_reaction);
		s.get(_target_dir);
	}
	virtual void calculate(const float dt);
	virtual Object* clone() const;

	virtual void onIdle(const float dt);
	
private: 
	const bool validateFire(const int idx) {
		if (idx == 0) 
			return canFire();
		return true;
	}
	
	virtual const int getComfortDistance(const Object *other) const;

	Alarm _reaction;
	int _target_dir;
	
	//no need for serialize it:
	std::set<std::string> _targets;
};

const int AITrooper::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.ai-trooper.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper" || other->classname == "kamikaze")?cd:-1;
}

#include "world.h"

void AITrooper::onIdle(const float dt) {
	int summoner = getSummoner();
	if (_variants.has("old-school")) {
		ai::OldSchool::calculateV(_velocity, this);
	} else if ((summoner != 0 && summoner != OWNER_MAP) || _variants.has("herd")) {
		Object *parent = World->getObjectByID(summoner);
		if (parent != NULL) {
			v2<float> dpos = getRelativePosition(parent);
			float dist = dpos.length();
			//LOG_DEBUG(("%d: %s: summoner distance: %g", getID(), animation.c_str(), dist));
			if (dist > 800) {
				//teleportation! 
				LOG_DEBUG(("%d: %s: teleports from distance: %g", getID(), animation.c_str(), dist));
				v2<float> dir;
				dir.fromDirection(getID() % 16, 16);
				dir *= (parent->size.x + parent->size.x) / 3;

				Object *new_me = parent->spawn(registered_name, animation, dir, v2<float>());

				new_me->updateVariants(getVariants(), true);
				new_me->copyOwners(this);
				new_me->hp = hp;
				new_me->addEffect("teleportation", 1);

				Object::emit("death", NULL);
				return;
			}
			
		}
		float range = getWeaponRange(_object);
		ai::Herd::calculateV(_velocity, this, summoner, range);
	} else {
		_velocity.clear();
	}
	_state.fire = false;

	GET_CONFIG_VALUE("objects.ai-trooper.rotation-time", float, rt, 0.05);
	calculateWayVelocity();
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();	
}

void AITrooper::onSpawn() {
	ai::Base::onSpawn(this);
	ai::OldSchool::onSpawn(this);
	GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.15f);
	mrt::randomize(rt, rt / 10);
	//LOG_DEBUG(("rt = %g", rt));
	_reaction.set(rt);	
	Trooper::onSpawn();
}

Object* AITrooper::clone() const  {
	return new AITrooper(*this);
}


void AITrooper::calculate(const float dt) {
	//calculateWayVelocity();
	//LOG_DEBUG(("calculate"));
	if (_target_dir != -1 && isEffectActive("panic")) {
		//LOG_DEBUG(("panic: %d", _target_dir));
		_velocity.fromDirection(_target_dir, getDirectionsNumber());
	
		GET_CONFIG_VALUE("objects.ai-trooper.rotation-time", float, rt, 0.05f);
		limitRotation(dt, rt, true, false);
		updateStateFromVelocity();
		return;
	}
	
	if (!_reaction.tick(dt) || isDriven()) {
		calculateWayVelocity();
		return;
	}

	{
		static std::set<std::string> bullets; 
		if (bullets.empty()) {
			bullets.insert("bullet");
			bullets.insert("missile");
		}
		//checking for a bullets 
		v2<float> pos, vel;
		float r = speed * 5.0f; 

		if (getNearest(bullets, r, pos, vel, false)) {
			float ct = getCollisionTime(pos, vel, 16);
			//LOG_DEBUG(("bullet at %g %g, est: %g", pos.x, pos.y, ct));
			if (ct > 0 && ct > 0.15f) {
				v2<float> dpos = -(pos + vel * ct);
				//LOG_DEBUG(("AAAAAAA!!"));
				dpos.normalize();
				int dirs = getDirectionsNumber(), escape = dpos.getDirection(dirs) - 1;
				if (escape >= 0) {
					_target_dir = escape;
					setDirection(escape);
					_velocity.fromDirection(_target_dir, getDirectionsNumber());
					_direction.fromDirection(_target_dir, getDirectionsNumber());
					addEffect("panic", ct);
					return;
				}
			}
		}
	}

	if (getState() == "fire") {
		_state.fire = true; //just to be sure.
		return;
	}
	
	_state.fire = false;
	
	float range = getWeaponRange(_object);

	_target_dir = getTargetPosition(_velocity, _targets, range);
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		if (_velocity.length() >= 9) {
			quantizeVelocity();
			_direction.fromDirection(getDirection(), getDirectionsNumber());
		} else {
			_velocity.clear();
			setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, getDirectionsNumber());
			_state.fire = true;
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
}
//==============================================================================
class TrooperInWatchTower : public Trooper, private ai::Base {
public: 
	TrooperInWatchTower(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("fighting-vehicle");
			_targets.insert("monster");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");		
			_targets.insert("helicopter");
	}
	virtual Object * clone() const { return new TrooperInWatchTower(*this); }
	
	virtual void onSpawn() { 
		ai::Base::onSpawn(this);
	
		GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.15f);
		mrt::randomize(rt, rt/10);
		_reaction.set(rt);
	
		Trooper::onSpawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		ai::Base::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		ai::Base::deserialize(s);
		s.get(_reaction);
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		//LOG_DEBUG(("range = %g", range));

		_state.fire = false;

		const Object * result = NULL;
		float dist = -1;
		
		std::set<const Object *> objects;
		enumerateObjects(objects, range, &_targets);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *target = *i;
			if (hasSameOwner(target) || target->aiDisabled())
				continue;
			
			v2<float> dpos = getRelativePosition(target);
			if (checkDistance(getCenterPosition(), target->getCenterPosition(), getZ(), true)) {
				if (result == NULL || dpos.quick_length() < dist) {
					result = target;
					dist = dpos.quick_length();
				}
			}
		}
		
		if (result != NULL) {
			_state.fire = true;
			_direction = getRelativePosition(result);
			_direction.normalize();
			setDirection(_direction.getDirection(getDirectionsNumber()) - 1);
		}
	}
private: 
	const bool validateFire(const int idx) {
		if (idx == 0) 
			return canFire();
		return true;
	}
	
	Alarm _reaction; 

	//no need to serialize it
	std::set<std::string> _targets;
};

REGISTER_OBJECT("machinegunner", AITrooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", AITrooper, ("thrower-missile", false));

REGISTER_OBJECT("machinegunner-in-watchtower", TrooperInWatchTower, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower-in-watchtower", TrooperInWatchTower, ("thrower-missile", false));