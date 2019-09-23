#include <QApplication>
#include "src/views/mainwindow.h"
#include "qgsapplication.h"

int main(int argc, char *argv[]) {
    QgsApplication a(argc, argv,true);
    QgsApplication::setPrefixPath("C:/qgis3.4.9_vs2017_qt5.12.4", true);
    QgsApplication::initQgis();
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
