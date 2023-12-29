#include "centralwidget.hpp"
#include "serialcomm.hpp"
#include "server.hpp"
#include "connection.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QIcon>

#include <type_traits>

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget{parent}
{
    auto serialPortLabel = new QLabel("Serial port:");
    serialPortNameLineEdit_ = new QLineEdit();
    auto serialOpenButton = new QPushButton("Open");

    QHBoxLayout *serialLayout = new QHBoxLayout();
    serialLayout->addWidget(serialPortLabel);
    serialLayout->addWidget(serialPortNameLineEdit_);
    serialLayout->addWidget(serialOpenButton);

    connectionList_ = new QListWidget();
    noneItem_ = new QListWidgetItem(QIcon::fromTheme("call-stop"), "None", connectionList_);
    connectionList_->setCurrentItem(noneItem_);
    listItemToConnection_.insert(noneItem_, nullptr);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(serialLayout);
    mainLayout->addWidget(connectionList_);

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

        delete expiredItem;
    });

    server_->listen(8080);
}

void CentralWidget::openSerial() {
    serial_->open(serialPortNameLineEdit_->text());
}

void CentralWidget::receiveClientCommand(Connection *sender) {
    using namespace server::incoming;

    qDebug("TCP sender %s %d", qUtf8Printable(sender->address()), sender->port());

    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ConnectDesktop>) {
            Q_ASSERT(sender->connectionType() != Connection::ConnectionType::none);

            auto newItem = new QListWidgetItem(QIcon::fromTheme("computer"), QString("%1 %2").arg(sender->address()).arg(sender->port()), connectionList_);
            listItemToConnection_.insert(newItem, sender);
            connectionToListItem_.insert(sender, newItem);

            qDebug("CONNECT_DESKTOP");
        } else if constexpr (std::is_same_v<T, ConnectMobile>) {
            Q_ASSERT(sender->connectionType() != Connection::ConnectionType::none);

            auto newItem = new QListWidgetItem(QIcon::fromTheme("phone"), QString("%1 %2").arg(sender->address()).arg(sender->port()), connectionList_);
            listItemToConnection_.insert(newItem, sender);
            connectionToListItem_.insert(sender, newItem);

            qDebug("CONNECT_MOBILE");
        } else if constexpr (std::is_same_v<T, SetBalanceCoord>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::SetBalanceCoord(arg.x, arg.y));
            }

            qDebug("SET_BALANCE_COORD %d %d", arg.x, arg.y);
        } else if constexpr (std::is_same_v<T, MoveLeft>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveLeft());
            }

            qDebug("MOVE_LEFT");
        } else if constexpr (std::is_same_v<T, MoveRight>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveRight());
            }

            qDebug("MOVE_RIGHT");
        } else if constexpr (std::is_same_v<T, MoveForward>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveForward());
            }

            qDebug("MOVE_FORWARD");
        } else if constexpr (std::is_same_v<T, MoveBackward>) {
            if (sender == toggled_) {
                serial_->sendCommand(esp32::outgoing::MoveBackward());
            }

            qDebug("MOVE_BACKWARD");
        } else {
            static_assert(!sizeof(T), "non-exhaustive visitor");
        }
    }, *sender->incomingCommand());
}

void CentralWidget::receiveSerialCommand() {
    using namespace esp32::incoming;

    qDebug("serial sender");

    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Coord>) {
            qDebug("COORD %d %d", arg.x, arg.y);

            server_->broadcastCommand(server::outgoing::Coord(arg.x, arg.y));
        } else if constexpr (std::is_same_v<T, BalanceCoord>) {
            qDebug("BALANCE_COORD %d %d", arg.x, arg.y);

            server_->broadcastCommand(server::outgoing::BalanceCoord(arg.x, arg.y));
        } else {
            static_assert(!sizeof(T), "non-exhaustive visitor");
        }
    }, *serial_->incomingCommand());
}
