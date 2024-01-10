#include "centralwidget.hpp"
#include "serialcomm.hpp"
#include "server.hpp"
#include "connection.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QIcon>
#include <QTime>

#include <type_traits>

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget{parent}
{
    auto serialPortLabel = new QLabel("Serial port:");
    serialPortNameLineEdit_ = new QLineEdit();
    auto serialOpenButton = new QPushButton("Open");

    auto *serialLayout = new QHBoxLayout();
    serialLayout->addWidget(serialPortLabel);
    serialLayout->addWidget(serialPortNameLineEdit_);
    serialLayout->addWidget(serialOpenButton);

    connectionList_ = new QListWidget();
    noneItem_ = new QListWidgetItem(QIcon::fromTheme("call-stop"), "None", connectionList_);
    connectionList_->setCurrentItem(noneItem_);
    listItemToConnection_.insert(noneItem_, nullptr);

    auto *controlLayout = new QVBoxLayout();
    controlLayout->addLayout(serialLayout);
    controlLayout->addWidget(connectionList_);

    auto *serialReceiveLayout = new QVBoxLayout();
    serialReceiveLayout->addWidget(new QLabel("Serial Receive"));
    serialReceiveTextEdit_ = new QTextEdit();
    serialReceiveTextEdit_->setReadOnly(true);
    serialReceiveLayout->addWidget(serialReceiveTextEdit_);

    auto *serialSendLayout = new QVBoxLayout();
    serialSendLayout->addWidget(new QLabel("Serial Send"));
    serialSendTextEdit_ = new QTextEdit();
    serialSendTextEdit_->setReadOnly(true);
    serialSendLayout->addWidget(serialSendTextEdit_);

    auto *clientReceiveLayout = new QVBoxLayout();
    clientReceiveLayout->addWidget(new QLabel("Client Receive"));
    clientReceiveTextEdit_ = new QTextEdit();
    clientReceiveTextEdit_->setReadOnly(true);
    clientReceiveLayout->addWidget(clientReceiveTextEdit_);

    auto *clientSendLayout = new QVBoxLayout();
    clientSendLayout->addWidget(new QLabel("Client Send"));
    clientSendTextEdit_ = new QTextEdit();
    clientSendTextEdit_->setReadOnly(true);
    clientSendLayout->addWidget(clientSendTextEdit_);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(serialReceiveLayout);
    mainLayout->addLayout(serialSendLayout);
    mainLayout->addLayout(clientReceiveLayout);
    mainLayout->addLayout(clientSendLayout);

    setLayout(mainLayout);

    serial_ = new SerialComm(115200, this);
    server_ = new Server(this);

    connect(connectionList_, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *){
        toggled_ = listItemToConnection_[current];
        qDebug("%s toggled", qUtf8Printable(current->text()));
    });

    connect(serialOpenButton, &QPushButton::clicked, this, &CentralWidget::openSerial);
    connect(serial_, &SerialComm::readyIncomingCommand, this, &CentralWidget::receiveSerialCommand);

    connect(server_, &Server::receivedClientCommand, this, &CentralWidget::receiveClientCommand);

    connect(server_, &Server::removedConnection, this, [this](Connection *connection) {
        auto expiredItem = connectionToListItem_.take(connection);
        listItemToConnection_.remove(expiredItem);

        if (toggled_ == connection) {
            toggled_ = nullptr;
            connectionList_->setCurrentItem(noneItem_);
        }

        QTime now = QTime::currentTime();
        auto log = QString("%1 %2 %3 closed").arg(now.toString("HH:mm:ss")).arg(connection->address()).arg(connection->port());

        clientReceiveTextEdit_->append(log);

        delete expiredItem;
    });

    server_->listen(8080);
}

void CentralWidget::openSerial() {
    serial_->open(serialPortNameLineEdit_->text());
}

void CentralWidget::receiveClientCommand(Connection *sender) {
    using namespace server::incoming;

    QTime now = QTime::currentTime();
    auto log = QString("%1 %2 %3").arg(now.toString("HH:mm:ss")).arg(sender->address()).arg(sender->port());

    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ConnectDesktop>) {
            Q_ASSERT(sender->connectionType() != Connection::ConnectionType::none);

            auto newItem = new QListWidgetItem(QIcon::fromTheme("computer"), QString("%1 %2").arg(sender->address()).arg(sender->port()), connectionList_);
            listItemToConnection_.insert(newItem, sender);
            connectionToListItem_.insert(sender, newItem);

            log.append(" CONNECT_DESKTOP");
        } else if constexpr (std::is_same_v<T, ConnectMobile>) {
            Q_ASSERT(sender->connectionType() != Connection::ConnectionType::none);

            auto newItem = new QListWidgetItem(QIcon::fromTheme("phone"), QString("%1 %2").arg(sender->address()).arg(sender->port()), connectionList_);
            listItemToConnection_.insert(newItem, sender);
            connectionToListItem_.insert(sender, newItem);

            log.append(" CONNECT_MOBILE");
        } else if constexpr (std::is_same_v<T, SetBalanceCoord>) {
            auto command = QString("SET_BALANCE_COORD %1 %2").arg(arg.x).arg(arg.y);
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::SetBalanceCoord(arg.x, arg.y));
                serialSendTextEdit_->append(log + ' ' + command);
            }

            log.append(' ').append(command);
        } else if constexpr (std::is_same_v<T, MoveLeft>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveLeft());
                serialSendTextEdit_->append(log + " MOVE_LEFT");
            }

            log.append(" MOVE_LEFT");
        } else if constexpr (std::is_same_v<T, MoveRight>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveRight());
                serialSendTextEdit_->append(log + " MOVE_RIGHT");
            }

            log.append(" MOVE_RIGHT");
        } else if constexpr (std::is_same_v<T, MoveForward>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveForward());
                serialSendTextEdit_->append(log + " MOVE_FORWARD");
            }

            log.append(" MOVE_FORWARD");
        } else if constexpr (std::is_same_v<T, MoveBackward>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveBackward());
                serialSendTextEdit_->append(log + " MOVE_BACKWARD");
            }

            log.append(" MOVE_BACKWARD");
        } else {
            static_assert(!sizeof(T), "non-exhaustive visitor");
        }
    }, *sender->incomingCommand());

    clientReceiveTextEdit_->append(log);
}

void CentralWidget::receiveSerialCommand() {
    using namespace esp32::incoming;

    auto now = QTime::currentTime();
    auto log = QString("%1 %2 %3").arg(now.toString("HH:mm:ss"));

    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Coord>) {
            log.append(QString(" COORD %1 %2").arg(arg.x).arg(arg.y));

            server_->broadcastCommand(server::outgoing::Coord(arg.x, arg.y));

            clientSendTextEdit_->append(log);
        } else if constexpr (std::is_same_v<T, BalanceCoord>) {
            log.append(QString(" BALANCE_COORD %1 %2").arg(arg.x).arg(arg.y));

            server_->broadcastCommand(server::outgoing::BalanceCoord(arg.x, arg.y));

            clientSendTextEdit_->append(log);
        } else {
            static_assert(!sizeof(T), "non-exhaustive visitor");
        }
    }, *serial_->incomingCommand());

    serialReceiveTextEdit_->append(log);
}
