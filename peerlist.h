#ifndef PEERLIST_H
#define PEERLIST_H

#include <QWidget>
#include <QTableView>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>

#include <boost/json/object.hpp>

class PeerList: public QWidget
{
    Q_OBJECT

    std::vector<QStringList> clientData;

    void render();
    void setupSignal();

    QVBoxLayout *mainLayout;
    QTableView *clientListView;
    void fillData();
    QStandardItemModel model;
    void jsonToVector(boost::json::object &json);

public:
    PeerList(QWidget *parent = nullptr);
    void setList(boost::json::object json);

signals:
    void peerSelected(const std::string username);
};

#endif // PEERLIST_H
