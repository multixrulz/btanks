#ifndef __BTANKS_CLIENT_H__
#define __BTANKS_CLIENT_H__

#include <string>

class PlayerState;
class Monitor;

class Client {
public:
	Client();
	~Client();
	void init(const std::string &host, const unsigned port);
	void notify(const int id, const PlayerState &state);
	void tick(const float dt);

protected:
	Monitor *_monitor;
};


#endif
