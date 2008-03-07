#ifndef CLUNK_OBJECT_H__
#define CLUNK_OBJECT_H__

#include "export_clunk.h"
#include "v3.h"
#include <deque>

namespace clunk {
class Context;
class Source;

class CLUNKAPI Object {
public: 
	~Object();
	void updatePV(const v3<float> &pos, const v3<float> &vel);
	void add(Source *source);

private: 
	friend class Context;
	Object(Context *context);
	Context *context;
	v3<float> pos, vel;

	std::deque<Source *> sources;
};
}

#endif
