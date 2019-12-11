#include "../include/ncomm.hpp"
#include <iostream>

using namespace std;
using namespace ncomm;

typedef unsigned char u8;

bool test_exchange_all(Network &nw)
{
    auto num = 1000*65537;
    vector<vector<u8>> sbs (nw.size());
    vector<vector<u8>> rbs (nw.size());
    for (size_t i = 0; i < nw.size(); i++) {
	rbs[i].resize(num);
	sbs[i] = vector<u8>(num, (2 + (u8)nw.id()));
    }

    nw.exchange_all(sbs, rbs);

    for (size_t i = 0; i < nw.size(); i++) {
	for (size_t j = 0; j < (size_t)num; j++) {
	    if (rbs[i][j] != (2 + (u8)i)) {
		cout << "got=" << (int)rbs[i][j] << ", expected=" << (int)(2 + (u8)i) << "\n";
		return false;
	    }

	}
    }
    return true;
}

bool test_broadcast(Network &nw, partyid_t bcid) {

    if (nw.id() == bcid) {
	vector<u8> sb (10, nw.id());

	nw.broadcast_send(sb);

    } else {
	vector<u8> rb (10);

	nw.broadcast_recv(bcid, rb);

	for (auto &x: rb) {
	    if (x != bcid)
		return false;
	}
    }
    return true;
}

int main(int argc, char **argv) {

    if (argc < 4) {
	cout << "missing args\n";
	return -1;
    }

    partyid_t id = stoul(argv[1]);
    size_t n = stoul(argv[2]);

    network_info_t info = {
	.id    = id,
	.size  = n,
	.addrs = vector<string>(n, "127.0.0.1")
    };

    Network nw (info);
    nw.connect();

    bool result = false;
    string s (argv[3]);
    string t = "exchange_all";

    // if (s == t) {
    result = test_exchange_all(nw);
    // }

    cout << "test_exchange result: " << (result ? "true" : "false")  << endl;
}
