#include "chatserver.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QThreadPool>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent) {
    QThreadPool::globalInstance()->setMaxThreadCount(5);
}

void ChatServer::incomingConnection(qintptr socketDescriptor)//监听新用户链接
{
    ServerWorker *worker = new ServerWorker(this);//获取链接
    if(!worker->setSocketDescriptor(socketDescriptor)){
        worker->deleteLater();
        return;
    }


    connect(worker, &ServerWorker::logMessage,this,&ChatServer::logMessage);
    connect(worker, &ServerWorker::jsonReceived,this,&ChatServer::jsonReceived);
    connect(worker, &ServerWorker::disconnectedFromClient,this,std::bind(&ChatServer::userDisconnected,this,worker));


    emit logMessage("new user has connected");
}

//向所有用户广播信息
void ChatServer::broadcast(const QJsonObject &message, ServerWorker *exclude)
{
    for(ServerWorker *worker:m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::stopServer()
{
    close();
}

void ChatServer::jsonReceived(ServerWorker *sender, const QJsonObject &docObj)//接受json信息，并将信息向所有用户广播
{
    const QJsonValue typeVal = docObj.value("type");//信息类型是否为空
    if(typeVal.isNull() || !typeVal.isString())
        return;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)==0){//如果是普通信息
        if(!sender->userStatus()){
            return;
        }
        const QJsonValue textVal = docObj.value("text");//信息内容是否为空
        if(textVal.isNull() || !textVal.isString())
            return;
        const QString text = textVal.toString().trimmed();
        if(text.isEmpty())
            return;
        QJsonObject message;
        message[QStringLiteral("type")]="message";//信息类型
        message[QStringLiteral("text")]=text;//信息内容
        message[QStringLiteral("sender")]=sender->userName();//谁发送的

        broadcast(message,sender);//向所有用户广播
    }else if(typeVal.toString().compare("login",Qt::CaseInsensitive)==0){//如果是登录信息
        const QJsonValue userNameVal = docObj.value("text");//用户名是否为空
        const QJsonValue userIdentity = docObj.value("userIdentity");//用户身份是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;
        if(userIdentity.isNull() || !userIdentity.isString())
            return;

        // 检查用户名是否已经被使用
        if (m_userMap.contains(userNameVal.toString())) {
            // 如果用户已经连接，拒绝新连接并关闭socket
            qDebug()<<userNameVal.toString()+" are already logged in from another device.";
            sender->deleteLater();
            return;
        }

        // 如果用户名没有被占用，则添加到m_userMap中
        sender->setUserName(userNameVal.toString());
        sender->setUserIdentity(userIdentity.toString());
        sender->setUserStatus(true);
        m_userMap[sender->userName()] = sender;
        m_clients.append(sender);//将新用户加入链接池

        QJsonObject connectedMessage;
        connectedMessage["type"] = "newUser";//信息类型
        connectedMessage["username"] = userNameVal.toString();//加入的用户名
        broadcast(connectedMessage,sender);//向所有用户广播

        //给新登录用户发送聊天室用户表
        QJsonObject userListMessage;
        userListMessage["type"] = "userlist";//信息类型
        QJsonArray userList;
        for(ServerWorker *worker : m_clients){
            if(worker == sender)
                userList.append(worker->userName()+ "(you)");
            else
                userList.append(worker->userName());
        }
        userListMessage["userlist"] = userList;
        sender->sendJson(userListMessage);
    }else if(typeVal.toString().compare("getUsers",Qt::CaseInsensitive)==0){//如果是管理员请求信息
        //给新登录用户发送聊天室用户表
        QJsonObject userListMessage;
        userListMessage["type"] = "resUsers";//信息类型
        QJsonArray userList;
        for(ServerWorker *worker : m_clients){
            if(worker->userIdentity()!="管理员"){
                userList.append(worker->userName());
            }
        }
        userListMessage["resUsers"] = userList;
        sender->sendJson(userListMessage);
    }else if(typeVal.toString().compare("kickoutUser",Qt::CaseInsensitive)==0){//踢出用户
        const QJsonValue userNameVal = docObj.value("text");//用户名是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;
        QJsonObject message;
        message[QStringLiteral("type")]="kickout";//信息类型
        message[QStringLiteral("text")]=userNameVal.toString();//目标用户
        message[QStringLiteral("sender")]=sender->userName();//谁发送的
        m_userMap[userNameVal.toString()]->sendJson(message);
    }else if(typeVal.toString().compare("banUser",Qt::CaseInsensitive)==0){//禁言或解除禁言
        const QJsonValue userNameVal = docObj.value("text");//用户名是否为空
        const QJsonValue status = docObj.value("userIdentity");//禁言状态是否为空
        if(userNameVal.isNull() || !userNameVal.isString())
            return;
        if(status.isNull() || !status.isString())
            return;
        const bool nstatus=status.toString()=="true"?true:false;
        m_userMap[userNameVal.toString()]->setUserStatus(nstatus);
    }
}

void ChatServer::userDisconnected(ServerWorker *sender)//处理用户断开链接
{
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if(!userName.isEmpty()){//移除的用户名广播出去，让其余用户的用户列表删除该用户
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = "userdisconnected";
        disconnectedMessage["username"] = userName;
        broadcast(disconnectedMessage,nullptr);
        emit logMessage(userName + " disconnected");
    }
    sender->deleteLater();
    m_userMap.remove(userName);
}
