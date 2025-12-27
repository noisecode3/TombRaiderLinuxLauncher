#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>
#include <QFrame>
#include <QTextBrowser>
#include <QVBoxLayout>

class FrameAbout : public QFrame
{
    Q_OBJECT
 public:
    explicit FrameAbout(QWidget *parent = nullptr);
    QTextBrowser* textBrowserAbout{nullptr};
 private:
    QVBoxLayout* layout{nullptr};
};

class UiAbout : public QWidget
{
    Q_OBJECT
public:
    /*
     * (Ui->Tabs)
     * UiAbout
     * └── frameAbout
     *      └── textBrowserAbout
     */
    explicit UiAbout(QWidget *parent = nullptr);
    FrameAbout* frameAbout;
private:
    QGridLayout *layout{nullptr};
};

#endif // ABOUT_H
