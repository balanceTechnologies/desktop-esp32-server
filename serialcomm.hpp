#pragma once

#include "esp32commands.hpp"

#include <QObject>

#include <optional>
#include <array>

QT_BEGIN_NAMESPACE
class QSerialPort;
QT_END_NAMESPACE

class SerialComm : public QObject
{
    Q_OBJECT
public:
    explicit SerialComm(quint32 baudRate = 115200, QObject *parent = nullptr);

    void open(QString portName);
    bool isOpen();
    std::optional<esp32::incoming::Command> incomingCommand();
    void sendCommand(esp32::outgoing::Command command);

signals:
    void readyIncomingCommand();

private slots:
    void readSerial();

private:
    QSerialPort *serial_ = nullptr;
    std::optional<esp32::incoming::Command> incomingCommand_;
    QByteArray buf_;
    bool inBraces_ = false;
};
