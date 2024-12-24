#include "idatabase.h"

void Idatabase::initDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");//设置数据库驱动
    QString aFile = "E:/SQLITEprojects/project.sqlite";
    database.setDatabaseName(aFile);//设置数据库名称

    if(!database.open()){
        qDebug()<<"Failed to open database";
    }else{
        qDebug()<<"Open database successfu;" << database.connectionName();
    }
}

void Idatabase::saveRoomChatData(const QString& speaker,const QString& content)//保存大厅聊天记录
{
    if(speaker.isEmpty()||content.isEmpty())
        return;

    QSqlQuery query;
    query.prepare("insert into roomchathistory(chatusername, chatdate, content) VALUES(:SPEAKER,datetime(),:CONTENT)");
    query.bindValue(":SPEAKER",speaker);
    query.bindValue(":CONTENT",content);
    query.exec();
}

void Idatabase::saveSelfChatData(const QString& speaker,const QString& reciver,const QString& content)//保存用户私聊记录
{
    if(speaker.isEmpty()||reciver.isEmpty()||content.isEmpty())
        return;

    QSqlQuery query;
    query.prepare("insert into userchathistory(speaker, reciver, chatdate, content) VALUES(:SPEAKER,:RECIVER,datetime(),:CONTENT)");
    query.bindValue(":SPEAKER",speaker);
    query.bindValue(":RECIVER",reciver);
    query.bindValue(":CONTENT",content);
    query.exec();
}

Idatabase::Idatabase(QObject *parent)
    : QObject{parent}
{
    initDatabase();
}
