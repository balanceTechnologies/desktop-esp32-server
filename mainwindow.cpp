#include "mainwindow.hpp"
#include "centralwidget.hpp"

#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Server");
    centralWidget_ = new CentralWidget(this);
    setCentralWidget(centralWidget_);
}

MainWindow::~MainWindow()
{
}

