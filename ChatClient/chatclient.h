#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(QObject *parent = nullptr);

signals:
    void connected();
    void messageReceived(const QString &text);
    void jsonReceived(const QJsonObject &docObj);

private:
    QTcpSocket *m_clientSocket;
    QString m_identity;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message",const QString &identity=nullptr);
    void connectToServer(const QHostAddress &address, quint16 port);
    void disconnectFromHost();

    QString userIdentity();
    void setUserIdentity(const QString& identity);
};

#endif // CHATCLIENT_H
