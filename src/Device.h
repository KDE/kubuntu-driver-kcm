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

#ifndef DEVICE_H
#define DEVICE_H

#include <QDBusArgument>
#include <QDebug>

struct Driver
{
    /** Constructor */
    Driver();

    /** the distribution package providing this driver */
    QString packageName;
    /** this is the recommended driver for the associated device */
    bool recommended;
    /** this driver is an official distribution driver (otherwise 3rd party) */
    bool fromDistro;
    /** this is a free driver */
    bool free;
    /** this driver is built into the platform (e.g. official xorg drivers) */
    bool builtin;
    /** driver was manually installed */
    bool manualInstall;

    bool operator<(const Driver &other) const;
};

struct Device
{
    /** /dev system id of the device */
    QString id;
    /** modalias */
    QString modalias;
    /** model e.g. "GK104 [GeForce GTX 660 OEM]" */
    QString model;
    /** vendor e.g. "NVIDIA Corporation" */
    QString vendor;
    /** list of applicable drivers */
    QList<Driver> drivers;
};

typedef QList<Device> DeviceList;
Q_DECLARE_METATYPE(DeviceList)

// -------------------------------------------------------------------------- //
// --------------------------------- Debug ---------------------------------- //
// -------------------------------------------------------------------------- //

/**
 * QDebug operator for Device. This fung deep-debugs all members, without
 * additional operators for the specific member types.
 */
QDebug operator<<(QDebug dbg, const Device &device);

// -------------------------------------------------------------------------- //
// ---------------------------------- DBus ---------------------------------- //
// -------------------------------------------------------------------------- //

// We are doing nested demarshalling as auto-marshalling into basic types
// such as QMap<QString, QVariant> will not give us any advantage other than
// flattening out the marshalling, thus causing shitty to read functions;
// we'd still have to do string mapping and variant casting.

/** demarshall driver */
const QDBusArgument &operator>>(const QDBusArgument &argument, Driver &driver);
/** demarshall list of drivers */
const QDBusArgument &operator>>(const QDBusArgument &argument, QList<Driver> &driverList);
/** demarshall device */
const QDBusArgument &operator>>(const QDBusArgument &argument, Device &device);
/** demarshall list of devices */
const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceList &deviceList);

/** marshall list of devices. this function does nothing. */
QDBusArgument &operator<<(QDBusArgument &argument, const DeviceList &deviceList);

#endif // DEVICE_H
