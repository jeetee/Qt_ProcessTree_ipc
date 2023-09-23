#ifndef RUNMODE_H
#define RUNMODE_H

namespace mu::framework {
class IApplication
{
public:
    virtual ~IApplication() = default;

    enum class RunMode {
        GuiApp,
        ConsoleApp,
        AudioPluginRegistration,
    };
};
}
#endif // RUNMODE_H
