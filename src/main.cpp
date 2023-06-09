#include "mainwindow.h"
#include "iostream"
#include "PipeAlgo/include/PipeAlgo.h"
#include "yaml-cpp/yaml.h"
#include "yaml-cpp/parser.h"
#include <QApplication>
#include "string"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
