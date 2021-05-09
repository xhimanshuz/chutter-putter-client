#ifndef NETWORKBACKEND_H
#define NETWORKBACKEND_H

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/json/parse.hpp>
#include <iostream>
#include <chrono>
#include <array>

#include <QObject>
#include "PeerSession.h"

class Ui;

enum ASK_PEER: int
{
    REJECTED = 0,
    ACCEPTED = 1,
    WAITING = 2
};

namespace net = boost::asio;
using namespace std::literals::chrono_literals;

class NetworkBackend: public QObject
{
    Q_OBJECT

    std::shared_ptr<net::io_context> ioc;
    std::shared_ptr<net::io_context::work> work;
    net::ip::tcp::socket serverConnectSocket;
    std::shared_ptr<net::ip::tcp::acceptor> acceptor;
    net::ip::tcp::endpoint ep;
    boost::asio::ip::tcp::resolver resolver;
    std::string server_ip;
    unsigned short server_port;
    char _buffer[1024];
    bool is_connect;
    std::string username;

    void onConnect();
    void iocPool();
    void handleRequest(boost::json::object &request);
    void handleResponse(boost::json::object &response);
    void doAsyncRead();
    void onRead(boost::system::error_code ec, size_t size);
    void getClientList();
    boost::json::object createDownGradeRequest(const std::string username = "");
    void sendDowngrade();
    boost::json::object toJson(size_t size);
    std::string jsonToString(boost::json::object json);
    void clearBuffer();
    void askPeerResponse(boost::json::object &response);
    void doPostAcceptJson(boost::json::object &response);
    void doAsyncWrite(const std::string buffer) ;
    void doAsyncWrite(boost::json::object json);
    void onWrite(boost::system::error_code ec, size_t size);
    void askPeerRequestReceived(boost::json::object &request, boost::json::object &response);
    void createSuccessResponse(boost::json::object &response, const std::string& responseType);
    void createUnSuccessResponse(boost::json::object &response, const std::string& responseType, const std::string &message);
    void onAcceptRequest(boost::json::object &request, boost::json::object &response);
    std::array<std::string, 5> peerJsonToArray(boost::json::object &data);
    void clientListReceived(boost::json::object& response);
public:
    std::shared_ptr<net::io_context>& getioc();
    explicit NetworkBackend(const std::string& ip, unsigned short port, QObject *parent = nullptr);
    virtual ~NetworkBackend();
    void connectToServer(const std::string& ip, unsigned short port);
    void run();
    void sendMsgToServer(void *msg, size_t size);
    void setUsername(const std::string& _username);
    void doAskPeer(const std::string &username);
    void doPostAccept(std::array<std::string, 5> arr);

signals:
    void peerListUpdated(boost::json::object json);
    void connectedToServerSignal(const std::string endpoint);
    void connected(std::unique_ptr<net::ip::tcp::socket> &socket);
};

#endif // NETWORKBACKEND_H
