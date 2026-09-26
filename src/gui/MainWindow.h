#pragma once

#include "analysis/WorldAnalyzer.h"
#include "gui/MapView.h"
#include "minecraft/Models.h"

#include <QMainWindow>

#include <vector>

class QLabel;
class QStackedWidget;
class QPushButton;
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
    QWidget* createSidebar();
    QWidget* createDashboard();
    QWidget* createPlayersPage();
    QWidget* createContainersPage();
    QWidget* createMapPage();
    QWidget* createAboutPage();

    QPushButton* createNavButton(const QString& text, int index);
    QTableWidget* createItemTable(const analysis::WorldAnalysis& analysis, QWidget* parent);

    void switchPage(int index);

    minecraft::WorldInfo world_;
    std::vector<minecraft::Player> players_;
    std::vector<minecraft::Container> containers_;
    analysis::WorldAnalysis analysis_;

    QStackedWidget* pages_ = nullptr;
    std::vector<QPushButton*> navButtons_;
    QLabel* pageTitle_ = nullptr;
    QLabel* pageSubtitle_ = nullptr;
};

}  // namespace sanbucraft::gui
