#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__

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
#include <map>
#include <set>
#include <queue>

#include "export_btanks.h"
#include "base_object.h"
#include "alarm.h"
#include "math/v2.h"
#include "math/matrix.h"
#include "object_common.h"

namespace sdlx {
	class Rect;
	class Surface;
	class CollisionMap;
}

class AnimationModel;
class Animation;
class Pose;

class BTANKSAPI Object : public BaseObject {
public:
	const std::string registered_name; 
	
	std::string animation;
	float fadeout_time;

	Object(const std::string &classname); //do not use parent - internal
	~Object();
	
	void init(const std::string &animation); //do not use it, needed for resman

	virtual Object * clone() const;
	
	void playSound(const std::string &name, const bool loop, const float gain = 1.0);
	void playRandomSound(const std::string &classname, const bool loop, const float gain = 1.0);

	inline const int getDirection() const { return _direction_idx; }
	inline const int getDirectionsNumber() const { return _directions_n; }
	virtual void setDirection(const int dir);
	void setDirectionsNumber(const int dirs);
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	const bool collides(const Object *other, const int x, const int y, const bool hidden_by_other = false) const;
	const bool collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other = false) const;

	// animation:
	void play(const std::string &id, const bool repeat = false);
	void playNow(const std::string &id);
	void cancel();
	void cancelRepeatable();
	void cancelAll();
	inline const std::string& getState() const {
		static const std::string empty;
		if (_events.empty())
			return empty;
		return _events.front().name;
	}
	const float getStateProgress() const;
	//effects
	void addEffect(const std::string &name, const float ttl = -1);
	inline const bool isEffectActive(const std::string &name) const {
		return _effects.find(name) != _effects.end();
	}
	const float getEffectTimer(const std::string &name) const;
	void removeEffect(const std::string &name);
	
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);
	void addDamage(Object *from, const bool emitDeath = true);

	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	void serializeAll(mrt::Serializator &s) const;
	
	virtual void onSpawn();
	
	inline const bool rotating() const { return _direction_idx != _dst_direction; }

	virtual void calculate(const float dt);
	
	virtual const std::string getType() const;
	virtual const int getCount() const;

	const Object *get(const std::string &name) const;
	const bool has(const std::string &name) const;

	const float getWeaponRange(const std::string &weapon) const;

	const int getTargetPosition(v2<float> &relative_position, const v2<float> &target, const std::string &weapon) const;
	const int getTargetPosition(v2<float> &relative_position, const v2<float> &target, const float range) const;
	const int getTargetPosition(v2<float> &relative_position, const std::set<std::string> &targets, const std::string &weapon) const;
	const int getTargetPosition(v2<float> &relative_position, const std::set<std::string> &targets, const float range) const;

	void quantizeVelocity();
	
	inline const Way& getWay() const { return _way; } 
	void setWay(const Way & way);
	const bool isDriven() const;
	
	const std::string getNearestWaypoint(const std::string &classname) const;

	void setZBox(const int z);

	virtual const bool detachVehicle();
	virtual const bool attachVehicle(Object *vehicle);

	const int getChildren(const std::string &classname) const;
	void getImpassabilityMatrix(Matrix<int> &matrix, const Object *dst) const;
	void enumerateObjects(std::set<const Object *> &o_set, const float range, const std::set<std::string> *classfilter) const;

	static const bool checkDistance(const v2<float> &map1, const v2<float>& map2, const int z, const bool use_pierceable_fixes);

	const bool aiDisabled() const;
	virtual const bool take(const BaseObject *obj, const std::string &type);

	Object * spawn(const std::string &classname, const std::string &animation, const v2<float> &dpos = v2<float>(), const v2<float> &vel = v2<float>(), const int z = 0);

	void invalidate() { setSync(true); }

protected:

	//pathfinding

	struct Point {
		inline Point() : id(), parent(), g(0), h(0), dir(-1) {}
		v2<int> id, parent;
		int g, h, dir;
	};
	
	struct PD {
		int f;
		v2<int> id;
		inline PD(const int f, const v2<int> &id) : f(f), id(id) {}
		
		inline const bool operator<(const PD &other) const {
			return f > other.f;
		}
	};

	typedef std::set<v2<int> > CloseList;
	typedef std::priority_queue<PD> OpenList;
	typedef std::map<const v2<int>, Point> PointMap;


	void findPath(const v2<int> target, const int step);
	const bool findPathDone(Way &way);
	inline const bool calculatingPath() const { return !_open_list.empty(); }

	//grouped object handling
	Object *add(const std::string &name, const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type);
	Object *get(const std::string &name);
	void remove(const std::string &name);
	void groupEmit(const std::string &name, const std::string &event);

	const bool getRenderRect(sdlx::Rect &src) const;

	void calculateWayVelocity();

	Object * spawnGrouped(const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type);

	const bool old_findPath(const v2<float> &position, Way &way) const;
	const bool old_findPath(const Object *target, Way &way) const;

	const bool getNearest(const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity, const bool check_shooting_range) const;
	const Object * getNearestObject(const std::set<std::string> &classnames, const float range, const bool check_shooting_range) const;
	
	void limitRotation(const float dt, const float speed, const bool rotate_even_stopped, const bool allow_backward);
	
	void checkSurface() const;
	
	virtual const bool skipRendering() const;
	
	const sdlx::Surface * getSurface() const;
	const Matrix<int> &getImpassabilityMatrix() const;
	
	inline const AnimationModel * getAnimationModel() const { return _model; }
	
	Object * _parent;
	
private: 
//pathfinding stuff
	void close(const v2<int>& vertex); 

	OpenList _open_list;
	PointMap _points;
	CloseList _close_list;
	v2<int> _end, _begin;
	int _step;
//end of pathfinding stuff

	struct Event : public mrt::Serializable {
		std::string name;
		bool repeat;
		std::string sound;
		float gain;
		bool played;
		mutable const Pose * cached_pose;
		
		Event();
		Event(const std::string name, const bool repeat, const std::string &sound, const float gain, const Pose * cached_pose = NULL);
		virtual void serialize(mrt::Serializator &s) const;
		virtual void deserialize(const mrt::Serializator &s);
	};
	
	void checkAnimation() const;
	void getSubObjects(std::set<Object *> &objects);
	void groupTick(const float dt);
	
	mutable const Animation *_animation;
	mutable const AnimationModel *_model;
	
	mutable const sdlx::Surface *_surface;
	
	sdlx::Surface *_fadeout_surface;
	int _fadeout_alpha;
	const sdlx::CollisionMap *_cmap;
	
	typedef std::deque<Event> EventQueue;
	EventQueue _events;
	
	typedef std::map<const std::string, float> EffectMap;
	EffectMap _effects;
	
	int _tw, _th;
	int _direction_idx, _directions_n;
	float _pos;

	//waypoints stuff
	Way _way;
	v2<float> _next_target, _next_target_rel;
	
	//rotation stuff
	float _rotation_time;	
	int _dst_direction;
	
	//grouped objects stuff
	v2<float> _position_delta;
	typedef std::map<const std::string, Object *> Group;
	Group _group;
	
	Alarm _blinking;

	void setSync(const bool sync);
	
	friend class IWorld;
	friend class ai::Buratino;
	friend class ai::Waypoints;
};



#endif
