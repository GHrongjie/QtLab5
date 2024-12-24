#include "chatclient.h"

#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ChatClient::ChatClient(QObject *parent)
    : QObject{parent}
{
    m_clientSocket = new QTcpSocket(this);//新建链接
    //将TCP的connected信号传递给ChatClient的connected
    connect(m_clientSocket,&QTcpSocket::connected,this,&ChatClient::connected);
    //将TCP的readyRead信号传递给ChatClient的onReadyRead
    connect(m_clientSocket,&QTcpSocket::readyRead,this,&ChatClient::onReadyRead);
}

void ChatClient::onReadyRead()//确认收到TCP的readyRead信号，客户端启动
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);

    for(;;){//循环接受信息
        socketStream.startTransaction();
        socketStream >> jsonData;
        if(socketStream.commitTransaction()){
            //收到信息，信息作为信号发送给ChatClient的messageReceived
            // emit messageReceived(QString::fromUtf8(jsonData));

            //json解析
            QJsonParseError parserError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData,&parserError);
            if(parserError.error == QJsonParseError::NoError){
                emit jsonReceived(jsonDoc.object());
            }
        }else{
            break;
        }
    }
}

void ChatClient::sendMessage(const QString &text, const QString &type,const QString &autoV)//发送信息给服务器端
{
    if(m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty()){
        //为m_serverSocket创建一个QDataStream
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        //将发送的信息转换为json数据
        QJsonObject message;
        message["type"] = type;
        message["text"] = text;
        message["autoV"] = autoV;

        //用QDataStream发送json数据
        serverStream << QJsonDocument(message).toJson();
    }
}

void ChatClient::connectToServer(const QHostAddress &address, quint16 port)//链接地址端口
{
    m_clientSocket->connectToHost(address,port);
}

void ChatClient::disconnectFromHost()//断开链接
{
    m_clientSocket->disconnectFromHost();
}

QString ChatClient::userIdentity()
{
    return m_identity;
}

void ChatClient::setUserIdentity(const QString &identity)
{
    m_identity=identity;
}
