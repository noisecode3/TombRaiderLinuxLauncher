#include "Controller.h"

UiController::UiController(QWidget *parent)
    : QWidget{parent}
{
    setObjectName("Controller");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}
