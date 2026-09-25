#pragma once

#include "analysis/WorldAnalyzer.h"
#include "gui/MapView.h"
#include "minecraft/Models.h"

#include <QMainWindow>

#include <vector>

class QLabel;
class QTableWidget;

namespace sanbucraft::gui {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(const minecraft::WorldInfo& world,
               const std::vector<minecraft::Player>& players,
               const std::vector<minecraft::Container>& containers,
               QWidget* parent = nullptr);

private:
    static QTableWidget* createItemTable(const analysis::WorldAnalysis& analysis, QWidget* parent);
};

}  // namespace sanbucraft::gui
