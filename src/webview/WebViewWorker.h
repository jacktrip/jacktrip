#ifndef __WEBVIEWWORKER_H__
#define __WEBVIEWWORKER_H__

#include <QObject>

class WebViewWorker : public QObject
{
    Q_OBJECT
public:
    WebViewWorker();

public slots:
    void doWork();

signals:
    void resultReady(int code);
};

#endif //__WEBVIEWWORKER_H__
