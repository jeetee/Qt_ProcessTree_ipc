#ifndef MU_LAUNCHER_COMMANDLINEPARSER_H
#define MU_LAUNCHER_COMMANDLINEPARSER_H

#include "runmode.h"

#include <optional>
#include <QCommandLineParser>

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <QByteArray>
#endif

namespace mu::launcher {
class CommandLineParser
{
public:
    struct Options {
        struct {
            std::optional<double> physicalDotsPerInch;
        } ui;

        struct {
            std::optional<bool> templateModeEnabled;
            std::optional<bool> testModeEnabled;
        } notation;

        struct {
            std::optional<bool> fullMigration;
        } project;

        struct {
            std::optional<int> trimMarginPixelSize;
            std::optional<float> pngDpiResolution;
        } exportImage;

        struct {
            std::optional<int> mp3Bitrate;
        } exportAudio;

        struct {
            std::optional<bool> linkedTabStaffCreated;
            std::optional<bool> experimental;
        } guitarPro;

        struct {
            std::optional<bool> revertToFactorySettings;
            //std::optional<haw::logger::Level> loggerLevel;
        } app;

        struct {
            std::optional<std::string> type;
            std::optional<std::string> scorePath;
            std::optional<QString> scoreDisplayNameOverride;
        } startup;
    };

private:
    // Resulting pre-processed arguments with correct UTF-8 encoding
    int argc;
    char** argv;
#if (defined (_MSCVER) || defined (_MSC_VER))
    // On MSVC under Windows, we need to manually retrieve the command-line arguments and convert them
    // from UTF-16 to UTF-8 to prevent data loss.
    // This is a place to store the converted arguments for further processing.
    std::vector<QByteArray> argvUTF8Q; // Storing the actual data
    std::vector<char*> argvUTF8; // Storing the raw pointer to it to be argv compatible
#endif

    QCommandLineParser m_parser;
    struct Options m_options;
    framework::IApplication::RunMode m_runMode = framework::IApplication::RunMode::GuiApp;
public:
    CommandLineParser(int argc, char** argv);
    [[nodiscard]] int argumentCount() const;
    [[nodiscard]] char** argumentValues() const;
    [[nodiscard]] QStringList argumentsAsQStringList(void) const;
    [[nodiscard]] framework::IApplication::RunMode runMode(void) const;

    void processApplication(const QCoreApplication& app);

private:
    void ensureUTF8Encoding(int argc, char** argv);
    void initOptions(void);
    void parse(void);
};
} // namespace mu::launcher

#endif // MU_LAUNCHER_COMMANDLINEPARSER_H
