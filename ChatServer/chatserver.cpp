#include "chatserver.h"
#include "serverworker.h"

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent) {

}

void ChatServer::incomingConnection(qintptr socketDescriptor)//监听新用户链接
{
    ServerWorker *worker = new ServerWorker(this);//获取链接
    if(!worker->setSocketDescriptor(socketDescriptor)){
        worker->deleteLater();
        return;
    }
    connect(worker, &ServerWorker::logMessage,this,&ChatServer::logMessage);
    m_clients.append(worker);//将新用户加入链接池
    emit logMessage("new user has connected");
}

void ChatServer::stopServer()
{
    close();
}
