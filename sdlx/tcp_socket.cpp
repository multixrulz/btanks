#include "tcp_socket.h"
#include "net_ex.h"

using namespace sdlx;

TCPSocket::TCPSocket() : _sock(NULL) {}

void TCPSocket::listen(const unsigned port) {
	close();
	
	IPaddress ip;
	
	if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
		throw_net(("SDLNet_ResolveHost"));
	        
	_sock = SDLNet_TCP_Open(&ip);
	if (!_sock) {
		throw_net(("SDLNet_TCP_Open"));
	}
}

void TCPSocket::connect(const std::string &host, const int port) {
	close();
	
	IPaddress ip;
	
	if(SDLNet_ResolveHost(&ip, host.c_str(), port)==-1) 
		throw_net(("SDLNet_ResolveHost"));
	        
	_sock = SDLNet_TCP_Open(&ip);
	if(!_sock) 
		throw_net(("SDLNet_TCP_Open"));	                
}

void TCPSocket::accept(sdlx::TCPSocket &client) {
	client.close();
	client._sock = SDLNet_TCP_Accept(_sock);
	if (client._sock == NULL)
		throw_net(("SDLNet_TCP_Accept"));
}


const bool TCPSocket::ready() const {
	return SDLNet_SocketReady(_sock) != 0;
}

void TCPSocket::close() {
	if (_sock == NULL) 
		return;
	SDLNet_TCP_Close(_sock);
	_sock = NULL;
}

TCPSocket::~TCPSocket() {
	close();
}
