#ifndef SERVERNAVIGATOR_H
#define SERVERNAVIGATOR_H

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class ServerNavigator;
}

class ServerNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit ServerNavigator(QWidget *parent = nullptr);
    ~ServerNavigator();

private:
    Ui::ServerNavigator *ui;
    void showEvent(QShowEvent *) override;

    void joinGame(std::string& ip, int port);

    QStringList headerList;

private slots:
    void addServerInList(QString);

    void on_rejoindreButton_clicked();

    void on_createGameButton_clicked();

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

signals:
    void updateList(QString);
};

#endif // SERVERNAVIGATOR_H
