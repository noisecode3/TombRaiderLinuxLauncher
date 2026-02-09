/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "view/Levels/Select/Filter.hpp"
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <qnamespace.h>

Filter::Filter(QWidget *parent)
    : QWidget(parent),
    filterFirstInputRow(new FilterFirstInputRow(this)),
    filterSecondInputRow(new FilterSecondInputRow(this)),
    layout(new QVBoxLayout(this))
{
    setObjectName("filter");
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);

    layout->addWidget(filterFirstInputRow);
    layout->addWidget(filterSecondInputRow);
}

FilterFirstInputRow::FilterFirstInputRow(QWidget *parent)
    : QWidget(parent),
    filterGroupBoxSearch(new FilterGroupBoxSearch(this)),
    filterGroupBoxFilter(new FilterGroupBoxFilter(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    layout->addWidget(filterGroupBoxSearch);
    layout->addWidget(filterGroupBoxFilter);
}

FilterSecondInputRow::FilterSecondInputRow(QWidget *parent)
    : QWidget(parent),
    filterGroupBoxToggle(new FilterGroupBoxToggle(this)),
    filterGroupBoxSort(new FilterGroupBoxSort(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    layout->addWidget(filterGroupBoxToggle);
    layout->addSpacerItem(
    new QSpacerItem(0, 0,
        QSizePolicy::Expanding,
        QSizePolicy::Minimum)
    );
    layout->addWidget(filterGroupBoxSort);
}

FilterGroupBoxSearch::FilterGroupBoxSearch(QWidget *parent)
    : QGroupBox(parent),
    comboBoxSearch(new QComboBox(this)),
    lineEditSearch(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("filterGroupBoxSearch");
    setTitle("Search");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    comboBoxSearch->addItems(QStringList()
        << "Level" << "Author"
        );
    layout->addWidget(comboBoxSearch);
    layout->addWidget(lineEditSearch);
}

FilterGroupBoxFilter::FilterGroupBoxFilter(QWidget *parent)
    : QGroupBox(parent),
    labelType(new QLabel(tr("Type"), this)),
    comboBoxType(new QComboBox(this)),
    labelClass(new QLabel(tr("Class"), this)),
    comboBoxClass(new QComboBox(this)),
    labelDifficulty(new QLabel(tr("Difficulty"), this)),
    comboBoxDifficulty(new QComboBox(this)),
    labelDuration(new QLabel(tr("Duration"), this)),
    comboBoxDuration(new QComboBox(this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("filterGroupBoxFilter");
    setTitle("Filter");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(labelType);
    comboBoxType->addItems(QStringList()
        << " - All -" << "TR1" << "TR2" << "TR3" << "TR4" << "TR5" << "TEN"
    );
    layout->addWidget(comboBoxType);
    layout->addWidget(labelClass);
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
    layout->addWidget(labelDifficulty);
    comboBoxDifficulty->addItems(QStringList()
        << " - All -"
        << "easy"
        << "medium"
        << "challenging"
        << "very challenging"
    );
    layout->addWidget(comboBoxDifficulty);
    layout->addWidget(labelDuration);
    comboBoxDuration->addItems(QStringList()
        << " - All -"
        << "short"
        << "medium"
        << "long"
        << "very long"
    );
    layout->addWidget(comboBoxDuration);
}

void FilterGroupBoxFilter::showFilterSelectionMenu()
{
    QPoint globalPos = this->mapToGlobal(QPoint(42, 0));
    QMenu menu;
    // Add actions for each filter
    QAction *classAct = menu.addAction(tr("&Class"));
    QAction *diffAct = menu.addAction(tr("&Difficulty"));
    QAction *durAct = menu.addAction(tr("&Duration"));
    QAction *typeAct = menu.addAction(tr("&Type"));

    // Show the menu at the given position
    QAction *chosen = menu.exec(globalPos);
    if (!chosen) return;

    // Focus the right combo box
    if (chosen == classAct) {
        comboBoxClass->setFocus(Qt::ShortcutFocusReason);
        comboBoxClass->showPopup();
    } else if (chosen == diffAct) {
        comboBoxDifficulty->setFocus(Qt::ShortcutFocusReason);
        comboBoxDifficulty->showPopup();
    } else if (chosen == durAct) {
        comboBoxDuration->setFocus(Qt::ShortcutFocusReason);
        comboBoxDuration->showPopup();
    } else if (chosen == typeAct) {
        comboBoxType->setFocus(Qt::ShortcutFocusReason);
        comboBoxType->showPopup();
    }
}


FilterGroupBoxToggle::FilterGroupBoxToggle(QWidget *parent)
    : QGroupBox(parent),
    checkBoxOriginal(new QCheckBox(tr("Core Design Games"), this)),
    checkBoxInstalled(new QCheckBox(tr("Installed Only"), this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("filterGroupBoxToggle");
    setTitle("Show");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(checkBoxOriginal);
    layout->addWidget(checkBoxInstalled);
}

FilterGroupBoxSort::FilterGroupBoxSort(QWidget *parent)
    : QGroupBox(parent),
        radioButtonLevelName(new QRadioButton(tr("Level Name"), this)),
        radioButtonDifficulty(new QRadioButton(tr("Difficulty"), this)),
        radioButtonDuration(new QRadioButton(tr("Duration"), this)),
        radioButtonClass(new QRadioButton(tr("Class"), this)),
        radioButtonType(new QRadioButton(tr("Type"), this)),
        radioButtonReleaseDate(new QRadioButton(tr("Release Date"), this)),
        layout(new QHBoxLayout(this))
{
    setObjectName("filterGroupBoxSort");
    setTitle("Sort");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(radioButtonLevelName);
    layout->addWidget(radioButtonDifficulty);
    layout->addWidget(radioButtonDuration);
    layout->addWidget(radioButtonClass);
    layout->addWidget(radioButtonType);
    layout->addWidget(radioButtonReleaseDate);
    radioButtonReleaseDate->blockSignals(true);
    radioButtonReleaseDate->setChecked(true);
    radioButtonReleaseDate->blockSignals(false);
}

void FilterGroupBoxSort::isSelected(QRadioButton *selected) {
    selected->setFocus(Qt::ShortcutFocusReason);
}

void FilterGroupBoxSort::focusSelected() {
    if (radioButtonLevelName->isChecked()) {
        isSelected(radioButtonLevelName);
    } else if (radioButtonDifficulty->isChecked()) {
        isSelected(radioButtonDifficulty);
    } else if (radioButtonDuration->isChecked()) {
        isSelected(radioButtonDuration);
    } else if (radioButtonClass->isChecked()) {
        isSelected(radioButtonClass);
    } else if (radioButtonType->isChecked()) {
        isSelected(radioButtonType);
    } else if (radioButtonReleaseDate->isChecked()) {
        isSelected(radioButtonReleaseDate);
    }
}
