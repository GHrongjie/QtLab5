#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QObject>
#include <QTcpSocket>

class ServerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ServerWorker(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor);

    QString userName();
    void setUserName(const QString& user);
    QString userIdentity();
    void setUserIdentity(const QString& identity);
    bool userStatus();
    void setUserStatus(const bool& status);

signals:
    void logMessage(const QString &msg);
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);
    void disconnectedFromClient();

private:
    QTcpSocket *m_serverSocket;
    QString m_userName;
    QString m_identity;
    bool m_status;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message");
    void sendJson(const QJsonObject &json);
};

#endif // SERVERWORKER_H
