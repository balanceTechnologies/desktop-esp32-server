#include "serialcomm.hpp"
#include "esp32commands.hpp"

#include <QSerialPort>
#include <QTextStream>

SerialComm::SerialComm(quint32 baudRate, QObject *parent)
    : QObject{parent}
{
    serial_ = new QSerialPort(this);
    serial_->setBaudRate(baudRate);
    serial_->setDataBits(QSerialPort::Data8);
    serial_->setParity(QSerialPort::NoParity);
    serial_->setFlowControl(QSerialPort::NoFlowControl);
    serial_->setStopBits(QSerialPort::OneStop);

    connect(serial_, &QIODevice::readyRead, this, &SerialComm::readSerial);

    buf_.reserve(128);
}

void SerialComm::open(QString portName) {
    if (serial_->isOpen()) {
        serial_->close();
    }

    serial_->setPortName(portName);
    serial_->open(QIODeviceBase::ReadWrite);
}

bool SerialComm::isOpen() {
    return serial_->isOpen();
}

std::optional<esp32::incoming::Command> SerialComm::incomingCommand() {
    if (!incomingCommand_) {
        return std::nullopt;
    }

    auto command = std::move(incomingCommand_);
    incomingCommand_ = std::nullopt;
    return command;
}

void SerialComm::sendCommand(esp32::outgoing::Command command) {
    using namespace esp32::outgoing;

    if (!isOpen()) {
        return;
    }

    QTextStream ts(serial_);

    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, SetBalanceCoord>) {
            ts << "{SET_BALANCE_COORD " << arg.x << ' ' << arg.y << '\n';
        } else if constexpr (std::is_same_v<T, MoveLeft>) {
            ts << "A";
        } else if constexpr (std::is_same_v<T, MoveRight>) {
            ts << "D";
        } else if constexpr (std::is_same_v<T, MoveForward>) {
            ts << "W";
        } else if constexpr (std::is_same_v<T, MoveBackward>) {
            ts << "S";
        } else {
            static_assert(!sizeof(T), "non-exhaustive visitor");
        }
    }, command);
}

static std::optional<esp32::incoming::Command> readSerialParse(QSerialPort *serial)
{
    using namespace esp32::incoming;

    QTextStream ts(serial);

    serial->startTransaction();

    QChar startIndicator;
    if ((ts >> startIndicator).status() != QTextStream::Status::Ok || startIndicator != '{') {
        serial->commitTransaction();
        return std::nullopt;
    }

    QString commandType;
    if ((ts >> commandType).status() != QTextStream::Status::Ok) {
        serial->commitTransaction();
        return std::nullopt;
    }

    if (commandType == "COORD") {
        quint32 x, y;
        QString xStr, yStr;
        ts >> xStr >> yStr;

        if (ts.status() == QTextStream::Status::Ok) {
            bool okX, okY;
            x = xStr.toLong(&okX);
            y = yStr.toLong(&okY);

            if (okX && okY && x > 0 && y > 0) {
                serial->commitTransaction();
                return Coord(x, y);
            }
        }
    } else if (commandType == "BALANCE_COORD") {
        quint32 x, y;
        QString xStr, yStr;
        ts >> xStr >> yStr;

        if (ts.status() == QTextStream::Status::Ok) {
            bool okX, okY;
            x = xStr.toLong(&okX);
            y = yStr.toLong(&okY);

            if (okX && okY && x > 0 && y > 0) {
                serial->commitTransaction();
                return BalanceCoord(x, y);
            }
        }
    }

    if (ts.status() == QTextStream::Status::ReadPastEnd) {
        serial->rollbackTransaction(); // rollback incomplete read
    } else {
        serial->commitTransaction(); // discard useless
    }

    return std::nullopt;
}


void SerialComm::readSerial() {
    qDebug("readSerial");

    auto command = readSerialParse(serial_);
    if (command) {
        incomingCommand_ = command;
        emit readyIncomingCommand();
    }
}
