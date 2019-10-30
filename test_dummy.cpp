#include "src/ncomm.hpp"
#include <iostream>

using namespace std;

typedef unsigned char u8;

int main() {

    vector<u8> buf {1,2,3,4,5,6};

    ncomm::DummyChannel chl (0);

    chl.Send(buf);

    vector<u8> rbuf (buf.size());

    chl.Recv(rbuf);

    for (auto &x: rbuf)
	cout << (int)x << endl;

}
