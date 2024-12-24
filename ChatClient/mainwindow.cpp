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


    connect(ui->userListWidget,&QListWidget::itemClicked,this,[this](QListWidgetItem *item){
        // 获取点击项的文本
        QString userName = item->text();
        // 调用槽函数，将用户的名字传递过去
        goToOtherChatPage(userName);
    });
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
    m_chatPageIndex.clear();


    ui->manageUserButton->setVisible(false);
    ui->chatHistoryButton->setVisible(false);
    ui->userListWidget->clear();


    for(int i=1;i<ui->chatStackedWidget->count();i++){//移除所有非主聊天室界面
        qDebug()<<"remove chatpage(index:"+QString::number(i);
        ui->chatStackedWidget->widgetRemoved(i);
    }

    QMap<QString,QTextEdit*>::iterator itor;

    for(itor=m_chatPages.begin();itor!=m_chatPages.end();itor++){//删除创建的界面
        delete itor.value();
    }

    m_chatPages.clear();
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);

}


void MainWindow::on_sayButton_clicked()//发送消息
{
    if(!ui->sayLineEdit->text().isEmpty()){
        if(m_targetPage=="主聊天室"){
            m_chatClient->sendMessage(ui->sayLineEdit->text());
        }else{
            m_chatClient->sendMessage(ui->sayLineEdit->text(),"selfMessage",m_targetPage);
        }
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

void MainWindow::messageReceived(const QString &sender,const QString &text,const QString &reciver)//收到ChatClient的messageReceived信号，将信息添加显示
{
    if(reciver.isNull()){
        ui->roomTextEdit->append(QString("%1  :  %2").arg(sender).arg(text));
    }else{
        if(reciver==ui->usernameEdit->text())
            m_chatPages[sender]->append(QString("%1  :  %2").arg(sender).arg(text));
        else
            m_chatPages[reciver]->append(QString("%1  :  %2").arg(sender).arg(text));
    }
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

        messageReceived(senderVal.toString(),textVal.toString(),nullptr);//收到的信息添加显示

    }else if(typeVal.toString().compare("selfMessage",Qt::CaseInsensitive)==0){//如果是私聊信息
        const QJsonValue textVal = docObj.value("text");//信息内容是否为空
        const QJsonValue senderVal = docObj.value("sender");//发送者是否为空
        const QJsonValue reciverVal = docObj.value("reciver");//发送者是否为空
        if(textVal.isNull() || !textVal.isString())
            return;

        if(senderVal.isNull() || !senderVal.isString())
            return;

        if(reciverVal.isNull() || !reciverVal.isString())
            return;

        if(senderVal.toString()!=ui->usernameEdit->text()){
            if(!m_chatPages.contains(senderVal.toString())){//是否已经创建聊天页面
                QTextEdit *textEdit = new QTextEdit(this);//创建一个页面
                m_chatPages[senderVal.toString()]=textEdit;//放入map中
                ui->chatStackedWidget->addWidget(textEdit);//添加到chatStackedWidget中
                m_chatPageIndex[senderVal.toString()]=ui->chatStackedWidget->count()-1;
                qDebug()<<"add chatpage for "+senderVal.toString();
                qDebug()<<"now has chatpage "+QString::number(ui->chatStackedWidget->count());
            }
        }

        messageReceived(senderVal.toString(),textVal.toString(),reciverVal.toString());//收到的信息添加显示
    }else if(typeVal.toString().compare("newUser",Qt::CaseInsensitive)==0){//如果是登录信息
        const QJsonValue userNameVal = docObj.value("username");//用户名是否为空

        if(userNameVal.isNull() || !userNameVal.isString())
            return;

        if(userNameVal.toString()==ui->usernameEdit->text())//用户登录时收到服务器返回的信息才进行页面跳转
        {
            //获取用户身份并设置界面
            ui->chatHistoryButton->setVisible(true);
            if(m_chatClient->userIdentity()=="管理员"){
                ui->manageUserButton->setVisible(true);
            }
            m_manageUsersDialog->setChatClient(m_chatClient);
            m_chatHistoryDialog->setUserName(userNameVal.toString());
            ui->stackedWidget->setCurrentWidget(ui->ChatPage);//转到聊天界面
            m_targetPage="主聊天室";
            ui->chatStackedWidget->setCurrentWidget(ui->mainChatPage);//聊天界面内转到主聊天室窗口
        }else{
            return;
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
        qDebug()<<"remove user "+user;
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }
    if(m_chatPages.contains(user)){//是否创建了该用户的聊天界面
        QMap<QString,int>::Iterator itor;

        if(ui->chatStackedWidget->currentIndex()==m_chatPageIndex[user]){//如果当且对话页面为离开的用户那就返回主聊天界面
            ui->chatStackedWidget->setCurrentWidget(ui->mainChatPage);
        }
        ui->chatStackedWidget->widgetRemoved(m_chatPageIndex[user]);//移除对应页面
        int index=m_chatPageIndex[user];//获取被删除页面的索引

        for(itor=m_chatPageIndex.begin();itor!=m_chatPageIndex.end();itor++){//更新对话页面的索引
            if(itor.value()>index){
                itor.value()--;
            }
        }

        m_targetPage="主聊天室";//重置当前对话目标

        qDebug()<<"remove userchatpage "+user;
        delete m_chatPages[user];//删除用户对话界面
        qDebug()<<"delete userchatpage "+user;
        m_chatPages.remove(user);//移除用户对应的map
    }
}

void MainWindow::userListReceived(const QStringList &list)
{
    ui->userListWidget->clear();
    ui->userListWidget->addItem("主聊天室");
    ui->userListWidget->addItems(list);
}

void MainWindow::goToOtherChatPage(const QString& username)
{
    qDebug()<<username;

    if(username==ui->usernameEdit->text()+"(you)"){
        return;
    }

    if(username=="主聊天室"){
        qDebug()<<"jump to chatpage for 主聊天室";
        ui->chatStackedWidget->setCurrentWidget(ui->mainChatPage);
        m_targetPage="主聊天室";
        qDebug()<<"now pageIndex :"+QString::number(0);
        return;
    }

    if(!m_chatPages.contains(username)){//是否已经创建聊天页面
        QTextEdit *textEdit = new QTextEdit(this);//创建一个页面
        m_chatPages[username]=textEdit;//放入map中
        ui->chatStackedWidget->addWidget(textEdit);//添加到chatStackedWidget中
        m_chatPageIndex[username]=ui->chatStackedWidget->count()-1;
        qDebug()<<"add chatpage for "+username;
        qDebug()<<"now has chatpage "+QString::number(ui->chatStackedWidget->count());
    }
    qDebug()<<"jump to chatpage for "+username;
    m_targetPage=username;//更新当前对话目标
    qDebug()<<"now pageIndex :"+QString::number(m_chatPageIndex[username]);
    ui->chatStackedWidget->setCurrentIndex(m_chatPageIndex[username]);//转到该用户界面或转入主界面
}

void MainWindow::on_chatHistoryButton_clicked()
{
    m_chatHistoryDialog->show();
}


void MainWindow::on_manageUserButton_clicked()
{
    m_manageUsersDialog->show();
}

