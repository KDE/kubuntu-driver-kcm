/*
    SPDX-FileCopyrightText: 2013 Rohan Garg <rohangarg@kubuntu.org>
    SPDX-FileCopyrightText: 2013 Harald Sitter <apachelogger@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KPluginFactory>

#include "Module.h"

K_PLUGIN_FACTORY(KcmDriverFactory,
                 registerPlugin<Module>("kcm-driver-manager");)

#include "main.moc"
