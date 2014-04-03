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

#include "ubuntulocalrunconfiguration.h"
#include "ubuntuproject.h"
#include "ubuntuprojectguesser.h"
#include "ubuntuclickmanifest.h"

#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>
#include <projectexplorer/target.h>
#include <projectexplorer/localenvironmentaspect.h>
#include <projectexplorer/buildconfiguration.h>
#include <utils/environment.h>
#include <utils/qtcprocess.h>
#include <cmakeprojectmanager/cmakeproject.h>
using namespace Ubuntu::Internal;

UbuntuLocalRunConfiguration::UbuntuLocalRunConfiguration(ProjectExplorer::Target *parent, Core::Id id)
    : ProjectExplorer::LocalApplicationRunConfiguration(parent, id)
{
    setDisplayName(parent->project()->displayName());
    addExtraAspect(new ProjectExplorer::LocalEnvironmentAspect(this));
}

UbuntuLocalRunConfiguration::UbuntuLocalRunConfiguration(ProjectExplorer::Target *parent, UbuntuLocalRunConfiguration *source)
    : ProjectExplorer::LocalApplicationRunConfiguration(parent,source)
{
}

QWidget *UbuntuLocalRunConfiguration::createConfigurationWidget()
{
    return NULL;
}

bool UbuntuLocalRunConfiguration::isEnabled() const
{
    return true;
}

QString UbuntuLocalRunConfiguration::executable() const
{
    return m_executable;
}

QString UbuntuLocalRunConfiguration::workingDirectory() const
{
    return m_workingDir;
}

QString UbuntuLocalRunConfiguration::commandLineArguments() const
{
    return Utils::QtcProcess::joinArgs(m_args);
}

QString UbuntuLocalRunConfiguration::dumperLibrary() const
{
    return QtSupport::QtKitInformation::dumperLibrary(target()->kit());
}

QStringList UbuntuLocalRunConfiguration::dumperLibraryLocations() const
{
    return QtSupport::QtKitInformation::dumperLibraryLocations(target()->kit());
}

ProjectExplorer::LocalApplicationRunConfiguration::RunMode UbuntuLocalRunConfiguration::runMode() const
{
    return Gui;
}

bool UbuntuLocalRunConfiguration::ensureConfigured(QString *errorMessage)
{
    if(!LocalApplicationRunConfiguration::ensureConfigured(errorMessage))
        return false;

    if(UbuntuProjectGuesser::isClickAppProject(target()->project()))
        return ensureClickAppConfigured(errorMessage);
    else if(UbuntuProjectGuesser::isScopesProject(target()->project()))
        return ensureScopesAppConfigured(errorMessage);

    return ensureUbuntuProjectConfigured(errorMessage);
}

QString UbuntuLocalRunConfiguration::getDesktopFile(ProjectExplorer::RunConfiguration* config, QString *appId, QString *errorMessage)
{
    QString manifestPath;
    if(qobject_cast<UbuntuLocalRunConfiguration*>(config)) {
        //For a locally compiled project, we have no CLICK_MODE enabled, that means we have to search
        //the project and build trees for our desktop file so we can query the main qml file from the desktop file
        *appId = config->target()->project()->displayName();
        QRegularExpression desktopFinder = QRegularExpression(QString::fromLatin1("^%1.desktop$")
                                                              .arg(*appId));
        QFileInfo desktopInfo = UbuntuProjectGuesser::findFileRecursive(config->target()->activeBuildConfiguration()->buildDirectory(),
                                                                        desktopFinder).toFileInfo();
        if (!desktopInfo.exists()) {

            //search again in the project directory
            desktopInfo = UbuntuProjectGuesser::findFileRecursive(Utils::FileName::fromString(config->target()->project()->projectDirectory()),
                                                                                    desktopFinder).toFileInfo();

            if(!desktopInfo.exists()) {
               if(errorMessage)
                   *errorMessage = tr("Could not find a desktop file for project");
            }
        }

        return desktopInfo.absoluteFilePath();
    }

    QDir package_dir(config->target()->activeBuildConfiguration()->buildDirectory().toString()+QDir::separator()+QLatin1String(Constants::UBUNTU_DEPLOY_DESTDIR));
    if(!package_dir.exists()) {
        if(errorMessage)
            *errorMessage = tr("No packaging directory available, please check if the deploy configuration is correct.");

        return QString();
    }
    manifestPath = package_dir.absoluteFilePath(QLatin1String("manifest.json"));

    //read the manifest
    UbuntuClickManifest manifest;
    if(!manifest.load(manifestPath,config->target()->project()->displayName())) {
        if(errorMessage)
            *errorMessage = tr("Could not open the manifest file in the package directory, maybe you have to create one.");

        return QString();
    }

    //get the hooks property that contains the desktop file
    QScriptValue hooks = manifest.hooks();
    if(hooks.isNull()) {
        if(errorMessage)
            *errorMessage = tr("No hooks property in the manifest");
        return QString();
    }

    //check if the hooks property is a object
    QVariantMap hookMap = hooks.toVariant().toMap();
    if(hookMap.isEmpty()){
        if(errorMessage)
            *errorMessage = tr("Hooks needs to be a object");
        return QString();
    }

    //get the app id and check if the first property is a object
    *appId = hookMap.firstKey();
    QVariant app = hookMap.first();
    if(!app.type() == QVariant::Map) {
        if(errorMessage)
            *errorMessage = tr("The hooks property can only contain objects");
        return QString();
    }

    //get the desktop file path in the package
    QVariantMap hook = app.toMap();
    if(!hook.contains(QLatin1String("desktop"))) {
        if(errorMessage)
            *errorMessage = tr("No desktop file found in hook: %1").arg(*appId);
        return QString();
    }

    return QDir::cleanPath(package_dir.absoluteFilePath(hook[QLatin1String("desktop")].toString()));
}

bool UbuntuLocalRunConfiguration::readDesktopFile(const QString &desktopFile, QString *executable, QStringList *arguments, QString *errorMessage)
{
    QFile desktop(desktopFile);
    if(!desktop.exists()) {
        if(errorMessage)
            *errorMessage = tr("Desktop file does not exist");
        return false;
    }
    if(!desktop.open(QIODevice::ReadOnly)) {
        if(errorMessage)
            *errorMessage = tr("Could not open desktop file for reading");
        return false;
    }

    QString execLine;
    QString name;

    QTextStream in(&desktop);
    while(!in.atEnd()) {
        QString line = in.readLine();
        if(line.startsWith(QLatin1String("#")))
            continue;

        line = line.mid(0,line.indexOf(QChar::fromLatin1('#'))).trimmed();
        if(line.startsWith(QLatin1String("Exec"),Qt::CaseInsensitive)) {
            execLine = line.mid(line.indexOf(QChar::fromLatin1('='))+1);
            qDebug()<<"Found exec line: "<<execLine;
            continue;
        } else if(line.startsWith(QLatin1String("Name"),Qt::CaseInsensitive)) {
            name = line.mid(line.indexOf(QChar::fromLatin1('='))+1);
            qDebug()<<"Found name line: "<<name;
            continue;
        }
    }

    QStringList args = Utils::QtcProcess::splitArgs(execLine);
    *executable = args.takeFirst();
    *arguments  = args;

    return true;
}

bool UbuntuLocalRunConfiguration::ensureClickAppConfigured(QString *errorMessage)
{
    QString appId;
    QString desktopFile = getDesktopFile(this,&appId,errorMessage);
    if(desktopFile.isEmpty())
        return false;

    /*
     * Tries to read the Exec line from the desktop file, to
     * extract arguments and to know which "executor" is used
     */
    QStringList args;
    QString command;

    if(!UbuntuLocalRunConfiguration::readDesktopFile(desktopFile,&command,&args,errorMessage))
        return false;

    m_workingDir = target()->activeBuildConfiguration()->buildDirectory().toString()
            + QDir::separator()
            + QLatin1String(Constants::UBUNTU_DEPLOY_DESTDIR);

    QFileInfo commInfo(command);
    if(commInfo.fileName().startsWith(QLatin1String("qmlscene"))) {
        m_executable = QtSupport::QtKitInformation::qtVersion(target()->kit())->qmlsceneCommand();

        QString mainQml;
        foreach(const QString &arg, args) {
            if(arg.contains(QLatin1String(".qml"))) {
                QFileInfo info(arg);
                mainQml = info.fileName();
            }
        }

        if(mainQml.isEmpty()) {
            if(errorMessage)
                *errorMessage = tr("Could not get the main qml file from the desktop EXEC line.");
            return false;
        }

        QFileInfo mainFileInfo = UbuntuProjectGuesser::findFileRecursive(Utils::FileName::fromString(target()->project()->projectDirectory()),
                                                                         QString::fromLatin1("^%1$").arg(mainQml)).toFileInfo();

        if(!mainFileInfo.exists()) {
            if(errorMessage)
                *errorMessage = tr("Could not find the main qml file (%1) in the project directory.").arg(mainQml);
            return false;
        }

        m_args = QStringList()<<mainFileInfo.absoluteFilePath();
    } else if(commInfo.completeBaseName().startsWith(QLatin1String(Ubuntu::Constants::UBUNTUHTML_PROJECT_LAUNCHER_EXE))) {
        if(errorMessage)
            *errorMessage = tr("There is no local run support for html click apps using cmake yet.");
        return false;
        /*
        Utils::Environment env = Utils::Environment::systemEnvironment();
        m_executable = env.searchInPath(QString::fromLatin1(Ubuntu::Constants::UBUNTUHTML_PROJECT_LAUNCHER_EXE));
        m_args = args;
        */
    } else {
        //looks like a application without a launcher
        CMakeProjectManager::CMakeProject* proj = static_cast<CMakeProjectManager::CMakeProject*> (target()->project());
        QList<CMakeProjectManager::CMakeBuildTarget> targets = proj->buildTargets();
        foreach (const CMakeProjectManager::CMakeBuildTarget& t, targets) {
            if(t.library)
                continue;

            QFileInfo targetInfo(t.executable);
            if(!targetInfo.exists())
                continue;

            if(targetInfo.fileName() == commInfo.fileName()) {
                m_executable = targetInfo.absoluteFilePath();
                break;
            }
        }

        if (m_executable.isEmpty()) {
            if(errorMessage)
                tr("Could not find executable specified in the desktop file");
            return false;
        }
        m_args = args;
        m_workingDir = target()->project()->projectDirectory();
    }
    qDebug()<<"Configured with: "<<m_executable<<" "<<m_args.join(QChar::fromLatin1(' '));
    return true;
}

bool UbuntuLocalRunConfiguration::ensureScopesAppConfigured(QString *errorMessage)
{
    m_workingDir = target()->activeBuildConfiguration()->buildDirectory().toString();

    Utils::Environment env = Utils::Environment::systemEnvironment();
    m_executable = env.searchInPath(QString::fromLatin1(Ubuntu::Constants::UBUNTUSCOPES_PROJECT_LAUNCHER_EXE));

    if(m_executable.isEmpty()) {
        if(errorMessage)
            *errorMessage = tr("Could not find launcher %1 in path").arg(QLatin1String(Ubuntu::Constants::UBUNTUSCOPES_PROJECT_LAUNCHER_EXE));
        return false;
    }


    Utils::FileName iniFile = UbuntuProjectGuesser::findScopesIniRecursive(target()->activeBuildConfiguration()->buildDirectory());
    if(iniFile.toFileInfo().exists())
        m_args = QStringList()<<QString(iniFile.toFileInfo().absoluteFilePath());
    return true;
}

bool UbuntuLocalRunConfiguration::ensureUbuntuProjectConfigured(QString *errorMessage)
{
    UbuntuProject* ubuntuProject = qobject_cast<UbuntuProject*>(target()->project());
    if (ubuntuProject) {
        m_workingDir = ubuntuProject->projectDirectory();
        if (ubuntuProject->mainFile().compare(QString::fromLatin1("www/index.html"), Qt::CaseInsensitive) == 0) {
            Utils::Environment env = Utils::Environment::systemEnvironment();
            m_executable = env.searchInPath(QString::fromLatin1(Ubuntu::Constants::UBUNTUHTML_PROJECT_LAUNCHER_EXE));
            m_args = QStringList()<<QString::fromLatin1("--www=%0/www").arg(ubuntuProject->projectDirectory())
                                  <<QString::fromLatin1("--inspector");
        } else {
            m_executable = QtSupport::QtKitInformation::qtVersion(target()->kit())->qmlsceneCommand();
            m_args = QStringList()<<QString(QLatin1String("%0.qml")).arg(ubuntuProject->displayName());
        }

        if(m_executable.isEmpty()) {
            if(errorMessage)
                *errorMessage = tr("Could not find a launcher for this projecttype in path");
            return false;
        }
        return true;
    }
    if(errorMessage)
        *errorMessage = tr("Unsupported Project Type used with UbuntuRunConfiguration");

    return false;
}


void UbuntuLocalRunConfiguration::addToBaseEnvironment(Utils::Environment &env) const
{
    if(UbuntuProjectGuesser::isClickAppProject(target()->project())) {
        CMakeProjectManager::CMakeProject* proj = static_cast<CMakeProjectManager::CMakeProject*> (target()->project());
        QList<CMakeProjectManager::CMakeBuildTarget> targets = proj->buildTargets();
        QSet<QString> usedPaths;
        foreach (const CMakeProjectManager::CMakeBuildTarget& t, targets) {
            if(t.library) {
                qDebug()<<"Looking at executable "<<t.executable;
                QFileInfo inf(t.executable);
                if(inf.exists()) {
                    QDir d = inf.absoluteDir();
                    qDebug()<<"Looking in the dir: "<<d.absolutePath();
                    if(d.exists(QLatin1String("qmldir"))) {
                        QString path = QDir::cleanPath(d.absolutePath()+QDir::separator()+QLatin1String(".."));
                        if(usedPaths.contains(path))
                            continue;

                        env.appendOrSet(QLatin1String("QML2_IMPORT_PATH"),path,QString::fromLatin1(":"));
                        usedPaths.insert(path);
                    }
                }
            }
        }
    }
}

