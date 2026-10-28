#include "mainwindow.h"
#include "./ui_mainwindow.h"

QVector<QVector<QString>> gecmis;

QTimer *genelZamanliyici;
QTimer *epilepsiZamanliyici;
bool zamanlayiciAktifMi = false;
bool gecmisiKaydetme = false;
bool epilepsi = false;

int temaSayici = 0;
bool epilepsiMod = 0;
// 0 = dark
// 1 = aydinlik

QMediaPlayer *oynatici;
QAudioOutput *cikis;

QTime alarmZamani;
QString alarmIsmi;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QDir().mkdir("../sources");

    QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

    if(ayarlarFile.value("general/arkaPlan").isNull())
    {
        ayarlarFile.setValue("general/arkaPlan", "true");
    }
    else
    {
        if(ayarlarFile.value("general/arkaPlan").value<QString>().contains("false"))
        {
            qApp->setStyle(QStyleFactory::create("windowsVista"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/light.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);

            temaSayici = 1;
        }
        else
        {
            qApp->setStyle(QStyleFactory::create("windows11"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/dark.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);
        }
    }

    if(ayarlarFile.value("general/isKaydetme").isNull())
    {
        ayarlarFile.setValue("general/isKaydetme", "true");
    }    else
    {
        if(ayarlarFile.value("general/isKaydetme").value<QString>().contains("false"))
        {
            gecmisiKaydetme = false;
        }
        else
        {
            gecmisiKaydetme = true;
        }
    }

    if(!QFile::exists("../sources/alarm.mp3"))
    {
        QMessageBox(QMessageBox::Icon::Critical, "Alarm Ses Dosyası Bulunmadı!", "Alarm sesi bulunamadı. Alarm sesi yükleyiniz.").exec();

        QFileDialog alarmSesDosyasiDialog(this);
        alarmSesDosyasiDialog.setNameFilter(QString("Ses Dosyası (*.mp3)"));

        alarmSesDosyasiDialog.exec();

        if(!alarmSesDosyasiDialog.selectedFiles().isEmpty())
        {
            if(QFile::copy(alarmSesDosyasiDialog.selectedFiles().at(0), "../sources/alarm.mp3"))
            {
                QMessageBox(QMessageBox::Icon::Information, "Başarılı!", "Alarm sesi başarılı bir şekilde aktarıldı.").exec();
            }
            else
            {
                QMessageBox(QMessageBox::Icon::Critical, "Başarısız!", "Sistem beklenmeyen bir hata sonucu ses dosyasını aktaramadı. Alarm zamanında herhangi bir ses çalınmıcaktır.").exec();
            }
        }
        else
        {
            QMessageBox(QMessageBox::Icon::Critical, "Alarm Ses Dosyası Alınamadı!", "Alarm sesi seçilmedi. Alarm zamanında herhangi bir ses çalınmıcaktır.").exec();
        }
    }

    ui->setupUi(this);

    genelZamanliyici = new QTimer();
    epilepsiZamanliyici = new QTimer();

    oynatici = new QMediaPlayer;
    cikis = new QAudioOutput;

    //oynatici->setSource(QUrl::fromLocalFile("../sources/alarm.mp3"));
    oynatici->setAudioOutput(cikis);
    oynatici->setLoops(10);

    cikis->setVolume(1);

    ui->guncelZaman->setText(QTime().currentTime().toString());
    ui->alarmZaman->setVisible(false);
    ui->progressBar->setVisible(false);

    genelZamanliyici->setInterval(1000);
    epilepsiZamanliyici->setInterval(150);

    QFile gecmisFile2("../gecmis");
    if(gecmisFile2.open(QFile::OpenModeFlag::Append));
    gecmisFile2.close();

    ayarlarPenceresi = new ayarlar;

    gecmisOku();

    connect(genelZamanliyici, &QTimer::timeout, this, &MainWindow::genelZaman_timeout);
    connect(epilepsiZamanliyici, &QTimer::timeout, this, &MainWindow::genelZaman_timeout2);
    connect(ui->alarmKur, &QPushButton::clicked, this, &MainWindow::alarmKur_clicked);

    connect(ui->disaAktar, &QPushButton::clicked, this, &MainWindow::disaAktar_clicked);
    connect(ui->iceAktar, &QPushButton::clicked, this, &MainWindow::iceAktar_clicked);
    connect(ui->gecmisiTemizle, &QPushButton::clicked, this, &MainWindow::gecmisiTemizle_clicked);
    connect(ui->cvsAktarma, &QPushButton::clicked, this, &MainWindow::cvsAktarma_clicked);
    connect(ui->ayarlar, &QPushButton::clicked, this, &MainWindow::ayarlar_clicked);
    connect(ui->tema, &QPushButton::clicked, this, &MainWindow::tema_clicked);

    genelZamanliyici->start();
    epilepsiZamanliyici->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::genelZaman_timeout()
{
    if(!zamanlayiciAktifMi)
    {
        ui->guncelZaman->setText(QTime().currentTime().toString());
    }
    else
    {
        if(QTime().currentTime().toString().contains(alarmZamani.toString()))
        {
            oynatici->setSource(QUrl::fromLocalFile("../sources/alarm.mp3"));
            oynatici->setLoops(10);
            oynatici->play();

            epilepsi = true;

            int alarmZamanDegeri = (alarmZamani.hour() * 3600) + (alarmZamani.minute() * 60) + alarmZamani.second();
            int guncelZamanDegeri = (QTime().currentTime().hour() * 3600) + (QTime().currentTime().minute() * 60) + QTime().currentTime().second();

            alarmZamanDegeri -= guncelZamanDegeri;

            int h = 0,m = 0;

            while(alarmZamanDegeri >= 3600)
            {
                h++;

                alarmZamanDegeri -= 3600;
            }
            while(alarmZamanDegeri >= 60)
            {
                m++;

                alarmZamanDegeri -= 60;
            }

            ui->progressBar->setValue(guncelZamanDegeri);
        }
        else
        {
            int guncelZamanDegeri = (QTime().currentTime().hour() * 3600) + (QTime().currentTime().minute() * 60) + QTime().currentTime().second();
            int alarmZamanDegeri = (alarmZamani.hour() * 3600) + (alarmZamani.minute() * 60) + alarmZamani.second();

            alarmZamanDegeri -= guncelZamanDegeri;

            if(alarmZamanDegeri <= 0)
            {
                if(!oynatici->isPlaying())
                    alarmZamanDegeri += 86400;
                else
                {
                    ui->guncelZaman->setText("00:00:00");

                    return;
                }
            }

            int h = 0,m = 0;

            while(alarmZamanDegeri >= 3600)
            {
                h++;

                alarmZamanDegeri -= 3600;
            }
            while(alarmZamanDegeri >= 60)
            {
                m++;

                alarmZamanDegeri -= 60;
            }

            ui->progressBar->setValue(guncelZamanDegeri);

            QTime kalanZaman(h,m, alarmZamanDegeri);

            ui->guncelZaman->setText(kalanZaman.toString());

            /*qDebug() << h;
            qDebug() << m;
            qDebug() << alarmZamanDegeri;*/
        }
    }
}

void MainWindow::alarmKur_clicked()
{
    if(!zamanlayiciAktifMi)
    {
        zamanlayiciAktifMi = true;

        alarmZamani = QTime(ui->saat->currentIndex(), ui->dakika->currentIndex(), 0);

        int alarmZamanDegeri = (alarmZamani.hour() * 3600) + (alarmZamani.minute() * 60) + alarmZamani.second();
        int zamanDegeri = (QTime().currentTime().hour() * 3600) + (QTime().currentTime().minute() * 60) + QTime().currentTime().second();

        int progBarDegeriMax = alarmZamanDegeri - zamanDegeri;

        ui->progressBar->setMinimum(zamanDegeri);
        if(progBarDegeriMax <= 0)
        {
            ui->progressBar->setMaximum(alarmZamanDegeri + 86400);
        }
        else
        {
            ui->progressBar->setMaximum(alarmZamanDegeri);
        }

        ui->progressBar->setValue(zamanDegeri);

        ui->alarmZaman->setText(alarmZamani.toString());

        ui->alarmKur->setText("Durdur");

        ui->alarmZaman->setVisible(true);
        ui->progressBar->setVisible(true);

        ui->saat->setEnabled(false);
        ui->dakika->setEnabled(false);
        ui->lineEdit->setEnabled(false);

        if(gecmisiKaydetme)
        {
            QVector<QString> alarmMetaDataKurulma;

            alarmIsmi = ui->lineEdit->text();

            alarmMetaDataKurulma.append(alarmIsmi);
            alarmMetaDataKurulma.append(QTime().currentTime().toString());
            alarmMetaDataKurulma.append(alarmZamani.toString());

            gecmis.append(alarmMetaDataKurulma);

            gecmisYazdir();
        }
    }
    else
    {
        oynatici->stop();
        oynatici->setSource(QUrl());
        epilepsi = false;

        QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

        if(ayarlarFile.value("general/arkaPlan").value<QString>().contains("false"))
        {
            qApp->setStyle(QStyleFactory::create("windowsVista"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/light.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);
        }
        else
        {
            qApp->setStyle(QStyleFactory::create("windows11"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/dark.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);
        }

        ui->alarmZaman->setVisible(false);
        ui->progressBar->setVisible(false);

        ui->saat->setEnabled(true);
        ui->dakika->setEnabled(true);
        ui->lineEdit->setEnabled(true);

        ui->lineEdit->setText("alarmIsmi");

        ui->alarmKur->setText("Kur");

        genelZaman_timeout();

        alarmZamani = QTime(0,0,0);

        zamanlayiciAktifMi = false;
    }
}

void MainWindow::gecmisOku()
{
    gecmis.clear();

    QFile gecmisDosya("../gecmis");
    if(gecmisDosya.open(QFile::OpenModeFlag::ReadOnly))
    {
        QTextStream akis(&gecmisDosya);

        QString alarmIsmiInput;
        QString alarmBaslangiciInput;
        QString alarmSonInput;

        int fase = 0;

        QString line;

        while(!akis.atEnd())
        {
            line = akis.readLine();
            alarmIsmiInput.clear();
            alarmBaslangiciInput.clear();
            alarmSonInput.clear();
            fase = 0;

            for (auto i : line) {
                if(i == '|')
                {
                    fase++;

                    continue;
                }

                if(fase == 0)
                {
                    alarmIsmiInput.append(i);
                }
                else if(fase == 1)
                {
                    alarmBaslangiciInput.append(i);
                }
                else if(fase == 2)
                {
                    alarmSonInput.append(i);
                }
            }

            QVector<QString> alarmMetadata;

            alarmMetadata.append(alarmIsmiInput);
            alarmMetadata.append(alarmBaslangiciInput);
            alarmMetadata.append(alarmSonInput);

            gecmis.append(alarmMetadata);
        }
    }
    gecmisDosya.close();

    ui->listWidget->clear();

    std::reverse(gecmis.begin(), gecmis.end());

    for(auto i : gecmis)
    {
        QWidget *anaWidget = new QWidget(this);
        anaWidget->setMinimumSize(QSize(0,30));
        QHBoxLayout *anaLayout = new QHBoxLayout(anaWidget);

        QLabel *icon = new QLabel();
        icon->setPixmap(QPixmap(":/assets/icons/alarm.png"));
        icon->setScaledContents(true);
        icon->setMaximumSize(QSize(25,25));
        icon->setMinimumSize(QSize(25,25));
        anaLayout->addWidget(icon);

        QLabel *alarmIsmi = new QLabel(i.at(0));
        anaLayout->addWidget(alarmIsmi);

        QLabel *alarmZamani = new QLabel();
        alarmZamani->setText(QString(i.at(1)).append(" -> ").append(i.at(2)));
        anaLayout->addWidget(alarmZamani);

        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);

        item->setSizeHint(anaWidget->sizeHint());

        ui->listWidget->setItemWidget(item, anaWidget);
    }

    std::reverse(gecmis.begin(), gecmis.end());
}

void MainWindow::gecmisYazdir()
{
    QFile gecmisDosya("../gecmis");
    if(gecmisDosya.open(QFile::OpenModeFlag::WriteOnly))
    {
        QTextStream akis(&gecmisDosya);

        int ii = 0;

        for (auto i : gecmis) {
            akis << i.at(0) << "|"
                 << i.at(1) << "|"
                 << i.at(2) << Qt::endl;
        }
    }
    gecmisDosya.close();

    gecmisOku();
}

void MainWindow::disaAktar_clicked()
{
    QFileDialog disaAktarmaDialog;
    disaAktarmaDialog.setAcceptMode(QFileDialog::AcceptSave);
    disaAktarmaDialog.setWindowTitle("Dışa Aktar");

    disaAktarmaDialog.exec();

    if(!disaAktarmaDialog.selectedFiles().isEmpty())
    {
        QFile::copy("../gecmis",disaAktarmaDialog.selectedFiles().at(0));

        QMessageBox(QMessageBox::Icon::Information, "Başarılı!", "Dosya başarılı bir şekilde dışa aktarıldı.").exec();
    }
    else
    {
        QMessageBox(QMessageBox::Icon::Warning, "Seçilmedi!", "Dosya aktıralamadı.").exec();
    }
}

void MainWindow::iceAktar_clicked()
{
    QFileDialog iceAktarmaDialog;
    iceAktarmaDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    iceAktarmaDialog.setWindowTitle("İçe Aktar");

    iceAktarmaDialog.exec();

    if(!iceAktarmaDialog.selectedFiles().isEmpty())
    {
        QFile gecmisDosya(iceAktarmaDialog.selectedFiles().at(0));
        if(gecmisDosya.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream akis(&gecmisDosya);

            QString alarmIsmiInput;
            QString alarmBaslangiciInput;
            QString alarmSonInput;

            int fase = 0;

            QString line;

            while(!akis.atEnd())
            {
                line = akis.readLine();
                alarmIsmiInput.clear();
                alarmBaslangiciInput.clear();
                alarmSonInput.clear();
                fase = 0;

                for (auto i : line) {
                    if(i == '|')
                    {
                        fase++;

                        continue;
                    }

                    if(fase == 0)
                    {
                        alarmIsmiInput.append(i);
                    }
                    else if(fase == 1)
                    {
                        alarmBaslangiciInput.append(i);
                    }
                    else if(fase == 2)
                    {
                        alarmSonInput.append(i);
                    }
                }

                QVector<QString> alarmMetadata;

                alarmMetadata.append(alarmIsmiInput);
                alarmMetadata.append(alarmBaslangiciInput);
                alarmMetadata.append(alarmSonInput);

                gecmis.append(alarmMetadata);
            }
        }
        gecmisDosya.close();

        QMessageBox(QMessageBox::Icon::Information, "Başarılı!", "Dosya başarılı bir şekilde içe aktarıldı.").exec();
    }
    else
    {
        QMessageBox(QMessageBox::Icon::Warning, "Seçilmedi!", "Dosya aktıralamadı.").exec();
    }

    if(gecmisiKaydetme)
        gecmisYazdir();
}

void MainWindow::gecmisiTemizle_clicked()
{
    gecmis.clear();

    gecmisYazdir();
}

void MainWindow::cvsAktarma_clicked()
{
    QFileDialog cvsDisaAktarmaDialog(this);
    cvsDisaAktarmaDialog.setAcceptMode(QFileDialog::AcceptSave);
    cvsDisaAktarmaDialog.setNameFilter("Virgülle Ayrılmış Değerler (*.csv)");
    cvsDisaAktarmaDialog.setWindowTitle("CSV Aktar");

    cvsDisaAktarmaDialog.exec();

    if(!cvsDisaAktarmaDialog.selectedFiles().isEmpty())
    {
        QString dosyaIsmi = cvsDisaAktarmaDialog.selectedFiles().at(0);
        if(!dosyaIsmi.contains(".csv"))
        {
            dosyaIsmi.append(".csv");
        }

        QFile gecmisCvsDosya(dosyaIsmi);
        if(gecmisCvsDosya.open(QFile::OpenModeFlag::WriteOnly))
        {
            QTextStream akis(&gecmisCvsDosya);

            int ii = 0;

            for (auto i : gecmis) {
                akis << i.at(0) << ";"
                     << i.at(1) << ";"
                     << i.at(2) << Qt::endl;
            }
        }
        gecmisCvsDosya.close();

        QMessageBox(QMessageBox::Icon::Information, "Başarılı!", "Dosya başarılı bir şekilde dışa aktarıldı.").exec();
    }
    else
    {
        QMessageBox(QMessageBox::Icon::Warning, "Seçilmedi!", "Dosya aktıralamadı.").exec();
    }
}

void MainWindow::ayarlar_clicked()
{
    ayarlarPenceresi->baslat();
}

void ayarlar::kaydet()
{
    this->setVisible(false);

    QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

    if(this->mod->isChecked())
    {
        ayarlarFile.setValue("general/arkaPlan","true");
    }
    else
    {
        ayarlarFile.setValue("general/arkaPlan","false");
    }

    if(this->isGecmisKaydetme->isChecked())
    {
        ayarlarFile.setValue("general/isKaydetme","true");
    }
    else
    {
        ayarlarFile.setValue("general/isKaydetme","false");
    }

    if(ayarlarFile.value("general/arkaPlan").value<QString>().contains("false"))
    {
        qApp->setStyle(QStyleFactory::create("windowsVista"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/light.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);

        temaSayici = 1;
    }
    else
    {
        qApp->setStyle(QStyleFactory::create("windows11"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/dark.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);

        temaSayici = 0;
    }

    if(ayarlarFile.value("general/isKaydetme").value<QString>().contains("false"))
    {
        gecmisiKaydetme = false;
    }
    else
    {
        gecmisiKaydetme = true;
    }
}

void ayarlar::iptal()
{
    this->setVisible(false);

    QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

    if(ayarlarFile.value("general/arkaPlan").value<QString>().contains("false"))
    {
        qApp->setStyle(QStyleFactory::create("windowsVista"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/light.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);
    }
    else
    {
        qApp->setStyle(QStyleFactory::create("windows11"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/dark.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);
    }

    if(ayarlarFile.value("general/isKaydetme").value<QString>().contains("false"))
    {
        gecmisiKaydetme = false;
    }
    else
    {
        gecmisiKaydetme = true;
    }
}

void MainWindow::tema_clicked()
{
    if(temaSayici == 0)
    {
        QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

        ayarlarFile.setValue("general/arkaPlan","false");

        qApp->setStyle(QStyleFactory::create("windowsVista"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/light.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);

        temaSayici = 1;
    }
    else if(temaSayici == 1)
    {
        QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

        ayarlarFile.setValue("general/arkaPlan","true");

        qApp->setStyle(QStyleFactory::create("windows11"));

        QString mainQss;
        QFile mainDotQss(":/assets/qss/dark.qss");
        if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
        {
            QTextStream reader(&mainDotQss);

            mainQss = reader.readAll();
        }
        mainDotQss.close();

        qApp->setStyleSheet(mainQss);

        temaSayici = 0;
    }
}

void MainWindow::genelZaman_timeout2()
{
    if(epilepsi)
    {
        if(epilepsiMod == 0)
        {
            qApp->setStyle(QStyleFactory::create("windowsVista"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/light.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);

            epilepsiMod = 1;
        }
        else if(epilepsiMod == 1)
        {
            qApp->setStyle(QStyleFactory::create("windows11"));

            QString mainQss;
            QFile mainDotQss(":/assets/qss/dark.qss");
            if(mainDotQss.open(QFile::OpenModeFlag::ReadOnly))
            {
                QTextStream reader(&mainDotQss);

                mainQss = reader.readAll();
            }
            mainDotQss.close();

            qApp->setStyleSheet(mainQss);

            epilepsiMod = 0;
        }

        this->raise();
        this->activateWindow();
    }
}