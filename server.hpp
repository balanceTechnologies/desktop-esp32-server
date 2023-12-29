#pragma once

#include "servercommands.hpp"

#include <QObject>
#include <QSet>

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QTcpServer;
QT_END_NAMESPACE

class Connection;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    bool listen(quint16 port);
    void broadcastCommand(server::outgoing::Command command);

signals:
    void receivedClientCommand(Connection *sender);
    void newConnected(Connection *connection);
    void removedConnection(Connection *connection);

private slots:
    void addConnection();
    void removeConnection();
    void receiveCommand();

private:
    QTcpServer *tcpServer_;
    QSet<Connection*> connections_;
    QSet<Connection*> removedConnections_;
};
