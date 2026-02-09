#ifndef UISTATE_HPP_
#define UISTATE_HPP_

#include <QPushButton>
#include <QComboBox>

class UiState
{
    UiState();
public:
    static UiState& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static UiState instance;
        return instance;
    }
    QString getRunText();
    QString getRunnerTypeText();
    void setRunText(QString str);
    void setRunnerTypeIndex(qint64 i);
    void initRun(QPushButton *pushButton);
    void initRunnerType(QComboBox *comboBox);
private:
    QPushButton *run{nullptr};
    QComboBox *runnerType{nullptr};
};

#endif // UISTATE_HPP_
