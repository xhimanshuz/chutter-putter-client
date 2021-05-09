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
        std::cout <<"[>>] SENT: "<< enterBoxEdit.text().toStdString() <<std::endl;
        enterBoxEdit.clear();
    });
}

void MessageTab::sendText(QString msg)
{
    socket->write_some(boost::asio::buffer(enterBoxEdit.text().toStdString().data(), enterBoxEdit.text().size()));
}

void MessageTab::onRead(boost::system::error_code ec, size_t size)
{
    std::cout << "onRead: "<< buff <<std::endl;
    if(ec)
        std::cout << "[x] Error in Messange::onRead"<< ec.message()<< std::endl;
    std::string msg(buff, size);
    chatBoxEdit.appendPlainText(QString(msg.data()));
    chatBoxEdit.update();
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
