#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

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

#endif // CONTROLLER_HPP_
