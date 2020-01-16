#include "catch.hpp"
#include "../include/ncomm.hpp"

#include <thread>
#include <iostream>

using namespace ncomm;
using namespace std;
typedef unsigned char u8;

TEST_CASE("DummyChannel init") {
    DummyChannel chl {42};

    REQUIRE(chl.remote_id() == 42);
    REQUIRE(chl.local_id() == 42);
    REQUIRE(chl.is_alive() == false);

    chl.connect();

    REQUIRE(chl.is_alive() == true);

    chl.close();

    REQUIRE(chl.is_alive() == false);
}

TEST_CASE("DummyChannel communicate") {
    DummyChannel chl {42};

    vector<u8> sbuf {1,2,3};
    vector<u8> rbuf (sbuf.size());

    chl.connect();

    chl.send(sbuf);
    chl.recv(rbuf);

    for (size_t i = 0; i < rbuf.size(); i++)
	REQUIRE(rbuf[i] == sbuf[i]);

    rbuf.clear();

    chl.send(sbuf);
    chl.recv(rbuf);

    for (size_t i = 0; i < rbuf.size(); i++)
	REQUIRE(rbuf[i] == sbuf[i]);
}

TEST_CASE("network info stuff") {

    Network nw (0, 10, 12345);

    REQUIRE(nw.id() == 0);
    REQUIRE(nw.size() == 10);
    REQUIRE(nw.base_port() == 12345);
    nw.base_port() = 1000;
    REQUIRE(nw.base_port() == 1000);
}

TEST_CASE("self comm 3") {

    const size_t n = 3;

    vector<thread*> parties (n);
    vector<bool> results (n, true);

    auto h = [&](partyid_t id) {
	Network nw (id, n, 5000);
	nw.connect();
	vector<u8> sb (10000, (u8)id);
	vector<u8> rb (10000);

	nw.send_to(id, sb);
	nw.recv_from(id, rb);

	for (size_t i = 0; i < rb.size(); i++) {
	    bool x = sb[i] == rb[i];
	    x = x and (rb[i] == id);
	    results[id] = x and results[id];
	}
    };

    cout << "self comm 3 parties\n";

    for (size_t i = 0; i < n; i++) {
	parties[i] = new thread(h, i);
    }

    for (size_t i = 0; i < n; i++) {
	parties[i]->join();
    }

    for (size_t i = 0; i < n; i++) {
	REQUIRE(results[i]);
    }
}

TEST_CASE("self comm 7") {

    const size_t n = 7;

    vector<thread*> parties (n);
    vector<bool> results (n, true);

    auto h = [&](partyid_t id) {
	Network nw (id, n, 5500);
	nw.connect();
	vector<u8> sb (10000, (u8)id);
	vector<u8> rb (10000);

	nw.send_to(id, sb);
	nw.recv_from(id, rb);

	for (size_t i = 0; i < rb.size(); i++) {
	    bool x = sb[i] == rb[i];
	    x = x and (rb[i] == id);
	    results[id] = x and results[id];
	}
    };

    cout << "self comm 7 parties\n";

    for (size_t i = 0; i < n; i++) {
	parties[i] = new thread(h, i);
    }

    for (size_t i = 0; i < n; i++) {
	parties[i]->join();
    }

    for (size_t i = 0; i < n; i++) {
	REQUIRE(results[i]);
    }
}

TEST_CASE("ring comm", "[3 parties]") {

    const size_t n = 3;

    vector<thread*> parties (n);
    vector<bool> results (n, true);

    auto h = [&](partyid_t id) {
	Network nw (id, n, 5700);
	nw.connect();
	vector<u8> sb (10, (u8)id);
	vector<u8> rb (10);

	nw.exchange_ring(sb, rb);

	for (size_t i = 0; i < rb.size(); i++) {
	    // default exchange order is downwards.
	    bool x = rb[i] == nw.ident_of_next();
	    results[id] = x and results[id];
	}
    };

    cout << "ring comm 3 parties\n";

    for (size_t i = 0; i < n; i++) {
	parties[i] = new thread(h, i);
    }

    for (size_t i = 0; i < n; i++) {
	parties[i]->join();
    }

    for (size_t i = 0; i < n; i++) {
	REQUIRE(results[i]);
    }
}
