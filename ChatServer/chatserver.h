#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QObject>
#include "serverworker.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor)override;
    QVector<ServerWorker *> m_clients;//链接池，保存用户链接

signals:
    void logMessage(const QString &msg);

public slots:
    void stopServer();

};

#endif // CHATSERVER_H
