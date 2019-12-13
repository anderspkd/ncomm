#include "catch.hpp"
#include "../include/ncomm.hpp"

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

    chl.exchange(sbuf, rbuf);

    for (size_t i = 0; i < rbuf.size(); i++)
	REQUIRE(rbuf[i] == sbuf[i]);
}
