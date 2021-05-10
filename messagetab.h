#ifndef MESSAGETAB_H
#define MESSAGETAB_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QDebug>

#include "networkbackend.h"

class MessageTab: public QWidget
{
    Q_OBJECT

    char buff[1024];
    QTextEdit chatBoxEdit;
    QLineEdit enterBoxEdit;
    QPushButton sendButton;
    QVBoxLayout *mainLayout;
    NetworkBackend *net;
    std::unique_ptr<net::ip::tcp::socket> socket;
    void render();
    void signalSetup();
    void sendText(QString msg);
    void onRead(boost::system::error_code ec, size_t size);
    void doRead();
public:
    QString username;
    void setSocket(std::unique_ptr<net::ip::tcp::socket> _socket);
    MessageTab(std::unique_ptr<net::ip::tcp::socket> _socket, NetworkBackend *_net, QWidget *parent = nullptr);
};

#endif // MESSAGETAB_H
