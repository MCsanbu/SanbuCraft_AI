#include "gui/MainWindow.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QFormLayout>
#include <QLabel>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace sanbucraft::gui {

MainWindow::MainWindow(const minecraft::WorldInfo& world,
                       const std::vector<minecraft::Player>& players,
                       const std::vector<minecraft::Container>& containers,
                       QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QString::fromStdString("SanbuCraft AI — " + world.name));
    resize(1100, 720);
    const auto analysis = analysis::WorldAnalyzer::analyze(players, containers);
    auto* tabs = new QTabWidget(this);

    auto* dashboard = new QWidget(tabs);
    auto* dashboardLayout = new QVBoxLayout(dashboard);
    auto* summary = new QFormLayout();
    summary->addRow("World", new QLabel(QString::fromStdString(world.name), dashboard));
    summary->addRow("Seed", new QLabel(QString::number(world.seed), dashboard));
    summary->addRow("Game time", new QLabel(QString::number(world.gameTime), dashboard));
    summary->addRow("Players", new QLabel(QString::number(players.size()), dashboard));
    summary->addRow("Containers", new QLabel(QString::number(analysis.containers), dashboard));
    summary->addRow("Food items", new QLabel(QString::number(analysis.food), dashboard));
    dashboardLayout->addLayout(summary);
    dashboardLayout->addWidget(createItemTable(analysis, dashboard));
    tabs->addTab(dashboard, "Dashboard");

    auto* map = new MapView(tabs);
    map->setWorldData(players, containers);
    tabs->addTab(map, "World Map");
    setCentralWidget(tabs);
    statusBar()->showMessage("Read-only analysis: no Minecraft world files are modified.");
}

QTableWidget* MainWindow::createItemTable(const analysis::WorldAnalysis& analysis, QWidget* parent) {
    auto* table = new QTableWidget(static_cast<int>(analysis.items.size()), 2, parent);
    table->setHorizontalHeaderLabels({"Item", "Count"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    int row = 0;
    for (const auto& [item, count] : analysis.items) {
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(item)));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
        ++row;
    }
    table->resizeColumnsToContents();
    return table;
}

}  // namespace sanbucraft::gui
