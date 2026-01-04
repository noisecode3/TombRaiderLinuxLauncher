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

#ifndef VIEW_LEVELS_FILTER_HPP_
#define VIEW_LEVELS_FILTER_HPP_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

class FilterGroupBoxFilter : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterFirstInputRow)
     * FilterGroupBoxFilter
     * ├── labelClass
     * ├── comboBoxClass
     * ├── labelDifficulty
     * ├── comboBoxDifficulty
     * ├── labelDuration
     * ├── comboBoxDuration
     * ├── labelType
     * └── comboBoxType
     */
    explicit FilterGroupBoxFilter(QWidget *parent);
    QLabel *labelClass{nullptr};
    QComboBox *comboBoxClass{nullptr};
    QLabel *labelDifficulty{nullptr};
    QComboBox *comboBoxDifficulty{nullptr};
    QLabel *labelDuration{nullptr};
    QComboBox *comboBoxDuration{nullptr};
    QLabel *labelType{nullptr};
    QComboBox *comboBoxType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxSearch : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterFirstInputRow)
     * FilterGroupBoxSearch
     * ├── comboBoxSearch
     * └── lineEditSearch
     */
    explicit FilterGroupBoxSearch(QWidget *parent);
    QComboBox *comboBoxSearch{nullptr};
    QLineEdit *lineEditSearch{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxSort : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterSecondInputRow)
     * FilterGroupBoxSort
     * ├── radioButtonClass
     * ├── radioButtonDifficulty
     * ├── radioButtonDuration
     * ├── radioButtonLevelName
     * ├── radioButtonReleaseDate
     * └── radioButtonType
     */
    explicit FilterGroupBoxSort(QWidget *parent);
    QRadioButton *radioButtonClass{nullptr};
    QRadioButton *radioButtonDifficulty{nullptr};
    QRadioButton *radioButtonDuration{nullptr};
    QRadioButton *radioButtonLevelName{nullptr};
    QRadioButton *radioButtonReleaseDate{nullptr};
    QRadioButton *radioButtonType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxToggle : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter-filterSecondInputRow)
     * FilterGroupBoxToggle
     * ├── checkBoxInstalled
     * └── checkBoxOriginal
     */
    explicit FilterGroupBoxToggle(QWidget *parent);
    QCheckBox *checkBoxInstalled{nullptr};
    QCheckBox *checkBoxOriginal{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterFirstInputRow : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter)
     * FilterFirstInputRow
     * ├── filterGroupBoxFilter ->
     * └── filterGroupBoxSearch ->
     */
    explicit FilterFirstInputRow(QWidget* parent);
    FilterGroupBoxFilter *filterGroupBoxFilter;
    FilterGroupBoxSearch *filterGroupBoxSearch;
private:
    QHBoxLayout *layout{nullptr};
};

class FilterSecondInputRow : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter)
     * FilterSecondInputRow
     * ├── filterGroupBoxSort ->
     * └── filterGroupBoxToggle ->
     */
    explicit FilterSecondInputRow(QWidget *parent);
    FilterGroupBoxSort *filterGroupBoxSort;
    FilterGroupBoxToggle *filterGroupBoxToggle;
private:
    QHBoxLayout *layout{nullptr};
};

class Filter : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select)
     * Filter
     * ├── filterFirstInputRow ->
     * └── filterSecondInputRow ->
     */
    explicit Filter(QWidget *parent);
    FilterFirstInputRow *filterFirstInputRow{nullptr};
    FilterSecondInputRow *filterSecondInputRow{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

#endif  // VIEW_LEVELS_FILTER_HPP_
