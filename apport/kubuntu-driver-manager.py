# apport package hook for ubuntu-drivers-common
# SPDX-FileCopyrightText: 2012 Canonical Ltdt.
# Author: Martin Pitt <martin.pitt@ubuntu.com>
# SPDX-FileCopyrightText: 2014 Rohan Garg <rohangarg@kubuntu.org>

import apport.hookutils

def add_info(report, ui):
    report['UbuntuDriversDebug'] = apport.hookutils.command_output(['ubuntu-drivers', 'debug'])
    report['UbuntuDriversList']  = apport.hookutils.command_output(['ubuntu-drivers', 'devices'])
