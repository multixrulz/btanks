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

#include "object.h"
#include "resource_manager.h"

class DestructableObject : public Object {
public:
	DestructableObject() : 
		Object("destructable-object"), _broken(false) {}

	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void addDamage(BaseObject *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_broken);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_broken);
	}

private:
	std::string _pose;
	bool _broken;
};

void DestructableObject::addDamage(BaseObject *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	BaseObject::addDamage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
	}
}

void DestructableObject::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}

void DestructableObject::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void DestructableObject::onSpawn() {
	play("main", true);
}


Object* DestructableObject::clone() const  {
	return new DestructableObject(*this);
}

REGISTER_OBJECT("destructable-object", DestructableObject, ());
