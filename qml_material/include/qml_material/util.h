#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QColor>
#include <QQuickItem>
#include <QPointer>
#include <QQuickWindow>

#include "qml_material/corner.h"
#include "core/core.h"

namespace qml_material
{

class Util : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    using QObject::QObject;

    enum Track
    {
        TrackCreate = 0,
        TrackDelete
    };
    Q_ENUMS(Track)

    Q_INVOKABLE void track(QVariant, Track);

    Q_INVOKABLE bool hasIcon(const QJSValue& v) {
        auto name   = v.property("name");
        auto source = v.property("source");
        if (name.isString() && source.isVariant()) {
            return ! name.toString().isEmpty() || ! source.toString().isEmpty();
        }
        return false;
    }

    Q_INVOKABLE QColor transparent(QColor in, float alpha) {
        in.setAlphaF(alpha);
        return in;
    }

    Q_INVOKABLE void closePopup(QObject* obj) {
        do {
            auto meta = obj->metaObject();
            do {
                if (meta->className() == std::string("QQuickPopup")) {
                    QMetaObject::invokeMethod(obj, "close");
                    return;
                }
            } while (meta = meta->superClass(), meta);
        } while (obj = obj->parent(), obj);
    }

    Q_INVOKABLE QColor hoverColor(QColor in) {
        in.setAlphaF(0.08);
        return in;
    }

    Q_INVOKABLE QColor pressColor(QColor in) {
        in.setAlphaF(0.18);
        return in;
    }

    Q_INVOKABLE qreal devicePixelRatio(QQuickItem* in) {
        return in ? in->window() ? in->window()->devicePixelRatio() : 1.0 : 1.0;
    }

    // tl tr bl br
    Q_INVOKABLE CornersGroup corner(QVariant in);

    Q_INVOKABLE CornersGroup corner(qreal br, qreal tr, qreal bl, qreal tl);

private:
    usize m_tracked { 0 };
};
} // namespace qml_material