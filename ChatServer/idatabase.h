#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>

class Idatabase : public QObject
{
    Q_OBJECT
public:
    static Idatabase &getInstance(){
        static Idatabase instance;
        return instance;
    }

    void saveRoomChatData(const QString& speaker,const QString& content);
    void saveSelfChatData(const QString& speaker,const QString& reciver,const QString& content);

private:
    explicit Idatabase(QObject *parent = nullptr);
    Idatabase(Idatabase const &) = delete;
    void operator=(Idatabase const &) = delete;

    QSqlDatabase database;

    void initDatabase();

signals:
};

#endif // IDATABASE_H
