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

#ifndef MODULE_H
#define MODULE_H

#include <KCModule>

#include <LibQApt/Globals>

#include "drivermanagerdbustypes.h"

namespace Ui {
    class Module;
}

namespace Solid {
    class DeviceNotifier;
}

namespace QApt {
    class Backend;
    class Transaction;
}

class QDBusPendingCallWatcher;
class ComKubuntuDriverManagerInterface;
class QButtonGroup;
class QAbstractButton;

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
    void load();

    /**
     * Overloading the KCModule save() function.
     */
    void save();

    /**
     * Overloading the KCModule defaults() function.
     */
    void defaults();


private:
    /// UI
    Ui::Module *ui;
    ComKubuntuDriverManagerInterface* m_manager;
    bool m_refresh;
    QList<QButtonGroup*> m_buttonListGroup;
    Solid::DeviceNotifier *m_notifier;
    QStringList m_ModuleList;

    QApt::Backend *m_backend;
    QApt::Transaction *m_trans;


private Q_SLOTS:
    void driverDictFinished(QDBusPendingCallWatcher*);
    void driverMapFinished(QDBusPendingCallWatcher*);
    void refreshDriverList(bool);
    void emitDiff(QAbstractButton*);
    void progressChanged(int);
    void finished(QApt::ExitStatus);
    void handleError(QApt::ErrorCode);
    void restoreUi();

};

#endif // MODULE_H
