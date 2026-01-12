#include "Modding.hpp"

UiModding::UiModding(QWidget *parent)
    : QWidget{parent},
    layout(new QGridLayout(this))
{
    setObjectName("Modding");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}
