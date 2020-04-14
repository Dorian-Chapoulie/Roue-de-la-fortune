#ifndef SERVERNAVIGATOR_H
#define SERVERNAVIGATOR_H

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

    QStringList headerList;

private slots:
    void addServerInList(QString);

    void on_rejoindreButton_clicked();

signals:
    void updateList(QString);
};

#endif // SERVERNAVIGATOR_H