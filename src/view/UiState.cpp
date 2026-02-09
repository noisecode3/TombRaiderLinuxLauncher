#include "view/UiState.hpp"
UiState::UiState() {
}

QString UiState::getRunText() {
    QString str;
    if (run != nullptr) {
        str = run->text();
    }
    return str;
}

void UiState::setRunText(QString str) {
    if (run != nullptr) {
        run->setText(str);
    }
}

void UiState::initRun(QPushButton *pushButton) {
    run = pushButton;
}

QString UiState::getRunnerTypeText() {
    QString str;
    if (runnerType != nullptr) {
        str = runnerType->currentText();
    }
    return str;
}

void UiState::setRunnerTypeIndex(qint64 i) {
    runnerType->setCurrentIndex(i);
}

void UiState::initRunnerType(QComboBox *comboBox) {
    runnerType = comboBox;
}
