#ifndef MU_LAUNCHER_LAUNCHER_H
#define MU_LAUNCHER_LAUNCHER_H

#include "commandlineparser.h"

#include <QList>
#include <QLocalSocket>
#include <QProcess>

namespace mu::launcher {
class Launcher
{
private:
    Launcher() = delete;
    static QLocalSocket m_socket;
    static QList<QProcess* const> m_childProcesses;
public:
    static int Launch(CommandLineParser& commandLineParser);
};
} // namespace mu::launcher

#endif // MU_LAUNCHER_LAUNCHER_H
