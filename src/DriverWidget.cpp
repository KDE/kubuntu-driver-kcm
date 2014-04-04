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

#include "DriverWidget.h"

#include "ui_DriverWidget.h"

#include <QString>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>

#include <KDebug>

#include <LibQApt/Package>

#include "Device.h"

DriverWidget::DriverWidget(const Device &device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form)
    , m_radioGroup(new QButtonGroup(this))
{
    ui->setupUi(this);
    ui->label->setText(i18nc("%1 is hardware vendor, %2 is hardware model",
                             "<title>%1 %2</title>",
                             device.vendor,
                             device.model));

    // We want to sort drivers so they have consistent order across starts.
    QList<Driver> driverList = device.drivers;
    qSort(driverList);

    foreach (const Driver &driver, driverList) {
        // This driver is not manual, but also has no package, hence we cannot
        // do anything with it and should not display anything.
        if (driver.package == nullptr && !driver.manualInstall){
            kDebug() << "encountered invalid driver" << driver.package << driver.manualInstall << "for" << device.model;
            continue;
        }

        QAbstractButton *button;
        if (driverList.count() <= 1 ) {
            button = new QCheckBox(this);
            m_radioGroup->setExclusive(false);
        } else {
            button = new QRadioButton(this);
        }
        button->setProperty("package", driver.packageName);
        ui->verticalLayout->addWidget(button);
        m_radioGroup->addButton(button);

        if (driver.fuzzyActive) {
            button->setChecked(true);
        }

        if (driver.manualInstall) {
            button->setText(i18nc("Manually installed 3rd party driver",
                                  "This device is using a manually-installed driver : (%1)",
                                  driver.packageName));
            break; // Manually installed drivers have no additional information available.
        }

        if (driver.recommended) {
            button->setText(i18nc("%1 is description and %2 is package name; when driver is recommended for use",
                                  "Using %1 from %2 (Recommended Driver)",
                                  driver.package->shortDescription(),
                                  driver.package->name()));
        } else { // !recommended
            button->setText(i18nc("%1 is description and %2 is package name",
                                  "Using %1 from %2",
                                  driver.package->shortDescription(),
                                  driver.package->name()));
        }

        if (driver.free) {
            button->setToolTip(i18nc("The driver is under a open source license",
                                     "Open Source Driver"));
        } else { // !free
            button->setToolTip(i18nc("The driver is under a proprietary license",
                                     "Proprietary Driver"));
        }
    }

    m_indexSelected = m_radioGroup->checkedId();
    m_defaultSelection = m_indexSelected;
    connect(m_radioGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SIGNAL(selectionChanged()));
}

DriverWidget::~DriverWidget()
{
    delete ui;
}

#warning dat name
QString DriverWidget::selectedPackageName() const
{
    if (m_radioGroup->checkedButton()) {
        return m_radioGroup->checkedButton()->property("package").toString();
    }

    return QString();
}

QStringList DriverWidget::notSelectedPackageNames() const
{
    QStringList list;
    foreach (const QAbstractButton *button, m_radioGroup->buttons()) {
        if (button && button != m_radioGroup->checkedButton()) {
            list.append(button->property("package").toString());
        }
    }
    return list;
}

void DriverWidget::setDefaultSelection()
{
    QAbstractButton *defaultButton = m_radioGroup->button(m_defaultSelection);
    if (defaultButton) {
        defaultButton->setChecked(true);
    }
}

bool DriverWidget::hasChanged() const
{
    if (m_radioGroup->checkedId() /* current */ !=  /* reference */ m_indexSelected) {
        return true;
    }

    return false;
}
