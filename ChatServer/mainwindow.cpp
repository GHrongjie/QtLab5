#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_chatServer = new ChatServer(this);

    connect(m_chatServer,&ChatServer::logMessage,this,&MainWindow::logMessage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startStopButton_clicked()//服务器启动关闭
{
    if(m_chatServer->isListening()){
        m_chatServer->stopServer();
        ui->startStopButton->setText("启动服务器");
        logMessage("服务器已经停止");
    }else{
        if(!m_chatServer->listen(QHostAddress::Any,8080)){
            QMessageBox::critical(this,"错误","无法启动服务器");
            return;
        }
        ui->startStopButton->setText("停止服务器");
        logMessage("服务器已经启动");
    }


}

void MainWindow::logMessage(const QString &msg)//打印日志信息
{
    ui->logEditor->appendPlainText(msg);
}

