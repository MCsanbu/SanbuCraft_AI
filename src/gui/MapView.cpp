#include "gui/MapView.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QWheelEvent>

namespace sanbucraft::gui {

MapView::MapView(QWidget* parent) : QGraphicsView(parent) {
    setScene(&scene_);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QColor("#19221a"));
}

void MapView::setWorldData(const std::vector<minecraft::Player>& players,
                           const std::vector<minecraft::Container>& containers) {
    scene_.clear();
    constexpr int gridSize = 16;
    constexpr int range = 2048;
    QPen gridPen(QColor("#304535"));
    for (int coordinate = -range; coordinate <= range; coordinate += gridSize * 4) {
        scene_.addLine(coordinate, -range, coordinate, range, gridPen);
        scene_.addLine(-range, coordinate, range, coordinate, gridPen);
    }

    for (const auto& container : containers) {
        auto* marker = scene_.addRect(container.x - 3, container.z - 3, 6, 6,
                                      QPen(QColor("#f0b44d")), QBrush(QColor("#f0b44d")));
        marker->setToolTip(QString::fromStdString(container.type + "\nX " + std::to_string(container.x) +
                                                   "  Y " + std::to_string(container.y) +
                                                   "  Z " + std::to_string(container.z)));
    }
    for (const auto& player : players) {
        auto* marker = scene_.addEllipse(player.x - 5, player.z - 5, 10, 10,
                                         QPen(QColor("#54c9ff")), QBrush(QColor("#54c9ff")));
        marker->setToolTip(QString::fromStdString(player.uuid + "\nX " + std::to_string(static_cast<int>(player.x)) +
                                                   "  Y " + std::to_string(static_cast<int>(player.y)) +
                                                   "  Z " + std::to_string(static_cast<int>(player.z))));
    }
    scene_.setSceneRect(-range, -range, range * 2, range * 2);
}

void MapView::wheelEvent(QWheelEvent* event) {
    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    const double nextZoom = zoom_ * factor;
    if (nextZoom < 0.25 || nextZoom > 8.0) return;
    zoom_ = nextZoom;
    scale(factor, factor);
}

}  // namespace sanbucraft::gui
