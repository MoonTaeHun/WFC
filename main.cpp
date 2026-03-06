#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "wfcmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    WfcModel wfcModel;

    QQmlApplicationEngine engine;

    // QML 컨텍스트에 "wfcModel" 이름으로 등록
    engine.rootContext()->setContextProperty("wfcModel", &wfcModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("WFC", "Main");

    return app.exec();
}
