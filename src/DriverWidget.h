/*
    Copyright (C) 2014  Rohan Garg <rohan@kde.org>
    Copyright (C) 2014 Harald Sitter <apachelogger@kubuntu.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    QString getSelectedPackageStr() const;
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
