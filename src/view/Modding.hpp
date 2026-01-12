#ifndef MODDING_HPP_
#define MODDING_HPP_

#include <QWidget>
#include <QGridLayout>

class UiModding : public QWidget
{
    Q_OBJECT
public:
    /*
     * UiModding (Ui->Tabs)
     */
    explicit UiModding(QWidget *parent = nullptr);
private:
    QGridLayout *layout{nullptr};
};

#endif // MODDING_HPP_
