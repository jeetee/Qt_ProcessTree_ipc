#ifndef MU_APPLICATION_PROCESSMANAGER_H
#define MU_APPLICATION_PROCESSMANAGER_H

#include "commandlineparser.h"
#include "guiapp.h"

#include <QCoreApplication>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QStringList>

namespace mu::application {
class ProcessManager : public QApplication
{
public:
    ProcessManager(launcher::CommandLineParser& commandLineParser, int argc);

private:
    QList<GuiApp*> m_children;
    QLocalServer m_server;

private slots:
    void handleClientConnected();

private:

    void launchChildProcess(launcher::CommandLineParser& commandLineParser);
    void launchChildProcess(const QStringList& arguments);

public:
    void handleChildQuit(GuiApp* const childGuiApp);
};
} // namespace mu::application

#endif // MU_APPLICATION_PROCESSMANAGER_H
