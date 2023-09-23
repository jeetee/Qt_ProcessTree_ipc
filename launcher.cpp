#include "launcher.h"

#include "guiapp.h"
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
        // Connected to a parent, ask it if it opened us?
        std::stringstream command;
        command << ipc::REQ_IS_CHILD << ' ' << QCoreApplication::applicationPid();
        m_socket.write(command.str().c_str());
        if (m_socket.waitForBytesWritten(50) && m_socket.waitForReadyRead(ipc::TIMEOUT_MSEC)) {
            std::string reply = m_socket.readAll().toStdString();
            if (reply == ipc::REPLY_ACK_OK) {
                // If it did -> launch our actual application
                qInfo() << "We are a Child of parent, ready to be launched";
                QCoreApplication* app = new mu::application::GuiApp(commandLineParser.argumentCount(),
                                                                    commandLineParser,
                                                                    m_socket);
                // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
                return app->exec();
            } else if (reply == ipc::REPLY_NACK_NOK) {
                // If it did not -> ask it to make a child with our arguments and close ourselves down
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
                qWarning() << "Got an unexpected reply for" << ipc::REQ_IS_CHILD << '\n'
                           << reply.c_str();
            }
        } else {
            qInfo("Failed to ask server if we're it's child.");
            qDebug() << "Reason: " << m_socket.errorString() << '(' << m_socket.error() << ')';
        }
    } else {
        // Not connected to a parent, check reason?
        qInfo("Not connected to a parent");
        qDebug() << "Reason: " << m_socket.errorString() << '(' << m_socket.error() << ')';
        // We become the parent server
        QCoreApplication* app = new application::ProcessManager(commandLineParser, commandLineParser.argumentCount());
        qInfo() << "Running ProcessManager...";

        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        return app->exec();
    }
    return 0;
}
} // namespace mu::launcher
