#include "view/Ui.h"

Ui::Ui(QWidget *parent)
    : QWidget(parent)
{
    this->setObjectName("centralwidget");

    layout = new QGridLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(8);

    tabs = new QTabWidget(this);

    levels = new UiLevels(tabs);
    tabs->addTab(levels,     tr("Levels"));

    modding = new UiModding(tabs);
    tabs->addTab(modding,    tr("Modding"));

    controller = new UiController(tabs);
    tabs->addTab(controller, tr("Controller"));

    setup = new UiSetup(tabs);
    tabs->addTab(setup,      tr("Setup"));

    about = new UiAbout(tabs);
    tabs->addTab(about,      tr("About"));

    layout->addWidget(tabs);
}

/*
void Ui::setupUi()
{
    // Central widget and main grid

    gridLayoutCentalGrid = new QGridLayout(centralwidget);
    gridLayoutCentalGrid->setObjectName("gridLayoutCentalGrid");
    gridLayoutCentalGrid->setContentsMargins(6,6,6,6);
    gridLayoutCentalGrid->setSpacing(6);


    setupTabs();

    // Put Tabs to stretch to full available area
    gridLayoutCentalGrid->addWidget(Tabs, 0, 0);
}

void Ui::setupTabs()
{
    Tabs = new QTabWidget(centralwidget);
    Tabs->setObjectName("Tabs");
    Tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Levels Tab
    Levels = new QWidget();
    Levels->setObjectName("Levels");
    gridLayoutModdsGrid = new QGridLayout(Levels);
    gridLayoutModdsGrid->setObjectName("gridLayoutModdsGrid");
    gridLayoutModdsGrid->setContentsMargins(4,4,4,4);
    gridLayoutModdsGrid->setSpacing(6);

    setupLevelsTab();

    Tabs->addTab(Levels, tr("Levels"));

    // About Tab
    About = new QWidget();
    About->setObjectName("About");
    gridLayoutAboutGrid = new QGridLayout(About);
    gridLayoutAboutGrid->setObjectName("gridLayoutAboutGrid");
    gridLayoutAboutGrid->setContentsMargins(4,4,4,4);
    gridLayoutAboutGrid->setSpacing(6);

    setupAboutTab();

    Tabs->addTab(About, tr("About"));
}

void Ui::setupLevelsTab()
{
    // Stacked widget area (keeps UI similar to original .ui)
    stackedWidget = new QStackedWidget(Levels);
    stackedWidget->setObjectName("stackedWidget");
    stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // "select" page
    select = new QWidget();
    select->setObjectName("select");
    verticalLayout = new QVBoxLayout(select);
    verticalLayout->setObjectName("verticalLayout");
    verticalLayout->setContentsMargins(0,0,0,0);
    verticalLayout->setSpacing(6);

    // Filter area (collapsible in future if desired)
    setupFilterSection();

    // Main content area - use splitter to give user adjustable panes
    QSplitter *mainSplitter = new QSplitter(select);
    mainSplitter->setOrientation(Qt::Horizontal);
    mainSplitter->setObjectName("mainSplitter");

    // Left: list/tree of levels
    QWidget *leftPane = new QWidget(mainSplitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(4);

    levelsListView = new QListView(leftPane);
    levelsListView->setObjectName("levelsListView");
    levelsListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    leftLayout->addWidget(levelsListView);

    // Right: details / preview
    QWidget *rightPane = new QWidget(mainSplitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(0,0,0,0);
    rightLayout->setSpacing(4);

    levelsTreeView = new QTreeView(rightPane);
    levelsTreeView->setObjectName("levelsTreeView");
    levelsTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    rightLayout->addWidget(levelsTreeView);

    mainSplitter->addWidget(leftPane);
    mainSplitter->addWidget(rightPane);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 5);

    verticalLayout->addWidget(mainSplitter);

    // Paths / options area
    setupPathsSection();

    // Add stretch to push controls up
    verticalLayout->addStretch(1);

    stackedWidget->addWidget(select);

    // Add stackedWidget to the Levels tab grid
    gridLayoutModdsGrid->addWidget(stackedWidget, 0, 0);
}

void Ui::setupFilterSection()
{
    // Filter root
    filter = new QWidget(select);
    filter->setObjectName("filter");
    verticalLayout_12 = new QVBoxLayout(filter);
    verticalLayout_12->setObjectName("verticalLayout_12");
    verticalLayout_12->setContentsMargins(0,0,0,0);
    verticalLayout_12->setSpacing(4);

    // Filter input widget
    FilterInputWidget = new QWidget(filter);
    FilterInputWidget->setObjectName("FilterInputWidget");
    verticalLayout_11 = new QVBoxLayout(FilterInputWidget);
    verticalLayout_11->setObjectName("verticalLayout_11");
    verticalLayout_11->setContentsMargins(0,0,0,0);
    verticalLayout_11->setSpacing(4);

    // First input row
    firstInputRow = new QWidget(FilterInputWidget);
    firstInputRow->setObjectName("firstInputRow");
    horizontalLayout_10 = new QHBoxLayout(firstInputRow);
    horizontalLayout_10->setObjectName("horizontalLayout_10");
    horizontalLayout_10->setContentsMargins(0,0,0,0);
    horizontalLayout_10->setSpacing(6);

    // Search group box
    groupBoxSearch = new QGroupBox(firstInputRow);
    groupBoxSearch->setObjectName("groupBoxSearch");
    groupBoxSearch->setTitle(tr("Search"));
    groupBoxSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *gbLayout = new QHBoxLayout(groupBoxSearch);
    gbLayout->setContentsMargins(6,6,6,6);

    QLineEdit *searchEdit = new QLineEdit(groupBoxSearch);
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Filter levels..."));
    searchEdit->setClearButtonEnabled(true);

    QPushButton *searchButton = new QPushButton(groupBoxSearch);
    searchButton->setObjectName("searchButton");
    searchButton->setText(tr("Search"));

    gbLayout->addWidget(searchEdit);
    gbLayout->addWidget(searchButton);

    horizontalLayout_10->addWidget(groupBoxSearch);

    verticalLayout_11->addWidget(firstInputRow);

    // Potential extra filter rows - kept minimal
    QWidget *secondRow = new QWidget(FilterInputWidget);
    QHBoxLayout *secondRowLayout = new QHBoxLayout(secondRow);
    secondRowLayout->setContentsMargins(0,0,0,0);
    secondRowLayout->setSpacing(6);

    // Example: category combo or checkboxes could go here (placeholders)
    QLabel *placeholderFilter = new QLabel(tr("Filters: (type, tags...)"), secondRow);
    secondRowLayout->addWidget(placeholderFilter);
    secondRowLayout->addStretch(1);

    verticalLayout_11->addWidget(secondRow);

    verticalLayout_12->addWidget(FilterInputWidget);

    // Attach the filter widget at top of select page
    verticalLayout->addWidget(filter);
}

void Ui::setupPathsSection()
{
    // Extra game path container
    extraGamePathContainer = new QWidget(select);
    extraGamePathContainer->setObjectName("extraGamePathContainer");
    horizontalLayout_20 = new QHBoxLayout(extraGamePathContainer);
    horizontalLayout_20->setObjectName("horizontalLayout_20");
    horizontalLayout_20->setContentsMargins(0,0,0,0);
    horizontalLayout_20->setSpacing(6);

    extraGamePathLabel = new QLabel(extraGamePathContainer);
    extraGamePathLabel->setObjectName("extraGamePathLabel");
    extraGamePathLabel->setText(tr("Extra game path:"));
    extraGamePathEdit = new QLineEdit(extraGamePathContainer);
    extraGamePathEdit->setObjectName("extraGamePathEdit");

    horizontalLayout_20->addWidget(extraGamePathLabel);
    horizontalLayout_20->addWidget(extraGamePathEdit);

    verticalLayout->addWidget(extraGamePathContainer);

    // Level directory info label
    labelLevelDirectoryInfo = new QLabel(select);
    labelLevelDirectoryInfo->setObjectName("labelLevelDirectoryInfo");
    labelLevelDirectoryInfo->setText(tr("No level directory selected"));
    labelLevelDirectoryInfo->setWordWrap(true);

    verticalLayout->addWidget(labelLevelDirectoryInfo);

    // Level path container
    levelPathContainer = new QWidget(select);
    levelPathContainer->setObjectName("levelPathContainer");
    horizontalLayout_2 = new QHBoxLayout(levelPathContainer);
    horizontalLayout_2->setObjectName("horizontalLayout_2");
    horizontalLayout_2->setContentsMargins(0,0,0,0);
    horizontalLayout_2->setSpacing(6);

    levelPathLabel = new QLabel(levelPathContainer);
    levelPathLabel->setObjectName("levelPathLabel");
    levelPathLabel->setText(tr("Level path:"));
    levelPathEdit = new QLineEdit(levelPathContainer);
    levelPathEdit->setObjectName("levelPathEdit");

    horizontalLayout_2->addWidget(levelPathLabel);
    horizontalLayout_2->addWidget(levelPathEdit);

    setOptions = new QPushButton(select);
    setOptions->setObjectName("setOptions");
    setOptions->setText(tr("Set Options"));
    setOptions->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Place level path container and options button in one row
    QWidget *pathRow = new QWidget(select);
    QHBoxLayout *pathRowLayout = new QHBoxLayout(pathRow);
    pathRowLayout->setContentsMargins(0,0,0,0);
    pathRowLayout->setSpacing(6);
    pathRowLayout->addWidget(levelPathContainer);
    pathRowLayout->addWidget(setOptions);

    verticalLayout->addWidget(pathRow);
}

void Ui::setupAboutTab()
{
    frameAbout = new QFrame(About);
    frameAbout->setObjectName("frameAbout");
    frameAbout->setFrameShape(QFrame::NoFrame);
    frameAbout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    verticalLayout_5 = new QVBoxLayout(frameAbout);
    verticalLayout_5->setObjectName("verticalLayout_5");
    verticalLayout_5->setContentsMargins(4,4,4,4);
    verticalLayout_5->setSpacing(6);

    textBrowserAbout = new QTextBrowser(frameAbout);
    textBrowserAbout->setObjectName("textBrowserAbout");
    textBrowserAbout->setPlainText(tr("Tomb Raider Launcher\n\nThis application was migrated from a .ui Designer file into a stable, Designer-free C++ implementation. Replace this text with your about information."));
    textBrowserAbout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    verticalLayout_5->addWidget(textBrowserAbout);

    gridLayoutAboutGrid->addWidget(frameAbout, 0, 0);
}
*/
