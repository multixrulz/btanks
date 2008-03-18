#ifndef CLUNK_OBJECT_H__
#define CLUNK_OBJECT_H__

#include "export_clunk.h"
#include "v3.h"
#include <set>

namespace clunk {
class Context;
class Source;

class CLUNKAPI Object {
public: 
	~Object();
	void update(const v3<float> &pos, const v3<float> &vel);

	void play(Source *source);
	void remove(Source *source);
	void remove_all();

private: 
	friend class Context;
	Object(Context *context);
	Context *context;
	v3<float> position, velocity;

	std::set<Source *> sources;
};
}

#endif
