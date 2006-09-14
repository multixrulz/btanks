#include "object.h"
#include "resource_manager.h"

class Item : public Object {
public:
	Item(const std::string &classname) : Object(classname) {
		pierceable = true;
	}
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Item::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) 
		Object::emit("death", this);
}

void Item::onSpawn() {
	play("main", true);
}

void Item::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (emitter->classname != "player") 
			return;
		if (classname == "heal") {
			if (hp != 0)
				emitter->heal(hp);
			hp = 0;
			impassability = 0;
			cancelAll();
			play("take", false);
		} else LOG_WARN(("item '%s' was not implemented", classname.c_str()));
	} else Object::emit(event, emitter);
}


Object* Item::clone() const  {
	return new Item(*this);
}

/*  note that all heal objects have the same classname. this was done to simplify AI search/logic.*/

REGISTER_OBJECT("heal", Item, ("heal"));
REGISTER_OBJECT("megaheal", Item, ("heal"));
