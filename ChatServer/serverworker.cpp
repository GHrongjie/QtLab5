#include "serverworker.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>

ServerWorker::ServerWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket = new QTcpSocket(this);//新建链接
    //链接完毕，传信号给onReadyRead()来开始监听用户信息
    connect(m_serverSocket,&QTcpSocket::readyRead,this,&ServerWorker::onReadyRead);
    //链接断开，传信号给disconnectedFromClient()来开始处理用户断连
    connect(m_serverSocket,&QTcpSocket::disconnected,this,&ServerWorker::disconnectedFromClient);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)//返回链接描述符
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

QString ServerWorker::userName()
{
    return m_userName;
}

void ServerWorker::setUserName(QString user)
{
    m_userName=user;
}

void ServerWorker::onReadyRead()//服务器启动
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);

    for(;;){//循环从客户端接受信息
        socketStream.startTransaction();
        socketStream >> jsonData;
        if(socketStream.commitTransaction()){
            // emit logMessage(QString::fromUtf8(jsonData));
            // sendMessage("I recieved message");

            //json解析
            QJsonParseError parserError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData,&parserError);
            if(parserError.error == QJsonParseError::NoError){
                emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
                emit jsonReceived(this,jsonDoc.object());
            }
        }else{
            break;
        }
    }
}

void ServerWorker::sendMessage(const QString &text, const QString &type)//发送信息给客户端
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

void ServerWorker::sendJson(const QJsonObject &json)//发送Json信息
{
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage(QLatin1String("Sending to ") + userName() + QLatin1String(" - ")+
                    QString::fromUtf8(jsonData));
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);
    socketStream << jsonData;
}
