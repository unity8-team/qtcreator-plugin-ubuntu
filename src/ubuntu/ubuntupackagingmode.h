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

#ifndef UBUNTUPACKAGINGMODE_H
#define UBUNTUPACKAGINGMODE_H

#include <coreplugin/imode.h>
#include "ubuntupackagingwidget.h"
#include <QObject>
#include <projectexplorer/project.h>

namespace Ubuntu {
namespace Internal {

class UbuntuPackagingMode : public Core::IMode
{
    Q_OBJECT
public:
    explicit UbuntuPackagingMode(QObject *parent = 0);
    void initialize();

protected slots:
    void modeChanged(Core::IMode*);

    void on_projectAdded(ProjectExplorer::Project *project);
    void on_projectRemoved(ProjectExplorer::Project *project);
    void updateModeState();

protected:
    QWidget* m_modeWidget;
    UbuntuPackagingWidget m_ubuntuPackagingWidget;

    Core::Id previousMode;
};

}
}

#endif // UBUNTUPACKAGINGMODE_H
