#ifndef LIBUVCPP_SOCKADDR_H
#define LIBUVCPP_SOCKADDR_H
#include <string>
#include <cstring>
#include "utility.h"
#include "uv.h"

NAMESPACE_START

class SockAddr
{
public:

	SockAddr() {
		memset(&addr_, 0, sizeof(addr_));
	}

	SockAddr(int port)
			: SockAddr("0.0.0.0", port)
	{
	}

	SockAddr(const char *ip, int port, bool ipv6 = false) {
		if (ipv6) {
			uv_ip6_addr(ip, port, (sockaddr_in6*)&addr_);
		}
		else {
			uv_ip4_addr(ip, port, (sockaddr_in*)&addr_);
		}
	}

	std::string getIp() const {
		char buff[64];
		if (addr_.sa_family == AF_INET) {
			uv_ip4_name((sockaddr_in*)&addr_, buff, sizeof(buff));
		}
		else {
			uv_ip6_name((sockaddr_in6*)&addr_, buff, sizeof(buff));
		}
		return std::string(buff);
	}

	std::string getIpPort() const {
		char buff[64];
		u_short port;
		if (addr_.sa_family == AF_INET) {
			sockaddr_in *addr = (sockaddr_in*)&addr_;
			uv_ip4_name(addr, buff, sizeof(buff));

			port = ntohs(addr->sin_port);
		}
		else {
			sockaddr_in6 *addr = (sockaddr_in6*)&addr_;
			uv_ip6_name(addr, buff, sizeof(buff));
			port = ntohs(addr->sin6_port);
		}
		return std::string(buff) + ":" + std::to_string(port);
	}

	u_short getPort() const {
		u_short port;
		if (addr_.sa_family == AF_INET) {
			sockaddr_in *addr = (sockaddr_in*)&addr_;
			port = ntohs(addr->sin_port);
		}
		else {
			sockaddr_in6 *addr = (sockaddr_in6*)&addr_;
			port = ntohs(addr->sin6_port);
		}
		return port;
	}

	const struct sockaddr* getAddr() const {
		return &addr_;
	}

	struct sockaddr* getAddr() {
		return &addr_;
	}

	int getAddrLength() const
	{
		if (addr_.sa_family == AF_INET) {
			return sizeof(struct sockaddr_in);
		}
		else if (addr_.sa_family == AF_INET6) {
			return sizeof(struct sockaddr_in6);
		}
		return sizeof(sockaddr);
	}

private:
	sockaddr addr_;
};

NAMESPACE_END

#endif

