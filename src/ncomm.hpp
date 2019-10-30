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

} // ncomm

#endif // _NCOMM_HPP
