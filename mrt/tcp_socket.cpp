#include "tcp_socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#endif              

#include "ioexception.h"              

using namespace mrt;

TCPSocket::TCPSocket() {}

void TCPSocket::listen(const unsigned port) {
	_sock = create(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	if (bind(_sock, (const sockaddr *)&addr, sizeof(addr)) == -1)
		throw_io(("bind"));
	
	if (::listen(_sock, 10) == -1)
		throw_io(("listen"));
}

void TCPSocket::connect(const std::string &host, const int port) {
	_sock = create(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	//add gethostbyname here.

	if (::connect(_sock, (const struct sockaddr*)&addr, sizeof(addr))	 == -1)
		throw_io(("connect"));
}

const int TCPSocket::send(const void *data, const int len) const {
	return ::send(_sock, data, len, 0);
}
//void send(const mrt::Chunk &data) const;
const int TCPSocket::recv(void *data, const int len) const {
	return ::recv(_sock, data, len, 0);
}

void TCPSocket::accept(TCPSocket &client) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	socklen_t len = sizeof(addr);
	int s = ::accept(_sock, (struct sockaddr *)&addr, &len);
	if (s == -1)
		throw_io(("accept"));
	client.close();
	client._sock = s;
}

