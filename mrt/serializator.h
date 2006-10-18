#ifndef __BTANKS_SERIALIZATOR_H__
#define __BTANKS_SERIALIZATOR_H__

#include <sys/types.h>
#include <string>

namespace mrt {
class Chunk;
class Serializator {
public:
	Serializator();
	Serializator(const mrt::Chunk *chunk);
	~Serializator();
	
	void add(const int n);
	void add(const unsigned int n);
	void add(const float f);
	void add(const std::string &str);
	void add(const bool b);
	void add(const Chunk &c);

	const bool end() const;
	
	void get(int &n) const;
	void get(unsigned int &n) const;
	void get(float &f) const;
	void get(std::string &str) const;
	void get(bool &b) const;
	void get(Chunk &c) const;
	
	const Chunk & getData() const;
protected:

	Chunk *_data;
	mutable size_t _pos;
	bool _owns_data;

private:
	Serializator(const Serializator &s);
	const Serializator & operator=(const Serializator &s);
};
}

#endif
