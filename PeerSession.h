#ifndef PEERSESSION_H
#define PEERSESSION_H

#include <QObject>

#include <iostream>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <array>


namespace net = boost::asio;
using namespace std::literals::chrono_literals;

class PeerSession:  public QObject, public std::enable_shared_from_this<PeerSession>
{
    Q_OBJECT

    std::shared_ptr<net::io_context> ioc;
    std::unique_ptr<net::ip::tcp::acceptor> acceptor;
    std::unique_ptr<net::ip::tcp::socket> public_socket;
    std::unique_ptr<net::ip::tcp::socket> private_socket;
    std::unique_ptr<net::ip::tcp::socket> _socket;
    uint16_t acceptor_port;
    bool IS_CONNECTED;

    std::string private_ip;
    std::string public_ip;
    int private_port;
    int public_port;
    std::string username;

    void doAccepting();
    void onAccept(boost::system::error_code ec, net::ip::tcp::socket acceptedSocket);
    void doConnect(std::unique_ptr<net::ip::tcp::socket>& socket, net::ip::tcp::endpoint ep);
    void connectPublicEndpoint();
    void onConnect(std::unique_ptr<net::ip::tcp::socket> &socket);
public:
    PeerSession(std::shared_ptr<net::io_context> _ioc, std::array< std::string, 5> askPeerData
                , uint16_t localPort, QObject *parent = nullptr);
    virtual ~PeerSession();
    void doPostAccept();
signals:
    void connected(std::unique_ptr<net::ip::tcp::socket> &socket);
};

#endif // PEERSESSION_H
