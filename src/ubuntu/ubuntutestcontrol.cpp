#include "ubuntutestcontrol.h"
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <QAction>

namespace Ubuntu {
namespace Internal {

/*!
 * \class UbuntuTestControl::UbuntuTestControl
 * Used only for testing purposes, provides functionality otherwise not reachable
 * because its not part of the object tree
 */
UbuntuTestControl::UbuntuTestControl(QObject *parent) :
    QObject(parent),
    m_lastBuildSuccess(false)
{
    connect(ProjectExplorer::BuildManager::instance(),SIGNAL(buildQueueFinished(bool)),
            this,SLOT(setLastBuildSuccess(bool)));
}

bool UbuntuTestControl::lastBuildSuccess() const
{
    return m_lastBuildSuccess;
}

void UbuntuTestControl::triggerCommand(const QString &command)
{
    Core::Id id = Core::Id::fromString(command);
    Core::Command *cmd = Core::ActionManager::command(id);
    if(cmd)
        cmd->action()->trigger();
}

void UbuntuTestControl::setLastBuildSuccess(bool arg)
{
    if (m_lastBuildSuccess != arg) {
        qDebug()<<"Setting build success to "<<arg;
        m_lastBuildSuccess = arg;
        emit lastBuildSuccessChanged(arg);
    }
    emit buildFinished();
}

ProjectExplorer::BuildManager *UbuntuTestControl::buildManager()
{
    return static_cast<ProjectExplorer::BuildManager*>(ProjectExplorer::BuildManager::instance());
}



} // namespace Internal
} // namespace Ubuntu
