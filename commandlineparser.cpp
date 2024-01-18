#include "commandlineparser.h"

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <vector>

#include <QString>
#endif

namespace mu::launcher {
CommandLineParser::CommandLineParser(int argc, char** argv)
{
    ensureUTF8Encoding(argc, argv);
    initOptions();
    args = argumentsAsQStringList();
    parse();
}

CommandLineParser::CommandLineParser(const QStringList& args)
{
    initOptions();
    this->args = args;
    parse();
}

[[nodiscard]] int CommandLineParser::argumentCount() const
{
    return argc;
}

[[nodiscard]] char** CommandLineParser::argumentValues() const
{
    return argv;
}

[[nodiscard]] QStringList CommandLineParser::argumentsAsQStringList(void) const
{
    QStringList args;

    for (int i = 0; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);

#ifndef NDEBUG
        if (arg.startsWith("-qmljsdebugger")) {
            continue;
        }
#endif

        args << arg;
    }

    return args;
}

[[nodiscard]] framework::IApplication::RunMode CommandLineParser::runMode(void) const
{
    return m_runMode;
}

[[nodiscard]] std::string CommandLineParser::scorePath(void) const
{
    return m_options.startup.scorePath.value_or("");
}

void CommandLineParser::processApplication(const QCoreApplication& app)
{
    m_parser.process(app);
}

void CommandLineParser::ensureUTF8Encoding(int argc, char** argv)
{
#if (defined (_MSCVER) || defined (_MSC_VER))
    // Prevent data loss if there are any characters that wouldn't fit in the local ANSI code page by manually
    // retrieving the command-line arguments and converting them from UTF-16 to UTF-8.
    LPWSTR* argvUTF16 = CommandLineToArgvW(GetCommandLineW(), &argc);
    Q_UNUSED(argv);

    std::for_each(argvUTF16, argvUTF16 + argc, [this](const auto& arg) {
        argvUTF8Q.emplace_back(QString::fromUtf16(reinterpret_cast<const char16_t*>(arg), -1).toUtf8());
    });

    LocalFree(argvUTF16);

    // Now recreate a char** reference to be used as argv
    for (auto& arg : argvUTF8Q) {
        argvUTF8.push_back(arg.data());
    }

    this->argc = argc;
    this->argv = argvUTF8.data();
#else
    // No convertion needed for other platforms, we can just use the original arguments directly.
    this->argc = argc;
    this->argv = argv;
#endif
}

template<typename ... Args>
QCommandLineOption internalCommandLineOption(Args&& ... args)
{
    QCommandLineOption option(std::forward<Args>(args)...);
    option.setFlags(QCommandLineOption::HiddenFromHelp);
    return option;
}

void CommandLineParser::initOptions()
{
    // Common
    m_parser.addHelpOption(); // -?, -h, --help
    m_parser.addVersionOption(); // -v, --version

    m_parser.addPositionalArgument("scorefiles", "The files to open", "[scorefile...]");

    m_parser.addOption(QCommandLineOption("long-version", "Print detailed version information"));
    m_parser.addOption(QCommandLineOption({ "d", "debug" }, "Debug mode"));

    // UI module only?
//#ifdef MUE_BUILD_UI_MODULE
    m_parser.addOption(QCommandLineOption({ "D", "monitor-resolution" }, "Specify monitor resolution", "DPI"));
//#endif

    // Converter mode
//#ifdef MUE_BUILD_CONVERTER_MODULE
    m_parser.addOption(QCommandLineOption({ "j", "job" }, "Process a conversion batch job", "JSONfile"));
    m_parser.addOption(QCommandLineOption({ "o", "export-to" }, "Export to 'file'. Format depends on file's extension", "file"));
    m_parser.addOption(QCommandLineOption({ "f", "force" },
                                          "Use with '-o <file>', ignore warnings reg. score being corrupted or from wrong version"));
    m_parser.addOption(QCommandLineOption({ "P", "export-score-parts" }, "Use with '-o <file>.pdf', export score and parts"));
#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    m_parser.addOption(QCommandLineOption({ "T", "trim-image" },
                                          "Use with '-o <file>.png' and '-o <file.svg>'. Trim exported image with specified margin (in pixels)",
                                          "margin"));
#endif
    m_parser.addOption(QCommandLineOption({ "b", "bitrate" }, "Use with '-o <file>.mp3', sets bitrate, in kbps", "bitrate"));
#endif //MUE_BUILD_IMPORTEXPORT_MODULE
//#endif //MUE_BUILD_CONVERTER_MODULE

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    m_parser.addOption(QCommandLineOption({ "r", "image-resolution" }, "Set output resolution for image export", "DPI"));
#endif
    m_parser.addOption(QCommandLineOption({ "M", "midi-operations" }, "Specify MIDI import operations file", "file"));
    m_parser.addOption(QCommandLineOption("gp-linked", "create tabulature linked staves for guitar pro"));
    m_parser.addOption(QCommandLineOption("gp-experimental", "experimental features for guitar pro import"));
#endif
    m_parser.addOption(QCommandLineOption({ "F", "factory-settings" }, "Use factory settings"));
    m_parser.addOption(QCommandLineOption({ "R", "revert-settings" }, "Revert to factory settings, but keep default preferences"));

    m_parser.addOption(QCommandLineOption("template-mode", "Save template mode, no page size")); // and no platform and creationDate tags
    m_parser.addOption(QCommandLineOption({ "t", "test-mode" }, "Set test mode flag for all files")); // this includes --template-mode

    m_parser.addOption(QCommandLineOption("session-type", "Startup with given session type", "type")); // see StartupScenario::sessionTypeTromString

    m_parser.addOption(QCommandLineOption("score-media",
                                          "Export all media (excepting mp3) for a given score in a single JSON file and print it to stdout"));
    m_parser.addOption(QCommandLineOption("highlight-config", "Set highlight to svg, generated from a given score", "highlight-config"));
    m_parser.addOption(QCommandLineOption("score-meta", "Export score metadata to JSON document and print it to stdout"));
    m_parser.addOption(QCommandLineOption("score-parts", "Generate parts data for the given score and save them to separate mscz files"));
    m_parser.addOption(QCommandLineOption("score-parts-pdf",
                                          "Generate parts data for the given score and export the data to a single JSON file, print it to stdout"));
    m_parser.addOption(QCommandLineOption("score-transpose",
                                          "Transpose the given score and export the data to a single JSON file, print it to stdout",
                                          "options"));
    m_parser.addOption(QCommandLineOption("source-update", "Update the source in the given score"));

    m_parser.addOption(QCommandLineOption({ "S", "style" }, "Load style file", "style"));

    // Video export
#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
    m_parser.addOption(QCommandLineOption("score-video", "Generate video for the given score and export it to file"));
// not implemented
//    m_parser.addOption(QCommandLineOption("view-mode",
//                                          "View mode [paged-float, paged-original, paged-float-height, pano, auto]. Auto (default) will choose the best mode according to number of instruments etc... Will show piano for piano score only",
//                                          "auto"));
// not implemented
//    m_parser.addOption(QCommandLineOption("piano", "Show Piano, works only if one part and not auto or float modes"));
//    m_parser.addOption(QCommandLineOption("piano-position", "Show Piano top or bottom. Default bottom", "bottom"));
    m_parser.addOption(QCommandLineOption("resolution", "Resolution [2160p, 1440p, 1080p, 720p, 480p, 360p]", "1080p"));
    m_parser.addOption(QCommandLineOption("fps", "Frame per second [60, 30, 24]", "24"));
    m_parser.addOption(QCommandLineOption("ls", "Pause before playback in seconds (3.0)", "3.0"));
    m_parser.addOption(QCommandLineOption("ts", "Pause before end of video in seconds (3.0)", "3.0"));
#endif

    //! NOTE Currently only implemented `full` mode
    m_parser.addOption(QCommandLineOption("migration", "Whether to do migration with given mode, `full` - full migration", "mode"));

    // Diagnostic
    m_parser.addOption(QCommandLineOption("diagnostic-output", "Diagnostic output", "output"));
    m_parser.addOption(QCommandLineOption("diagnostic-gen-drawdata", "Generate engraving draw data", "scores-dir"));
    m_parser.addOption(QCommandLineOption("diagnostic-com-drawdata", "Compare engraving draw data"));
    m_parser.addOption(QCommandLineOption("diagnostic-drawdata-to-png", "Convert draw data to png", "file"));
    m_parser.addOption(QCommandLineOption("diagnostic-drawdiff-to-png", "Convert draw diff to png"));

    // Autobot
#ifdef MUE_BUILD_AUTOBOT_MODULE
    m_parser.addOption(QCommandLineOption("test-case", "Run test case by name or file", "nameOrFile"));
    m_parser.addOption(QCommandLineOption("test-case-context", "Set test case context by name or file", "nameOrFile"));
    m_parser.addOption(QCommandLineOption("test-case-context-value", "Set test case context value", "value"));
    m_parser.addOption(QCommandLineOption("test-case-func", "Call test case function", "name"));
    m_parser.addOption(QCommandLineOption("test-case-func-args", "Call test case function args", "args"));
#endif

    // Audio plugins
    m_parser.addOption(QCommandLineOption("register-audio-plugin",
                                          "Check an audio plugin for compatibility with the application and register it", "path"));
    m_parser.addOption(QCommandLineOption("register-failed-audio-plugin", "Register an incompatible audio plugin", "path"));

    // Internal
    m_parser.addOption(internalCommandLineOption("score-display-name-override",
                                                 "Display name to be shown in splash screen for the score that is being opened", "name"));
}

void CommandLineParser::parse(void)
{
    using mu::framework::IApplication;

    m_parser.parse(args);

    auto floatValue = [this](const QString& name) -> std::optional<float> {
        bool ok = true;
        float val = m_parser.value(name).toFloat(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    auto doubleValue = [this](const QString& name) -> std::optional<double> {
        bool ok = true;
        double val = m_parser.value(name).toDouble(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    auto intValue = [this](const QString& name) -> std::optional<int> {
        bool ok = true;
        int val = m_parser.value(name).toInt(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    QStringList scorefiles;
    for (const QString& arg : m_parser.positionalArguments()) {
        scorefiles << arg;
    }

    if (m_parser.isSet("D")) {
        std::optional<double> val = doubleValue("D");
        if (val) {
            m_options.ui.physicalDotsPerInch = val;
            //} else {
            //    LOGE() << "Option: -D not recognized DPI value: " << m_parser.value("D");
        }
    }

    //if (m_parser.isSet("T")) {
    //    std::optional<int> val = intValue("T");
    //    if (val) {
    //        m_options.exportImage.trimMarginPixelSize = val;
    //    } else {
    //        LOGE() << "Option: -T not recognized trim value: " << m_parser.value("T");
    //    }
    //}

    //if (m_parser.isSet("b")) {
    //    std::optional<int> val = intValue("b");
    //    if (val) {
    //        m_options.exportAudio.mp3Bitrate = val;
    //        //} else {
    //        //    LOGE() << "Option: -b not recognized bitrate value: " << m_parser.value("b");
    //    }
    //}

    if (m_parser.isSet("template-mode")) {
        m_options.notation.templateModeEnabled = true;
    }

    if (m_parser.isSet("t")) {
        m_options.notation.testModeEnabled = true;
    }

    if (m_parser.isSet("session-type")) {
        m_options.startup.type = m_parser.value("session-type").toStdString();
    }

    if (m_parser.isSet("register-audio-plugin")) {
        m_runMode = IApplication::RunMode::AudioPluginRegistration;
        //m_audioPluginRegistration.pluginPath = fromUserInputPath(m_parser.value("register-audio-plugin"));
        //m_audioPluginRegistration.failedPlugin = false;
    }

    if (m_parser.isSet("register-failed-audio-plugin")) {
        QStringList args1 = m_parser.positionalArguments();
        m_runMode = IApplication::RunMode::AudioPluginRegistration;
        //m_audioPluginRegistration.pluginPath = fromUserInputPath(m_parser.value("register-failed-audio-plugin"));
        //m_audioPluginRegistration.failedPlugin = true;
        //m_audioPluginRegistration.failCode = !args1.empty() ? args1[0].toInt() : -1;
    }

    // Converter mode
    //if (m_parser.isSet("r")) {
    //    std::optional<float> val = floatValue("r");
    //    if (val) {
    //        m_options.exportImage.pngDpiResolution = val;
    //    } else {
    //        LOGE() << "Option: -r not recognized DPI value: " << m_parser.value("r");
    //    }
    //}

    if (m_parser.isSet("o")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::File;
        //if (scorefiles.size() < 1) {
        //    LOGE() << "Option: -o no input file specified";
        //} else {
        //    if (scorefiles.size() > 1) {
        //        LOGW() << "Option: -o multiple input files specified; processing only the first one";
        //    }
        //    m_converterTask.inputFile = scorefiles[0];
        //    m_converterTask.outputFile = fromUserInputPath(m_parser.value("o"));
        //}
    }

    //if (m_parser.isSet("P")) {
    //if (m_converterTask.outputFile.isEmpty()) {
    //    LOGE() << "Option: -R no output file specified";
    //} else {
    //    m_converterTask.type = ConvertType::ConvertScoreParts;
    //}
    //}

    if (m_parser.isSet("j")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::Batch;
        //m_converterTask.inputFile = fromUserInputPath(m_parser.value("j"));
    }

    if (m_parser.isSet("score-media")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::ExportScoreMedia;
        //m_converterTask.inputFile = scorefiles[0];
        //if (m_parser.isSet("highlight-config")) {
        //    m_converterTask.params[CommandLineParser::ParamKey::HighlightConfigPath]
        //        = fromUserInputPath(m_parser.value("highlight-config"));
        //}
    }

    if (m_parser.isSet("score-meta")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::ExportScoreMeta;
        //m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-parts")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::ExportScoreParts;
        //m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-parts-pdf")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::ExportScorePartsPdf;
        //m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-transpose")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::ExportScoreTranspose;
        //m_converterTask.inputFile = scorefiles[0];
        //m_converterTask.params[CommandLineParser::ParamKey::ScoreTransposeOptions] = m_parser.value("score-transpose");
    }

    if (m_parser.isSet("source-update")) {
        QStringList args2 = m_parser.positionalArguments();

        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_converterTask.type = ConvertType::SourceUpdate;
        //m_converterTask.inputFile = fromUserInputPath(args2[0]);

        //if (args2.size() >= 2) {
        //    m_converterTask.params[CommandLineParser::ParamKey::ScoreSource] = args2[1];
        //} else {
        //    LOGW() << "Option: --source-update no source specified";
        //}
    }

    if (m_parser.isSet("F") || m_parser.isSet("R")) {
        m_options.app.revertToFactorySettings = true;
    }

    if (m_parser.isSet("f")) {
        //m_converterTask.params[CommandLineParser::ParamKey::ForceMode] = true;
    }

    if (m_parser.isSet("S")) {
        //m_converterTask.params[CommandLineParser::ParamKey::StylePath] = fromUserInputPath(m_parser.value("S"));
    }

    //if (m_parser.isSet("gp-linked")) {
    //    m_options.guitarPro.linkedTabStaffCreated = true;
    //}

    //if (m_parser.isSet("gp-experimental")) {
    //    m_options.guitarPro.experimental = true;
    //}

    if (m_runMode == IApplication::RunMode::ConsoleApp) {
        if (m_parser.isSet("migration")) {
            QString val = m_parser.value("migration");
            m_options.project.fullMigration = (val == "full") ? true : false;
        }
    }

    // Diagnostic
    if (m_parser.isSet("diagnostic-output")) {
        //m_diagnostic.output = m_parser.value("diagnostic-output");
    }

    if (m_parser.isSet("diagnostic-gen-drawdata")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_diagnostic.type = DiagnosticType::GenDrawData;
        //m_diagnostic.input << m_parser.value("diagnostic-gen-drawdata");
    }

    if (m_parser.isSet("diagnostic-com-drawdata")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_diagnostic.type = DiagnosticType::ComDrawData;
        //m_diagnostic.input = scorefiles;
    }

    if (m_parser.isSet("diagnostic-drawdata-to-png")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_diagnostic.type = DiagnosticType::DrawDataToPng;
        //m_diagnostic.input << m_parser.value("diagnostic-drawdata-to-png");
    }

    if (m_parser.isSet("diagnostic-drawdiff-to-png")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_diagnostic.type = DiagnosticType::DrawDiffToPng;
        //m_diagnostic.input = scorefiles;
    }

    // Autobot
#ifdef MUE_BUILD_AUTOBOT_MODULE
    if (m_parser.isSet("test-case")) {
        m_runMode = IApplication::RunMode::ConsoleApp;
        //m_autobot.testCaseNameOrFile = fromUserInputPath(m_parser.value("test-case"));
    }

    if (m_parser.isSet("test-case-context")) {
        //m_autobot.testCaseContextNameOrFile = fromUserInputPath(m_parser.value("test-case-context"));
    }

    if (m_parser.isSet("test-case-context-value")) {
        //m_autobot.testCaseContextValue = m_parser.value("test-case-context-value");
    }

    if (m_parser.isSet("test-case-func")) {
        //m_autobot.testCaseFunc = m_parser.value("test-case-func");
    }

    if (m_parser.isSet("test-case-func-args")) {
        //m_autobot.testCaseFuncArgs = m_parser.value("test-case-func-args");
    }
#endif

    if (m_parser.isSet("h") || m_parser.isSet("v")) {
        // Help or Version prints and exits
        m_runMode = IApplication::RunMode::ConsoleApp;
    }

    // Startup
    if (m_runMode == IApplication::RunMode::GuiApp) {
        if (!scorefiles.isEmpty()) {
            m_options.startup.scorePath = scorefiles[0].toStdString();
        }

        if (m_parser.isSet("score-display-name-override")) {
            m_options.startup.scoreDisplayNameOverride = m_parser.value("score-display-name-override");
        }
    }
}
} // namespace mu::launcher
