#include "SockAddr.h"
#include <cstring>
#include <assert.h>
#include <iostream>

NAMESPACE_START

SockAddr::SockAddr()
{
	reset();
}


SockAddr::SockAddr(const SockAddr &sockAddr)
{
	reset();
	memcpy(&sockaddr, &sockAddr.sockaddr, sizeof(sockAddr.sockaddr));
}


SockAddr::SockAddr(const struct sockaddr* sockAddr, socklen_t socklen)
{
	assert(set(sockAddr, socklen) == 0);
}


SockAddr::SockAddr(uint16_t port, const std::string &ip, uint8_t family)
{
	assert(set(ip, port, family) == 0);
}


SockAddr& SockAddr::operator=(const SockAddr &sockAddr)
{
	sockaddr_storage addr;
	memcpy(&addr, &sockAddr.sockaddr, sizeof(addr));
	memcpy(&sockaddr, &sockAddr.sockaddr, sizeof(sockaddr));
	return *this;
}


const struct sockaddr* SockAddr::getAddr() const
{
	return reinterpret_cast<const struct sockaddr*>(&sockaddr);
}

struct sockaddr* SockAddr::getAddr()
{
	return reinterpret_cast<struct sockaddr*>(&sockaddr);
}


std::string SockAddr::getIp() const
{
	const struct sockaddr *addr = reinterpret_cast<const struct sockaddr*>(&sockaddr);
	if (addr->sa_family == AF_INET) {
		const struct sockaddr_in *addr_in = reinterpret_cast<const struct sockaddr_in*>(&sockaddr);
		char buff[64];
		const char *ip = inet_ntop(AF_INET, &addr_in->sin_addr, buff, sizeof(buff));
		if (ip == NULL) {
			return std::string("");
		}
		return std::string(ip);
	}
	else if (addr->sa_family == AF_INET6) {
		const struct sockaddr_in6 *addr_in6 = reinterpret_cast<const struct sockaddr_in6*>(&sockaddr);
		char buff[64];
		const char *ip = inet_ntop(AF_INET6, &addr_in6->sin6_addr, buff, sizeof(buff));
		if (ip == NULL) {
			return std::string("");
		}
		return std::string(ip);
	}
	return std::string("");
}


int SockAddr::getPort() const
{
	const struct sockaddr *addr = reinterpret_cast<const struct sockaddr*>(&sockaddr);
	if (addr->sa_family == AF_INET) {
		const struct sockaddr_in *addr_in = reinterpret_cast<const struct sockaddr_in*>(&sockaddr);
		return ntohs(addr_in->sin_port);
	}
	else if (addr->sa_family == AF_INET6) {
		const struct sockaddr_in6 *addr_in6 = reinterpret_cast<const struct sockaddr_in6*>(&sockaddr);
		return ntohs(addr_in6->sin6_port);
	}
	return -1;
}


int SockAddr::getFamily() const
{
	const struct sockaddr *addr = reinterpret_cast<const struct sockaddr*>(&sockaddr);
	return addr->sa_family;
}


socklen_t SockAddr::getAddrLength() const
{
	const struct sockaddr *addr = reinterpret_cast<const struct sockaddr*>(&sockaddr);
	if (addr->sa_family == AF_INET) {
		return sizeof(struct sockaddr_in);
	}
	else if (addr->sa_family == AF_INET6) {
		return sizeof(struct sockaddr_in6);
	}
	return sizeof(sockaddr);
}


int SockAddr::set(const struct sockaddr* sockAddr, socklen_t socklen)
{
	reset();
	memcpy(&sockaddr, sockAddr, socklen);
	return 0;
}


int SockAddr::set(const std::string &ip, uint16_t port, uint8_t family)
{
	reset();
	int ret = setSockaddrFamily(family)
		| setSockaddrAddr(family, ip) 
		| setSockaddrPort(family, port);
	if (ret != 0) {
		return -1;
	}
	return 0;
}


int SockAddr::setIp(const std::string &ip)
{
	struct sockaddr *addr = reinterpret_cast<struct sockaddr*>(&sockaddr);
	return setSockaddrAddr(addr->sa_family, ip);
}


int SockAddr::setPort(uint16_t port)
{
	struct sockaddr *addr = reinterpret_cast<struct sockaddr*>(&sockaddr);
	return setSockaddrPort(addr->sa_family, port);
}


int SockAddr::setFamily(uint8_t family)
{
	return setSockaddrFamily(family);
}


int SockAddr::setAny()
{
	if (getFamily() == AF_INET) {
		struct sockaddr_in *addr = reinterpret_cast<sockaddr_in*>(&sockaddr);
		addr->sin_addr.s_addr = INADDR_ANY;
	}
	else if (getFamily() == AF_INET) {
		const struct in6_addr in6addrAny = IN6ADDR_ANY_INIT;
		struct sockaddr_in6 *addr = reinterpret_cast<sockaddr_in6*>(&sockaddr);
		addr->sin6_addr = in6addrAny;
	}
	else {
		return -1;
	}
	return 0;
}


std::string SockAddr::format() const
{
	return std::string( getIp() + ":" + std::to_string(getPort()) );
}


void SockAddr::reset()
{
	bzero(&sockaddr, sizeof(sockaddr));
}


int SockAddr::setSockaddrFamily(uint8_t family)
{
	if (family != AF_INET && family != AF_INET6) {
		return -1;
	}
	struct sockaddr *addr = reinterpret_cast<struct sockaddr*>(&sockaddr);
	addr->sa_family = family;
	return 0;
}


int SockAddr::setSockaddrAddr(uint8_t family, const std::string &ip)
{
	if (family == AF_INET) {
		struct sockaddr_in *addr = reinterpret_cast<sockaddr_in*>(&sockaddr);
		int ret = inet_pton(AF_INET, ip.c_str(), &addr->sin_addr);
		if (ret != 1) {
			return -1;
		}
	}
	else if (family == AF_INET6) {
		struct sockaddr_in6 *addr = reinterpret_cast<sockaddr_in6*>(&sockaddr);
		int ret = inet_pton(AF_INET6, ip.c_str(), &addr->sin6_addr);
		if (ret != 1) {
			return -1;
		}
	}
	else {
		return -1;
	}
	return 0;
}


int SockAddr::setSockaddrPort(uint8_t family, uint16_t port)
{
	if (family == AF_INET) {
		struct sockaddr_in *addr = reinterpret_cast<sockaddr_in*>(&sockaddr);
		addr->sin_port = htons(port);
	}
	else if (family == AF_INET6) {
		struct sockaddr_in6 *addr = reinterpret_cast<sockaddr_in6*>(&sockaddr);
		addr->sin6_port = htons(port);
	}
	else {
		return -1;
	}
	return 0;
}

NAMESPACE_END
