#include "view/Controller.hpp"

UiController::UiController(QWidget *parent)
    : QWidget{parent},
    layout(new QVBoxLayout(this))
{
    setObjectName("Controller");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}
