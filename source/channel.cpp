#include "../include/ncomm.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <thread>

namespace ncomm {

using std::vector;
using std::string;

typedef unsigned char u8;

string channel_info_t::to_string() const
{
    std::stringstream ss;
    if (role == channel_role::DUMMY) {
	ss << "<dummy (id=" << local_id << ")>";
    } else {
	ss << "<";
	if (role == channel_role::SERVER)
	    ss << "server ";
	else
	    ss << "client ";
	ss << "(id=" << local_id << ", remote=" << remote_id << ")";
	ss << ", port=" << port << ", hostname=" << hostname << ">";
    }
    return ss.str();
}

channel_info_t DummyChannel::generate_info(const partyid_t id)
{
    channel_info_t info = {
	.local_id = id,
	.remote_id = id,
	.port = -1,
	.hostname = "",
	.role = DUMMY
    };

    return info;
}

void DummyChannel::send(const vector<unsigned char> &buf) {
    _buffer = buf;
};

void DummyChannel::recv(vector<unsigned char> &buf) {
    NCOMM_DEBUG("recv %s", to_string().c_str());
    buf = _buffer;
};

void TCPChannel::connect_as_server()
{
    int opt = 1;
    int ssock = socket(AF_INET, SOCK_STREAM, 0);
    if (!ssock)
	throw std::runtime_error("(server) socket");

    if (setsockopt(ssock,
		   SOL_SOCKET,
		   SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY,
		   &opt, sizeof(opt)))
    {
	throw std::runtime_error("(server) setsockopt");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(info().port);

    if (bind(ssock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	throw std::runtime_error("(server) bind");

    if (listen(ssock, 1) < 0)
	throw std::runtime_error("(server) listen");

    auto addrlen = sizeof(addr);
    _sock = accept(ssock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);

    if (_sock < 0)
	throw std::runtime_error("(server) accept");

    NCOMM_DEBUG("server connected");
    _alive = true;
}

void TCPChannel::connect_as_client()
{
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0)
	throw std::runtime_error("(client) socket");

    int opt = 1;
    if (setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)))
    	throw std::runtime_error("(client) setsockopt");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(info().port);

    if (inet_pton(AF_INET, info().hostname.c_str(), &addr.sin_addr) <= 0)
	throw std::runtime_error("(client) inet_pton");

    int attempts = 0;
    while (!is_alive()) {
	if (::connect(_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	    attempts += 1;
	    sleep(1);
	} else {
	    _alive = true;
	    break;
	}
    }
    NCOMM_DEBUG("connect in %d attempts", attempts);
}

void TCPChannel::connect()
{
    switch (info().role) {
    case channel_role::SERVER:
 	connect_as_server();
	break;
    case channel_role::CLIENT:
	connect_as_client();
	break;
    default:
	throw std::runtime_error("TCPChannel with dummy role");
    }

    assert(is_alive());

    auto sender = [&]() {
	while (this->_alive) {
	    auto v = this->send_queue.front();
	    this->_send(v.data(), v.size());
	    this->send_queue.pop_front();
	}
    };

    std::thread s(sender);
    s.detach();

    NCOMM_DEBUG("conneted: %s", info().to_string().c_str());
}

void TCPChannel::close()
{
    _alive = false;
}

void TCPChannel::send(const vector<u8>& buf)
{
    send_queue.push_back(buf);
}

void TCPChannel::_send(const u8 *buf, const size_t length)
{
    size_t rem = length;
    size_t offset = 0;

    // TODO: Error checking. Avoid looping forever if other end is dead.
    while (rem > 0) {
	ssize_t n = ::write(_sock, buf + offset, rem);

	if (n < 0)
	    continue;

	rem -= n;
	offset += n;
    }

    assert (rem == 0);
}

void TCPChannel::recv(vector<u8> &buf)
{
    NCOMM_DEBUG("recv %s #bytes=%ld", to_string().c_str(), buf.size());

    size_t rem = buf.size();
    size_t offset = 0;

    // TODO: Error checking. Avoid looping forever if other end is dead.
    while (rem > 0) {
    	ssize_t n = ::read(_sock, buf.data() + offset, rem);

    	if (n < 0) {
    	    continue;
    	}

    	offset += n;
    	rem -= n;
    }

    assert (rem == 0);
}

} // ncomm
