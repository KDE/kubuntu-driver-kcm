/*
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
 */

#include "DriverWidget.h"

#include "ui_DriverWidget.h"

#include <QString>
#include <QRadioButton>
#include <QButtonGroup>

#include <KDebug>

#include <LibQApt/Backend>

DriverWidget::DriverWidget(const QVariantMapMap& map, const QString& label, QApt::Backend* backend, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Form)
    , m_manualInstalled(false)
    , m_nonFreeInstalled(false)
{
    m_backend = backend;
    ui->setupUi(this);
    ui->label->setText(label);

    QRadioButton *button;
    m_radioGroup = new QButtonGroup();

    Q_FOREACH (const QString &key, map.keys()) {
        QApt::Package *pkg = m_backend->package(key);

        if (pkg) {
            QString driverString = i18nc("%1 is description and %2 is package name",
                                            "Using %1 from %2",
                                            pkg->shortDescription(),
                                            pkg->name());
            if (map[key]["recommended"].toBool()) {
                driverString += i18nc("This particular driver is a recommended driver",
                                                        " (Recommended Driver)");
            }

            button = new QRadioButton(driverString);

            if (map[key]["free"].toBool()) {
                button->setToolTip(i18nc("The driver is under a open source license",
                                         "Open Source Driver"));
            } else {
                button->setToolTip(i18nc("The driver is under a proprietary license",
                                         "Proprietary Driver"));
            }

            button->setProperty("driver", key);
            if (isActive(key, map)) {
                button->setChecked(true);
            }
            ui->verticalLayout->addWidget(button);
            m_radioGroup->addButton(button);
            m_widgetList.append(button);
        } else {
            // *Most* likely a manually installed driver. Check and add a Manual radio button
            bool isManual = map[key].value("manual_install").toBool();
            if (isManual) {
                m_manualInstalled = true;
                button = new QRadioButton(i18nc("Manually installed 3rd party driver", "This device is using a manually-installed driver : (%1)", key));
                button->setChecked(true);
                ui->verticalLayout->addWidget(button);
                m_radioGroup->addButton(button);
                m_widgetList.append(button);
            }
        }

    }

    connect(m_radioGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(emitChanged(QAbstractButton*)));
}

DriverWidget::~DriverWidget()
{
    delete m_radioGroup;
    qDeleteAll(m_widgetList);
    delete ui;
}

QString DriverWidget::getSelectedPackageStr() const
{
    if (m_radioGroup->checkedButton()) {
        return m_radioGroup->checkedButton()->property("driver").toString();
    }

    return QString();
}


bool DriverWidget::isActive(QString key, QVariantMapMap map)
{
    // Nothing matters if manual driver or non free driver is installed
    if (m_manualInstalled || m_nonFreeInstalled) {
        return false;
    }

    QApt::Package *pkg = m_backend->package(key);
    bool isFree = map[key].value("free").toBool();

    // Handle non free and free drivers that are installed
    if (pkg->isInstalled()) {
        if (!isFree) {
            m_nonFreeInstalled = true;
        }
        return true;
    }

    return false;
}

void DriverWidget::emitChanged(QAbstractButton*)
{
    emit changed();
}
