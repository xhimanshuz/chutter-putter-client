#include "messagetab.h"


MessageTab::MessageTab(std::unique_ptr<net::ip::tcp::socket> _socket, NetworkBackend *_net, QWidget *parent): QWidget(parent), net(_net), socket(std::move(_socket))
{
    mainLayout = new QVBoxLayout(this);
    render();
    this->setLayout(mainLayout);
}

void MessageTab::render()
{
    sendButton.setText(">");
    mainLayout->addWidget(&chatBoxEdit);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(&enterBoxEdit);
    hbox->addWidget(&sendButton);
    mainLayout->addLayout(hbox);

    signalSetup();
}

void MessageTab::signalSetup()
{
    connect(&sendButton, &QPushButton::clicked, &enterBoxEdit, &QLineEdit::returnPressed);

    connect(&enterBoxEdit, &QLineEdit::returnPressed, [&]{
        if(enterBoxEdit.text().isEmpty())
            return ;
        sendText(enterBoxEdit.text());
        doRead();
        enterBoxEdit.clear();
    });
}

void MessageTab::sendText(QString msg)
{
    socket->write_some(boost::asio::buffer(msg.toUtf8().data(), sizeof(msg.toUtf8().data())));
    std::cout << "[>>] SENT: "<< msg.toStdString()<<std::endl;;
    chatBoxEdit.append(QString("<div align='right'><b>%0</b></div>").arg(msg));
    chatBoxEdit.viewport()->update();
}

void MessageTab::onRead(boost::system::error_code ec, size_t size)
{
    std::cout << "onRead: "<< buff <<std::endl;
    if(ec)
        std::cout << "[x] Error in Messange::onRead"<< ec.message()<< std::endl;
    QString msg = QString::fromUtf8(buff, size);
    chatBoxEdit.append(QString("<div align='left'><b>%0</b></div>").arg(msg));
    chatBoxEdit.viewport()->update();
    memset(buff, 0, sizeof(buff));
    doRead();

}

void MessageTab::doRead()
{
    socket->async_read_some(net::buffer(buff), std::bind(&MessageTab::onRead, this, std::placeholders::_1, std::placeholders::_2));
}

void MessageTab::setSocket(std::unique_ptr<boost::asio::ip::tcp::socket> _socket)
{
    socket.swap(_socket);
}
