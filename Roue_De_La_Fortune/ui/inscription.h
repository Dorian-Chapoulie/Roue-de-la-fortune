#ifndef INSCRIPTION_H
#define INSCRIPTION_H

#include <QWidget>

namespace Ui {
class Inscription;
}

class Inscription : public QWidget
{
    Q_OBJECT

public:
    explicit Inscription(QWidget *parent = nullptr);
    ~Inscription();

private slots:
    void on_buttonInscription_clicked();
    void inscriptionError(QString);
    void inscriptionSuccessful();

    void on_buttonCancel_clicked();

signals:
    void displayInscriptionError(QString);
    void showConnectionForm();

private:
    Ui::Inscription *ui;
};

#endif // INSCRIPTION_H
