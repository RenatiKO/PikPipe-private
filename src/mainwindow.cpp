#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
//#include <dxflib/dl_dxf.h>
//#include <dxflib/dl_global.h>
#include "PipeAlgo/PipeAlgo.h"
#include "dxfrw/libdxfrw.h"
//#include "./ui/ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    dxf_v_.reset(new QDxfViewer);
//    dxf_v_->show();

    ui->verticalLayout->addWidget(dxf_v_.get());


    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onSignalOpen);

    connect(dxf_v_.get(), &QDxfViewer::workspaceClicked, this, &MainWindow::onDxfViewerClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onSignalOpen() {
    parseFile(QFileDialog::getOpenFileName(this, tr("Open File"),
                                           "../data",
                                           tr("DXF format (*.dxf)")));
}

void MainWindow::parseFile(QString filename) {
    dxf_p_.reset(new DxfParser);
    dxfRW dxf(filename.toStdString().c_str());


    dxf_v_->clear();


    if (!dxf.read(dxf_p_.get(), false)) {
        std::cerr << "drawing.dxf could not be opened.\n";
    }

    dxf_p_->wallSkeletonWithHoles();

    int col_cnt = 0;

    redrawGraph();

//    for (auto l : dxf_p_->getLineVec()) {
//        dxf_v_->addLine(l.point(0).x(), l.point(0).y(), l.point(1).x(), l.point(1).y(), 5, /*rnb_colors[col_cnt++]*/Qt::black);
//        if (col_cnt >= rnb_colors.size()) col_cnt = 0;
//    }


//    auto sk = dxf_p_->wallSkeletonWithHoles();//f.wallSkeleton();//f.wallSkeleton();//

//    for (auto it = sk[0].begin()+1; it != sk[0].end(); ++it) {
//        dxf_v_->addLine((it-1)->x(), (it-1)->y(), it->x(), it->y(), /*rnb_colors[col_cnt++]*/Qt::black);
//        if (col_cnt >= rnb_colors.size()) col_cnt = 0;
//    }


//    for (auto it = sk.begin(); it != sk.end(); it += 2) {
//        dxf_v_->addLine((it)->x(), (it)->y(), (it+1)->x(), (it+1)->y(), /*Qt::red*/rnb_colors[col_cnt++]);
//         if (col_cnt >= rnb_colors.size()) col_cnt = 0;
//    }

//    for (auto v : dxf_p_->getVertices()) {
//        dxf_v_->addPoint(v.x(), v.y(), 1, Qt::red);
//    }

//    for (auto v : dxf_p_->getEdges()) {
//        dxf_v_->addLine(v.start().x(), v.start().y(), v.end().x(), v.end().y(), 5, rnb_colors[col_cnt++]);
//        if (col_cnt >= rnb_colors.size()) col_cnt = 0;
//    }
}

void MainWindow::redrawGraph() {
    std::vector<QColor> rnb_colors;

    rnb_colors.push_back(Qt::red);
    rnb_colors.push_back(Qt::darkYellow);
    rnb_colors.push_back(Qt::yellow);
    rnb_colors.push_back(Qt::green);
    rnb_colors.push_back(Qt::cyan);
    rnb_colors.push_back(Qt::blue);
    rnb_colors.push_back(Qt::magenta);

    int col_cnt = 0;

    dxf_v_->clear();
    for (auto l: dxf_p_->getLineVec()) {
        dxf_v_->addLine(l.point(0).x(), l.point(0).y(), l.point(1).x(), l.point(1).y(), 5, /*rnb_colors[col_cnt++]*/
                        Qt::black);
        if (col_cnt >= rnb_colors.size()) col_cnt = 0;
    }

    for (auto v: dxf_p_->getVertices()) {
        dxf_v_->addPoint(v.x(), v.y(), 10, Qt::red);
    }

    col_cnt = 0;

    for (auto v: dxf_p_->getEdges()) {
        dxf_v_->addLine(v.start().x(), v.start().y(), v.end().x(), v.end().y(), 5, rnb_colors[col_cnt++]);
        if (col_cnt >= rnb_colors.size()) col_cnt = 0;
    }
}

void MainWindow::onDxfViewerClicked(QPointF &p) {
    if (ui->actionSortirTool->isChecked()) {
        auto near_seg = dxf_p_->addVertexToGraph({p.x(), p.y(), 200});
        dxf_p_->addMission({p.x(), p.y(), 200}, SanType::TOILET);
        redrawGraph();
        ui->actionSortirTool->setChecked(false);
//        dxf_v_->addLine(near_seg.point(0).x(), near_seg.point(0).y(), near_seg.point(1).x(), near_seg.point(1).y(), 50, /*rnb_colors[col_cnt++]*/Qt::blue);
//        for (auto v : dxf_p_->getVertices()) {
//            dxf_v_->addPoint(v.x(), v.y(), 10, Qt::red);
//        }
    }

    if (ui->actionRakovinaTool->isChecked()) {
        auto near_seg = dxf_p_->addVertexToGraph({p.x(), p.y(), 500});
        dxf_p_->addMission({p.x(), p.y(), 500}, SanType::RAKOVINA);
        redrawGraph();
        ui->actionRakovinaTool->setChecked(false);
//        dxf_v_->addLine(near_seg.point(0).x(), near_seg.point(0).y(), near_seg.point(1).x(), near_seg.point(1).y(), 50, /*rnb_colors[col_cnt++]*/Qt::blue);
//        for (auto v : dxf_p_->getVertices()) {
//            dxf_v_->addPoint(v.x(), v.y(), 10, Qt::red);
//        }
    }

    if (ui->actionStoyakTool->isChecked()) {
        auto near_seg = dxf_p_->addVertexToGraph({p.x(), p.y(), 0});
        dxf_p_->setRootOfGraph({p.x(), p.y(), 0});
        auto res = dxf_p_->graphSearch();
        std::vector<Fitting> fittings;
        loadYamlFittings(fittings, "../data/avalible_fittings.yaml");
        auto graph = dxf_p_->getPipeNodes();
        PipeLine pipe_line(graph, fittings);
        YAML::Node result = pipe_line.makeFittingList();
        std::cout << result << std::endl;
        std::ofstream out("fittings_list.yaml");
        out << result;
        out.close();

        redrawGraph();
        for (auto g: res) {
            dxf_v_->addLine(g.pipe.point(0).x(), g.pipe.point(0).y(), g.pipe.point(1).x(), g.pipe.point(1).y(),
                            g.diameter / 2, /*rnb_colors[col_cnt++]*/Qt::blue);
        }
        ui->actionStoyakTool->setChecked(false);
//        dxf_v_->addLine(near_seg.point(0).x(), near_seg.point(0).y(), near_seg.point(1).x(), near_seg.point(1).y(), 50, /*rnb_colors[col_cnt++]*/Qt::blue);
//        for (auto v : dxf_p_->getVertices()) {
//            dxf_v_->addPoint(v.x(), v.y(), 10, Qt::red);
//        }
    }
}

