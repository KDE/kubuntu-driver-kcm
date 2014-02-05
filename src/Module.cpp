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
#include "DriverWidget.h"

#include "ui_Module.h"

#include "drivermanager_interface.h"

#include <KAboutData>
#include <KDebug>
#include <KPluginFactory>
#include <KMessageBox>
#include <KMessageWidget>
#include <KPixmapSequenceOverlayPainter>
#include <DebconfGui.h>

#include <QLabel>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProgressBar>
#include <QUuid>

#include <LibQApt/Backend>
#include <LibQApt/Transaction>

#include <qdbusmetatype.h>

#include "Version.h"


#include <unistd.h>

K_PLUGIN_FACTORY_DECLARATION(KcmDriverFactory);

Module::Module(QWidget *parent, const QVariantList &args)
    : KCModule(KcmDriverFactory::componentData(), parent, args)
    , ui(new Ui::Module)
    , m_refresh(false)
    , m_backend(new QApt::Backend)
{
    KAboutData *about = new KAboutData("kcm-driver-manager", 0,
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
    connect(ui->reloadButton, SIGNAL(clicked(bool)), SLOT(refreshDriverList()));

    qDBusRegisterMetaType<QVariantMapMap>();

    // We have no help so remove the button from the buttons.
    setButtons(buttons() ^ KCModule::Help);

    QApt::FrontendCaps caps = (QApt::FrontendCaps)(QApt::DebconfCap);
    m_backend->setFrontendCaps(caps);
    if (!m_backend->init())
        initError();
    ui->progressBar->setVisible(false);

    m_overlay = new KPixmapSequenceOverlayPainter(this);
    m_overlay->setWidget(this);

    //Debconf handling
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{').remove('}').remove('-');
    m_pipe = QDir::tempPath() % QLatin1String("/qapt-sock-") % uuid;
    m_debconfGui = new DebconfKde::DebconfGui(m_pipe, this);
    m_debconfGui->connect(m_debconfGui, SIGNAL(activated()), this, SLOT(showDebconf()));
    m_debconfGui->connect(m_debconfGui, SIGNAL(deactivated()), this, SLOT(hideDebconf()));
    m_debconfGui->hide();
}

Module::~Module()
{
    delete ui;
}

void Module::load()
{
    kDebug();

    m_overlay->start();
    ui->messageWidget->setMessageType(KMessageWidget::Information);
    ui->messageWidget->setText(i18n("Collecting information about your system"));
    ui->messageWidget->animatedShow();
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
    m_overlay->stop();
    ui->messageWidget->animatedHide();
    ui->messageWidget->setCloseButtonVisible(true);

    QDBusPendingReply<QVariantMapMap> mapReply = *data;
    if (mapReply.isError()) {
        kWarning() << "DBus data corrupted";
        ui->messageWidget->setText(i18n("Something went terribly wrong. Please hit the 'Refresh Driver List' button"));
        ui->messageWidget->setMessageType(KMessageWidget::Error);
        ui->messageWidget->animatedShow();
        return;
    }

    if (mapReply.value().isEmpty()) {
        QString label = i18n("<title>Your computer requires no proprietary drivers</title>");
        ui->driverOptionsVLayout->addWidget(new QLabel(label, this));
        return;
    }

    QVariantMapMap dictMap = mapReply.value();

    Q_FOREACH(const QString &key,dictMap.keys()) {
        QVariantMap mapValue = dictMap[key];
        //Device Name extraction from Map
        QVariant vendor = mapValue.value("vendor");
        QVariant model = mapValue.value("model");
        QString label = i18nc("%1 is hardware vendor, %2 is hardware model", "<title>%1 %2</title>", vendor.toString(), model.toString());

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
        ui->messageWidget->setText(i18n("Something went terribly wrong. Please hit the 'Refresh Driver List' button"));
        ui->messageWidget->setMessageType(KMessageWidget::Error);
        ui->messageWidget->animatedShow();
        return;
    }

    DriverWidget *widget = new DriverWidget(mapReply.value(), deviceName, m_backend, this);
    ui->driverOptionsVLayout->insertWidget(0, widget);
    connect(widget, SIGNAL(changed()), SLOT(emitDiff()));
    m_widgetList.append(widget);
    data->deleteLater();
}


void Module::emitDiff()
{
    emit changed();
}


void Module::refreshDriverList()
{
    kDebug();
    m_refresh=true;
    qDeleteAll(m_widgetList);
    m_widgetList.clear();
    load();
}


void Module::save()
{
    QApt::PackageList packages;
    QApt::Package *pkg;
    Q_FOREACH(const DriverWidget* widget, m_widgetList) {
        const QString pkgStr = widget->getSelectedPackageStr();
        pkg = m_backend->package(pkgStr);
        if (pkg) {
            if (!pkg->isInstalled()) {
                packages.append(pkg);
            }
        }
    }

    m_trans = m_backend->installPackages(packages);
    m_trans->setDebconfPipe(m_pipe);
    connect(m_trans, SIGNAL(progressChanged(int)), SLOT(progressChanged(int)));
    connect(m_trans, SIGNAL(finished(QApt::ExitStatus)), SLOT(finished(QApt::ExitStatus)));
    connect(m_trans, SIGNAL(errorOccurred(QApt::ErrorCode)), SLOT(handleError(QApt::ErrorCode)));
    m_trans->run();
    ui->progressBar->setVisible(true);
    ui->reloadButton->setEnabled(false);
}

void Module::progressChanged(int progress)
{
    ui->progressBar->setValue(progress);
}

void Module::finished(QApt::ExitStatus status)
{
    cleanup();
    refreshDriverList();
}

void Module::handleError(QApt::ErrorCode error)
{
    QString text;
    switch(error) {
        case QApt::InitError: {
            text = i18nc("@label",
                         "The package system could not be initialized, your "
                         "configuration may be broken.");
            break;
        }

        case QApt::LockError: {
            text = i18nc("@label",
                         "Another application seems to be using the package "
                         "system at this time. You must close all other package "
                         "managers before you will be able to install or remove "
                         "any packages.");
            break;
        }

        case QApt::DiskSpaceError: {
            text = i18nc("@label",
                         "You do not have enough disk space in the directory "
                         "at %1 to continue with this operation.", m_trans->errorDetails());
            break;
        }

        case QApt::FetchError: {
            text = i18nc("@label",
                         "Could not download packages");
            break;
        }

        case QApt::CommitError: {
            text = i18nc("@label", "An error occurred while applying changes:");
            break;
        }

        case QApt::AuthError: {
            text = i18nc("@label",
                         "This operation cannot continue since proper "
                         "authorization was not provided");
            break;
        }

        case QApt::WorkerDisappeared: {
            text = i18nc("@label", "It appears that the QApt worker has either crashed "
            "or disappeared. Please report a bug to the QApt maintainers");
            break;
        }

        case QApt::UntrustedError: {
            QStringList untrustedItems = m_trans->untrustedPackages();
            if (untrustedItems.size() == 1) {
                text = i18ncp("@label",
                              "The following package has not been verified by its author. "
                              "Downloading untrusted packages has been disallowed "
                              "by your current configuration.",
                              "The following packages have not been verified by "
                              "their authors. "
                              "Downloading untrusted packages has "
                              "been disallowed by your current configuration.",
                              untrustedItems.size());
            }
            break;
        }

        case QApt::NotFoundError: {
            text = i18nc("@label",
                         "The package \"%1\" has not been found among your software sources. "
                         "Therefore, it cannot be installed. ",
                         m_trans->errorDetails());
            break;
        }

        case QApt::UnknownError:
        default:
            break;
    }

    ui->messageWidget->setText(text);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->animatedShow();
    cleanup();
}

void Module::cleanup()
{
    m_backend->reloadCache();
    ui->progressBar->setVisible(false);
    ui->reloadButton->setEnabled(true);
}

void Module::initError()
{
    QString details = m_backend->initErrorMessage();

    QString text = i18nc("@label",
                         "The package system could not be initialized, your "
                         "configuration may be broken.");
    ui->messageWidget->setText(text);
    ui->messageWidget->setToolTip(details);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->animatedShow();
}

void Module::defaults()
{
}

void Module::showDebconf()
{
    m_debconfGui->show();
}

void Module::hideDebconf()
{
    m_debconfGui->hide();
}
