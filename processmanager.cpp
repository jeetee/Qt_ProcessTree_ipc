#include "processmanager.h"

#include "ipc.h"

#include <QDebug>
#include <QTimer>
#include <QStringList>

#include <algorithm>
#include <chrono>
#include <sstream>

namespace mu::application {
ProcessManager::ProcessManager(launcher::CommandLineParser& commandLineParser, int argc)
    : QApplication(argc, commandLineParser.argumentValues())
{
    setApplicationName("ProcessTree - ProcessManager");
    setApplicationVersion("2.0.0");
    qInfo() << "Created" << applicationName() << applicationVersion();

    commandLineParser.processApplication(*this);

    if (!m_server.listen(ipc::serverName())) {
        qWarning() << "Failed to launch ipc server. Shutting down.";
        qDebug() << "Reason:" << m_server.errorString() << '(' << m_server.serverError() << ')';
        QTimer::singleShot(0, this, [this](){
            this->exit(1);
        });
    }

    // Server is listening
    connect(&m_server, &QLocalServer::newConnection, this, &ProcessManager::handleClientConnected);

    // We were launched not just to be a server, but because someone wanted to end up with a GUI app
    launchChildProcess(commandLineParser);
}

void ProcessManager::handleClientConnected()
{
    auto childSocket = m_server.nextPendingConnection();
    // Connecting clients should've sent their PID upon connecting
    childSocket->waitForReadyRead(ipc::TIMEOUT_MSEC);
    std::stringstream message(childSocket->readAll().toStdString());
    std::string command;
    message >> command;
    if (command == ipc::REQ_MAKE_CHILD) {
        qDebug() << "Re-launching separate Gui launch as a child";
        QStringList arguments;
        arguments << "GuiApp"; // Inject a dummy application name, as normal argument parsing expects [0] to be the executable
        while (message >> command) {
            arguments << command.c_str();
        }
        launchChildProcess(arguments);
    } else {
        // Wrong command - disconnect them
        qInfo() << "Received unexpected command from new connection:\n"
                << command.c_str() << message.str().c_str() << '\n'
                << "Disconnecting it.";
        childSocket->disconnectFromServer();
    }
}

void ProcessManager::handleChildQuit(GuiApp* const childGuiApp)
{
    qDebug() << "Child closing, removing it from my childlist";
    m_children.erase(std::remove(std::begin(m_children), std::end(m_children), childGuiApp));
    if (m_children.empty()) {
        qDebug() << "Last child is gone, we're done";
        quit();
    }
}

void ProcessManager::launchChildProcess(launcher::CommandLineParser& commandLineParser)
{
    auto newChild = new GuiApp(commandLineParser, this);
    m_children.append({ /*.app = */ newChild });
    qDebug() << "Launched child process " << m_children.length();
}

void ProcessManager::launchChildProcess(const QStringList& arguments)
{
    launcher::CommandLineParser argsParser(arguments);
    launchChildProcess(argsParser);
}
} // namespace mu::application
