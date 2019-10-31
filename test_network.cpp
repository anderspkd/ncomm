#include "src/ncomm.hpp"
#include <iostream>

using namespace std;

using ncomm::u8;

int main(int argc, char **argv) {
    vector<string> addrs {"127.0.0.1", "127.0.0.1", "127.0.0.1"};

    if (argc == 1) {
	cout << "missing partyid\n";
	return 1;
    }

    ncomm::network_info_t info = {
	.id = stoul(argv[1]),
	.n = 3,
	.addrs = addrs
    };

    ncomm::Network network (info);
    network.Connect();

    cout << "done\n";

    vector<u8> sbuf (1000000,(u8)info.id);
    vector<u8> rbuf (sbuf.size());

    network.ExchangeRing(sbuf, rbuf, ncomm::exchange_order::DECREASING);

    // for (size_t i = 0; i < sbuf.size(); i++) {
    // 	cout << "sbuf = " << (int)sbuf[i] << endl;
    // 	cout << "rbuf = " << (int)rbuf[i] << endl << endl;
    // }
    cout << "last: " << (int)rbuf[sbuf.size()-1] << endl;
}
