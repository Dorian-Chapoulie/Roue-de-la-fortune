#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Connection; }
QT_END_NAMESPACE

class Connection : public QMainWindow
{
    Q_OBJECT

public:
    Connection(QWidget *parent = nullptr);
    ~Connection();

private slots:

    void on_connectionButton_clicked();
    void connection_error(QString);
    void connection_success();

    void on_inscriptionButton_clicked();

signals:
    void displayError(QString);
    void showServerNavigator() const;

private:
    Ui::Connection *ui;
};
#endif // CONNECTION_H
