/*
    Copyright (C) 2013 Rohan Garg <rohan@kde.org>
    Copyright (C) 2013-2014 Harald Sitter <apachelogger@kubuntu.org>

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
#include "DriverWidget.h"

#include "ui_Module.h"

#include <KAboutData>
#include <KDebug>
#include <KPluginFactory>
#include <KMessageBox>
#include <KMessageWidget>
#include <KPixmapSequenceOverlayPainter>
#include <DebconfGui.h>

#include <QDir>
#include <QLabel>
#include <QUuid>
#include <QStringBuilder>

#include "DriverManager.h"
#include "Version.h"

K_PLUGIN_FACTORY_DECLARATION(KcmDriverFactory);

Module::Module(QWidget *parent, const QVariantList &args)
    : KCModule(KcmDriverFactory::componentData(), parent, args)
    , ui(new Ui::Module)
    , m_manager(new DriverManager(this))
{
    KAboutData *about = new KAboutData("kcm-driver-manager", 0,
                                       ki18n("Driver Manager"),
                                       global_s_versionStringFull,
                                       KLocalizedString(),
                                       KAboutData::License_GPL_V3,
                                       ki18n("Copyright 2013 Rohan Garg"),
                                       KLocalizedString(), QByteArray(),
                                       "rohangarg@kubuntu.org");

    about->addAuthor(ki18n("Rohan Garg"), ki18n("Author"), "rohangarg@kubuntu.org");
    setAboutData(about);

    // Force libmuon as additional l10n catalog.
    // This is necessary to introduce a new set of strings that previously only
    // were available in muon and libmuon to resolve LP: #1315670 for 14.04 without
    // ending up with untranslated strings.
    KGlobal::locale()->insertCatalog(QLatin1String("libmuon"));

    // We have no help so remove the button from the buttons.
    setButtons(buttons() ^ KCModule::Help);

    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    connect(ui->reloadButton, SIGNAL(clicked(bool)), SLOT(load()));

    m_overlay = new KPixmapSequenceOverlayPainter(this);
    m_overlay->setWidget(this);

#warning variable name
    QString label = i18n("<title>Your computer requires no proprietary drivers</title>");
    m_label = new QLabel(label, this);
    m_label->hide();
    ui->driverOptionsVLayout->addWidget(m_label);

    //Debconf handling
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{').remove('}').remove('-');
    m_pipe = QDir::tempPath() % QLatin1String("/qapt-sock-") % uuid;
    m_debconfGui = new DebconfKde::DebconfGui(m_pipe, this);
    m_debconfGui->connect(m_debconfGui, SIGNAL(activated()), this, SLOT(showDebconf()));
    m_debconfGui->connect(m_debconfGui, SIGNAL(deactivated()), this, SLOT(hideDebconf()));
    m_debconfGui->hide();

    connect(m_manager, SIGNAL(refreshFailed()),
            this, SLOT(onRefreshFailed()));
    connect(m_manager, SIGNAL(devicesReady(DeviceList)),
            this, SLOT(onDevicesReady(DeviceList)));

    connect(m_manager, SIGNAL(changeProgressChanged(int)),
            this, SLOT(progressChanged(int)));
    connect(m_manager, SIGNAL(changeFinished()),
            this, SLOT(finished()));
    connect(m_manager, SIGNAL(changeFailed(QString)),
            this, SLOT(failed(QString)));
}

Module::~Module()
{
    delete ui;
}

void Module::load()
{
    kDebug();
    if (m_manager->isActive()) {
        kDebug() << "aborting load because manager is active";
        return;
    }

    qDeleteAll(m_widgetList);
    m_widgetList.clear();

    disableUi();
    m_label->hide();

    // We call refresh unconditionally because refresh has internal logic to make
    // sure it gets executed if the manager ever reaches ready state.
    m_manager->refresh();

    m_overlay->start();
    ui->messageWidget->setMessageType(KMessageWidget::Information);
    ui->messageWidget->setText(i18nc("The backend is trying to figure out what drivers are suitable for the users system",
                                     "Collecting information about your system"));
    ui->messageWidget->animatedShow();

    emit changed(false);
}

void Module::possiblyChanged()
{
    foreach (const DriverWidget *widget, m_widgetList) {
        if (widget->hasChanged()) {
            // Found a widget that has changed, global state is changed; abort.
            emit changed(true);
            return;
        }
    }
    emit changed(false);
}

void Module::save()
{
    ui->progressBar->setVisible(true);
    disableUi();

    QStringList packageListToInstall;
    QStringList packageListToRemove;
    foreach (const DriverWidget *widget, m_widgetList) {
        packageListToInstall.append(widget->selectedPackageName());
        packageListToRemove.append(widget->notSelectedPackageNames());
    }

    m_manager->changeDriverPackages(packageListToInstall, packageListToRemove, m_pipe);
}

void Module::progressChanged(int progress)
{
    ui->progressBar->setValue(progress);
}

void Module::finished()
{
    enableUi();
    load();
}

void Module::failed(QString details)
{
    enableUi();
    ui->messageWidget->setText(details);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->animatedShow();
    load();
}

void Module::enableUi()
{
    m_overlay->stop();
    ui->progressBar->setVisible(false);
    ui->reloadButton->setEnabled(true);
    ui->messageWidget->animatedHide();
    foreach (DriverWidget *widget, m_widgetList) {
        widget->setSelectionEnabled(true);
    }
}

void Module::disableUi()
{
    foreach (DriverWidget *widget, m_widgetList) {
        widget->setSelectionEnabled(false);
    }
    ui->reloadButton->setEnabled(false);
}

void Module::defaults()
{
    load();
}

void Module::showDebconf()
{
    m_debconfGui->show();
}

void Module::hideDebconf()
{
    m_debconfGui->hide();
}

void Module::onQaptFailed(QString details)
{
    ui->messageWidget->setText(i18nc("@label",
                                     "The package system could not be initialized, your "
                                     "configuration may be broken."));
    ui->messageWidget->setToolTip(details);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->animatedShow();
}

void Module::onRefreshFailed()
{
    enableUi();

    ui->messageWidget->setText(i18nc("The backend replied with a error",
                                     "Something went terribly wrong. Please hit the 'Refresh Driver List' button"));
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->animatedShow();
}

void Module::onDevicesReady(DeviceList devices)
{
    KConfig driver_manager("kcmdrivermanagerrc");
    KConfigGroup pciGroup( &driver_manager, "PCI" );

    foreach (Device device, devices) {
        pciGroup.writeEntry(device.id, "true");
    }

    enableUi();

    foreach (const Device &device, devices) {
        DriverWidget *widget = new DriverWidget(device, this);
        ui->driverOptionsVLayout->insertWidget(0, widget);
        connect(widget, SIGNAL(selectionChanged()), SLOT(possiblyChanged()));
        m_widgetList.append(widget);
    }

    if (devices.empty()) {
        m_label->show();
    }
}
