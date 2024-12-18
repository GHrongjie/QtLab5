#include "manageusersdialog.h"
#include "ui_manageusersdialog.h"

#include <QJsonValue>
#include <QJsonObject>

ManageUsersDialog::ManageUsersDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageUsersDialog)
{
    ui->setupUi(this);
}

ManageUsersDialog::~ManageUsersDialog()
{
    delete ui;
}

void ManageUsersDialog::setChatClient(ChatClient *chatClient)
{
    m_chatClient=chatClient;
    //ChatClient的jsonReceived信号传递给ManageUsersDialog的jsonReceived()
    connect(m_chatClient,&ChatClient::jsonReceived,this,&ManageUsersDialog::jsonReceived);
}

void ManageUsersDialog::on_reflashButton_clicked()
{
    m_chatClient->sendMessage("Reflash","getUsers");
}

void ManageUsersDialog::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");//信息类型是否为空
    if(typeVal.isNull() || !typeVal.isString())
        return;

    if(typeVal.toString().compare("resUsers",Qt::CaseInsensitive)==0){//如果是管理员信息
        const QJsonValue userlistVal = docObj.value("resUsers");
        if(userlistVal.isNull() || !userlistVal.isArray())
            return;

        qDebug() << userlistVal.toVariant().toStringList();
        userListReceived(userlistVal.toVariant().toStringList());
    }
}

void ManageUsersDialog::userListReceived(const QStringList &list)
{
    ui->userListWidget->clear();
    ui->userListWidget->addItems(list);
}

void ManageUsersDialog::on_kickoutUserButton_clicked()
{
    m_chatClient->sendMessage(ui->targetLineEdit->text(),"kickoutUser");
}


void ManageUsersDialog::on_banUserButton_clicked()
{
    m_chatClient->sendMessage(ui->targetLineEdit->text(),"banUser","false");
}


void ManageUsersDialog::on_ubanUserButton_clicked()
{
    m_chatClient->sendMessage(ui->targetLineEdit->text(),"banUser","true");
}

