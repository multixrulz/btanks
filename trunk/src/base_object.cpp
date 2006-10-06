#include "base_object.h"
#include "mrt/logger.h"
#include "world.h"

BaseObject::BaseObject(const std::string &classname)
 : mass(1), speed(0), ttl(-1), impassability(1), hp(1), max_hp(1), piercing(false), pierceable(false),
   classname(classname), _id(0), _follow(0), _direction(1,0,0),  _moving_time(0), _idle_time(0), 
   need_sync(false),
   _dead(false), _owner_id(0) {
	//LOG_DEBUG(("allocated id %ld", _id));
}

void BaseObject::inheritParameters(const BaseObject *other) {
	mass = other->mass;
	speed = other->speed;
	ttl = other->ttl;
	impassability = other->impassability;
	hp = other->hp;
	max_hp = other->max_hp;
	piercing = other->piercing;
	pierceable = other->pierceable;
	size = other->size;
}


void BaseObject::serialize(mrt::Serializator &s) const {
	s.add(_id);
	
	size.serialize(s);
	s.add(mass);
	s.add(speed);
	s.add(ttl);
	s.add(impassability);
	s.add(hp);
	s.add(max_hp);
	s.add(piercing);
	s.add(pierceable);
	s.add(classname);
	
	s.add(_follow);
	_follow_position.serialize(s);
	_velocity.serialize(s);
	_direction.serialize(s);
	_velocity_fadeout.serialize(s);
	s.add(_moving_time);	
	s.add(_idle_time);	

	s.add(_dead);
	_position.serialize(s);
	s.add(_owner_id);
}

void BaseObject::deserialize(const mrt::Serializator &s) {
	s.get(_id);
	
	size.deserialize(s);
	s.get(mass);
	s.get(speed);
	s.get(ttl);
	s.get(impassability);
	s.get(hp);
	s.get(max_hp);
	s.get(piercing);
	s.get(pierceable);
	s.get(classname);
	
	s.get(_follow);
	_follow_position.deserialize(s);
	_velocity.deserialize(s);
	_direction.deserialize(s);
	_velocity_fadeout.deserialize(s);
	s.get(_moving_time);	
	s.get(_idle_time);	

	s.get(_dead);
	_position.deserialize(s);
	s.get(_owner_id);
}

const std::string BaseObject::dump() const {
	return mrt::formatString("object '%s', mass: %g, speed: %g, ttl: %g, impassability: %g, hp: %d, piercing: %s, pierceable: %s, z: %g, dead: %s",
		classname.c_str(), mass, speed, ttl, impassability, hp, piercing?"true":"false", pierceable?"true":"false", _position.z, _dead?"true":"false"
	);
}

BaseObject::~BaseObject() { _dead = true; }

void BaseObject::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		_velocity.clear();
		_dead = true;
	} else if (event == "collision") {
		addDamage(emitter);
	} else LOG_WARN(("%s[%d]: unhandled event '%s'", classname.c_str(), _id, event.c_str()));
}

const float BaseObject::getCollisionTime(const v3<float> &dpos, const v3<float> &vel, const float r) const {
	//v3<float> dpos = pos - _position;
	float a = vel.x * vel.x + vel.y * vel.y;
	if (a == 0)
		return -1;
	//LOG_DEBUG(("a = %g", a));
	float b = 2 * (vel.x * dpos.x + vel.y * dpos.y) ;
	float c = dpos.x * dpos.x + dpos.y * dpos.y - r * r;
	//LOG_DEBUG(("dpos: %g %g", dpos.x, dpos.y));
	//LOG_DEBUG(("b = %g, c = %g, r = %g", b, c, r));
	
	if (b/a > 0 && c/a > 0) //both t1,t2 < 0
		return -2;
	
	float d = b * b - 4 * a * c;
	if (d < 0) 
		return -3; //no solution

	d = sqrt(d);
	
	float t1 = (-b + d) / 2 / a;
	if (t1 > 0) 
		return t1;
		
	float t2 = (-b - d) / 2 / a;
	if (t2 > 0)
		return t2;
	
	return -4;
}

void BaseObject::convertToAbsolute(v3<float> &pos, const v3<float> &dpos) {
	pos = _position;
	pos += dpos;
}

void BaseObject::calculate(const float dt) {
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
}

/*

void BaseObject::pretick() {
	if (_stateless)
		return;

	state2velocity();
}

void BaseObject::posttick() {
	//AI player will be easier to implement if operating directly with velocity
	if (!_stateless || _velocity.is0()) 
		return;
	
	//LOG_DEBUG(("class: %s", classname.c_str()));
	_velocity.normalize();
		
	v3<float>::quantize(_velocity.x);	
	v3<float>::quantize(_velocity.y);
	//LOG_DEBUG(("%s: _velocity: %g %g", classname.c_str(), _velocity.x, _velocity.y));
		
	_state.left = _velocity.x == -1;
	_state.right = _velocity.x == 1;
	_state.up = _velocity.y == -1;
	_state.down = _velocity.y == 1;

	state2velocity();
}

void BaseObject::state2velocity() {
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
}
*/
void BaseObject::follow(const BaseObject *obj, const GroupType mode) {
	_follow = obj->_id;
	if (mode == Centered) {
		_follow_position = (obj->size - size) / 2;
		//LOG_DEBUG(("follow: %g %g", _follow_position.x, _follow_position.y));
	}
}

void BaseObject::follow(const int id) {
	_follow = id;
}


#include "resource_manager.h"
#include "object.h"

void BaseObject::addDamage(BaseObject *from, const bool emitDeath) {
	if (!from->piercing || hp == -1 || from->hp == 0)
		return;

	addDamage(from, from->hp, emitDeath);
}

void BaseObject::addDamage(BaseObject *from, const int dhp, const bool emitDeath) {
	need_sync = true;
	
	hp -= dhp;	
	LOG_DEBUG(("%s: received %d hp of damage from %s. hp = %d", classname.c_str(), dhp, from->classname.c_str(), hp));
	if (emitDeath && hp <= 0) 
		emit("death", from);
		
	//look for a better place for that.
	Object *o = ResourceManager->createObject("damage-digits", "damage-digits");
	o->hp = dhp;
	if (hp < 0) 
		o->hp += hp;
	World->addObject(o, _position);
	
}


void BaseObject::setZ(const float z) {
	_position.z = z;
}

const bool BaseObject::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "heal") {
		need_sync = true;
		hp += obj->hp;
		if (hp >= max_hp)
			hp = max_hp;
		LOG_DEBUG(("%s: got %d hp (heal). result: %d", classname.c_str(), obj->hp, hp));	
		return true;
	}
	//LOG_WARN(("%s: cannot take %s (%s)", classname.c_str(), obj->classname.c_str(), type.c_str()));
	return false;
}

void BaseObject::disown() {
	_owner_id = 0;
}

const v3<float> BaseObject::getRelativePos(const BaseObject *obj) const {
	return obj->_position - _position + size / 2 - obj->size / 2;
}

const bool BaseObject::updatePlayerState(const PlayerState &state) {
	bool updated = _state != state;
	if (updated) {
		_state = state;
		need_sync = true;
	}
	return updated;
}

void BaseObject::getInfo(v3<float> &pos, v3<float> &vel) const {
	pos = _position;
	vel = _velocity;
	
	vel.normalize();
}

void BaseObject::updateStateFromVelocity() {
	_state.left = (_velocity.x < 0);
	_state.right = (_velocity.x > 0);
	_state.up = (_velocity.y < 0);
	_state.down = (_velocity.y > 0);
}

#include "resource_manager.h"
#include "object.h"
#include "config.h"

void BaseObject::getTargetPosition8(v3<float> &position, const v3<float> &target, const std::string &weapon) {
	const Object *wp = ResourceManager->getClass(weapon);
	float range = wp->ttl * wp->speed;
	
	GET_CONFIG_VALUE("engine.targeting-multiplier", float, tm, 0.75);
	if (tm <= 0 || tm >= 1) 
		throw_ex(("targeting multiplier must be greater than 0 and less than 1.0 (%g)", tm))
	range *= tm;
	float d = (target - _position).length();
	if (d < range) 
		range = d;
	
	LOG_DEBUG(("searching suitable position (range: %g)", range));
	float distance = 0;
	
	for(int i = 0; i < 8; ++i) {
		v3<float> pos;
		pos.fromDirection(i, 8);
		pos *= range;
		pos += target;
		pos -= _position;
		float d = pos.quick_length();
		if (i == 0 || d < distance) {
			distance = d;
			position = pos;
		}
		LOG_DEBUG(("target position: %g %g, distance: %g", pos.x, pos.y, d));
	}
	position += _position;
}
