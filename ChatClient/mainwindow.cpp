#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);
    m_chatClient = new ChatClient(this);//新建对话用户
    //ChatClient的connected信号传递给MainWindow的connectedToServer
    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectedToServer);
    //ChatClient的messageReceived信号传递给MainWindow的messageReceived()
    connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked()//尝试链接服务器
{
    m_chatClient->connectToServer(QHostAddress(ui->serverEdit->text()),8080);
}


void MainWindow::on_logoutButton_clicked()//退出
{
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);
}


void MainWindow::on_sayButton_clicked()//发送消息
{
    if(!ui->sayLineEdit->text().isEmpty()){
        m_chatClient->sendMessage(ui->sayLineEdit->text());
    }
}

void MainWindow::connectedToServer()//收到来自ChatClient的connected信号，进行登录
{
    ui->stackedWidget->setCurrentWidget(ui->ChatPage);
    m_chatClient->sendMessage(ui->userNameEdit->text(),"login");
}

void MainWindow::messageReceived(const QString &text)//收到ChatClient的messageReceived信号，将信息添加显示
{
    ui->roomTextEdit->append(text);
}

