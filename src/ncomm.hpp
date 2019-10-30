#ifndef _NCOMM_HPP
#define _NCOMM_HPP

#include <cstdint>
#include <vector>

#include <boost/asio.hpp>

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
} channel_info_t;

// TODO: local/remote should be server/client

class Channel {
public:

    const channel_info_t& LocalInfo() const {
	return local_info;
    };

    const channel_info_t& RemoteInfo() const {
	return remote_info;
    }

    Channel(const channel_info_t local_info, const channel_info_t remote_info)
	: local_info{local_info}, remote_info{remote_info} {};

    virtual void Connect() = 0;
    virtual void Close() = 0;
    virtual void Send(const vector<u8> &buf) = 0;
    virtual void Recv(vector<u8> &buf) = 0;

private:

    channel_info_t local_info;
    channel_info_t remote_info;
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

} // ncomm

#endif // _NCOMM_HPP
