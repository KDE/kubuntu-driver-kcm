#ifndef DRIVERMANAGER_H
#define DRIVERMANAGER_H

#include <QObject>

#include <LibQApt/Globals>

#include "Device.h"

class QDBusPendingCallWatcher;

class OrgKubuntuDriverManagerInterface;

namespace QApt {
    class Backend;
    class Transaction;
}

class DriverManager : public QObject
{
    Q_OBJECT
public:
    DriverManager(QObject *parent = 0);

    bool isReady() const;

    /**
     * Refreshes the manager content.
     * This does two things: reload the QApt backend (i.e. package cache),
     * and reloads the driver list from the DBus backend.
     * \see refreshFailed
     * \see devicesReady
     */
    void refresh();

    /** Install selected driver packages. */
    void installDriverPackages(QStringList driverPackageNames, QString debconfPipe = QString());

signals:
    /**
     * This signal is emitted when the qapt backend:
     *   - could not be initalized
     *   - could not reload its cache (after \see refresh)
     * \param details contains additional details, or an empty string
     * \warning This is a fatal error condition. Recovery from this is
     *          impossible.
     */
    void qaptFailed(QString details);

    /*** Querying the devices failed; can be retried. */
    void refreshFailed();

    void devicesReady(DeviceList);

    void ready(bool ready);

    void installationProgressChanged(int progress);

    /** Installation finished successfull. \note Requires refresh */
    void installationFinished();

    /**
     * Installation failed.
     * \param errorMessage localized error message to show
     * \note Requires refresh
     */
    void installationFailed(QString errorMessage);

private slots:
    void onDevicesReady(QDBusPendingCallWatcher *watcher);
    void onXapianUpdateFinished();
    void onInstallationFinished();
    void onInstallationFailed(QApt::ErrorCode errorCode);

private:
    /** Whether the Manager is ready (QApt needs to be inited) */
    bool m_ready;

    /** Got a refresh call while not ready. */
    bool m_pendingRefresh;

    OrgKubuntuDriverManagerInterface *m_manager;

    QApt::Backend *m_backend;
    QApt::Transaction *m_transaction;
};

#endif // DRIVERMANAGER_H
