#ifndef UI_H
#define UI_H

#include <QWidget>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

#include "messagetab.h"
#include "networkbackend.h"
#include "peerlist.h"

class Ui : public QWidget
{
    Q_OBJECT

    QVBoxLayout mainLayout;
    QTabWidget tabWidget;
    std::shared_ptr<NetworkBackend> net;
    PeerList pl;
    QString username;

    void render();
    void addMessangerTab(const QString &tabname);
    void setUpSignals();
    MessageTab *mt;
public:
    explicit Ui(QWidget *parent = nullptr);


signals:
    void connected(const std::string ip);
};

#endif // UI_H
