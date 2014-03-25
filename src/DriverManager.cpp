#include "DriverManager.h"

#include <QDBusMetaType>

#include <KDebug>
#include <KLocalizedString>

#include <LibQApt/Backend>
#include <LibQApt/Transaction>

#include "DriverMangerInterface.h"

DriverManager::DriverManager(QObject *parent)
    : QObject(parent)
    , m_ready(false)
    , m_pendingRefresh(false)
    , m_manager(new OrgKubuntuDriverManagerInterface(
                    "org.kubuntu.DriverManager",
                    "/DriverManager",
                    QDBusConnection::sessionBus(),
                    this))
    , m_backend(new QApt::Backend)
    , m_transaction(nullptr)
{
    qDBusRegisterMetaType<DeviceList>();

    if (!m_backend->init()) {
        emit qaptFailed(m_backend->initErrorMessage());
        return;
    }

    QApt::FrontendCaps caps = (QApt::FrontendCaps)(QApt::DebconfCap);
    m_backend->setFrontendCaps(caps);

#warning need function?
    if (m_backend->xapianIndexNeedsUpdate()) {
        m_backend->updateXapianIndex();
        connect(m_backend, SIGNAL(xapianUpdateFinished()), SLOT(onXapianUpdateFinished()));
    } else {
        onXapianUpdateFinished();
    }
}

void DriverManager::refresh()
{
    if (!m_ready) {
        m_pendingRefresh = true;
        return;
    }
    m_pendingRefresh = false;

    if (!m_backend->reloadCache()) {
        // Backend reloading failed, try again.
        if (!m_backend->reloadCache()) {
            // Failed a second time, we have now reached a fatal state.
            // This would constitute an oddball issue as opening the cache
            // after initial init should not be a big deal. It can still
            // happen though, so we make sure that we enter a fatal state.
            m_ready = false; // Cannot recover.
            emit qaptFailed(m_backend->initErrorMessage());
            return;
        }
    }

    QDBusPendingReply<DeviceList> reply = m_manager->devices();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onDevicesReady(QDBusPendingCallWatcher*)));
}

void DriverManager::installDriverPackages(QStringList driverPackageNames, QString debconfPipe)
{
#warning one probably ought to remove shit somehow, otherwise there is no switcheroo of drivers actually happening
    QApt::PackageList packages;
    foreach (const QString &packageName, driverPackageNames) {
        QApt::Package *package = m_backend->package(packageName);
        if (package && !package->isInstalled()) {
            packages.append(package);
        }
    }

    if (packages.length() == 0) {
        // Nothing to install; abort.
        onInstallationFinished();
        return;
    }

    Q_ASSERT(m_transaction == nullptr);

    m_transaction = m_backend->installPackages(packages);
    if (!debconfPipe.isEmpty()) {
        m_transaction->setDebconfPipe(debconfPipe);
    }
    connect(m_transaction, SIGNAL(progressChanged(int)), SIGNAL(installationProgressChanged(int)));
    connect(m_transaction, SIGNAL(finished(QApt::ExitStatus)), SLOT(onInstallationFinished()));
    connect(m_transaction, SIGNAL(errorOccurred(QApt::ErrorCode)), SLOT(onInstallationFailed(QApt::ErrorCode)));
    m_transaction->run();
}

void DriverManager::onDevicesReady(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<DeviceList> reply = *watcher;
    if (reply.isError()) {
        emit refreshFailed();
        return;
    }

    DeviceList devices = reply.value();
    watcher->deleteLater();

    // Enhance drivers with active property and a Package *.
    for (int j = 0; j < devices.length(); ++j) {
        Device device = devices.at(j);
        // This does two things at once for performance reasons:
        //  a) set driver.package for all drivers with a found package
        //  b) set driver.fuzzyActive, see blow

        // Find the active driver for each device:
        // - A driver with the manual property always is considered active,
        //   completely disregarding everything else.
        // - A nonfree driver that is installed is always considered active.
        // - A free driver that is installed is considered active when there's
        //   nothing else.

        int fuzzyActiveIndex = -1;
        bool foundManual = false;
        bool foundNonFreeInstalled = false;
        for (int i = 0; i < device.drivers.length(); ++i) {
            Driver driver = device.drivers.at(i);

            // Manual trumps everything! Once a manual one was found the package
            // dependent checks will be ignored.
            if (driver.manualInstall) {
                fuzzyActiveIndex = i;
                foundManual = true;
            }

            QApt::Package *package = m_backend->package(driver.packageName);
            if (package) {
                // Set the package for use in DriverWidget's string construction.
                // Packages are valid up until refresh() is called, before calling
                // refresh() the module is expected to clear the widgets, so no
                // problems should appear.
                driver.package = package;

                // Now for the fuzzyActive detection...
                if (!foundManual && package->isInstalled()) {
                    if (!driver.free) {
                        fuzzyActiveIndex = i;
                        foundNonFreeInstalled = true;
                    } else if (!foundNonFreeInstalled) {
                        // Don't change the fuzzy active when we already found a !free
                        // candidate, !free always trumps free.
                        fuzzyActiveIndex = i;
                    }
                }
            }

            // The driver object was changed, shuff the new one back into the list.
            device.drivers.replace(i, driver);
        }
        // Set actual active entry
        if (fuzzyActiveIndex != -1) {
            Driver driver = device.drivers.at(fuzzyActiveIndex);
            driver.fuzzyActive = true;
            device.drivers.replace(fuzzyActiveIndex, driver);
        }

        // The device object was changed, shuff the new one back into the list.
        devices.replace(j, device);
    }

    emit devicesReady(devices);
}

void DriverManager::onXapianUpdateFinished()
{
    kDebug();
    if (!m_backend->openXapianIndex()) {
        // Xapian refresh failed, we have reached a fatal state.
        emit qaptFailed(QString());
        return;
    }

    m_ready = true;
    emit ready(m_ready);

    // Ready will be emitted via direct connection, so the Module has time to
    // manually call refresh if it wants to. refresh forces a reset of the pending bool
    // so that we never get two refreshes at the same time (or so one hopes).
    if (m_pendingRefresh)
        refresh();
}

void DriverManager::onInstallationFinished()
{
    m_transaction = nullptr;
    emit installationFinished();
}

void DriverManager::onInstallationFailed(QApt::ErrorCode errorCode)
{
    QString text;

    switch(errorCode) {
    case QApt::InitError:
        text = i18nc("@label",
                     "The package system could not be initialized, your "
                     "configuration may be broken.");
        break;
    case QApt::LockError:
        text = i18nc("@label",
                     "Another application seems to be using the package "
                     "system at this time. You must close all other package "
                     "managers before you will be able to install or remove "
                     "any packages.");
        break;
    case QApt::DiskSpaceError:
        text = i18nc("@label",
                     "You do not have enough disk space in the directory "
                     "at %1 to continue with this operation.",
                     m_transaction->errorDetails());
        break;
    case QApt::FetchError:
        text = i18nc("@label",
                     "Could not download packages");
        break;
    case QApt::CommitError:
        text = i18nc("@label", "An error occurred while applying changes:");
        break;
    case QApt::AuthError:
        text = i18nc("@label",
                     "This operation cannot continue since proper "
                     "authorization was not provided");
        break;
    case QApt::WorkerDisappeared:
        text = i18nc("@label", "It appears that the QApt worker has either crashed "
                     "or disappeared. Please report a bug to the QApt maintainers");
        break;
    case QApt::UntrustedError: {
        const QStringList untrustedItems = m_transaction->untrustedPackages();
#warning what if not?
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
    case QApt::NotFoundError:
        text = i18nc("@label",
                     "The package \"%1\" has not been found among your software sources. "
                     "Therefore, it cannot be installed. ",
                     m_transaction->errorDetails());
        break;
    }

    m_transaction = nullptr;
    emit installationFailed(text);
}
