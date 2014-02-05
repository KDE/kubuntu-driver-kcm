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

#include <KGenericFactory>
#include <KPluginFactory>

#include "Module.h"

K_PLUGIN_FACTORY(KcmDriverFactory,
                 registerPlugin<Module>("kcm-driver-manager");)
K_EXPORT_PLUGIN(KcmDriverFactory("kcm-driver-manager"))
