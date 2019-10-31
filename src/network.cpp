#include "ncomm.hpp"

namespace ncomm {

int base_port = 5000;

void Network::Close() {
    for (auto &peer : peers) {
	peer->Close();
	delete peer;
    }
}

Network::Network(const string network_info_filename) {
    (void)network_info_filename;
}

void Network::Connect() {

    const auto myid = info.id;
    const auto n = info.n;

    peers.resize(n);

    DEBUG << "network id=" << myid << ", n=" << n << "\n";

    for (size_t i = 0; i < n; i++) {

	if (i == myid) {
	    peers[i] = new DummyChannel(myid);
	    peers[i]->Connect();

	    DEBUG << "connected dummy\n";

	} else {

	    channel_info_t server = {
		.id = myid,
		.port = base_port + static_cast<int>(myid),
		.hostname = "0.0.0.0",
		.role = SERVER
	    };

	    channel_info_t client = {
		.id = i,
		.port = base_port + static_cast<int>(i),
		.hostname = info.addrs[i],
		.role = CLIENT
	    };

	    peers[i] = new AsioChannel(server, client);
	    peers[i]->Connect();

	    DEBUG << "connected to " << i << "\n";
	}
    }
}

Channel *Network::GetNextPeer() const {
    return info.id == info.n - 1 ? peers[0] : peers[info.id+1];
}

Channel *Network::GetPrevPeer() const {
    return info.id == 0 ? peers[info.n-1] : peers[info.id-1];
}

void Network::ExchangeAll(const vector<vector<u8>> &sbufs, vector<vector<u8>> &rbufs) {
    for (auto &peer : peers)
	peer->Send(sbufs[peer->GetRemoteId()]);
    for (auto &peer : peers)
	peer->Recv(rbufs[peer->GetRemoteId()]);
}

void Network::BroadcastSend(const vector<u8> &buf) {
    for (auto &peer : peers)
	peer->Send(buf);
}

void Network::BroadcastRecv(const partyid_t broadcaster, vector<u8> &buf) {
    peers[broadcaster]->Recv(buf);
}

void Network::ExchangeRing(const vector<u8> &sbuf, vector<u8> &rbuf, exchange_order order) {
    if (order == exchange_order::INCREASING) {
	GetNextPeer()->Send(sbuf);
	DEBUG << "sent to next\n";
	GetPrevPeer()->Recv(rbuf);
	DEBUG << "recv from prev\n";
    } else {
	GetPrevPeer()->Send(sbuf);
	DEBUG << "sent to prev\n";
	GetNextPeer()->Recv(rbuf);
	DEBUG << "recv from next\n";
    }
}

} // ncomm
