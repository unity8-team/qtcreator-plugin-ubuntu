/*
 * Copyright 2014 Canonical Ltd.
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
 * Author: Benjamin Zeller <benjamin.zeller@canonical.com>
 */
#include "ubunturemoterunconfiguration.h"
#include "clicktoolchain.h"
#include "ubuntuclicktool.h"
#include "ubuntucmakebuildconfiguration.h"
#include "ubuntuconstants.h"
#include "ubuntulocalrunconfiguration.h"
#include "ubunturemotedeployconfiguration.h"
#include "ubuntupackagestep.h"
#include "ubuntuclickmanifest.h"

#include <projectexplorer/kitinformation.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildsteplist.h>
#include <remotelinux/remotelinuxenvironmentaspect.h>
#include <utils/qtcprocess.h>
#include <cmakeprojectmanager/cmakeproject.h>
#include <cmakeprojectmanager/cmakeprojectconstants.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVariantMap>
#include <QVariant>
#include <QFileInfo>
#include <QSettings>

namespace Ubuntu {
namespace Internal {

enum {
    debug = 0
};

UbuntuRemoteRunConfiguration::UbuntuRemoteRunConfiguration(ProjectExplorer::Target *parent, Core::Id id)
    : AbstractRemoteLinuxRunConfiguration(parent,id),
      m_running(false)
{
    setDisplayName(appId());
    addExtraAspect(new RemoteLinux::RemoteLinuxEnvironmentAspect(this));
}

UbuntuRemoteRunConfiguration::UbuntuRemoteRunConfiguration(ProjectExplorer::Target *parent, UbuntuRemoteRunConfiguration *source)
    : AbstractRemoteLinuxRunConfiguration(parent,source),
      m_running(false)
{
}

QString UbuntuRemoteRunConfiguration::localExecutableFilePath() const
{
    return m_localExecutable;
}

QString UbuntuRemoteRunConfiguration::remoteExecutableFilePath() const
{
    return m_remoteExecutable;
}

QStringList UbuntuRemoteRunConfiguration::arguments() const
{
    return m_arguments;
}

QString UbuntuRemoteRunConfiguration::workingDirectory() const
{
    return QString::fromLatin1("/home/phablet");
}

Utils::Environment UbuntuRemoteRunConfiguration::environment() const
{
    RemoteLinux::RemoteLinuxEnvironmentAspect *aspect = extraAspect<RemoteLinux::RemoteLinuxEnvironmentAspect>();
    QTC_ASSERT(aspect, return Utils::Environment());
    Utils::Environment env(Utils::OsTypeLinux);
    env.modify(aspect->userEnvironmentChanges());
    return env;
}

QStringList UbuntuRemoteRunConfiguration::soLibSearchPaths() const
{
    QStringList paths;
    CMakeProjectManager::CMakeProject *cmakeProj
            = qobject_cast<CMakeProjectManager::CMakeProject *>(target()->project());

    if(cmakeProj) {
        QList<CMakeProjectManager::CMakeBuildTarget> targets = cmakeProj->buildTargets();
        foreach(const CMakeProjectManager::CMakeBuildTarget& target, targets) {
            QFileInfo binary(target.executable);
            if(binary.exists()) {
                if(debug) qDebug()<<"Adding path "<<binary.absolutePath()<<" to solib-search-paths";
                paths << binary.absolutePath();
            }
        }
    }
    return paths;
}

QWidget *UbuntuRemoteRunConfiguration::createConfigurationWidget()
{
    return 0;
}

bool UbuntuRemoteRunConfiguration::isEnabled() const
{
    return (!m_running);
}

QString UbuntuRemoteRunConfiguration::disabledReason() const
{
    if(m_running)
        return tr("This configuration is already running on the device");
    return QString();
}

bool UbuntuRemoteRunConfiguration::isConfigured() const
{
    return true;
}

/*!
 * \brief UbuntuRemoteRunConfiguration::ensureConfigured
 * Configures the internal parameters and fetched the informations from
 * the manifest file. We have no way of knowing these things before the project
 * was build and all files are in <builddir>/package. That means we can not cache that
 * information, because it could change anytime
 */
bool UbuntuRemoteRunConfiguration::ensureConfigured(QString *errorMessage)
{
    if(debug) qDebug()<<"--------------------- Reconfiguring RunConfiguration ----------------------------";
    m_arguments.clear();
    m_clickPackage.clear();

    QDir package_dir(packageDir());
    if(!package_dir.exists()) {
        if(errorMessage)
            *errorMessage = tr("No packaging directory available, please check if the deploy configuration is correct.");

        return false;
    }

    ProjectExplorer::DeployConfiguration *deplConf = qobject_cast<ProjectExplorer::DeployConfiguration*>(target()->activeDeployConfiguration());
    if(!deplConf) {
        if(errorMessage)
            *errorMessage = tr("No valid deploy configuration is set.");

        return false;
    }

    ProjectExplorer::BuildStepList *bsList = deplConf->stepList();
    foreach(ProjectExplorer::BuildStep *currStep ,bsList->steps()) {
        UbuntuPackageStep *pckStep = qobject_cast<UbuntuPackageStep*>(currStep);
        if(!pckStep)
            continue;

        QFileInfo info(pckStep->packagePath());
        if(info.exists()) {
            m_clickPackage = info.fileName();
            break;
        }
    }

    if(m_clickPackage.isEmpty()) {
        if (errorMessage)
            *errorMessage = tr("Could not find a click package to run, please check if the deploy configuration has a click package step");
        return false;
    }

    ProjectExplorer::ToolChain* tc = ProjectExplorer::ToolChainKitInformation::toolChain(target()->kit());
    if(tc->type() != QString::fromLatin1(Constants::UBUNTU_CLICK_TOOLCHAIN_ID)) {
        if(errorMessage)
            *errorMessage = tr("Wrong toolchain type. Please check your build configuration.");
        return false;
    }

    ClickToolChain* clickTc = static_cast<ClickToolChain*>(tc);

    ProjectExplorer::BuildConfiguration *bc = target()->activeBuildConfiguration();
    if(!bc) {
        if(errorMessage)
            *errorMessage = tr("Invalid buildconfiguration");
        return false;
    }

    if(id().toString().startsWith(QLatin1String(Constants::UBUNTUPROJECT_REMOTE_RUNCONTROL_APP_ID))) {

        QString desktopFile = UbuntuLocalRunConfiguration::getDesktopFile(this,appId(),errorMessage);
        if(desktopFile.isEmpty())
            return false;
        /*
         * Tries to read the Exec line from the desktop file, to
         * extract arguments and to know which "executor" is used on
         * the phone
         */
        QStringList args;
        QString command;

        if(!UbuntuLocalRunConfiguration::readDesktopFile(desktopFile,&command,&args,errorMessage))
            return false;

        QFileInfo commInfo(command);
        QString executor = commInfo.completeBaseName();
        if(executor.startsWith(QStringLiteral("qmlscene"))) {
            m_localExecutable  = QString::fromLatin1("%1/usr/lib/%2/qt5/bin/qmlscene")
                    .arg(UbuntuClickTool::targetBasePath(clickTc->clickTarget()))
                    .arg(clickTc->gnutriplet());

            m_remoteExecutable = QStringLiteral("/usr/bin/qmlscene");
            m_arguments = args;
        } else if(executor.startsWith(QStringLiteral("ubuntu-html5-app-launcher"))
                  || executor.startsWith(QStringLiteral("webapp-container"))) {
            m_remoteExecutable = QStringLiteral("");
            m_arguments = args;
        } else {
            //looks like a application without a launcher
            CMakeProjectManager::CMakeProject* pro = static_cast<CMakeProjectManager::CMakeProject*> (target()->project());
            foreach(const CMakeProjectManager::CMakeBuildTarget &t, pro->buildTargets()) {
                if(t.library || t.executable.isEmpty())
                    continue;

                QFileInfo execInfo(t.executable);

                if(execInfo.fileName() == commInfo.fileName())
                    m_localExecutable = t.executable;
            }

            if (m_localExecutable.isEmpty()) {
                if(errorMessage)
                    *errorMessage = tr("Could not find %1 in the project targets").arg(command);
                return false;
            }

            m_remoteExecutable = command;
        }
        return true;
    } else if (id().toString().startsWith(QLatin1String(Constants::UBUNTUPROJECT_REMOTE_RUNCONTROL_SCOPE_ID))) {

        QDir package_dir(packageDir());
        if(!package_dir.exists()) {
            if(errorMessage)
                *errorMessage = tr("No packaging directory available, please check if the deploy configuration is correct.");
            return false;
        }

        QString manifestPath = package_dir.absoluteFilePath(QStringLiteral("manifest.json"));

        //read the manifest
        UbuntuClickManifest manifest;
        if(!manifest.load(manifestPath)) {
            if(errorMessage)
                *errorMessage = tr("Could not open the manifest file in the package directory, make sure its installed into the root of the click package.");
            return false;
        }

        QString iniFilePath;
        for(const UbuntuClickManifest::Hook &hook : manifest.hooks()) {
            if(hook.appId == appId()) {
                iniFilePath = bc->buildDirectory()
                        .appendPath(hook.scope)
                        .appendPath(manifest.name()+QStringLiteral("_")+hook.appId+QStringLiteral(".ini"))
                        .toString();
            }
        }

        if(iniFilePath.isEmpty()) {
            if(errorMessage)
                *errorMessage = tr("Could not find a hook with id %1 in the manifest file.").arg(appId());
            return false;
        }

        if(!QFile::exists(iniFilePath)){
            if(errorMessage)
                *errorMessage = tr("Ini file for scope: %1 does not exist.").arg(appId());
            return false;
        }

        QSettings iniFile(iniFilePath,QSettings::IniFormat);
        if(iniFile.status() != QSettings::NoError) {
            if(errorMessage)
                *errorMessage = tr("Could not read the ini file for scope: .").arg(appId());
            return false;
        }

        iniFile.beginGroup(QStringLiteral("ScopeConfig"));

        //the default exec line
        QString execLine = QStringLiteral("/usr/lib/%1/unity-scopes/scoperunner %R %S").arg(static_cast<ClickToolChain*>(tc)->gnutriplet());

        QString srKey(QStringLiteral("ScopeRunner"));
        if(iniFile.contains(srKey))
            execLine = iniFile.value(srKey).toString();

        QStringList args = Utils::QtcProcess::splitArgs(execLine);
        QString executable = args.takeFirst();

        //if debugging is enabled we inject the debughelper, so we need
        //to remove it here
        if(executable.contains(QStringLiteral("qtc_device_debughelper.py"))) {
            args = Utils::QtcProcess::splitArgs(args[0]);
            executable = args.takeFirst();
        }

        QFileInfo commandInfo(executable);
        if(commandInfo.completeBaseName().startsWith(QStringLiteral("scoperunner"))) {
            m_localExecutable  = QString::fromLatin1("%1/usr/lib/%2/unity-scopes/scoperunner")
                    .arg(UbuntuClickTool::targetBasePath(clickTc->clickTarget()))
                    .arg(clickTc->gnutriplet());

            m_remoteExecutable = QStringLiteral("/usr/lib/%1/unity-scopes/scoperunner").arg(clickTc->gnutriplet());
            m_arguments = args;
            return true;
        } else {
            if(errorMessage)
                *errorMessage = tr("Using a custom scopelauncher is not yet supported");
            return false;
        }
    }

    if(errorMessage)
        *errorMessage = tr("Incompatible runconfiguration type id");
    return false;
}

bool UbuntuRemoteRunConfiguration::fromMap(const QVariantMap &map)
{
    if(debug) qDebug()<<Q_FUNC_INFO;
    return AbstractRemoteLinuxRunConfiguration::fromMap(map);
}

QVariantMap UbuntuRemoteRunConfiguration::toMap() const
{
    QVariantMap m = AbstractRemoteLinuxRunConfiguration::toMap();
    if(debug) qDebug()<<Q_FUNC_INFO;
    return m;
}

QString UbuntuRemoteRunConfiguration::appId() const
{
    if(id().toString().startsWith(QLatin1String(Constants::UBUNTUPROJECT_REMOTE_RUNCONTROL_APP_ID)))
        return id().suffixAfter(Constants::UBUNTUPROJECT_REMOTE_RUNCONTROL_APP_ID);
    else
        return id().suffixAfter(Constants::UBUNTUPROJECT_REMOTE_RUNCONTROL_SCOPE_ID);
}

QString UbuntuRemoteRunConfiguration::clickPackage() const
{
    return m_clickPackage;
}

void UbuntuRemoteRunConfiguration::setArguments(const QStringList &args)
{
    m_arguments = args;
}

QString UbuntuRemoteRunConfiguration::packageDir() const
{
    ProjectExplorer::Project *p = target()->project();
    if (p->id() == CMakeProjectManager::Constants::CMAKEPROJECT_ID)
        return target()->activeBuildConfiguration()->buildDirectory().toString()+QDir::separator()+QLatin1String(Constants::UBUNTU_DEPLOY_DESTDIR);
    else if (p->id() == Ubuntu::Constants::UBUNTUPROJECT_ID || p->id() == "QmlProjectManager.QmlProject") {
        if (!target()->activeBuildConfiguration()) {
            //backwards compatibility, try to not crash QtC for old projects
            //they did not create a buildconfiguration back then
            QDir pDir(p->projectDirectory());
            return p->projectDirectory()+QDir::separator()+
                    QStringLiteral("..")+QDir::separator()+
                    pDir.dirName()+QStringLiteral("_build")+QDir::separator()+QLatin1String(Constants::UBUNTU_DEPLOY_DESTDIR);
        } else
            return target()->activeBuildConfiguration()->buildDirectory().toString()+QDir::separator()+QLatin1String(Constants::UBUNTU_DEPLOY_DESTDIR);
    }
    return QString();
}

void UbuntuRemoteRunConfiguration::setRunning(const bool set)
{
    if(m_running != set) {
        m_running = set;
        emit enabledChanged();
    }
}

} // namespace Internal
} // namespace Ubuntu
