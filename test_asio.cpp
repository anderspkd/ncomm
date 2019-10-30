#include "src/ncomm.hpp"
#include <iostream>
#include <time.h>

using namespace std;

typedef unsigned char u8;

int main(int argc, char **argv) {

    (void)argv;

    ncomm::channel_info_t info0, info1;

    if (argc == 1) {

	info0.id = 0;
	info0.port = 5000;
	info0.hostname = "127.0.0.1";
	info0.role = ncomm::SERVER;

	info1.id = 1;
	info1.port = 5001;
	info1.hostname = "127.0.0.1";
	info1.role = ncomm::CLIENT;

    } else {

	info0.id = 1;
	info0.port = 5001;
	info0.hostname = "127.0.0.1";
	info0.role = ncomm::SERVER;

	info1.id = 0;
	info1.port = 5000;
	info1.hostname = "127.0.0.1";
	info1.role = ncomm::CLIENT;

    }

    ncomm::AsioChannel chl(info0, info1);
    chl.Connect();

    vector<u8> buf (100, (u8)argc);
    vector<u8> rbuf (buf.size());

    chl.Send(buf);
    chl.Recv(rbuf);

    for (auto &x: rbuf)
	cout << (int)x << endl;
}
