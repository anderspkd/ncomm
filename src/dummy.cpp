#include "ncomm.hpp"

namespace ncomm {

void DummyChannel::Send(const vector<u8> &buf) {
    buffer = buf;
}

void DummyChannel::Recv(vector<u8> &buf) {
    buf = buffer;
    buffer.clear();
}

} // ncomm
