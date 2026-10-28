#include "ayarlar.h"
#include "ui_ayarlar.h"

ayarlar::ayarlar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ayarlar)
{
    ui->setupUi(this);

    mod = ui->mod;
    isGecmisKaydetme = ui->gecmisKaydet;

    this->setWindowFlag(Qt::WindowType::CoverWindow);

    connect(ui->mod, &QCheckBox::checkStateChanged, this, &ayarlar::mod_clicked);

    connect(ui->kaydet, &QPushButton::clicked, this, [this](){kaydet();});
    connect(ui->iptal, &QPushButton::clicked, this, [this](){iptal();});
    connect(ui->alarmSes, &QPushButton::clicked, this, &ayarlar::alarmSes_clicked);
}

ayarlar::~ayarlar()
{
    delete ui;
}

void ayarlar::baslat()
{
    QSettings ayarlarFile("ayarlar.ini", QSettings::Format::IniFormat);

    if(ayarlarFile.value("general/arkaPlan").value<QString>().contains("false"))
    {
        ui->mod->setChecked(false);
    }
    else
    {
        ui->mod->setChecked(true);
    }

    if(ayarlarFile.value("general/isKaydetme").value<QString>().contains("false"))
    {
        ui->gecmisKaydet->setChecked(false);
    }
    else
    {
        ui->gecmisKaydet->setChecked(true);
    }

    this->setVisible(true);
}

void ayarlar::mod_clicked(Qt::CheckState isMod)
{
    if(isMod)
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
    else
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
}

void ayarlar::alarmSes_clicked()
{
    QFileDialog alarmSesDosyasiDialog(this);
    alarmSesDosyasiDialog.setNameFilter(QString("Ses Dosyası (*.mp3)"));

    alarmSesDosyasiDialog.exec();

    if(!alarmSesDosyasiDialog.selectedFiles().isEmpty())
    {
        QFile::remove("../sources/alarm.mp3");

        if(QFile::copy(alarmSesDosyasiDialog.selectedFiles().at(0), "../sources/alarm.mp3"))
        {
            QMessageBox(QMessageBox::Icon::Information, "Başarılı!", "Alarm sesi başarılı bir şekilde aktarıldı.").exec();
        }
        else
        {
            QMessageBox(QMessageBox::Icon::Critical, "Başarısız!", "Sistem beklenmeyen bir hata sonucu ses dosyasını aktaramadı.").exec();
        }
    }
    else
    {
        QMessageBox(QMessageBox::Icon::Critical, "Alarm Ses Dosyası Alınamadı!", "Alarm sesi seçilmedi.").exec();
    }
}