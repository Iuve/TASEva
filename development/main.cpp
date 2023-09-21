#include "mainwindow.h"
#include <QApplication>

#include "project.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    Project* myProject = Project::get();

    if(argc == 2)
    {
    myProject->setProjectInputFileName(argv[1]);
    w.openProject(false);
    }

    w.show();

    return a.exec();
}
