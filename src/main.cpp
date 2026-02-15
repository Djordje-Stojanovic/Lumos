#include "common/Telemetry.h"

#if defined(LUMOS_WITH_QT)
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#endif

#include <iostream>

int main(int argc, char* argv[]) {
    lumos::common::Telemetry telemetry;
    telemetry.emit("app_started", {{"mode", "desktop"}});

#if defined(LUMOS_WITH_QT)
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

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

