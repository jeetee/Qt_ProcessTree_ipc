#include <QCoreApplication>
#include <QTextCodec>

#include "convertor.h"
#include "commandlineparser.h"
#include "launcher.h"

#include <iostream>

int main(int argc, char* argv[])
{
    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC
    // under Windows, which would otherwise default to the local ANSI code page and cause corruption of any non-ANSI
    // Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    mu::launcher::CommandLineParser commandLineParser(argc, argv);

    if (commandLineParser.runMode() == mu::framework::IApplication::RunMode::GuiApp) {
        // Need to check for already running master instance
        std::cout << "GUI version requested" << std::endl;
        return mu::launcher::Launcher::Launch(commandLineParser);
    }
    std::cout << "Running as standalone non-GUI application" << std::endl;
    QCoreApplication* const app = new mu::application::Convertor(commandLineParser, commandLineParser.argumentCount());
    return app->exec();
}
