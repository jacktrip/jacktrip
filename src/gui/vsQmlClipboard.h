#ifndef VSQMLCLIPBOARD_H
#define VSQMLCLIPBOARD_H

#include <QApplication>
#include <QClipboard>
#include <QObject>

class VsQmlClipboard : public QObject
{
    Q_OBJECT
   public:
    explicit VsQmlClipboard(QObject* parent = 0) : QObject(parent)
    {
        clipboard = QApplication::clipboard();
    }

    Q_INVOKABLE void setText(QString text)
    {
        clipboard->setText(text, QClipboard::Clipboard);
    }

   private:
    QClipboard* clipboard;
};

#endif  // VSQMLCLIPBOARD_H