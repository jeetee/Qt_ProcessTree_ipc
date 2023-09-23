#include "mainwindow.h"

#include <QCloseEvent>
#include <QGuiApplication>
#include <QStatusBar>

namespace mu::application::gui {
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow{parent}
{
    setWindowTitle(QGuiApplication::applicationName());
    statusBar()->showMessage(QString::number(QGuiApplication::applicationPid()));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    emit aboutToClose();
    event->accept();
}
} // namespace mu::application::gui
