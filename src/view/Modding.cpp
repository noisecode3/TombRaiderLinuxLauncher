#include "Modding.hpp"

UiModding::UiModding(QWidget *parent)
    : QWidget{parent}
{
    setObjectName("Modding");
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}
