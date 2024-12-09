#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QJsonValue>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);
    m_chatClient = new ChatClient(this);//新建对话用户
    //ChatClient的connected信号传递给MainWindow的connectedToServer
    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectedToServer);
    // //ChatClient的messageReceived信号传递给MainWindow的messageReceived()
    // connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);

    //ChatClient的jsonReceived信号传递给MainWindow的jsonReceived()
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked()//尝试链接服务器
{
    m_chatClient->connectToServer(QHostAddress(ui->serverEdit->text()),8080);
}


void MainWindow::on_logoutButton_clicked()//退出并断开链接
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

void MainWindow::messageReceived(const QString &sender,const QString &text)//收到ChatClient的messageReceived信号，将信息添加显示
{
    ui->roomTextEdit->append(QString("%1  :  %2").arg(sender).arg(text));
}

void MainWindow::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");//信息类型是否为空
    if(typeVal.isNull() || !typeVal.isString())
        return;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)==0){//如果是普通信息
        const QJsonValue textVal = docObj.value("text");//信息内容是否为空
        const QJsonValue senderVal = docObj.value("sender");//发送者是否为空
        if(textVal.isNull() || !textVal.isString())
            return;

        if(senderVal.isNull() || !senderVal.isString())
            return;

        messageReceived(senderVal.toString(),textVal.toString());//收到的信息添加显示

    }else if(typeVal.toString().compare("newUser",Qt::CaseInsensitive)==0){//如果是登录信息
        const QJsonValue userNameVal = docObj.value("username");//用户名是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;


        userJoined(userNameVal.toString());
    }
}

void MainWindow::userJoined(const QString &user)//将新用户添加到聊天室列表
{
    ui->userListWidget->addItem(user);
}

