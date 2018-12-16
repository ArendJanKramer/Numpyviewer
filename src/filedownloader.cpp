#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
 QObject(parent)
{
 connect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );

 QNetworkRequest request(imageUrl);
 m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
    int statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200){
        QByteArray bytes = pReply->readAll();
        m_DownloadedData = QString::fromUtf8(bytes.data(), bytes.size());
        emit downloaded();
    }else{
        qInfo("Update checker failed with code: %d", statusCode);
    }
}

QString FileDownloader::downloadedData() const {
 return m_DownloadedData;
}
