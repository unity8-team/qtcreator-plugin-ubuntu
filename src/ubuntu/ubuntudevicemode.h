/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 2.1.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Juhapekka Piiroinen <juhapekka.piiroinen@canonical.com>
 */

#ifndef UBUNTUDEVICEMODE_H
#define UBUNTUDEVICEMODE_H

#include "ubuntudevice.h"
#include <coreplugin/imode.h>

class QQuickView;

namespace Ubuntu {
namespace Internal {

class UbuntuDevicesModel;
class UbuntuEmulatorModel;

class UbuntuDeviceMode : public Core::IMode
{
    Q_OBJECT

public:
    UbuntuDeviceMode(QObject *parent = 0);
    void initialize();

    static UbuntuDeviceMode *instance();
    UbuntuDevice::ConstPtr device();

signals:
    void updateDeviceActions ();

public slots:
    void deviceSelected ( const QVariant index );

protected slots:
    void modeChanged(Core::IMode*);

protected:
    static UbuntuDeviceMode *m_instance;
    UbuntuDevicesModel  *m_devicesModel;
    UbuntuEmulatorModel *m_emulatorModel;
    QQuickView *m_modeView;
    QWidget* m_modeWidget;
    QVariant m_deviceIndex;
};


} // Internal
} // Ubuntu


#endif // UBUNTUDEVICEMODE_H
