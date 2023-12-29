#include "connection.hpp"
#include "servercommands.hpp"

#include <QTcpSocket>

Connection::Connection(QTcpSocket *socket, QObject *parent)
    : QObject{parent}
    , socket_{socket}
{
    socket_->setParent(this);
    address_ = socket_->peerAddress().toString();
    port_ = socket_->peerPort();

    connect(socket_, &QAbstractSocket::disconnected, this, [&] { emit disconnected(); });
    connect(socket_, &QIODevice::readyRead, this, &Connection::readSocket);
}

QString Connection::address()
{
    return address_;
}

quint16 Connection::port()
{
    return port_;
}

Connection::ConnectionType Connection::connectionType() {
    return connectionType_;
}

std::optional<server::incoming::Command> Connection::incomingCommand()
{
    if (!incomingCommand_) {
        return std::nullopt;
    }

    auto command = std::move(incomingCommand_);
    incomingCommand_ = std::nullopt;
    return command;
}

void Connection::sendCommand(const server::outgoing::Command &command)
{
    using namespace server::outgoing;

    class Encoder
    {
    public:
        explicit Encoder(QIODevice *device)
            : ts_(device)
        {}

        void operator()(const Coord &command) { ts_ << "COORD " << command.x << ' ' << command.y << '\n'; }

        void operator()(const BalanceCoord &command)
        {
            ts_ << "BALANCE_COORD " << command.x << ' ' << command.y << '\n';
        }

    private:
        QTextStream ts_;
    };

    std::visit(Encoder(socket_), command);
}

static std::optional<server::incoming::Command> readSocketParse(QTcpSocket *socket)
{
    using namespace server::incoming;

    QTextStream ts(socket);

    socket->startTransaction();

    QString commandType;
    if ((ts >> commandType).status() != QTextStream::Status::Ok) {
        socket->commitTransaction();
        return std::nullopt;
    }

    // TEMPORARY
    if (commandType == "MOVE_LEFT") {
        socket->commitTransaction();
        return MoveLeft();
    } else if (commandType == "MOVE_RIGHT") {
        socket->commitTransaction();
        return MoveRight();
    } else if (commandType == "MOVE_FORWARD") {
        socket->commitTransaction();
        return MoveForward();
    } else if (commandType == "MOVE_BACKWARD") {
        socket->commitTransaction();
        return MoveBackward();
    }

    if (commandType == "CONNECT_DESKTOP") {
        socket->commitTransaction();
        return ConnectDesktop();
    } else if (commandType == "CONNECT_MOBILE") {
        socket->commitTransaction();
        return ConnectMobile();
    } else if (commandType == "SET_BALANCE_COORD") {
        quint32 x, y;
        QString xStr, yStr;
        ts >> xStr >> yStr;

        if (ts.status() == QTextStream::Status::Ok) {
            bool okX, okY;
            x = xStr.toLong(&okX);
            y = yStr.toLong(&okY);

            if (okX && okY && x > 0 && y > 0) {
                socket->commitTransaction();
                return SetBalanceCoord(x, y);
            }
        }
    }

    if (ts.status() == QTextStream::Status::ReadPastEnd) {
        socket->rollbackTransaction(); // rollback incomplete read
    } else {
        socket->commitTransaction(); // discard useless
    }

    return std::nullopt;
}

void Connection::readSocket()
{
    using namespace server::incoming;

    qDebug("socketRead %s %d", qUtf8Printable(address()), port());

    auto command = readSocketParse(socket_);
    if (command) {
        if (connectionType_ == ConnectionType::none) {
            if (std::holds_alternative<ConnectDesktop>(*command)) {
                connectionType_ = ConnectionType::desktop;
            } else if (std::holds_alternative<ConnectMobile>(*command)) {
                connectionType_ = ConnectionType::mobile;
            } else {
                socket_->close();
                return;
            }
        } else if (std::holds_alternative<ConnectDesktop>(*command)
                   || std::holds_alternative<ConnectMobile>(*command)) {
            socket_->close();
            return;
        }

        incomingCommand_ = command;
        emit readyIncomingCommand();
    }
}
