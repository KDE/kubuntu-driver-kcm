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
#include <QButtonGroup>

#include <KDebug>

#include <LibQApt/Backend>

#include "Device.h"

DriverWidget::DriverWidget(const Device &device, QApt::Backend *backend, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form)
    , m_manualInstalled(false)
    , m_nonFreeInstalled(false)
{
    m_backend = backend;
    ui->setupUi(this);
    ui->label->setText(i18nc("%1 is hardware vendor, %2 is hardware model",
                             "<title>%1 %2</title>",
                             device.vendor,
                             device.model));

    QRadioButton *button;
    m_radioGroup = new QButtonGroup(this);

    // We want to sort drivers so they have consistent order across starts.
    QList<Driver> driverList = device.drivers;
    qSort(driverList);

    foreach (const Driver &driver, device.drivers) {
        const QApt::Package *package = m_backend->package(driver.packageName);

        if (package) {
            QString driverString = i18nc("%1 is description and %2 is package name",
                                            "Using %1 from %2",
                                            package->shortDescription(),
                                            package->name());
            if (driver.recommended) {
                driverString += i18nc("This particular driver is a recommended driver",
                                                        " (Recommended Driver)");
            }

            button = new QRadioButton(driverString, this);

            if (driver.free) {
                button->setToolTip(i18nc("The driver is under a open source license",
                                         "Open Source Driver"));
            } else {
                button->setToolTip(i18nc("The driver is under a proprietary license",
                                         "Proprietary Driver"));
            }

            button->setProperty("package", driver.packageName);
            if (isActive(driver, package)) {
                button->setChecked(true);
            }
            ui->verticalLayout->addWidget(button);
            m_radioGroup->addButton(button);
        } else {
            // *Most* likely a manually installed driver. Check and add a Manual radio button
            if (driver.manualInstall) {
                m_manualInstalled = true;
                button = new QRadioButton(i18nc("Manually installed 3rd party driver",
                                                "This device is using a manually-installed driver : (%1)",
                                                driver.packageName),
                                          this);
                button->setChecked(true);
                ui->verticalLayout->addWidget(button);
                m_radioGroup->addButton(button);
            }
        }
    }

    m_indexSelected = m_radioGroup->checkedId();
    m_defaultSelection = m_indexSelected;
    connect(m_radioGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(hasChanged(QAbstractButton*)));
}

DriverWidget::~DriverWidget()
{
    delete ui;
}

#warning dat name
QString DriverWidget::getSelectedPackageStr() const
{
    if (m_radioGroup->checkedButton()) {
        return m_radioGroup->checkedButton()->property("package").toString();
    }

    return QString();
}

bool DriverWidget::isActive(const Driver &driver, const QApt::Package *package)
{
    // Nothing matters if manual driver or non free driver is installed
#warning why is m_nonFreeInstalled cached at all, this function only ought to be called once
    if (driver.manualInstall || m_nonFreeInstalled) {
        return false;
    }

    // Handle non free and free drivers that are installed
    if (package->isInstalled()) {
        if (!driver.free) {
            m_nonFreeInstalled = true;
        }
        return true;
    }

    return false;
}

void DriverWidget::hasChanged(QAbstractButton *button)
{
#warning the question is why
    Q_UNUSED(button);

    const int id = m_radioGroup->checkedId();

    if (id != m_indexSelected) {
        m_indexSelected = m_radioGroup->checkedId();
        emit changed(true);
    }

    if (id == m_defaultSelection) {
        emit changed(false);
    }
}

void DriverWidget::setDefaultSelection()
{
    QAbstractButton *defaultButton = m_radioGroup->button(m_defaultSelection);
    if (defaultButton) {
        defaultButton->setChecked(true);
    }
}
