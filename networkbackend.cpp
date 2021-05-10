#include "networkbackend.h"

NetworkBackend::NetworkBackend(const std::string& ip, unsigned short port, QObject *parent): QObject(parent), ioc{std::make_shared<net::io_context>()}, work{std::make_shared<net::io_context::work>(*ioc)},
    serverConnectSocket{*ioc},
    resolver{*ioc}, server_ip{ip}, server_port{port}
{
    clearBuffer();
}

NetworkBackend::~NetworkBackend()
{
    std::cout << "[~] NETWORKBACKEND EXIT"<<std::endl;
    serverConnectSocket.close();
}

void NetworkBackend::onConnect()
{
    serverConnectSocket.set_option(net::ip::tcp::socket::reuse_address(true));
    sendDowngrade();
    doAsyncRead();
    //    getClientList();
}

void NetworkBackend::iocPool()
{

}


void NetworkBackend::handleResponse(boost::json::object &response)
{
    std::string response_type = "unknown_response";
    try {
        response_type = response["response"].get_string().data();

        if(response_type == "askPeer")
            askPeerResponse(response);
        else if(response_type == "clientList")
            clientListReceived(response);
        else if(response_type == "downgrade")
            std::cout << "[!] Downgrade reponse received!"<<std::endl;
        else
            std::cout << "Unknow Response "<< response_type << std::endl;

    }  catch (std::exception &ex) {
        std::cout <<"[x] Exception occured at Handle Response: "<< ex.what()<<std::endl;
    }
}

void NetworkBackend::doAsyncRead()
{
    std::cout << "[!] "<< __FUNCTION__ << std::endl;
    serverConnectSocket.async_read_some(net::buffer(_buffer), [&](boost::system::error_code ec, size_t size)
    {
        onRead(ec, size);
        doAsyncRead();
    });
}

void NetworkBackend::onRead(boost::system::error_code ec, size_t size)
{
    std::cout << "[!] onRead: "<< std::string(_buffer, 0, size) << std::endl;
    if(ec)
    {
        std::cout << "Error in reading" + ec.message();
        return;
    }
    if(!_buffer[0])
        return ;

    auto jsons = toJson(size);

    for(auto& json: jsons)
    {
        auto type = json["type"].as_string();
        if(type == "request")
            handleRequest(json);
        else if(type == "response")
            handleResponse(json);
        else
            std::cout << "[x] Invalid Json type"<< std::endl;
    }
}

void NetworkBackend::getClientList()
{
    boost::json::object json = {
        {"type", "request"},
        {"request", "clientList"}
    };

    doAsyncWrite(jsonToString(json));
}

boost::json::object NetworkBackend::createDownGradeRequest(const std::string username)
{
    boost::json::object json = {
        {"type", "request"},
        {"request", "downgrade"},
        {"private_ip", serverConnectSocket.local_endpoint().address().to_string()},
        {"private_port", serverConnectSocket.local_endpoint().port()},
        {"username", username}
    };
    return json;
}

void NetworkBackend::sendDowngrade()
{
    auto json = createDownGradeRequest(username);
    doAsyncWrite(json);
}

std::vector<boost::json::object> NetworkBackend::toJson(size_t size)
{
    std::string buffer(_buffer);
    std::vector<boost::json::object> jsons;

    try {
        boost::json::object json{{"type", "invalid"}};
        size_t pos = 0;
        std::string split;
        while((pos = buffer.find("\n"))!= std::string::npos)
        {
            split = buffer.substr(0, pos);
            json = boost::json::parse(split).get_object();
            jsons.push_back(json);
            buffer.erase(0, pos+1);
        }
        clearBuffer();
    }
    catch(std::exception& ex) {
        std::cout <<"Exception Occured while parsing json "<< ex.what() << std::endl;
    }
    return jsons;
}

std::string NetworkBackend::jsonToString(boost::json::object json)
{
    return boost::json::serialize(json).data();
}

void NetworkBackend::clearBuffer()
{
    memset(&_buffer, '\0', sizeof(_buffer));
}

void NetworkBackend::askPeerResponse(boost::json::object &response)
{
    auto data = response["data"].get_object();
    switch (data["status"].get_int64())
    {
    case ASK_PEER::ACCEPTED:
        doPostAcceptJson(data);
        break;
    case ASK_PEER::REJECTED:
        break;
    default:
        break;
    }
}

void NetworkBackend::doPostAcceptJson(boost::json::object &data)
{
    auto arr = peerJsonToArray(data["peer_data"].get_object());
    doPostAccept(arr);
}

void NetworkBackend::doAsyncWrite(const std::string buffer)
{
    serverConnectSocket.async_write_some(net::buffer(buffer.data(), buffer.size()), std::bind(&NetworkBackend::onWrite, this, std::placeholders::_1, std::placeholders::_2));
    std::cout <<"[>>] SENDING: "<< buffer <<std::endl;
}

void NetworkBackend::doAsyncWrite(boost::json::object json)
{
    auto buffer = jsonToString(json);
    doAsyncWrite(buffer);
}

void NetworkBackend::onWrite(boost::system::error_code ec, size_t size)
{
    if(ec)
        std::cout <<"Error Occured while writing, "<< ec.message()<<std::endl;
    std::cout <<"[!] Writed Size: "<< size <<std::endl;
}

void NetworkBackend::askPeerRequestReceived(boost::json::object &request, boost::json::object &response)
{
    onAcceptRequest(request, response);
}

void NetworkBackend::createSuccessResponse(boost::json::object &response, const std::string &responseType)
{    
    response.emplace("type", "response");
    response.emplace("response", responseType);
    response.emplace("data", true);
    response.emplace("error", false);
    response.emplace("message", "successful");
}

void NetworkBackend::createUnSuccessResponse(boost::json::object &response, const std::string &responseType, const std::string &message)
{
    response.emplace("type", "response");
    response.emplace("response", responseType);
    response.emplace("data", false);
    response.emplace("error", true);
    response.emplace("message", message);
}

void NetworkBackend::onAcceptRequest(boost::json::object &request, boost::json::object &response)
{
    auto arr = peerJsonToArray(request);

    createSuccessResponse(response, "askPeer");
    boost::json::object data = {
        {"status", 1},
        {"username", arr[4]}
    };
    response["data"] = data;

    doPostAccept(arr);
}

std::array<std::string, 5> NetworkBackend::peerJsonToArray(boost::json::object &data)
{
    const std::string username = data["username"].get_string().data();
    auto private_ip = data["private_ip"].as_string().data();
    auto public_ip = data["private_ip"].as_string().data();
    auto private_port = data["private_port"].as_int64();
    auto public_port = data["private_port"].as_int64();
    std::array<std::string, 5> arr = {private_ip, public_ip, std::to_string(private_port), std::to_string(public_port), username};
    return arr;
}

void NetworkBackend::clientListReceived(boost::json::object &response)
{
    emit peerListUpdated(response);
}

std::shared_ptr<boost::asio::io_context> &NetworkBackend::getioc()
{
    return ioc;
}

void NetworkBackend::doAskPeer(const std::string &username)
{
    boost::json::object json = {
        {"type", "request"},
        {"request", "askPeer"},
        {"username", username}
    };

    doAsyncWrite(json);
}

void NetworkBackend::doPostAccept(std::array<std::string, 5> arr)
{
    std::cout << " -- doPostAccept"<< std::endl;

    auto  peer = std::make_shared<PeerSession>(ioc, arr, serverConnectSocket.local_endpoint().port());
    connect(peer.get(), &PeerSession::connected, [&](std::unique_ptr<net::ip::tcp::socket> &socket, const std::string username)
    {
        emit connected(socket, username);
    });
    peer->doPostAccept();

}

void NetworkBackend::handleRequest(boost::json::object &request)
{
    boost::json::object response;
    std::string request_type = "invalid_request";
    try {
        request_type = request["request"].get_string().data();
        if(request_type == "askPeer")
            askPeerRequestReceived(request, response);

    }  catch (std::exception &ex) {
        createUnSuccessResponse(response, request_type, ex.what());
    }

    doAsyncWrite(jsonToString(response));
}

void NetworkBackend::run()
{
    iocPool();                                   // RUNNING THE IOC IN THREAD POOLS;
    connectToServer(server_ip, server_port);     // CONNECTION TO SERVER
    ioc->run();
}

void NetworkBackend::sendMsgToServer(void *msg, size_t size)
{

}

void NetworkBackend::setUsername(const std::string &_username)
{
    username = _username;
}

void  NetworkBackend::connectToServer(const std::string& ip, unsigned short port)
{
    while(!serverConnectSocket.is_open())
    {
        try
        {
            auto endpoint = *resolver.resolve(ip, std::to_string(port));
            serverConnectSocket.connect(endpoint);
            std::cout <<"[!] SUCCESSFULY CONNECTED TO SERVER: "<< ip<<":"<<port<<std::endl;
            onConnect();
            ioc->run();
            return;
        }
        catch(std::exception &ex)
        {
            std::cerr<< "Exception Occured with connecting Server: "<<ip<<":"<<port<<", "<< ex.what() << std::endl;
            std::this_thread::sleep_for(5s);
            serverConnectSocket.close();
        }
    }
}
