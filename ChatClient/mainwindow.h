#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMap>

#include "chatclient.h"
#include "chathistorydialog.h"
#include "manageusersdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:


private slots:
    void on_loginButton_clicked();

    void on_logoutButton_clicked();

    void on_sayButton_clicked();

    void on_chatHistoryButton_clicked();

    void connectedToServer();

    void messageReceived(const QString &sender,const QString &text,const QString &reciver);

    void userJoined(const QString &user);
    void userLeft(const QString &user);
    void jsonReceived(const QJsonObject &docObj);
    void userListReceived(const QStringList &list);
    void goToOtherChatPage(const QString& username);

    void on_manageUserButton_clicked();

private:
    Ui::MainWindow *ui;

    ChatHistoryDialog *m_chatHistoryDialog;
    ManageUsersDialog *m_manageUsersDialog;
    ChatClient *m_chatClient;//对话用户

    QString m_targetPage;//当前的对话目标
    QMap<QString,int> m_chatPageIndex;//用户对应的页面索引，用来切换页面
    QMap<QString,QTextEdit*> m_chatPages;//记录用户聊天界面

};
#endif // MAINWINDOW_H
