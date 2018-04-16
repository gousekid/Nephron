#include "global.hpp"
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

#define LOG_PATH   "/home/superbin/Nephron/Log"

int globalLogLevel = LOGLEVEL_VERY_LOW;

void setCurrentLogLevel(int level) {
   globalLogLevel = level;
}

void qLog(int level, const QString & owner, const QString & category, const QString &description){
    if (level > globalLogLevel) {
        return;
    }

    QString sTitleColor, sDescriptionColor,sDescriptionBackColor;
    QByteArray titleColor;

    QString sPath = LOG_PATH;
    QDateTime time = QDateTime::currentDateTime();


    sPath += time.toString("/Nep'h'ron_yyyy-MM-dd.log");
    //qDebug() << "Log Path : " << sPath;

    QFile *file = new QFile(sPath);

    if(owner.compare("Main",Qt::CaseInsensitive)==0)
    {
        sTitleColor = "\x1b[32m";
        //titleColor.append()
    }
    else if(owner.compare("Classifier",Qt::CaseInsensitive)==0)
    {
        sTitleColor = "\x1b[33m";
    }
    else if(owner.compare("PLC",Qt::CaseInsensitive)==0)
    {
        sTitleColor = "\x1b[34m";
    }
    else if(owner.compare("ui",Qt::CaseInsensitive)==0)
    {
        sTitleColor = "\x1b[36m";
    }
    else if(owner.compare("PRT",Qt::CaseInsensitive)==0)
    {
        sTitleColor = "\x1b[35m";
    }
    else
    {
        sTitleColor = "\x1b[39m";
    }

    if(category.compare("Info",Qt::CaseInsensitive)==0)
    {
        sDescriptionBackColor = "\x1b[40m";
        sDescriptionColor = "\x1b[34m";
    }
    else if(category.compare("Warning",Qt::CaseInsensitive)==0)
    {
        sDescriptionBackColor = "\x1b[43m";
        sDescriptionColor = "\x1b[33m";
    }
    else if(category.compare("Error",Qt::CaseInsensitive)==0)
    {
        sDescriptionBackColor = "\x1b[41m";
        sDescriptionColor = "\x1b[31m";
    }
    else\
    {
        sDescriptionBackColor = "\x1b[40m";
        sDescriptionColor = "\x1b[39m";
    }

    if(file->open(QFile::Append | QFile::Text))
    {
        file->seek(file->size());
        QTextStream out(file);
        out << time.toString("[yyyy-MM-dd hh:mm:ss.zzz]") << "[" << sTitleColor << owner << "\x1b[0m][" << sDescriptionColor << category << "\x1b[0m]" << sDescriptionBackColor << description << "\x1b[0m\r\n";
        file->close();


    }
}
