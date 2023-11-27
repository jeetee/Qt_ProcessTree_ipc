#include "guiapp.h"

#include "mainwindow.h"

#include <QApplication>

namespace mu::application {
GuiApp::GuiApp(int argc, launcher::CommandLineParser& commandLineParser, QLocalSocket& ipcSocket)
    : QApplication(argc, commandLineParser.argumentValues())
{
    QApplication::setApplicationName("ProcessTree - GUI");
    QApplication::setApplicationVersion("1.0.0");

    commandLineParser.processApplication(*this);

    auto window = new gui::MainWindow();
    connect(window, &gui::MainWindow::aboutToClose, this, [this, &ipcSocket](){
        ipcSocket.close();
        ipcSocket.disconnectFromServer();
        this->quit();
    });
    window->show();

    // Multiple MainWindows is likely just fine; as those don't launch a separate process/thread..
    auto anotherWindow = new gui::MainWindow();
    anotherWindow->setWindowTitle(anotherWindow->windowTitle() + " - not primary MainWindow");
    anotherWindow->show();
}
} // namespace mu::application
