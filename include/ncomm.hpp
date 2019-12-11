// Copyright (c) 2019, Anders Dalskov
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <https://www.gnu.org/licenses/>.

// ncomm defines a small set of functionalities concerned with performing
// communication between a set of n peers.
//
// This library is not meant to be extremely fast or feature-rich, but rather
// easy to use when prototyping MPC protocols.

#ifndef _NCOMM_HPP
#define _NCOMM_HPP

#include <cstdint>
#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>
#include <atomic>
#include <thread>
#include <mutex>

#ifdef NCOMM_PRINT
#define NCOMM_L(...) do {						\
	fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);		\
	fprintf(stderr, ##__VA_ARGS__);					\
	fputs("\n", stderr);						\
    } while(0)
#else
#define NCOMM_L(...) do { } while(0)
#endif

#define NCOMM_LOCALHOST_IP "0.0.0.0"

namespace ncomm {

typedef unsigned int  partyid_t;

enum channel_role {
    SERVER,
    CLIENT,
    DUMMY
};

typedef struct {

    partyid_t       local_id;
    partyid_t       remote_id;
    int             port;
    std::string     hostname;
    channel_role    role;

    std::string to_string() const;

} channel_info_t;

class Channel {
public:

    Channel(const channel_info_t info)
	: _info{info},
	  _remote_id{info.remote_id},
	  _local_id{info.local_id},
	  _alive{false}
	{};

    const channel_info_t& info() const {
	return _info;
    };

    partyid_t remote_id() const {
	return _info.remote_id;
    };

    partyid_t local_id() const {
	return _info.local_id;
    };

    virtual ~Channel() {};

    virtual void connect() = 0;
    virtual void close() = 0;
    virtual void send(const std::vector<unsigned char> &buf) = 0;
    virtual void recv(std::vector<unsigned char> &buf) = 0;

    void exchange(const std::vector<unsigned char> &sbuf, std::vector<unsigned char> &rbuf) {
	this->send(sbuf);
	this->recv(rbuf);
    };

    bool is_alive() const {
	return _alive;
    };

protected:

    channel_info_t _info;

    partyid_t _remote_id;
    partyid_t _local_id;

    bool _alive;
};

class DummyChannel : public Channel {
public:

    static channel_info_t generate_info(const partyid_t id);

    DummyChannel(partyid_t id)
	: Channel{DummyChannel::generate_info(id)}
	{};


    ~DummyChannel() {};

    void connect() {
	this->_alive = true;
    };

    void close() {
	this->_alive = false;
    };

    void send(const std::vector<unsigned char> &buf) {
	_buffer = buf;
    };

    void recv(std::vector<unsigned char> &buf) {
	buf = _buffer;
	_buffer.clear();
    };

    using Channel::exchange;

private:
    std::vector<unsigned char> _buffer;
};

class TCPChannel : public Channel {
public:

    using Channel::Channel;

    ~TCPChannel() {};

    void connect();
    void close();

    void send(const std::vector<unsigned char> &buf);
    void recv(std::vector<unsigned char> &buf);

    using Channel::exchange;

private:

    void connect_as_server();
    void connect_as_client();

    std::mutex _lock;
    int _sock;
};

typedef struct {

    partyid_t      id;
    size_t         size;
    std::vector<std::string> addrs;

    std::string to_string() const;

} network_info_t;

enum exchange_order {
    INCREASING,
    DECREASING
};

class Network {
public:

    Network(const network_info_t &info)
	: _info{info}
	{};

    Network(const partyid_t id, const std::string network_info_filename);

    void connect();
    void close();

    Channel* operator[](const size_t idx) const {
	assert (idx < this->size());
	return _peers[idx];
    };

    int& base_port() {
	return _base_port;
    };

    size_t size() const {
	return _info.size;
    };

    partyid_t id() const {
	return _info.id;
    };

    network_info_t info() const {
	return _info;
    };

    void exchange_all(
	const std::vector<std::vector<unsigned char>> &sbufs,
	std::vector<std::vector<unsigned char>> &rbufs) const;

    void broadcast_send(
	const std::vector<unsigned char> &buf) const;

    void broadcast_recv(
	const partyid_t broadcaster,
	std::vector<unsigned char> &buf) const;

    void exchange_ring(
	const std::vector<unsigned char> &sbuf,
	std::vector<unsigned char> &rbuf,
	exchange_order order = DECREASING) const;

    inline void send_to_next(const std::vector<unsigned char> &buf) const {
	next_peer()->send(buf);
    };

    inline void recv_from_next(std::vector<unsigned char> &buf) const {
	next_peer()->recv(buf);
    };

    inline void send_to_prev(const std::vector<unsigned char> &buf) const {
	prev_peer()->send(buf);
    };

    inline void recv_from_prev(std::vector <unsigned char> &buf) const {
	prev_peer()->recv(buf);
    };

    Channel* next_peer() const {
	return id() == size() - 1 ? _peers[0] : _peers[id() + 1];
    };

    Channel* prev_peer() const {
	return id() == 0 ? _peers[size() - 1] : _peers[id() - 1];
    };

private:

    network_info_t _info;
    std::vector<Channel*> _peers;

    channel_info_t make_info(const partyid_t id, const std::string hostname) const;

    int _base_port = 5000;
};

} // ncomm

#endif // _NCOMM_HPP
