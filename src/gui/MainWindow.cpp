#include "gui/MainWindow.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <algorithm>

namespace sanbucraft::gui {

namespace {
const QString STYLE = R"(
*{font-family:"Segoe UI","Microsoft YaHei",sans-serif;}
QMainWindow{background:#101510;} QWidget{color:#e8eee8;}
QFrame#sidebar{background:#0b100c;border-right:1px solid #283329;}
QFrame#brandIcon{background:#4caf50;border-radius:9px;}
QLabel#brandTitle{font-size:17px;font-weight:700;color:#f2f7f2;}
QLabel#brandSub{font-size:10px;color:#849184;}
QPushButton#nav{text-align:left;padding:11px 13px;border:0;border-radius:8px;color:#9da99e;background:transparent;}
QPushButton#nav:hover{color:#fff;background:#182019;}
QPushButton#nav:checked{color:#fff;background:#243526;font-weight:600;border-left:3px solid #66d36b;}
QLabel#sectionTitle{font-size:24px;font-weight:700;color:#f4f8f4;}
QLabel#sectionSub,QLabel#muted{font-size:11px;color:#849184;}
QFrame#card,QFrame#playerCard{background:#171e18;border:1px solid #29342b;border-radius:12px;}
QLabel#cardCaption{font-size:10px;font-weight:600;color:#829083;}
QLabel#cardValue{font-size:25px;font-weight:700;color:#f2f7f2;}
QLabel#cardHint,QLabel#badge{color:#72d879;font-size:10px;}
QLabel#badge{background:#203424;border-radius:6px;padding:4px 7px;font-weight:600;}
QLabel#playerName{font-size:14px;font-weight:600;color:#f0f6f0;}
QTableWidget{background:#141a15;alternate-background-color:#111611;border:1px solid #29342b;border-radius:10px;gridline-color:#202920;selection-background-color:#2a4930;}
QTableWidget::item{padding:7px;} QHeaderView::section{background:#1c251d;color:#93a095;border:0;padding:8px;font-size:11px;font-weight:600;}
QLineEdit{background:#141a15;border:1px solid #29342b;border-radius:8px;padding:9px;color:#e8eee8;}
QPushButton#action{background:#4caf50;color:#081008;border:0;border-radius:8px;padding:9px 14px;font-weight:700;}
QStatusBar{background:#0b100c;color:#778378;border-top:1px solid #283329;}
QScrollArea{border:0;background:transparent;}
)";

QLabel* title(const QString& s){auto* x=new QLabel(s);x->setObjectName("sectionTitle");return x;}
QLabel* sub(const QString& s){auto* x=new QLabel(s);x->setObjectName("sectionSub");x->setWordWrap(true);return x;}
QFrame* card(const QString& a,const QString& b,const QString& c){
    auto* f=new QFrame;f->setObjectName("card");auto* l=new QVBoxLayout(f);l->setContentsMargins(15,13,15,13);
    auto* x=new QLabel(a);x->setObjectName("cardCaption");auto* y=new QLabel(b);y->setObjectName("cardValue");auto* z=new QLabel(c);z->setObjectName("cardHint");
    l->addWidget(x);l->addWidget(y);l->addWidget(z);return f;
}
QString dim(const std::string& s){if(s=="minecraft:overworld")return "Overworld";if(s=="minecraft:the_nether")return "Nether";if(s=="minecraft:the_end")return "The End";return QString::fromStdString(s);}
QWidget* scroll(QWidget* w){auto* s=new QScrollArea;s->setWidgetResizable(true);s->setWidget(w);return s;}
}

MainWindow::MainWindow(const minecraft::WorldInfo& world,const std::vector<minecraft::Player>& players,
                       const std::vector<minecraft::Container>& containers,QWidget* parent)
    :QMainWindow(parent),world_(world),players_(players),containers_(containers),
     analysis_(analysis::WorldAnalyzer::analyze(players,containers)){
    setWindowTitle(QString::fromStdString("SanbuCraft AI  —  "+world_.name));resize(1280,800);setMinimumSize(980,640);setStyleSheet(STYLE);
    auto* root=new QWidget;auto* rl=new QHBoxLayout(root);rl->setContentsMargins(0,0,0,0);rl->setSpacing(0);rl->addWidget(createSidebar());
    auto* main=new QWidget;auto* ml=new QVBoxLayout(main);ml->setContentsMargins(28,22,28,14);ml->setSpacing(12);
    auto* top=new QHBoxLayout;auto* hv=new QVBoxLayout;hv->setSpacing(2);pageTitle_=title("World Overview");
    pageSubtitle_=sub(QString::fromStdString(world_.name+"  •  Minecraft Java Edition"));hv->addWidget(pageTitle_);hv->addWidget(pageSubtitle_);top->addLayout(hv);top->addStretch();
    auto* badge=new QLabel("●  ANALYZER READY");badge->setObjectName("badge");top->addWidget(badge);ml->addLayout(top);
    pages_=new QStackedWidget;pages_->addWidget(createDashboard());pages_->addWidget(createPlayersPage());pages_->addWidget(createContainersPage());pages_->addWidget(createMapPage());pages_->addWidget(createAboutPage());ml->addWidget(pages_,1);
    rl->addWidget(main,1);setCentralWidget(root);
    statusBar()->showMessage(QString("Read-only analysis  •  %1 players  •  %2 containers").arg(players_.size()).arg(containers_.size()));switchPage(0);
}

QWidget* MainWindow::createSidebar(){
    auto* s=new QFrame; s->setObjectName("sidebar");s->setFixedWidth(220);auto* l=new QVBoxLayout(s);l->setContentsMargins(18,20,18,18);l->setSpacing(6);
    auto* b=new QHBoxLayout;auto* icon=new QFrame;s->setObjectName("sidebar");icon->setObjectName("brandIcon");icon->setFixedSize(38,38);
    auto* it=new QLabel("S",icon);it->setAlignment(Qt::AlignCenter);it->setStyleSheet("font-size:20px;font-weight:800;color:#081008;");
    auto* bt=new QVBoxLayout;auto* n=new QLabel("SanbuCraft AI");n->setObjectName("brandTitle");auto* st=new QLabel("WORLD INTELLIGENCE");st->setObjectName("brandSub");bt->addWidget(n);bt->addWidget(st);b->addWidget(icon);b->addLayout(bt);l->addLayout(b);l->addSpacing(24);
    auto* w=new QLabel("WORKSPACE");w->setObjectName("brandSub");l->addWidget(w);
    l->addWidget(createNavButton("▣   Overview",0));l->addWidget(createNavButton("●   Players",1));l->addWidget(createNavButton("▤   Containers",2));l->addWidget(createNavButton("⌖   World Map",3));
    l->addSpacing(14);auto* sys=new QLabel("SYSTEM");sys->setObjectName("brandSub");l->addWidget(sys);l->addWidget(createNavButton("⚙   About",4));l->addStretch();
    auto* info=new QLabel("READ ONLY\\nWorld files are never modified.\\n\\nSanbuCraft AI v0.1.0");info->setObjectName("muted");l->addWidget(info);return s;
}
QPushButton* MainWindow::createNavButton(const QString& text,int index){
    auto* b=new QPushButton(text);b->setObjectName("nav");b->setCheckable(true);b->setAutoExclusive(true);b->setCursor(Qt::PointingHandCursor);
    connect(b,&QPushButton::clicked,this,[this,index]{switchPage(index);});navButtons_.push_back(b);return b;
}
void MainWindow::switchPage(int i){
    pages_->setCurrentIndex(i);for(int n=0;n<(int)navButtons_.size();++n)navButtons_[n]->setChecked(n==i);
    static const char* t[]={"World Overview","Players","Containers","World Map","About SanbuCraft AI"};
    static const char* d[]={"A compact snapshot of your Minecraft world.","Players discovered in the selected save.","Storage blocks and their inventory contents.","A lightweight coordinate view of the loaded world.","Local analysis engine • read-only mode."};
    pageTitle_->setText(t[i]);pageSubtitle_->setText(d[i]);
}
QWidget* MainWindow::createDashboard(){
    auto* p=new QWidget;auto* l=new QVBoxLayout(p);l->setContentsMargins(0,0,0,0);l->setSpacing(12);auto* g=new QGridLayout;g->setSpacing(10);
    g->addWidget(card("PLAYERS",QString::number(players_.size()),"entities detected"),0,0);g->addWidget(card("CONTAINERS",QString::number(analysis_.containers),"storage blocks"),0,1);
    g->addWidget(card("FOOD ITEMS",QString::number(analysis_.food),"total detected"),0,2);g->addWidget(card("GAME TIME",QString::number(world_.gameTime),"ticks since start"),0,3);l->addLayout(g);
    auto* row=new QHBoxLayout;row->setSpacing(10);
    auto* wi=new QFrame;wi->setObjectName("card");auto* wl=new QVBoxLayout(wi);wl->addWidget(new QLabel("WORLD INFO"));
    wl->addWidget(sub(QString("Name      %1\\nVersion   %2\\nSeed      %3\\nData ver. %4").arg(QString::fromStdString(world_.name)).arg(QString::fromStdString(world_.versionName.empty()?"Unknown":world_.versionName)).arg(world_.seed).arg(world_.dataVersion)));row->addWidget(wi,1);
    auto* pp=new QFrame;pp->setObjectName("card");auto* pl=new QVBoxLayout(pp);pl->addWidget(new QLabel("ACTIVE PLAYERS"));
    if(players_.empty())pl->addWidget(sub("No player data was found in this save."));else for(int i=0;i<std::min<int>(3,players_.size());++i){auto& x=players_[i];auto* q=new QLabel(QString("●  %1   %2   HP %3").arg(QString::fromStdString(x.name.empty()?x.uuid:x.name)).arg(dim(x.dimension)).arg(x.health,0,'f',1));q->setObjectName("muted");pl->addWidget(q);}
    row->addWidget(pp,1);l->addLayout(row);
    auto* ic=new QFrame;ic->setObjectName("card");auto* il=new QVBoxLayout(ic);auto* ih=new QHBoxLayout;ih->addWidget(new QLabel("ITEM INVENTORY"));ih->addStretch();auto* ct=new QLabel(QString("%1 item types").arg(analysis_.items.size()));ct->setObjectName("muted");ih->addWidget(ct);il->addLayout(ih);il->addWidget(createItemTable(analysis_,ic));l->addWidget(ic,1);return p;
}
QWidget* MainWindow::createPlayersPage(){
    auto* c=new QWidget;auto* l=new QVBoxLayout(c);l->setContentsMargins(0,0,0,0);
    if(players_.empty()){l->addWidget(sub("No players were discovered in this world."));l->addStretch();return scroll(c);}
    for(auto& x:players_){auto* f=new QFrame;f->setObjectName("playerCard");auto* r=new QHBoxLayout(f);
        auto* v=new QVBoxLayout;auto* n=new QLabel(QString::fromStdString(x.name.empty()?x.uuid:x.name));n->setObjectName("playerName");auto* u=new QLabel(QString::fromStdString(x.uuid));u->setObjectName("muted");v->addWidget(n);v->addWidget(u);r->addLayout(v,1);
        auto* st=new QLabel(QString("HP %1   •   Food %2   •   XP %3   •   %4").arg(x.health,0,'f',1).arg(x.foodLevel).arg(x.xpLevel).arg(dim(x.dimension)));st->setObjectName("muted");r->addWidget(st);
        auto* pos=new QLabel(QString("X %1  Y %2  Z %3").arg(x.x,0,'f',1).arg(x.y,0,'f',1).arg(x.z,0,'f',1));pos->setObjectName("badge");r->addWidget(pos);l->addWidget(f);}
    l->addStretch();return scroll(c);
}
QWidget* MainWindow::createContainersPage(){
    auto* p=new QWidget;auto* l=new QVBoxLayout(p);l->setContentsMargins(0,0,0,0);auto* search=new QLineEdit;search->setPlaceholderText("Search containers by type, dimension, or coordinates...");l->addWidget(search);
    auto* t=new QTableWidget(containers_.size(),5);t->setHorizontalHeaderLabels({"Type","Dimension","X","Y","Z"});t->setEditTriggers(QAbstractItemView::NoEditTriggers);t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setAlternatingRowColors(true);t->verticalHeader()->setVisible(false);t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for(int r=0;r<(int)containers_.size();++r){auto& x=containers_[r];t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(x.type)));t->setItem(r,1,new QTableWidgetItem(dim(x.dimension)));t->setItem(r,2,new QTableWidgetItem(QString::number(x.x)));t->setItem(r,3,new QTableWidgetItem(QString::number(x.y)));t->setItem(r,4,new QTableWidgetItem(QString::number(x.z)));}
    connect(search,&QLineEdit::textChanged,t,[t](const QString& q){for(int r=0;r<t->rowCount();++r){bool ok=q.trimmed().isEmpty();for(int c=0;c<t->columnCount()&&!ok;++c)ok=t->item(r,c)->text().contains(q,Qt::CaseInsensitive);t->setRowHidden(r,!ok);}});
    l->addWidget(t,1);return p;
}
QWidget* MainWindow::createMapPage(){auto* m=new MapView;m->setWorldData(players_,containers_);return m;}
QWidget* MainWindow::createAboutPage(){
    auto* p=new QWidget;auto* l=new QVBoxLayout(p);auto* f=new QFrame;f->setObjectName("card");auto* v=new QVBoxLayout(f);
    v->addWidget(title("SanbuCraft AI"));v->addWidget(sub("Minecraft Java Edition world intelligence dashboard. The current client focuses on safe, read-only world analysis."));
    v->addWidget(new QLabel(QString("World: %1\\nVersion: %2\\nPlayers: %3\\nContainers: %4\\nItem types: %5").arg(QString::fromStdString(world_.name)).arg(QString::fromStdString(world_.versionName.empty()?"Unknown":world_.versionName)).arg(players_.size()).arg(containers_.size()).arg(analysis_.items.size())));
    auto* b=new QPushButton("Back to Overview");b->setObjectName("action");connect(b,&QPushButton::clicked,this,[this]{switchPage(0);});v->addWidget(b,0,Qt::AlignLeft);l->addWidget(f);l->addStretch();return p;
}
QTableWidget* MainWindow::createItemTable(const analysis::WorldAnalysis& a,QWidget* parent){
    auto* t=new QTableWidget(a.items.size(),2,parent);t->setHorizontalHeaderLabels({"Item","Count"});t->setEditTriggers(QAbstractItemView::NoEditTriggers);t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setAlternatingRowColors(true);t->verticalHeader()->setVisible(false);t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    int r=0;for(const auto& [id,count]:a.items){t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(id)));t->setItem(r,1,new QTableWidgetItem(QString::number(count)));++r;}return t;
}
} // namespace sanbucraft::gui
