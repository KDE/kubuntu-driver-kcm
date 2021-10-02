/*
    SPDX-FileCopyrightText: 2013 Rohan Garg <rohan@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Harald Sitter <apachelogger@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MODULE_H
#define MODULE_H

#include <KCModule>

#include "drivermanagerdbustypes.h"

namespace Ui {
    class Module;
}

namespace QApt {
    class Backend;
    class Transaction;
}

namespace DebconfKde {
    class DebconfGui;
}

class KPixmapSequenceOverlayPainter;
class DriverWidget;
class QLabel;

class DriverManager;

class Module : public KCModule
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * @param parent Parent widget of the module
     * @param args Arguments for the module
     */
    explicit Module(QWidget *parent, const QVariantList &args = QVariantList());

    /**
     * Destructor.
     */
    ~Module();

    /**
     * Overloading the KCModule load() function.
     */
    void load() final override;

    /**
     * Overloading the KCModule save() function.
     */
    void save() final override;

    /**
     * Overloading the KCModule defaults() function.
     */
    void defaults() final override;

private:
    /// UI
    Ui::Module *ui;

    DriverManager *m_manager;

    QStringList m_ModuleList;
    QList<DriverWidget*> m_widgetList;

    KPixmapSequenceOverlayPainter *m_overlay;
    QString m_pipe;
    QLabel *m_label;
    DebconfKde::DebconfGui* m_debconfGui;

    void enableUi();
    void disableUi();

private Q_SLOTS:
    void possiblyChanged();
    void progressChanged(int);
    void finished();
    void failed(QString details);
    void showDebconf();
    void hideDebconf();

    void onQaptFailed(QString details);
    void onRefreshFailed();
    void onDevicesReady(DeviceList devices);
};

#endif // MODULE_H
