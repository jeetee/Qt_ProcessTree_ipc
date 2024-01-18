#ifndef MU_APPLICATION_GUIAPP_H
#define MU_APPLICATION_GUIAPP_H

#include "commandlineparser.h"

#include <QApplication>

namespace mu::application {
class ProcessManager;

class GuiApp
{
private:
    ProcessManager* const m_processManager;
public:
    GuiApp(launcher::CommandLineParser& commandLineParser, ProcessManager* const parent);
};
} // namespace mu::application

#endif // MU_APPLICATION_GUIAPP_H
