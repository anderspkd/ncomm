#include "../include/ncomm.hpp"

namespace ncomm {

Network::Network(const partyid_t id, const string network_info_filename) {
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

    info = {
	.id = id,
	.n  = n,
	.addrs = addrs
    };
}

Channel* Network::operator[](const size_t idx) const {
    return peers[idx];
}

void Network::SetBasePort(const int port) {
    base_port = port;
}

int Network::GetBasePort() const {
    return base_port;
}

size_t Network::Size() const {
    return info.n;
}

partyid_t Network::GetId() const {
    return info.id;
}

network_info_t Network::GetInfo() const {
    return info;
}

void Network::Close() {
    for (auto &peer : peers) {
	peer->Close();
	delete peer;
    }
}

channel_info_t Network::MakeClientInfo(const partyid_t id, const string hostname) const {
    auto offset = info.n * id + info.id;
    channel_info_t cinfo = {
	.id = id,
	.port = base_port + static_cast<int>(offset),
	.hostname = hostname,
	.role = CLIENT
    };
    return cinfo;
}

channel_info_t Network::MakeServerInfo(const partyid_t id) const {
    auto offset = info.n * info.id + id;
    channel_info_t sinfo = {
	.id = info.id,
	.port = base_port + static_cast<int>(offset),
	.hostname = NCOMM_LOCALHOST_IP,
	.role = SERVER
    };
    return sinfo;
}

void Network::Connect() {

    const auto myid = info.id;
    const auto n = info.n;

    peers.resize(n);

    for (size_t i = 0; i < n; i++) {

	if (i == myid) {
	    peers[i] = new DummyChannel(myid);
	    peers[i]->Connect();

	} else {

	    auto server = MakeServerInfo(i);
	    auto client = MakeClientInfo(i, info.addrs[i]);

	    peers[i] = new AsioChannel(server, client);
	    peers[i]->Connect();

	}
    }
}

Channel *Network::NextPeer() const {
    return info.id == info.n - 1 ? peers[0] : peers[info.id+1];
}

Channel *Network::PrevPeer() const {
    return info.id == 0 ? peers[info.n-1] : peers[info.id-1];
}

void Network::ExchangeAll(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs) const {
    for (auto &peer : peers) {
	const auto rid = peer->GetRemoteId();
	// In case caller hasn't reserved space in receiver buffer, we assume
	// that we'll be receiving the same amount of data as we're sending.
	// TODO: emit warning.
	if (!rbufs[rid].size())
	    rbufs[rid].resize(sbufs[rid].size());
	peer->Exchange(sbufs[rid], rbufs[rid]);
    }
}

void Network::BroadcastSend(const vector<u8> &buf) const {
    for (auto &peer : peers)
	peer->Send(buf);
}

void Network::BroadcastRecv(const partyid_t broadcaster, vector<u8> &buf) const {
    peers[broadcaster]->Recv(buf);
}

void Network::ExchangeRing(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order) const {
    if (order == exchange_order::INCREASING) {
	NextPeer()->Send(sbuf);
	PrevPeer()->Recv(rbuf);
    } else { // order == exchange_order::DECREASING
	PrevPeer()->Send(sbuf);
	NextPeer()->Recv(rbuf);
    }
}

inline void Network::SendToNext(const vector<u8> &buf) const {
    NextPeer()->Send(buf);
}

inline void Network::RecvFromNext(vector<u8> &buf) const {
    NextPeer()->Recv(buf);
}

inline void Network::SendToPrev(const vector<u8> &buf) const {
    PrevPeer()->Send(buf);
}

inline void Network::RecvFromPrev(vector<u8> &buf) const {
    PrevPeer()->Recv(buf);
}

} // ncomm
