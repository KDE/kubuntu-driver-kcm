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
