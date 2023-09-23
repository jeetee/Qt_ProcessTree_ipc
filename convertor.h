#ifndef MU_APPLICATION_CONVERTOR_H
#define MU_APPLICATION_CONVERTOR_H

#include "commandlineparser.h"

#include <QCoreApplication>

namespace mu::application {
class Convertor : public QCoreApplication
{
public:
    Convertor(launcher::CommandLineParser& commandLineParser, int argc);
private slots:
    void done(void) const;
};
} // namespace mu::application

#endif // MU_APPLICATION_CONVERTOR_H
