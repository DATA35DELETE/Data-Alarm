#ifndef AYARLAR_H
#define AYARLAR_H

#include <QWidget>
#include <QSettings>
#include <QStyleFactory>
#include <QFile>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
class ayarlar;
}

class ayarlar : public QWidget
{
    Q_OBJECT

public:
    explicit ayarlar(QWidget *parent = nullptr);
    ~ayarlar();

    QCheckBox *mod;
    QCheckBox *isGecmisKaydetme;

    void baslat();

    void kaydet();
    void iptal();

public slots:
    void mod_clicked(Qt::CheckState);
    void alarmSes_clicked();

private:
    Ui::ayarlar *ui;
};

#endif // AYARLAR_H
