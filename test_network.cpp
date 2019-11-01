#include "src/ncomm.hpp"
#include <iostream>

using namespace std;

using ncomm::u8;

int main(int argc, char **argv) {

    if (argc < 3)
	return 0;

    ncomm::partyid_t id = stoul(argv[1]);

    ncomm::Network network (id, argv[2]);
    network.Connect();

    cout << "done\n";

    vector<u8> sbuf (1000000,(u8)id);
    vector<u8> rbuf (sbuf.size());

    network.ExchangeRing(sbuf, rbuf, ncomm::exchange_order::DECREASING);

    // for (size_t i = 0; i < sbuf.size(); i++) {
    // 	cout << "sbuf = " << (int)sbuf[i] << endl;
    // 	cout << "rbuf = " << (int)rbuf[i] << endl << endl;
    // }
    cout << "last: " << (int)rbuf[sbuf.size()-1] << endl;
}
