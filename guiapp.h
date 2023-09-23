#ifndef MU_APPLICATION_GUIAPP_H
#define MU_APPLICATION_GUIAPP_H

#include "commandlineparser.h"

#include <QApplication>
#include <QLocalSocket>

namespace mu::application {
class GuiApp : public QApplication
{
public:
    GuiApp(int argc, launcher::CommandLineParser& commandLineParser, QLocalSocket& ipcSocket);
};
} // namespace mu::application

#endif // MU_APPLICATION_GUIAPP_H
