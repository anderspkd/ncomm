#include "catch.hpp"
#include "../include/ncomm.hpp"

using namespace ncomm;

TEST_CASE("DummyChannel init") {
    DummyChannel chl {42};

    REQUIRE(chl.GetRemoteId() == 42);
    REQUIRE(chl.GetLocalId() == 42);
    REQUIRE(chl.IsAlive() == false);

    chl.Connect();

    REQUIRE(chl.IsAlive() == true);

    chl.Close();

    REQUIRE(chl.IsAlive() == false);
}

TEST_CASE("DummyChannel communicate") {
    DummyChannel chl {42};

    vector<u8> sbuf {1,2,3};
    vector<u8> rbuf (sbuf.size());

    chl.Connect();

    chl.Send(sbuf);
    chl.Recv(rbuf);

    for (size_t i = 0; i < rbuf.size(); i++)
	REQUIRE(rbuf[i] == sbuf[i]);

    rbuf.clear();

    chl.Exchange(sbuf, rbuf);

    for (size_t i = 0; i < rbuf.size(); i++)
	REQUIRE(rbuf[i] == sbuf[i]);
}
