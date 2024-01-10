#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QTextEdit;
QT_END_NAMESPACE

class SerialComm;
class Server;
class Connection;

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);

private slots:
    void openSerial();
    void receiveClientCommand(Connection *sender);
    void receiveSerialCommand();

private:
    QLineEdit *serialPortNameLineEdit_ = nullptr;
    SerialComm *serial_ = nullptr;
    Server *server_ = nullptr;
    QListWidget *connectionList_ = nullptr;
    QHash<QListWidgetItem*, Connection*> listItemToConnection_;
    QHash<Connection*, QListWidgetItem*> connectionToListItem_;
    QListWidgetItem *noneItem_ = nullptr;
    Connection *toggled_ = nullptr;
    QTextEdit *serialReceiveTextEdit_ = nullptr;
    QTextEdit *serialSendTextEdit_ = nullptr;
    QTextEdit *clientReceiveTextEdit_ = nullptr;
    QTextEdit *clientSendTextEdit_ = nullptr;
};
