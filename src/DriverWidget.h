/*
    SPDX-FileCopyrightText: 2014 Rohan Garg <rohan@kde.org>
    SPDX-FileCopyrightText: 2014 Harald Sitter <apachelogger@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DRIVERWIDGET_H
#define DRIVERWIDGET_H

#include <QWidget>

namespace Ui {
class Form;
}

class QButtonGroup;
class QAbstractButton;

class Device;

class DriverWidget : public QWidget
{
    Q_OBJECT
public:
    DriverWidget(const Device &device, QWidget *parent = 0);
    ~DriverWidget();

    QString selectedPackageName() const;
    QStringList notSelectedPackageNames() const;

    void setSelectionEnabled(bool enabled);
    void setDefaultSelection();

    bool hasChanged() const;

signals:
    void selectionChanged();

private:
    Ui::Form *ui;
    QButtonGroup *m_radioGroup;
    int m_indexSelected;
    int m_defaultSelection;
};

#endif // DRIVERWIDGET_H
