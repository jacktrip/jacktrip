#ifndef VSCLIPBOARD_H
#define VSCLIPBOARD_H

#include <QApplication>
#include <QClipboard>
#include <QObject>

class VsClipboard : public QObject
{
    Q_OBJECT
   public:
    explicit VsClipboard(QObject* parent = 0) : QObject(parent)
    {
        clipboard = QApplication::clipboard();
    }

    Q_INVOKABLE void setText(QString text)
    {
        clipboard->setText(text, QClipboard::Clipboard);
    }

    Q_INVOKABLE void copyText(QString text)
    {
        clipboard->setText(text, QClipboard::Clipboard);
    }

   private:
    QClipboard* clipboard;
};

#endif  // VSCLIPBOARD_H