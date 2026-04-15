#include <iostream>
#include "config.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "GameModel.h"

int main(int argc, char **argv) {
	using namespace othello;

	QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;

	GameModel game;
	engine.rootContext()->setContextProperty("gameModel", &game);

	engine.load(QUrl(QStringLiteral("qrc:/OthelloQmlApp/main.qml")));
	if (engine.rootObjects().isEmpty()) return -1;
	return app.exec();
}
