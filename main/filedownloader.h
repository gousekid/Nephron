#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl imageUrl, int code, const QByteArray &md5, const QByteArray& sha1, QObject *parent = 0);
    virtual ~FileDownloader();

    QByteArray downloadedData() const;
    void saveData(const QString& sPath);

signals:
    void downloaded(int code);
    void downloadFailed();

private slots:

     void fileDownloaded(QNetworkReply* pReply);

private:

    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;
    QByteArray m_MD5;
    QByteArray m_SHA1;

    int nCode;

};

#endif // FILEDOWNLOADER_H
