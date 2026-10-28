#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidgetItem>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QSettings>
#include <QStyleFactory>

#include "ayarlar.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void gecmisOku();
    void gecmisYazdir();

public slots:
    void genelZaman_timeout();
    void genelZaman_timeout2();
    void alarmKur_clicked();

    void disaAktar_clicked();
    void iceAktar_clicked();
    void gecmisiTemizle_clicked();
    void cvsAktarma_clicked();
    void ayarlar_clicked();
    void tema_clicked();

private:
    Ui::MainWindow *ui;
    ayarlar *ayarlarPenceresi;
};
#endif // MAINWINDOW_H
