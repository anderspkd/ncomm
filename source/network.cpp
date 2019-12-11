#include "../include/ncomm.hpp"

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
    NCOMM_L("%s connect()", info().to_string().c_str());

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

void Network::exchange_all(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs) const
{
    NCOMM_L("exchange_all()");
    for (auto &peer : _peers) {
	const auto rid = peer->remote_id();
	peer->exchange(sbufs[rid], rbufs[rid]);
    }
}

void Network::broadcast_send(const vector<u8> &buf) const
{
    NCOMM_L("broadcast_send()");
    for (auto &peer : _peers)
	peer->send(buf);
}

void Network::broadcast_recv(const partyid_t broadcaster, vector<u8> &buf) const
{
    NCOMM_L("broadcast_recv()");
    assert (broadcaster < size());
    _peers[broadcaster]->recv(buf);
}

void Network::exchange_ring(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order) const
{
    NCOMM_L("exchange_ring()");
    if (order == exchange_order::INCREASING) {
	next_peer()->send(sbuf);
	prev_peer()->recv(rbuf);
    } else { // order == exchange_order::DECREASING
	prev_peer()->send(sbuf);
	next_peer()->recv(rbuf);
    }
}

} // ncomm
