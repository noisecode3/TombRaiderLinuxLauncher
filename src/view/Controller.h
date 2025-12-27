#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QWidget>
#include <QVBoxLayout>

class UiController : public QWidget
{
    Q_OBJECT
public:
    /*
     * (Ui->Tabs)
     * UiController
     */
    explicit UiController(QWidget *parent = nullptr);
private:
    QVBoxLayout *layout{nullptr};
};

#endif // CONTROLLER_H
