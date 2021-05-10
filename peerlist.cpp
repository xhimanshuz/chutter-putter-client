#include "peerlist.h"

PeerList::PeerList(QWidget *parent): QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    render();
    setLayout(mainLayout);
}

void PeerList::setList(boost::json::object json)
{
    jsonToVector(json);
    fillData();
}

void PeerList::render()
{
    clientListView = new QTableView;
    clientListView->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
    clientListView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    clientListView->setEditTriggers(QTableView::NoEditTriggers);
    clientListView->setModel(&model);
//    fillData();
    mainLayout->addWidget(clientListView);

    setupSignal();
}



void PeerList::setupSignal()
{
    connect(clientListView, &QTableView::doubleClicked, [&](const QModelIndex &index){
        auto username = model.data(model.index(index.row(), 0)).toString().toStdString();
        emit peerSelected(username);
    });
}

void PeerList::fillData()
{
    model.clear();
    model.setHorizontalHeaderLabels(QStringList()<<"Username"<<"Public IP"<<"Private IP"<< "Public Port"<< "Private Port");
    for(auto& cd: clientData)
    {
        QList<QStandardItem*> list= {new QStandardItem(cd.at(0)), new QStandardItem(cd.at(1)),
                                    new QStandardItem(cd.at(2)), new QStandardItem(cd.at(3)),
                                    new QStandardItem(cd.at(4))};
        model.appendRow(list);
    }

    clientListView->viewport()->update();
}

void PeerList::jsonToVector(boost::json::object &json)
{
    clientData.clear();
    auto arr = json["data"].get_object()["clients"].as_array();
    for(auto& cl: arr)
    {
        auto client = cl.get_object();
        auto username = client["name"].get_string().data();
        auto pub_ip = client["public_ip"].get_string().data();
        auto pub_port = client["public_port"].get_int64();
        auto pri_ip = client["private_ip"].get_string().data();
        auto pri_port = client["public_port"].get_int64();
        clientData.push_back({username, pub_ip, std::to_string(pub_port).data(), pri_ip, std::to_string(pri_port).data()});
    }
}


