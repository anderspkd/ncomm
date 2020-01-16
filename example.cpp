// Small example program

#include "include/ncomm.hpp"

#include <iostream>

using namespace ncomm;
using namespace std;

int main(int argc, char** argv) {

    if (argc < 3) {
	cout << "usage: " << argv[0] << " <party ID> <network_info.txt>\n";
	return -1;
    }

    partyid_t id = stoul(argv[1]);

    Network nw (id, argv[2]);
    nw.connect();

    cout << "\nsending in a ring. P_(i+1) -> P_i -> P_(i-1)\n\n";

    vector<unsigned char> sb (10, 42 + nw.id());
    vector<unsigned char> rb (sb.size());

    nw.exchange_ring(sb, rb);

    cout << "\nparty " << id << " got: ";
    for (auto &x: rb) {
	cout << (int)x << " ";
    }
    cout << "from party " << nw.ident_of_next() << "\n\n";
}
