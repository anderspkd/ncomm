#ifndef _NCOMM_HPP
#define _NCOMM_HPP

#include <cstdint>
#include <vector>

namespace ncomm {

using std::vector;
using std::string;

typedef uint8_t u8;
typedef size_t partyid_t;

enum channel_role_t {
    SERVER,
    CLIENT
};

typedef struct {
    partyid_t       id;
    size_t          size;
    vector<string>  addrs;
} network_info_t;

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

    virtual void Connect();
    virtual void Close();

private:

    channel_info_t local_info;
    channel_info_t remote_info;
}

class Network {
public:

    // Size of network
    size_t Size() const {
	return n;
    };

    // Identity of local party
    partyid_t LocalId() const {
	return id;
    };

    // Establish the network object, but do not connect to anything.
    Network(const struct network_info_t info);

    // Establish connections between parties. Refer to documentation in the
    // Channel class for more information.
    void Connect();

    // Close all connections;
    void Close() {
	for (auto &c : channels) c->Close();
    };

    inline void ExchangeAll(
	const vector<vector<u8>> &outbuf,
	vector<vector<u8>> &inbuf) {

	// send first
	for (size_t i = 0; i < n; i++)
	    channels[i]->Send(outbuf[i]);
	for (size_t i = 0; i < n; i++)
	    channels[i]->Recv(inbuf[i]);
    };

    inline ExchangeRing(
	const vector<u8> &outbuf,
	vector<u8> &inbuf,
	bool reverse_order = false;
	) {

	if (reverse_order) {
	    PrevChannel()->Send(outbuf);
	    NextChannel()->Recv(inbuf);
	} else {
	    NextChannel()->Send(outbuf);
	    PrevChannel()->Recv(inbuf);
	}
    };

    inline void Send(
	const partyid_t receiver_id,
	const vector<u8> &buf) {

	channels[receiver_id]->Send(buf);
    };

    inline void Recv(
	const partyid_t sender_id,
	vector<u8> &buf) {

	channels[sender_id]->Recv(buf);
    };

private:
    size_t n;
    partyid_t id;

    vector<Channel *> channels;

    inline Channel *PrevChannel() const {
	return channels[id ? id - 1 : n - 1];
    };

    inline Channel *NextChannel() const {
	return channels[id == n - 1 ? 0 : id + 1];
    };
};

} // ncomm

#endif // _NCOMM_HPP
