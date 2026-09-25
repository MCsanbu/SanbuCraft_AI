#pragma once

#include "minecraft/Models.h"

#include <QGraphicsView>

#include <vector>

namespace sanbucraft::gui {

class MapView final : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);

    void setWorldData(const std::vector<minecraft::Player>& players,
                      const std::vector<minecraft::Container>& containers);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene scene_;
    double zoom_ = 1.0;
};

}  // namespace sanbucraft::gui
