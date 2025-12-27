#ifndef MODDING_H
#define MODDING_H

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

#endif // MODDING_H
