#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "idatabase.h"
#include "chathistorydialog.h"
#include "manageusersdialog.h"
#include <QHostAddress>
#include <QJsonValue>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_manageUsersDialog=new ManageUsersDialog(this);
    m_chatHistoryDialog=new ChatHistoryDialog(this);
    m_chatClient = new ChatClient(this);//新建对话用户

    ui->manageUserButton->setVisible(false);
    ui->chatHistoryButton->setVisible(false);

    ui->stackedWidget->setCurrentWidget(ui->LoginPage);

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
    QString status=Idatabase::getInstance().userLogin(ui->usernameEdit->text(),ui->passwordEdit->text());

    if(status == "loginOK")
    {
        m_chatClient->connectToServer(QHostAddress(ui->serverEdit->text()),8080);
    }
}


void MainWindow::on_logoutButton_clicked()//退出并断开链接
{
    m_chatClient->disconnectFromHost();

    ui->manageUserButton->setVisible(false);
    ui->chatHistoryButton->setVisible(false);

    ui->stackedWidget->setCurrentWidget(ui->LoginPage);

    for( auto aItem : ui->userListWidget->findItems(ui->passwordEdit->text(),Qt::MatchExactly)){
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }

}


void MainWindow::on_sayButton_clicked()//发送消息
{
    if(!ui->sayLineEdit->text().isEmpty()){
        m_chatClient->sendMessage(ui->sayLineEdit->text());
        ui->sayLineEdit->clear();
    }
}

void MainWindow::connectedToServer()//收到来自ChatClient的connected信号，进行登录
{
    //获取身份
    const QString userIdentity = Idatabase::getInstance().userIdentity(ui->usernameEdit->text());
    m_chatClient->setUserIdentity(userIdentity);

    m_chatClient->sendMessage(ui->usernameEdit->text(),"login",m_chatClient->userIdentity());
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

        if(userNameVal==ui->usernameEdit->text())//用户登录时收到服务器返回的信息才进行页面跳转
        {
            //获取用户身份并设置界面
            ui->chatHistoryButton->setVisible(true);
            if(m_chatClient->userIdentity()=="管理员"){
                ui->manageUserButton->setVisible(true);
            }
            m_manageUsersDialog->setChatClient(m_chatClient);
            ui->stackedWidget->setCurrentWidget(ui->ChatPage);
        }

        userJoined(userNameVal.toString());
    }else if(typeVal.toString().compare("userdisconnected",Qt::CaseInsensitive)==0){//如果是退出信息
        const QJsonValue userNameVal = docObj.value("username");//用户名是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;

        userLeft(userNameVal.toString());
    }else if(typeVal.toString().compare("userlist",Qt::CaseInsensitive)==0){//如果是在线用户信息
        const QJsonValue userlistVal = docObj.value("userlist");
        if(userlistVal.isNull() || !userlistVal.isArray())
            return;

        qDebug() << userlistVal.toVariant().toStringList();
        userListReceived(userlistVal.toVariant().toStringList());
    }else if(typeVal.toString().compare("kickout",Qt::CaseInsensitive)==0){
        const QJsonValue userNameVal = docObj.value("text");//用户名是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;
        if(ui->usernameEdit->text()==userNameVal.toString()){
            on_logoutButton_clicked();
        }
    }
}

void MainWindow::userJoined(const QString &user)//将新用户添加到聊天室列表
{
    ui->userListWidget->addItem(user);
}

void MainWindow::userLeft(const QString &user)//将新用户从聊天室列表移除
{
    for( auto aItem : ui->userListWidget->findItems(user,Qt::MatchExactly)){
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }
}

void MainWindow::userListReceived(const QStringList &list)
{
    ui->userListWidget->clear();
    ui->userListWidget->addItems(list);
}

void MainWindow::on_chatHistoryButton_clicked()
{
    m_chatHistoryDialog->show();
}


void MainWindow::on_manageUserButton_clicked()
{
    m_manageUsersDialog->show();
}

