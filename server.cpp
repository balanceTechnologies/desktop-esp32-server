#include "server.hpp"
#include "connection.hpp"

#include <QTcpServer>
#include <QTcpSocket>

Server::Server(QObject *parent)
    : QObject{parent}
{
    tcpServer_ = new QTcpServer(this);

    connect(tcpServer_, &QTcpServer::pendingConnectionAvailable, this, &Server::addConnection);
}

bool Server::listen(quint16 port) {
    if (tcpServer_->isListening()) {
        tcpServer_->close();
    }

    return tcpServer_->listen(QHostAddress::Any, port);
}

void Server::broadcastCommand(server::outgoing::Command command) {
    connections_.subtract(removedConnections_);
    removedConnections_.clear();

    for (auto *connection : connections_) {
        if (!removedConnections_.contains(connection) && connection->connectionType() != Connection::ConnectionType::none) {
            connection->sendCommand(command);
        }
    }

    connections_.subtract(removedConnections_);
    removedConnections_.clear();
}

void Server::addConnection() {
    Connection *connection = new Connection(tcpServer_->nextPendingConnection(), this);
    connections_.insert(connection);

    connect(connection, &Connection::disconnected, this, &Server::removeConnection);
    connect(connection, &Connection::readyIncomingCommand, this, &Server::receiveCommand);
}

void Server::removeConnection() {
    Connection* connection = qobject_cast<Connection*>(sender());
    qDebug("removed %s %d", qUtf8Printable(connection->address()), connection->port());

    connection->deleteLater();
    removedConnections_.insert(connection);

    emit removedConnection(connection);
}

void Server::receiveCommand() {
    Connection* connection = qobject_cast<Connection*>(sender());
    emit receivedClientCommand(connection);
}
