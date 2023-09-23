#include "convertor.h"

#include <chrono>
#include <QTimer>
#include <QDebug>

namespace mu::application {
Convertor::Convertor(launcher::CommandLineParser& commandLineParser, int argc)
    : QCoreApplication(argc, commandLineParser.argumentValues())
{
    commandLineParser.processApplication(*this);
    qInfo() << "Convertor running";
    QTimer::singleShot(std::chrono::seconds(5), this, &Convertor::done);
}

void Convertor::done(void) const
{
    qInfo() << "Convertor job done";
    quit();
}
} // namespace mu::application
