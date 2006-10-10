
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

class Bullet : public Object {
public:
	Bullet(const std::string &type) : Object("bullet"), _type(type) {
		impassability = 1;
		piercing = true;
	}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
	}

private: 
	std::string _type;
};


void Bullet::calculate(const float dt) {}

void Bullet::onSpawn() {
	play("shot", false);
	play("move", true);
	_velocity.normalize();
	
	int dir = _velocity.getDirection8();
	if (dir) {
		setDirection(dir - 1);
	}
	_velocity.quantize8();
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision" || event == "death") {
		v3<float> dpos;
		if (emitter)
			dpos = getRelativePos(emitter) / 2;
			dpos.z = 0;
		if (_type == "regular") {
			spawn("explosion", "explosion", dpos);
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos);
		}
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	Object *a = new Bullet(*this);
	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ("regular"));
REGISTER_OBJECT("dirt-bullet", Bullet, ("dirt"));
