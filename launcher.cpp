#include "launcher.h"

#include "ipc.h"
#include "processmanager.h"

#include <QCoreApplication>
#include <QtDebug>

#include <sstream>

namespace mu::launcher {
QLocalSocket Launcher::m_socket;

int Launcher::Launch(CommandLineParser& commandLineParser)
{
    // Attempt to connect to a server, under the assumption we'll be it's child
    m_socket.connectToServer(ipc::serverName());
    if (m_socket.waitForConnected(ipc::TIMEOUT_MSEC)) {
        qInfo("Connected to parent");
        // Connected to a parent, ask it to launch us and close ourselves down
        std::stringstream command;
        qInfo() << "We are not yet a Child of parent, ask it to relaunch us and die";
        command.str("");
        command << ipc::REQ_MAKE_CHILD;
        auto fwdArguments = commandLineParser.argumentsAsQStringList();
        for (auto arg = std::next(fwdArguments.begin()); arg != fwdArguments.end(); ++arg) {
            command << ' ' << arg->toStdString();
        }
        m_socket.write(command.str().c_str());
        m_socket.waitForBytesWritten(ipc::TIMEOUT_MSEC);
        m_socket.disconnectFromServer();
    } else {
        // Not connected to a parent, check reason?
        qInfo("Not connected to a parent");
        qDebug() << "Reason: " << m_socket.errorString() << '(' << m_socket.error() << ')';
        // We become the parent server
        QCoreApplication* app = new application::ProcessManager(commandLineParser, commandLineParser.argumentCount());
        qInfo() << "Running ProcessManager...";

        auto ret = app->exec();
        delete app;
        return ret;
    }
    return 0;
}
} // namespace mu::launcher
