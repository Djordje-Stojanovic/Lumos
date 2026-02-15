#if defined(LUMOS_WITH_QT)
#include "app/EnhancementController.h"
#include "common/Telemetry.h"
#include "engine/CpuStubPipeline.h"
#include "ui/EnhanceViewModel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#endif

#include <iostream>

int main(int argc, char* argv[]) {
#if defined(LUMOS_WITH_QT)
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    lumos::common::Telemetry telemetry;
    lumos::engine::CpuStubPipeline pipeline;
    lumos::app::EnhancementController controller(pipeline, telemetry);
    lumos::ui::EnhanceViewModel enhance_view_model(controller);
    engine.rootContext()->setContextProperty("enhanceViewModel", &enhance_view_model);

    const QUrl main_window_url = QUrl::fromLocalFile(QStringLiteral("src/ui/qml/MainWindow.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [main_window_url](QObject* object, const QUrl& loaded_url) {
            if (object == nullptr && loaded_url == main_window_url) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(main_window_url);
    return app.exec();
#else
    (void)argc;
    (void)argv;
    std::cout << "Lumos core initialized (Qt UI skipped: Qt6 not found at configure time)." << '\n';
    return 0;
#endif
}

