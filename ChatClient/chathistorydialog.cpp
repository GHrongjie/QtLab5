#include "chathistorydialog.h"
#include "ui_chathistorydialog.h"
#include "idatabase.h"
#include <QDebug>

ChatHistoryDialog::ChatHistoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatHistoryDialog)
{
    qDebug()<<"ChatHistoryDialog create";
    ui->setupUi(this);

    ui->roomChatCheckBox->setChecked(true);
    ui->startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
}

ChatHistoryDialog::~ChatHistoryDialog()
{
    qDebug()<<"ChatHistoryDialog delete";
    delete ui;
}

void ChatHistoryDialog::setUserName(const QString &userName)
{
    m_userName=userName;
}

void ChatHistoryDialog::on_searchButton_clicked()
{
    ui->historySelectResult->clear();
    QStringList res;

    QString speakerCondition=ui->userNameLineEdit->text();
    QString keyWordsCondition=ui->keyLineEdit->text();
    QString startTime=ui->startTimeEdit->text();
    QString endTime=ui->endTimeEdit->text();

    if(ui->roomChatCheckBox->isChecked()){
        res=Idatabase::getInstance().getRoomChatHistory(speakerCondition,keyWordsCondition,startTime,endTime);
    }else{
        res=Idatabase::getInstance().getSelfChatHistory(m_userName,speakerCondition,keyWordsCondition,startTime,endTime);
    }
    for(int i=0;i<res.count();i++)
    {
        ui->historySelectResult->append(res[i]);
    }

}


void ChatHistoryDialog::on_roomChatCheckBox_clicked()
{
    ui->roomChatCheckBox->setChecked(true);
    ui->selfChatCheckBox->setChecked(false);
}


void ChatHistoryDialog::on_selfChatCheckBox_clicked()
{
    ui->roomChatCheckBox->setChecked(false);
    ui->selfChatCheckBox->setChecked(true);
}

