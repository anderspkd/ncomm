#include "../include/ncomm.hpp"
#include <iostream>

#include <thread>
#include <chrono>

#define REPEAT(x) for (size_t i = 0; i < (x); i++)
#define SLEEP_MS(x) this_thread::sleep_for(chrono::milliseconds((0)))

using namespace std;
using namespace ncomm;

typedef unsigned char u8;

static size_t n = 1000*65537;

bool test_self(Network &nw) {
    vector<u8> sb (n, 123);
    vector<u8> rb (sb.size());

    nw.send_to(nw.id(), sb);
    nw.recv_from(nw.id(), rb);

    for (size_t i = 0; i < n; i++)
	if (sb[i] != rb[i])
	    return false;

    return true;
}

bool test_simple(Network &nw)
{
    if (!(nw.id() == 0 || nw.id() == 1))
	return true;

    vector<u8> sb (n, (u8)nw.id());
    vector<u8> rb (n);

    if (nw.id() == 0) {
	// cout << "0 -> 1 (" << sb.size() << " bytes)\n";
	nw.send_to(1, sb);
    } else {
	// cout << "1 <- 0 (" << sb.size() << " bytes)\n";
	nw.recv_from(0, rb);
	for (size_t i = 0; i < n; i++)
	    if (rb[i] != 0)
		return false;
    }

    if (nw.id() == 1) {
	// cout << "1 -> 0 (" << sb.size() << " bytes)\n";
	nw.send_to(0, sb);
    } else {
	// cout << "0 <- 1 (" << sb.size() << " bytes)\n";
	nw.recv_from(1, rb);
	for (size_t i = 0; i < n; i++)
	    if (rb[i] != 1)
		return false;
    }

    return true;
}

size_t iter = 0;

bool test_exchange_all(Network &nw, size_t n)
{
    vector<vector<u8>> sbs (nw.size());
    vector<vector<u8>> rbs (nw.size());

    auto f = [](u8 i) {
	return (u8)(123 + i + iter);
    };

    iter++;

    for (size_t i = 0; i < nw.size(); i++) {
	rbs[i].resize(n);
	sbs[i] = vector<u8>(n, f(nw.id()));
    }

    nw.exchange_all(sbs, rbs);

    for (size_t i = 0; i < nw.size(); i++) {
	for (size_t j = 0; j < (size_t)n; j++) {
	    // cout << (int)i << ": " << (int)rbs[i][j] << ", ";
	    if (rbs[i][j] != f(i)) {
	    	cerr << "got=" << (int)rbs[i][j] << ", expected=" << (int)f(i) << "\n";
	    	return false;
	    }
	}
    }

    cout << endl;
    return true;
}

bool test_ring(Network &nw) {
    vector<u8> sb (n, 123);
    vector<u8> rb (n);

    nw.exchange_ring(sb, rb);

    for (size_t i = 0; i < n; i++) {
	if (rb[i] != 123)
	    return false;
    }

    return true;
}

int main(int argc, char **argv) {

    if (argc < 3) {
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

    REPEAT(10) {
    	result = test_self(nw);
    	cout << "test_self result: " << result << endl;
    }

    SLEEP_MS(100);

    REPEAT(10) {
    	result = test_simple(nw);
    	cout << "test_simple result: " << result << endl;
    	SLEEP_MS(100);
    }

    REPEAT(10) {
	result = test_exchange_all(nw, 10);
	cout << "test_exchange result (1024): " << result << endl;
	SLEEP_MS(100);
    }

    REPEAT(10) {
    	result = test_ring(nw);
    	cout << "test_ring result: " << result << endl;
    	SLEEP_MS(100);
    }

    // result = test_exchange_all(nw, n);
    // cout << "test_exchange result: " << result << endl;
}
