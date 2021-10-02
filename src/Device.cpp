/*
    SPDX-FileCopyrightText: 2014 Harald Sitter <apachelogger@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
