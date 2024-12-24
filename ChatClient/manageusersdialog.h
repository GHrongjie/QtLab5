#ifndef MANAGEUSERSDIALOG_H
#define MANAGEUSERSDIALOG_H

#include <QDialog>

#include "chatclient.h"

namespace Ui {
class ManageUsersDialog;
}

class ManageUsersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageUsersDialog(QWidget *parent = nullptr);
    ~ManageUsersDialog();

    void setChatClient(ChatClient *chatClient);

private:
    Ui::ManageUsersDialog *ui;
    ChatClient *m_chatClient=nullptr;//对话用户

public slots:

private slots:
    void on_reflashButton_clicked();
    void on_kickoutUserButton_clicked();
    void on_banUserButton_clicked();
    void on_ubanUserButton_clicked();

    void jsonReceived(const QJsonObject &docObj);
    void userListReceived(const QStringList &list);
    void showUserNameInEdit(const QString& username);
};

#endif // MANAGEUSERSDIALOG_H
