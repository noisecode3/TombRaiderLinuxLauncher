#include "Levels.h"
#include "LevelViewList.hpp"
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayoutitem.h>
#include <qnamespace.h>
#include <qradiobutton.h>
#include <qwidget.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qwebengineview.h>

UiLevels::UiLevels(QWidget *parent)
    : QWidget{parent}
{
    setObjectName("Levels");
    layout = new QGridLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    stackedWidget = new QStackedWidget(this);
    layout->addWidget(stackedWidget,0,0);

    dialog = new UiDialog(stackedWidget);
    stackedWidget->addWidget(dialog);
    stackedWidget->setCurrentWidget(dialog);

    info = new Info(stackedWidget);
    stackedWidget->addWidget(info);

    loading = new Loading(stackedWidget);
    stackedWidget->addWidget(loading);

    select = new Select(stackedWidget);
    stackedWidget->addWidget(select);

    walkthough = new Walkthrough(stackedWidget);
    stackedWidget->addWidget(walkthough);
}

UiDialog::UiDialog(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("dialog");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
}

Info::Info(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("info");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    infoContent = new InfoContent(this);
    layout->addWidget(infoContent);

    infoBar = new InfoBar(this);
    layout->addWidget(infoBar);

}

InfoContent::InfoContent(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    coverListWidget = new QListWidget(this);
    layout->addWidget(coverListWidget);

    infoWebEngineView = new QWebEngineView(this);
    layout->addWidget(infoWebEngineView);
}

InfoBar::InfoBar(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    pushButtonWalkthrough = new QPushButton(this);
    layout->addWidget(pushButtonWalkthrough);

    pushButtonBack = new QPushButton(this);
    layout->addWidget(pushButtonBack);
} 

Loading::Loading(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("loading");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
}

Walkthrough::Walkthrough(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("walkthrough");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    walkthroughWebEngineView = new QWebEngineView(this);
    layout->addWidget(walkthroughWebEngineView);

    walkthroughBar = new WalkthroughBar(this);
    layout->addWidget(walkthroughBar);
}

WalkthroughBar::WalkthroughBar(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    walkthroughBackButton = new QPushButton(this);
    layout->addWidget(walkthroughBackButton);
}

Select::Select(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("select");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    filter = new Filter(this);
    layout->addWidget(filter, 0);

    levelViewList = new LevelViewList(this);
    levelViewList->setViewMode(QListView::ListMode);
    levelViewList->setFlow(QListView::LeftToRight);
    levelViewList->setWrapping(true);

    levelViewList->setResizeMode(QListView::ResizeMode::Fixed);
    levelViewList->setUniformItemSizes(true);

    levelViewList->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    levelViewList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    levelViewList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(levelViewList, 1);

    stackedWidgetBar = new StackedWidgetBar(this);
    layout->addWidget(stackedWidgetBar, 0);
}

Filter::Filter(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("filter");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    filterFirstInputRow = new FilterFirstInputRow(this);
    layout->addWidget(filterFirstInputRow);

    filterSecondInputRow = new FilterSecondInputRow(this);
    layout->addWidget(filterSecondInputRow);
}

FilterFirstInputRow::FilterFirstInputRow(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    filterGroupBoxSearch = new FilterGroupBoxSearch(this);
    layout->addWidget(filterGroupBoxSearch);

    filterGroupBoxFilter = new FilterGroupBoxFilter(this);
    layout->addWidget(filterGroupBoxFilter);
}

FilterSecondInputRow::FilterSecondInputRow(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    filterGroupBoxToggle = new FilterGroupBoxToggle(this);
    layout->addWidget(filterGroupBoxToggle);
    layout->addSpacerItem(
    new QSpacerItem(0, 0,
        QSizePolicy::Expanding,
        QSizePolicy::Minimum)
    );
    filterGroupBoxSort = new FilterGroupBoxSort(this);
    layout->addWidget(filterGroupBoxSort);
}

FilterGroupBoxSearch::FilterGroupBoxSearch(QWidget *parent)
    : QGroupBox(parent)
{
    setObjectName("filterGroupBoxSearch");
    setTitle("Search");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    comboBoxSearch = new QComboBox(this);
    comboBoxSearch->addItems(QStringList()
        << "Level" << "Author"
        );
    layout->addWidget(comboBoxSearch);

    lineEditSearch = new QLineEdit(this);
    layout->addWidget(lineEditSearch);
}

FilterGroupBoxFilter::FilterGroupBoxFilter(QWidget *parent)
    : QGroupBox(parent)
{
    setObjectName("filterGroupBoxFilter");
    setTitle("Filter");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    labelType = new QLabel(tr("Type"), this);
    layout->addWidget(labelType);

    comboBoxType = new QComboBox(this);
    comboBoxType->addItems(QStringList()
        << " - All -" << "TR1" << "TR2" << "TR3" << "TR4" << "TR5" << "TEN"
    );
    layout->addWidget(comboBoxType);

    labelClass = new QLabel(tr("Class"), this);
    layout->addWidget(labelClass);

    comboBoxClass = new QComboBox(this);
    comboBoxClass->addItems(QStringList()
        << " - All -" << "Alien/Space" << "Atlantis" << "Base/Lab" << "Cambodia"
        << "Castle" << "Cave/Cat" << "City" << "Coastal" << "Cold/Snowy"
        << "Desert" << "Easter" << "Egypt" << "Fantasy/Surreal" << "Home"
        << "Ireland" << "Joke" << "Jungle" << "Kids" << "Library"
        << "Mystery/Horror" << "nc" << "Nordic/Celtic" << "Oriental" << "Persia"
        << "Pirates" << "Remake" << "Rome/Greece" << "Ship" << "Shooter"
        << "South America" << "South Pacific" << "Steampunk" << "Train"
        << "Venice" << "Wild West" << "Xmas" << "Young Lara"
    );
    layout->addWidget(comboBoxClass);

    labelDifficulty = new QLabel(tr("Difficulty"), this);
    layout->addWidget(labelDifficulty);

    comboBoxDifficulty = new QComboBox(this);
    comboBoxDifficulty->addItems(QStringList()
        << " - All -"
        << "easy"
        << "medium"
        << "challenging"
        << "very challenging"
    );
    layout->addWidget(comboBoxDifficulty);

    labelDuration = new QLabel(tr("Duration"), this);
    layout->addWidget(labelDuration);

    comboBoxDuration = new QComboBox(this);
    comboBoxDuration->addItems(QStringList()
        << " - All -"
        << "short"
        << "medium"
        << "long"
        << "very long"
    );
    layout->addWidget(comboBoxDuration);
}

FilterGroupBoxToggle::FilterGroupBoxToggle(QWidget *parent)
    : QGroupBox(parent)
{
    setObjectName("filterGroupBoxToggle");
    setTitle("Show");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    checkBoxInstalled = new QCheckBox(tr("Core Design Games"), this);
    layout->addWidget(checkBoxInstalled);

    checkBoxOriginal = new QCheckBox(tr("Installed Only"), this);
    layout->addWidget(checkBoxOriginal);
}

FilterGroupBoxSort::FilterGroupBoxSort(QWidget *parent)
    : QGroupBox(parent)
{
    setObjectName("filterGroupBoxSort");
    setTitle("Sort");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    radioButtonLevelName = new QRadioButton(tr("Level Name"), this);
    layout->addWidget(radioButtonLevelName);

    radioButtonDifficulty = new QRadioButton(tr("Difficulty"), this);
    layout->addWidget(radioButtonDifficulty);

    radioButtonDuration = new QRadioButton(tr("Duration"), this);
    layout->addWidget(radioButtonDuration);

    radioButtonClass = new QRadioButton(tr("Class"), this);
    layout->addWidget(radioButtonClass);

    radioButtonType = new QRadioButton(tr("Type"), this);
    layout->addWidget(radioButtonType);

    radioButtonReleaseDate = new QRadioButton(tr("Release Date"), this);
    layout->addWidget(radioButtonReleaseDate);
}

NavigateWidgetBar::NavigateWidgetBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("navigateWidgetBar");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
    //setMaximumHeight(20);

    pushButtonRun = new QPushButton(("Run"), this);
    layout->addWidget(pushButtonRun);

    checkBoxSetup = new QCheckBox(("Setup"), this);
    layout->addWidget(checkBoxSetup);

    pushButtonFiler = new QPushButton(("Filter/Sort"), this);
    layout->addWidget(pushButtonFiler);

    pushButtonInfo = new QPushButton(("Info"), this);
    layout->addWidget(pushButtonInfo);

    pushButtonDownload = new QPushButton(("Download and install"), this);
    layout->addWidget(pushButtonDownload);
}

ProgressWidgetBar::ProgressWidgetBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("progressWidgetBar");
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    progressBar = new QProgressBar(this);
    layout->addWidget(progressBar);
}

StackedWidgetBar::StackedWidgetBar(QWidget *parent)
    : QStackedWidget(parent)
{
    setObjectName("stackedWidgetBar");
    navigateWidgetBar = new NavigateWidgetBar(this);
    this->addWidget(navigateWidgetBar);
    this->setCurrentWidget(navigateWidgetBar);
    progressWidgetBar = new ProgressWidgetBar(this);
    this->addWidget(progressWidgetBar);
}

