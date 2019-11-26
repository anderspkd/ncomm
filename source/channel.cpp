#include "../include/ncomm.hpp"

namespace ncomm {

string channel_info_t::to_string() const {
    std::stringstream ss;
    ss << "<id=" << id << ", " <<
	"addr=\"" << hostname << "\", " <<
	"port=" << port << ", " <<
	"role=" << role << ">";
    return ss.str();
}

const channel_info_t& Channel::ServerInfo() const {
    return server_info;
}

const channel_info_t& Channel::ClientInfo() const {
    return client_info;
}

partyid_t Channel::GetRemoteId() const {
    return remote_id;
}

partyid_t Channel::GetLocalId() const {
    return local_id;
}

bool Channel::IsAlive() const {
    return this->alive;
}

void Channel::Exchange(const vector<u8> &sbuf, vector<u8> &rbuf) {
    this->Send(sbuf);
    this->Recv(rbuf);
}

channel_info_t DummyChannel::DummyInfo(const partyid_t id) {
    channel_info_t info = {
	.id = id,
	.port = -1,
	.hostname = "",
	.role = DUMMY
    };
    return info;
}

void DummyChannel::Connect() {
    this->alive = true;
}

void DummyChannel::Close() {
    this->alive = false;
}

void DummyChannel::Send(const vector<u8> &buf) {
    buffer = buf;
}

void DummyChannel::Recv(vector<u8> &buf) {
    buf = buffer;
    buffer.clear();
}

void AsioChannel::ConnectClient() {

    auto info = ClientInfo();
    NCOMM_L("client: %s", info.to_string().c_str());

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

    auto info = ServerInfo();
    NCOMM_L("server: %s", info.to_string().c_str());

    tcp::acceptor acceptor(ios_receiver, tcp::endpoint(tcp::v4(), info.port));
    rsock = new tcp::socket(ios_receiver);
    acceptor.accept(*rsock);
    boost::asio::ip::tcp::no_delay no_delay(true);
    rsock->set_option(no_delay);
}

void AsioChannel::Connect() {

    if (alive)
	return;

    // validate connection information
    auto sinfo = ServerInfo();
    auto cinfo = ClientInfo();

    if (sinfo.role == cinfo.role)
	throw std::runtime_error("remote and local with same role");

    if (sinfo.id == cinfo.id)
	throw std::runtime_error("use DummyChannel when connecting to self");

    if (sinfo.id < cinfo.id) {
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
