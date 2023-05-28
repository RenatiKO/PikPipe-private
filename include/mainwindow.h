#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qdxfviewer.h"
#include "dxf_parser.h"
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<QDxfViewer> dxf_v_;
    std::shared_ptr<DxfParser> dxf_p_;

    void parseFile(QString filename);

    void redrawGraph();

private slots:
    void onDxfViewerClicked(QPointF& p);
    void onSignalOpen();
};
#endif // MAINWINDOW_H
