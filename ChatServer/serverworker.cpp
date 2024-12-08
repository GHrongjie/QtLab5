#include "serverworker.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ServerWorker::ServerWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket = new QTcpSocket(this);//新建链接
    connect(m_serverSocket,&QTcpSocket::readyRead,this,&ServerWorker::onReadyRead);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)//返回链接描述符
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

void ServerWorker::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);

    for(;;){//循环接受信息
        socketStream.startTransaction();
        socketStream >> jsonData;
        if(socketStream.commitTransaction()){
            emit logMessage(QString::fromUtf8(jsonData));
            sendMessage("I recieved message");
        }else{
            break;
        }
    }
}

void ServerWorker::sendMessage(const QString &text, const QString &type)
{
    if(m_serverSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty()){
        //为m_serverSocket创建一个QDataStream
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        //将发送的信息转换为json数据
        QJsonObject message;
        message["type"] = type;
        message["text"] = text;

        //用QDataStream发送json数据
        serverStream << QJsonDocument(message).toJson();
    }
}
