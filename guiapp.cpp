#include "guiapp.h"

#include "mainwindow.h"
#include "processmanager.h"

#include <QApplication>

namespace mu::application {
GuiApp::GuiApp(launcher::CommandLineParser& commandLineParser, ProcessManager* const parent)
    : m_processManager(parent)
{
    QApplication::setApplicationName("ProcessTree - GUI");
    QApplication::setApplicationVersion("2.0.1");

    // Access commandLineParser options as required

    auto window = new gui::MainWindow();
    window->connect(window, &gui::MainWindow::aboutToClose, [this](){
        this->m_processManager->handleChildQuit(this);
    });
    window->setWindowTitle(window->windowTitle() + " - " + commandLineParser.scorePath().c_str());
    window->show();
}
} // namespace mu::application
