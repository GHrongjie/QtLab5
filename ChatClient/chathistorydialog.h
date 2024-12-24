#ifndef CHATHISTORYDIALOG_H
#define CHATHISTORYDIALOG_H

#include <QDialog>

namespace Ui {
class ChatHistoryDialog;
}

class ChatHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatHistoryDialog(QWidget *parent = nullptr);
    ~ChatHistoryDialog();


    void setUserName(const QString& userName);

private slots:
    void on_searchButton_clicked();


    void on_roomChatCheckBox_clicked();

    void on_selfChatCheckBox_clicked();

private:
    Ui::ChatHistoryDialog *ui;
    QString m_userName;
};

#endif // CHATHISTORYDIALOG_H
