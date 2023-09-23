#ifndef MU_APPLICATION_PROCESSMANAGER_H
#define MU_APPLICATION_PROCESSMANAGER_H

#include "commandlineparser.h"

#include <QCoreApplication>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QStringList>

namespace mu::application {
class ProcessManager : public QCoreApplication
{
public:
    ProcessManager(launcher::CommandLineParser& commandLineParser, int argc);

private:
    struct child {
        QProcess* const process;
        QLocalSocket* socket;
        bool operator==(const struct child& other) const
        {
            return (process == other.process) && (socket == other.socket);
        }
    };
    QList<struct child> m_childProcesses;
    QLocalServer m_server;

private slots:
    void handleClientConnected();

private:
    void handleChildReadyRead(const struct child& child);
    void handleChildDisconnected(const struct child& child);

    void handleOrphanReadyRead(QLocalSocket* const socket);
    void handleOrphanDisconnected(QLocalSocket* const socket);

    void launchChildProcess(const QStringList& arguments);
};
} // namespace mu::application

#endif // MU_APPLICATION_PROCESSMANAGER_H
