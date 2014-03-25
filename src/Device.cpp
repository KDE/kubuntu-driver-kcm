/*
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

#include "Device.h"

#include <QDebug>

Driver::Driver()
    : packageName()
    , recommended(false)
    , fromDistro(false)
    , free(false)
    , builtin(false)
    , manualInstall(false)
    // Set by manager; qapt dependent:
    , fuzzyActive(false)
    , package(nullptr)
{
}

bool Driver::operator<(const Driver &other) const
{
    return (packageName < other.packageName);
}

QDebug operator<<(QDebug dbg, const Device &device)
{
    dbg.nospace() << "Dev(";
    dbg.nospace() << "\n  id: " << device.id;
    dbg.nospace() << "\n  modalias: " << device.modalias;
    dbg.nospace() << "\n  model: " << device.model;
    dbg.nospace() << "\n  vendor: " << device.vendor;
    foreach (const Driver &driver, device.drivers) {
        dbg.nospace() << "\n  driver(" << driver.packageName;
        dbg.nospace() << " recommended[" << driver.recommended << "]";
        dbg.nospace() << " free[" << driver.free << "]";
        dbg.nospace() << " fromDistro[" << driver.fromDistro << "]";
        dbg.nospace() << " builtin[" << driver.builtin << "]";
        dbg.nospace() << " manualInstall[" << driver.manualInstall << "]";
        dbg.nospace() << " fuzzyActive[" << driver.fuzzyActive << "]";
        dbg.nospace() << " package[" << driver.package << "]";
        dbg.nospace() << ")";
    }
    dbg.nospace() << "\n)";
    return dbg.maybeSpace();
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Driver &driver)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        bool value;
        argument.beginMapEntry();
        argument >> key >> value;
        if (key == QLatin1String("recommended")) {
            driver.recommended = value;
        } else if (key == QLatin1String("free")) {
            driver.free = value;
        } else if (key == QLatin1String("from_distro")) {
            driver.fromDistro = value;
        } else if (key == QLatin1String("builtin")) {
            driver.builtin = value;
        } else if (key == QLatin1String("manual_install")) {
            driver.manualInstall = value;
        }
        argument.endMapEntry();
    }

    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QList<Driver> &driverList)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        Driver driver;
        argument.beginMapEntry();
        argument >> driver.packageName >> driver;
        argument.endMapEntry();
        driverList.append(driver);
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Device &device)
{
    argument.beginMap();

    while (!argument.atEnd()) {
        QString key;
        QVariant value;

        argument.beginMapEntry();
        argument >> key >> value;

        if (key == QLatin1String("modalias")) {
            device.modalias = value.toString();
        } else if (key == QLatin1String("vendor")) {
            device.vendor = value.toString();
        } else if (key == QLatin1String("model")) {
            device.model = value.toString();
        } else if (value.canConvert<QDBusArgument>()) {
            QDBusArgument arg = value.value<QDBusArgument>();
            arg >> device.drivers;
        }

        argument.endMapEntry();
    }

    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceList &deviceList)
{
    qDebug() << Q_FUNC_INFO;
    argument.beginMap();
    while (!argument.atEnd()) {
        Device device;
        argument.beginMapEntry();
        argument >> device.id >> device;
        argument.endMapEntry();
        deviceList.append(device);
        qDebug() << device;
    }
    argument.endMap();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DeviceList &deviceList)
{
    Q_UNUSED(deviceList);
    qDebug() << Q_FUNC_INFO << "is noop";
    argument.beginMap(QVariant::String, QVariant::Map);
    argument.endMap();
    return argument;
}
