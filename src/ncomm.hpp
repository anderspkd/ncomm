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

namespace ncomm {

using std::vector;
using std::string;

using boost::asio::ip::tcp;

typedef uint8_t u8;
typedef size_t partyid_t;

enum channel_role_t {
    SERVER,
    CLIENT,
    DUMMY
};

typedef struct {

    partyid_t       id;
    int             port;
    string          hostname;
    channel_role_t  role;

    string to_string() const {
	std::stringstream ss;
	ss << "<id=" << id << ", " <<
	    "addr=\"" << hostname << "\", " <<
	    "port=" << port << ", " <<
	    "role=" << role << ">";
	return ss.str();
    };

} channel_info_t;

class Channel {
public:

    const channel_info_t& ServerInfo() const {
	return server_info;
    };

    const channel_info_t& ClientInfo() const {
	return client_info;
    }

    partyid_t GetRemoteId() const {
	return remote_id;
    };

    partyid_t GetLocalId() const {
	return local_id;
    };

    Channel(const channel_info_t server_info, const channel_info_t client_info)
	: server_info{server_info}, client_info{client_info},
	  remote_id{client_info.id}, local_id{server_info.id} {};

    virtual ~Channel() {};

    virtual void Connect() = 0;
    virtual void Close() = 0;
    virtual void Send(const vector<u8> &buf) = 0;
    virtual void Recv(vector<u8> &buf) = 0;

private:

    channel_info_t server_info;
    channel_info_t client_info;

    partyid_t remote_id;
    partyid_t local_id;
};

class DummyChannel : public Channel {
public:

    static channel_info_t DummyInfo(const partyid_t id) {
	channel_info_t info = {
	    .id = id,
	    .port = -1,
	    .hostname = "",
	    .role = DUMMY
	};
	return info;
    };

    DummyChannel(partyid_t id)
	: Channel{DummyChannel::DummyInfo(id), DummyChannel::DummyInfo(id)} {};

    ~DummyChannel() {};

    void Connect() {};
    void Close() {};
    void Send(const vector<u8> &buf);
    void Recv(vector<u8> &buf);

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

    const Channel *operator[](const size_t idx) const {
	return peers[idx];
    };

    void SetBasePort(const int port) {
	base_port = port;
    };

    int GetBasePort() const {
	return base_port;
    };

    void ExchangeAll(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs);
    void BroadcastSend(const vector<u8> &buf);
    void BroadcastRecv(const partyid_t broadcaster, vector<u8> &buf);
    void ExchangeRing(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order = INCREASING);

private:

    network_info_t info;
    vector<Channel *> peers;

    Channel *GetNextPeer() const;
    Channel *GetPrevPeer() const;

    channel_info_t MakeClientInfo(const partyid_t id, const string hostname) const;
    channel_info_t MakeServerInfo(const partyid_t id) const;

    int base_port = 5000;
};

} // ncomm

#endif // _NCOMM_HPP
