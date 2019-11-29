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
#include <vector>
#include <fstream>
#include <sstream>

#include <boost/asio.hpp>

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

using std::vector;
using std::string;

using boost::asio::ip::tcp;

typedef uint8_t u8;
typedef size_t partyid_t;

// A channel can be either SERVER, CLIENT or DUMMY. The former two are self
// explanatory. A DUMMY channel is a special channel which "connects to itself"
// and is needed in order to support the following login in a transparent way:
//
//  For all parties, P_i sends to P_j.
//
// For the case when i = j, a DUMMY channel is used.
enum channel_role_t {
    SERVER,
    CLIENT,
    DUMMY
};

// A channel consists of an identifier, a port, a hostname and which role the
// channel is playing.
//
// The values of `id', `port' and `hostname' depend on `role' in the following
// way:
//
// If `role == SERVER' then `id' is the identifier of this machine, `port' is
// the port we're using and `hostname' is the string defined by
// NCOMM_LOCALHOST_IP.
//
// If `role == CLIENT' then `id' is the identifier of the remote machine, `port'
// is the port we're connecting to and `hostname' is the IP of the remote
// machine.
//
// If `role == DUMMY', then `id' is the local identifier while `port == -1' and
// `hostname == ""'.
typedef struct {

    partyid_t       id;
    int             port;
    string          hostname;
    channel_role_t  role;

    string to_string() const;

} channel_info_t;

// A channel is a pair of sockets such that each peer plays both client and
// server towards the other peer. Taking this approach allows us to disregard
// the order of writes (i.e., if P_i first sends the receives, we do not need to
// enforce that P_j first receives and then sends).
class Channel {
public:

    // Information about server part of this connection.
    //
    // @return channel_info_t object with server information.
    const channel_info_t& ServerInfo() const;

    // Information about client part of this connection.
    //
    // @return channel_info_t object with client information.
    const channel_info_t& ClientInfo() const;

    // Remote identifier.
    //
    // Equivalent to ClientInfo().id
    //
    // @return identifier of remote peer
    partyid_t GetRemoteId() const;

    // Local identifier.
    //
    // Identifier of this peer. Equivalent to ServerInfo().id;
    //
    // @return identifier of local peer
    partyid_t GetLocalId() const;

    // Constructs a channel given server and client info.
    Channel(const channel_info_t server_info, const channel_info_t client_info)
	: server_info{server_info}, client_info{client_info},
	  remote_id{client_info.id}, local_id{server_info.id},
	  alive{false} {};

    virtual ~Channel() {};

    // Connect to remote peer.
    //
    // Before a Channel can be used it must be connected by calling this
    // method. Calling Connect() multiple times have no effect.
    //
    // @throw may throw a runtime_error in case of configuration errors
    virtual void Connect() = 0;

    // Closes connections.
    virtual void Close() = 0;

    // Send content of buf to remote peer.
    virtual void Send(const vector<u8> &buf) = 0;

    // Receive into buf from remote peer.
    virtual void Recv(vector<u8> &buf) = 0;

    // Exchange data with remote peer.
    //
    // Equivalent to calling Send then Recv.
    void Exchange(const vector<u8> &sbuf, vector<u8> &rbuf);

    // Indicates whether the channel is alive or not.
    //
    // @return alive
    bool IsAlive() const;

protected:

    channel_info_t server_info;
    channel_info_t client_info;

    partyid_t remote_id;
    partyid_t local_id;

    bool alive;
};

class DummyChannel : public Channel {
public:

    static channel_info_t DummyInfo(const partyid_t id);

    DummyChannel(partyid_t id)
	: Channel{DummyChannel::DummyInfo(id), DummyChannel::DummyInfo(id)} {};

    ~DummyChannel() {};

    void Connect();
    void Close();
    void Send(const vector<u8> &buf);
    void Recv(vector<u8> &buf);

    using Channel::Exchange;

private:
    vector<u8> buffer;
};

class AsioChannel : public Channel {
public:

    using Channel::Channel;

    ~AsioChannel() {};

    void Connect();
    void Close() {};

    void Send(const vector<u8> &buf);
    void Recv(vector<u8> &buf);

    using Channel::Exchange;

private:

    void ConnectClient();
    void ConnectServer();

    boost::asio::io_service ios_sender;
    boost::asio::io_service ios_receiver;
    tcp::socket *ssock;
    tcp::socket *rsock;
};

typedef struct {
    partyid_t id;
    size_t n;
    vector<string> addrs;
} network_info_t;

enum exchange_order {
    INCREASING,
    DECREASING
};

class Network {
public:

    Network(const network_info_t &info) : info{info} {};
    Network(const partyid_t id, const string network_info_filename);

    void Connect();
    void Close();

    Channel* operator[](const size_t idx) const;

    void SetBasePort(const int port);

    int GetBasePort() const;
    size_t Size() const;
    partyid_t GetId() const;
    network_info_t GetInfo() const;

    void ExchangeAll(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs) const;
    void BroadcastSend(const vector<u8> &buf) const;
    void BroadcastRecv(const partyid_t broadcaster, vector<u8> &buf) const;
    void ExchangeRing(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order = DECREASING) const;

    inline void SendToNext(const vector<u8> &buf) const;
    inline void RecvFromNext(vector<u8> &buf) const;
    inline void SendToPrev(const vector<u8> &buf) const;
    inline void RecvFromPrev(vector <u8> &buf) const;

    Channel* NextPeer() const;
    Channel* PrevPeer() const;

private:

    network_info_t info;
    vector<Channel *> peers;

    channel_info_t MakeClientInfo(const partyid_t id, const string hostname) const;
    channel_info_t MakeServerInfo(const partyid_t id) const;

    int base_port = 5000;
};

} // ncomm

#endif // _NCOMM_HPP
