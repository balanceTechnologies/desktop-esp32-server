#pragma once

#include "servercommands.hpp"

#include <QObject>

#include <optional>

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QTextStream;
QT_END_NAMESPACE

class Connection : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionType {
        none,
        desktop,
        mobile,
    };

    explicit Connection(QTcpSocket *socket, QObject *parent = nullptr);

    QString address();
    quint16 port();
    ConnectionType connectionType();
    std::optional<server::incoming::Command> incomingCommand();
    void sendCommand(const server::outgoing::Command &command);

signals:
    void readyIncomingCommand();
    void disconnected();

private slots:
    void readSocket();

private:
    QTcpSocket *socket_;
    QString address_;
    quint16 port_;
    std::optional<server::incoming::Command> incomingCommand_;
    ConnectionType connectionType_ = ConnectionType::none;
};
