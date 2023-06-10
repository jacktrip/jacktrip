#include "WebViewWorker.h"
#include <QDebug>
#include <QSharedPointer>
#include "webview.h"

QSharedPointer<webview::webview> mWebView;

WebViewWorker::WebViewWorker()
{
    qDebug() << "constructor";
    webview::webview w(false, nullptr);
    qDebug() << "a";
    mWebView.reset(&w);
    qDebug() << "b";
    mWebView->set_title("Basic Example");
    qDebug() << "c";
    mWebView->set_size(480, 320, WEBVIEW_HINT_NONE);
    qDebug() << "d";
    mWebView->navigate("https://en.m.wikipedia.org/wiki/Main_Page");
    qDebug() << "done";
}

void WebViewWorker::doWork()
{
    qDebug() << "doWork";
    mWebView->run();
    qDebug() << "resultReady";
    emit resultReady(0);
}
