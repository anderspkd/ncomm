#include "../include/ncomm.hpp"

#include <thread>

namespace ncomm {

using std::string;
using std::vector;

typedef unsigned char u8;

string network_info_t::to_string() const
{
    std::stringstream ss;
    ss << "(network: id=" << id << ", size=" << size << ")";
    return ss.str();
}

Network::Network(const partyid_t id, const string network_info_filename)
{
    std::string line;
    std::ifstream network_info (network_info_filename);

    if (!network_info.is_open())
	throw std::runtime_error("could not open network info file");

    size_t n = 0;
    std::vector<std::string> addrs;

    while (std::getline(network_info, line)) {
	addrs.emplace_back(line);
	n++;
    }

    network_info.close();

    if (id >= n)
	throw std::runtime_error("invalid party id");

    _info = {
	.id    = id,
	.size  = n,
	.addrs = addrs
    };
}

void Network::close()
{
    for (auto &peer : _peers) {
	peer->close();
	delete peer;
    }
}

channel_info_t Network::make_info(const partyid_t remote_id, const string hostname) const
{
    channel_info_t cinfo;

    cinfo.local_id  = id();
    cinfo.remote_id = remote_id;

    if (id() == remote_id)
	cinfo.role = channel_role::DUMMY;
    else
	cinfo.role = id() < remote_id ? channel_role::CLIENT : channel_role::SERVER;

    if (cinfo.role == channel_role::CLIENT) {
	auto offset = size() * id() + remote_id;
	cinfo.port = _base_port + offset;
	cinfo.hostname = hostname;
    } else {
	auto offset = size() * remote_id + id();
	cinfo.port = _base_port + offset;
	cinfo.hostname = NCOMM_LOCALHOST_IP;
    }

    return cinfo;
}

void Network::connect()
{
    NCOMM_DEBUG("%s connect()", info().to_string().c_str());

    _peers.resize(size());

    for (size_t i = 0; i < size(); i++) {

	auto chl_info = make_info(i, _info.addrs[i]);

	if (chl_info.role == channel_role::DUMMY)
	    _peers[i] = new DummyChannel(i);
	else
	    _peers[i] = new TCPChannel(chl_info);

	_peers[i]->connect();
    }
}

void Network::send_to(const partyid_t receiver, const vector<u8> &buf) const
{
    assert (receiver < size());
    _peers[receiver]->send(buf);
}

void Network::recv_from(const partyid_t sender, vector<u8> &buf) const
{
    assert (sender < size());
    _peers[sender]->recv(buf);
}

void Network::exchange_with(const partyid_t other, const vector<u8> &sbuf, vector<u8> rbuf) const
{
    if (other == id()) {
	// less efficient than need be, but more consistent behaviorwise
	send_to(id(), sbuf);
	recv_from(id(), rbuf);
	return;
    }

    auto one_shot = [&]() {
	this->send_to((partyid_t)other, sbuf);
    };

    std::thread sender (one_shot);

    recv_from(other, rbuf);

    sender.join();
}

void Network::exchange_all(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs) const
{
    NCOMM_DEBUG("exchange_all()");

    auto handler = [&](){
	for (size_t i = 0; i < size(); i++) {
	    if (i == this->id())
		continue;
	    this->send_to((partyid_t)i, sbufs[i]);
	}
    };

    // we need to ensure that we "send" to ourselves before read is called.
    send_to(this->id(), sbufs[this->id()]);

    std::thread sender (handler);

    for (size_t i = 0; i < size(); i++)
	recv_from((partyid_t)i, rbufs[i]);

    sender.join();
}

void Network::broadcast_send(const vector<u8> &buf) const
{
    NCOMM_DEBUG("broadcast_send()");
    for (auto &peer : _peers)
	peer->send(buf);
}

void Network::broadcast_recv(const partyid_t broadcaster, vector<u8> &buf) const
{
    NCOMM_DEBUG("broadcast_recv()");
    assert (broadcaster < size());
    _peers[broadcaster]->recv(buf);
}

void Network::exchange_ring(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order) const
{
    NCOMM_DEBUG("exchange_ring()");

    partyid_t send_id, recv_id;
    if (order == exchange_order::INCREASING) {
	send_id = ident_of_next();
	recv_id = ident_of_prev();
    } else {
	send_id = ident_of_prev();
	recv_id = ident_of_next();
    }

    auto handler = [&]() {
	this->send_to(send_id, sbuf);
    };

    std::thread sender (handler);

    this->recv_from(recv_id, rbuf);

    sender.join();
}

} // ncomm
