/*
 * CanvasTrigger.h
 *
 *  Created on: Apr 13, 2026
 *      Author: sandro
 */
#pragma once

#include <QObject>

class CanvasTrigger : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    Q_INVOKABLE void trigger() { emit requestPaint(); }
signals:
    void requestPaint();
};
