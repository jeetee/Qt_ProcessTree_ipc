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
    : QCoreApplication(argc, commandLineParser.argumentValues())
{
    setApplicationName("ProcessTree - ProcessManager");
    setApplicationVersion("1.0.0");
    qInfo() << "Created" << applicationName() << applicationVersion();

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
    launchChildProcess(commandLineParser.argumentsAsQStringList());
}

void ProcessManager::handleClientConnected()
{
    auto childSocket = m_server.nextPendingConnection();
    // Connecting clients should've sent their PID upon connecting
    childSocket->waitForReadyRead(ipc::TIMEOUT_MSEC);
    std::stringstream message(childSocket->readAll().toStdString());
    std::string command;
    message >> command;
    if (command == ipc::REQ_IS_CHILD) {
        qint64 pid;
        message >> pid;
        qDebug() << "Checking if" << pid << "for a new connection was launched by us.";
        auto childIt = std::find_if(std::begin(m_childProcesses), std::end(m_childProcesses),
                                    [&pid](const auto& childProcess){
            return (childProcess.process != nullptr) && (childProcess.process->processId() == pid);
        }
                                    );
        if (childIt != std::end(m_childProcesses)) {
            qDebug() << " it was, monitoring CHILD for further ipc comms";
            childIt->socket = childSocket;
            childSocket->write(ipc::REPLY_ACK_OK);
            connect(childSocket, &QLocalSocket::readyRead, this, [this, childIt](){
                handleChildReadyRead(*childIt);
            });
            connect(childSocket, &QLocalSocket::disconnected, this, [this, childIt](){
                handleChildDisconnected(*childIt);
            });
        } else {
            qDebug() << " it is not, monitoring SOCKET for further ipc comms";
            childSocket->write(ipc::REPLY_NACK_NOK);
            connect(childSocket, &QLocalSocket::readyRead, this, [this, childSocket](){
                handleOrphanReadyRead(childSocket);
            });
            connect(childSocket, &QLocalSocket::disconnected, this, [this, childSocket](){
                handleOrphanDisconnected(childSocket);
            });
        }
    } else {
        // Wrong command - disconnect them
        qInfo() << "Received unexpected command from new connection:\n"
                << command.c_str() << message.str().c_str() << '\n'
                << "Disconnecting it.";
        childSocket->disconnectFromServer();
    }
}

void ProcessManager::handleChildReadyRead(const struct child& child)
{
    qDebug() << "handleChildReadyRead for" << child.process->processId();
}

void ProcessManager::handleChildDisconnected(const struct child& child)
{
    qDebug() << "Child disconnected, dropping child" << child.process->processId();
    m_childProcesses.removeOne(child);
    if (m_childProcesses.empty()) {
        qDebug() << "Last child is gone, we're done";
        quit();
    }
}

void ProcessManager::handleOrphanReadyRead(QLocalSocket* const socket)
{
    std::stringstream message(socket->readAll().toStdString());
    qDebug() << "handleOrphanReadyRead" << message.str().c_str();

    std::string command;
    message >> command;
    if (command == ipc::REQ_MAKE_CHILD) {
        qDebug() << "Re-launch it as a child";
        QStringList arguments;
        while (message >> command) {
            arguments << command.c_str();
        }
        launchChildProcess(arguments);
    } else {
        qDebug() << "Orphan sent unhandled command" << command.c_str() << "\nIgnoring it.";
    }
}

void ProcessManager::handleOrphanDisconnected(QLocalSocket* const socket)
{
    Q_UNUSED(socket);
    qDebug() << "an Orphan Disconnected";
}

void ProcessManager::launchChildProcess(const QStringList& arguments)
{
    auto newChildProcess = new QProcess(this);
    m_childProcesses.append({ /*.process = */ newChildProcess, /*.socket = */ nullptr });
    newChildProcess->start(QCoreApplication::applicationFilePath(), arguments);
    qDebug() << "Launched child process" << newChildProcess->processId();
}
} // namespace mu::application
