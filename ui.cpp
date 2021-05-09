#include "ui.h"

Ui::Ui(QWidget *parent) : QWidget(parent)
{
    net = std::make_shared<NetworkBackend>("127.0.0.1", 8000, this);
    username = QInputDialog::getText(this, "Enter Username", "Username: ");
    std::cout << "[!] Username: "<< username.toStdString()<<std::endl;
    render();
    setLayout(&mainLayout);
    setUpSignals();
    net->setUsername(username.toStdString());
    std::thread(&NetworkBackend::run, net).detach();
}

void Ui::render()
{
    mainLayout.addWidget(new QLabel(QString("<h1>CHUTTER-PUTTER - [%0]</H1>").arg(username)));
    mainLayout.addWidget(&pl);
    mainLayout.addWidget(&tabWidget);

    mt = new MessageTab(std::make_unique<boost::asio::ip::tcp::socket>(*(net->getioc())), net.get(), this);
    addMessangerTab("peer");
}

void Ui::addMessangerTab(const QString &tabname)
{
    tabWidget.addTab( mt, tabname);
}

void Ui::setUpSignals()
{
    connect(net.get(), &NetworkBackend::connectedToServerSignal, [&](const std::string ip){
        QMessageBox mb(this);
        mb.setText(ip.data());
        mb.exec();
    });

    connect(net.get(), &NetworkBackend::peerListUpdated, [&](boost::json::object json){
        pl.setList(json);
    });

    connect(&pl, &PeerList::peerSelected, net.get(), &NetworkBackend::doAskPeer);

    connect(net.get(), &NetworkBackend::connected, [&](std::unique_ptr<net::ip::tcp::socket> &socket) {
        std::cout << "[!] Socket Transfered, "<< socket->remote_endpoint()<< std::endl;
        mt->setSocket(std::move(socket));
//        tabWidget.addTab( new MessageTab(std::move(socket), net.get(), this), "connected");
    });
}
