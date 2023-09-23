#ifndef MU_APPLICATION_GUI_MAINWINDOW_H
#define MU_APPLICATION_GUI_MAINWINDOW_H

#include <QMainWindow>

namespace mu::application::gui {
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

signals:
    void aboutToClose(void);

protected:
    virtual void closeEvent(QCloseEvent* event) override;
};
} // namespace mu::application::gui

#endif // MU_APPLICATION_GUI_MAINWINDOW_H
