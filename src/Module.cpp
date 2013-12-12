/*
    Copyright (C) 2013 Rohan Garg <rohangarg@kubuntu.org>
    Copyright (C) 2013 Harald Sitter <apachelogger@kubuntu.org>

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

#include "Module.h"
#include "ui_Module.h"

#include "drivermanager_interface.h"

#include <KAboutData>
#include <KDebug>
#include <KPluginFactory>
#include <KToolInvocation>
#include <KFontRequester>

#include <QLabel>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRadioButton>
#include <QButtonGroup>
#include <qdbusmetatype.h>

#include "Version.h"


#include <unistd.h>

K_PLUGIN_FACTORY_DECLARATION(KcmDriverFactory);

Module::Module(QWidget *parent, const QVariantList &args)
    : KCModule(KcmDriverFactory::componentData(), parent, args)
    , ui(new Ui::Module)
    , m_refresh(false)
{
    KAboutData *about = new KAboutData("kcm-drivermanager", 0,
                                       ki18n("((Name))"),
                                       global_s_versionStringFull,
                                       KLocalizedString(),
                                       KAboutData::License_GPL_V3,
                                       ki18n("Copyright 2013 Rohan Garg"),
                                       KLocalizedString(), QByteArray(),
                                       "rohangarg@kubuntu.org");

    about->addAuthor(ki18n("Rohan Garg"), ki18n("Author"), "rohangarg@kubuntu.org");
    setAboutData(about);
    m_manager = new ComKubuntuDriverManagerInterface("org.kubuntu.DriverManager", "/DriverManager", QDBusConnection::sessionBus());
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked(bool)), SLOT(refreshDriverList(bool)));
//     connect(this, SIGNAL(this->buttons().))
    qDBusRegisterMetaType<QVariantMapMap>();

    // We have no help so remove the button from the buttons.
    setButtons(buttons() ^ KCModule::Help);
}

Module::~Module()
{
    delete ui;
}

void Module::load()
{
    kDebug();
    QDBusPendingReply<QVariantMapMap> map = m_manager->getDriverDict(m_refresh);
    if (m_refresh) {
        m_refresh = false;
    }
    QDBusPendingCallWatcher *async = new QDBusPendingCallWatcher(map, this);

    connect(async, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(driverDictFinished(QDBusPendingCallWatcher*)));
}

void Module::driverDictFinished(QDBusPendingCallWatcher* data)
{
    kDebug();
    QDBusPendingReply<QVariantMapMap> mapReply = *data;
    if (mapReply.isError()) {
        kWarning() << "DBus data corrupted";
        return;
    }

    QVariantMapMap dictMap = mapReply.value();

    Q_FOREACH(const QVariantMap &mapValue, dictMap.values()) {
        QString key = dictMap.key(mapValue);

        //Device Name extraction from Map
        QVariant vendor = mapValue.value("vendor");
        QVariant model = mapValue.value("model");
        // TODO: Fix the following so it uses KDE Font sizes
        QString label = "<h3>" + vendor.toString() + " " + model.toString() + "</h3>";

        QDBusPendingReply<QVariantMapMap> driverForDeviceMap = m_manager->getDriverMapForDevice(key);
        QDBusPendingCallWatcher *async = new QDBusPendingCallWatcher(driverForDeviceMap, this);
        async->setProperty("Name", label);
        connect(async, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(driverMapFinished(QDBusPendingCallWatcher*)));
    }
    data->deleteLater();
}

void Module::driverMapFinished(QDBusPendingCallWatcher* data)
{
    kDebug();
    QString deviceName = data->property("Name").toString();
    QDBusPendingReply<QVariantMapMap> mapReply = *data;
    if (mapReply.isError()) {
        kWarning() << "DBus data corrupted";
        return;
    }

    QVariantMapMap driverMap = mapReply.value();
    ui->driverOptionsVLayout->addWidget(new QLabel(deviceName));
    QButtonGroup *radioGroup = new QButtonGroup(this);
    m_buttonListGroup.append(radioGroup);
        Q_FOREACH(const QString &key, driverMap.keys()) {
            QVBoxLayout *internalVLayout = new QVBoxLayout();
            internalVLayout->setParent(ui->driverOptionsVLayout);
            ui->driverOptionsVLayout->addLayout(internalVLayout);
            QString driverString = key;
            QRadioButton *button = new QRadioButton(driverString);
            button->setProperty("driver", key);
            internalVLayout->addWidget(button);
            radioGroup->addButton(button);
        }

    data->deleteLater();
}

void Module::refreshDriverList(bool)
{
    kDebug();
    delete ui;
    ui = new Ui::Module;
    ui->setupUi(this);
    m_refresh=true;
    load();
}


void Module::save()
{

}

void Module::defaults()
{

}
