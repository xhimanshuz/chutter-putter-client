#include "PeerSession.h"


PeerSession::PeerSession(std::shared_ptr<boost::asio::io_context> _ioc, std::array< std::string, 5> askPeerData, unsigned short localPort, QObject *parent): ioc{_ioc},
    acceptor{std::make_unique<net::ip::tcp::acceptor>(*ioc, net::ip::tcp::acceptor::protocol_type::v4())}, public_socket{std::make_unique<net::ip::tcp::socket>(*ioc)},
    private_socket{std::make_unique<net::ip::tcp::socket>(*ioc)}, acceptor_port{localPort}, private_ip{askPeerData[0]},
    public_ip{askPeerData[1]}, private_port{std::atoi(askPeerData[2].c_str())}, public_port{std::atoi(askPeerData[3].c_str())}, username{""}, QObject(parent)
{
    std::cout << "[!] Peer Session Started Username: "<< username <<std::endl;
}

PeerSession::~PeerSession()
{
    std::cerr << "PeerSession::~PeerSession()"<<std::endl;
}

void PeerSession::doPostAccept()
{
    std::thread(std::bind(&PeerSession::doConnect, shared_from_this(), std::ref(private_socket), net::ip::tcp::endpoint(net::ip::address::from_string(public_ip), public_port))).detach();

    std::thread(std::bind(&PeerSession::doConnect, shared_from_this(), std::ref(private_socket), net::ip::tcp::endpoint(net::ip::address::from_string(private_ip), private_port))).detach();

    doAccepting();

}

void PeerSession::doAccepting()
{
    std::cout << __FUNCTION__ << "()"<< std::endl;
    acceptor->set_option(net::ip::tcp::acceptor::reuse_address(true));
    auto ep = net::ip::tcp::endpoint{net::ip::address_v4(), acceptor_port};
    acceptor->bind(ep);
    acceptor->listen();
    acceptor->async_accept(std::bind(&PeerSession::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void PeerSession::onAccept(boost::system::error_code ec, boost::asio::ip::tcp::socket acceptedSocket)
{
    if(ec)
    {
        std::cout << "Error While Accepting "<< ec.message()<<std::endl;
        return;
    }
    IS_CONNECTED = true;
    std::cout << "[!] ACCEPTED ON: "<< acceptedSocket.remote_endpoint().address().to_string()<<":"<< acceptedSocket.remote_endpoint().port()<<std::endl;
    auto su = std::make_unique<boost::asio::ip::tcp::socket>(std::move(acceptedSocket));
    emit connected(su);
}

void PeerSession::doConnect(std::unique_ptr<boost::asio::ip::tcp::socket>& socket, net::ip::tcp::endpoint ep)
{
    std::cout << __FUNCTION__ << "()" << ep.port() << std::endl;
    while(!IS_CONNECTED && !socket->is_open() )
    {
        try {
            socket->connect(ep);
            onConnect(socket);
            return;
        }  catch (std::exception ex) {
            std::cout <<"Exception while connecting."<< ex.what()<<std::endl;
//            socket->close();
            std::this_thread::sleep_for(3s);
        }
    }
//    std::cout << "[~] doConnect: "<< IS_CONNECTED << " "<< socket->is_open()<<std::endl;
}


void PeerSession::connectPublicEndpoint()
{

}

void PeerSession::onConnect(std::unique_ptr<net::ip::tcp::socket> &socket)
{
    IS_CONNECTED = true;
//    _socket = std::move();
    std::cout << "[!] CONNECTED ON: "<< socket->remote_endpoint().address().to_string()<<":"<< socket->remote_endpoint().port()<<std::endl;

    emit connected(socket);

}
