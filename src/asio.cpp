#include "ncomm.hpp"

namespace ncomm {

void AsioChannel::ConnectClient() {
    auto info = RemoteInfo();
    const auto addr = boost::asio::ip::address::from_string(info.hostname);
    tcp::endpoint ep(addr, info.port);
    ssock = new tcp::socket(ios_sender);
    boost::system::error_code err = boost::asio::error::host_not_found;
    while (err) {
	ssock->close();
	ssock->connect(ep, err);
    }
    boost::asio::ip::tcp::no_delay no_delay(true);
    ssock->set_option(no_delay);
}

void AsioChannel::ConnectServer() {
    auto info = LocalInfo();
    tcp::acceptor acceptor(ios_receiver, tcp::endpoint(tcp::v4(), info.port));
    rsock = new tcp::socket(ios_receiver);
    acceptor.accept(*rsock);
    boost::asio::ip::tcp::no_delay no_delay(true);
    rsock->set_option(no_delay);
}

void AsioChannel::Connect() {

    // validate connection information
    auto linfo = LocalInfo();
    auto rinfo = RemoteInfo();

    if (linfo.role == rinfo.role)
	throw std::runtime_error("remote and local with same role");

    if (linfo.id == rinfo.id)
	throw std::runtime_error("use DummyChannel when connecting to self");

    if (linfo.id < rinfo.id) {
	ConnectServer();
	ConnectClient();
    } else {
	ConnectClient();
	ConnectServer();
    }
}

void AsioChannel::Send(const vector<u8> &buf) {
    boost::system::error_code err;
    boost::asio::write(*ssock,
		       boost::asio::buffer(buf),
		       boost::asio::transfer_all(),
		       err);
    (void)err;
}

void AsioChannel::Recv(vector<u8> &buf) {
    boost::asio::read(*rsock, boost::asio::buffer(buf, buf.size()));
}

} // ncomm
