/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2014  Rohan Garg <rohan16garg@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DRIVERWIDGET_H
#define DRIVERWIDGET_H

#include "drivermanagerdbustypes.h"
#include <QWidget>

namespace Ui {
    class Form;
}

namespace QApt {
    class Backend;
}

class QButtonGroup;
class QAbstractButton;
class DriverWidget : public QWidget
{
    Q_OBJECT


public:
    DriverWidget(const QVariantMapMap& map, const QString& label, QApt::Backend* backend, QWidget* parent);
    ~DriverWidget();
    QString getSelectedPackageStr() const;

Q_SIGNALS:
    void changed();

private:
    //UI
    Ui::Form *ui;
    QList<QWidget*> m_widgetList;
    bool m_manualInstalled;
    bool m_nonFreeInstalled;
    QApt::Backend* m_backend;
    QButtonGroup* m_radioGroup;
    bool isActive(QString key, QVariantMapMap map);

private Q_SLOTS:
    void emitChanged(QAbstractButton*);
};

#endif // DRIVERWIDGET_H
