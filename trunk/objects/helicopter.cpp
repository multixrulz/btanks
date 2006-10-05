#include "object.h"
#include "alarm.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "mrt/logger.h"
#include "resource_manager.h"
#include "config.h"
#include "world.h"

class Helicopter : public Object {
public:
	Helicopter(const std::string &para) :
		 Object("helicopter"), _active(false), _spawn(true), _paratrooper(para) {}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_next_target.serialize(s);
		s.add(_active);
		s.add(_paratrooper);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_next_target.deserialize(s);
		s.get(_active);
		s.get(_paratrooper);
	}

private: 
	v3<float> _next_target;
	bool _active;
	Alarm _spawn;
	std::string _paratrooper;
};

void Helicopter::onSpawn() {
	play("main", true);
	GET_CONFIG_VALUE("objects.helicopter-with-kamikazes.spawn-rate", float, sr, 1.5);
	_spawn.set(sr);
}

void Helicopter::tick(const float dt) {
	Object::tick(dt);
	if (_active && _spawn.tick(dt)) {
		Matrix<int> matrix; 
		World->getImpassabilityMatrix(matrix, this, NULL);

		v3<int> pos, pos2;
		getCenterPosition(pos); 
		pos -= IMap::pathfinding_step / 2;
		
		pos2 = pos;
		pos2 += v3<int>(IMap::pathfinding_step - 1, IMap::pathfinding_step - 1, 0);

		pos /= IMap::pathfinding_step;
		pos2 /= IMap::pathfinding_step;
		/*
		LOG_DEBUG(("%d %d", matrix.get(pos.y, pos.x), matrix.get(pos.y, pos2.x)));
		LOG_DEBUG(("%d %d", matrix.get(pos2.y, pos.x), matrix.get(pos2.y, pos2.x)));
		*/
		if (matrix.get(pos.y, pos.x) == -1 || matrix.get(pos.y, pos2.x) == -1 || 
			matrix.get(pos2.y, pos.x) == -1 || matrix.get(pos2.y, pos2.x) == -1) {
				LOG_DEBUG(("cannot drop paratrooper, sir!"));
			} else 
				spawn(_paratrooper, "paratrooper", v3<float>(0,0,-1), v3<float>());
	}
}


void Helicopter::calculate(const float dt) {
	GET_CONFIG_VALUE("objects.helicopter-with-kamikazes.delay-before-next-target", float, delay, 1.0);
	if (!_active && _idle_time > delay) { 
		v3<float> pos;
		getPosition(pos);

		v3<int> size = Map->getSize();
		_next_target.x = mrt::random(size.x);
		_next_target.y = mrt::random(size.y);
		//LOG_DEBUG(("picking up random target: %g %g", _next_target.x, _next_target.x));
		_active = true;
	}
	if (_active) {
		v3<float> pos;
		getPosition(pos);
		if (pos.quick_distance(_next_target) <= 10000) {
			_active = false; 
			_velocity.clear();
		} else 
			_velocity = _next_target - pos;
	}
	
	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, 8, rt, true, false);
}

void Helicopter::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* Helicopter::clone() const  {
	return new Helicopter(*this);
}

REGISTER_OBJECT("helicopter-with-kamikazes", Helicopter, ("paratrooper-kamikaze"));
