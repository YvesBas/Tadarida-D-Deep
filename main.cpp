
#include "deteclaunch.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDir>
#include <QString>

class DetecLaunch;

/*
int main(int argc, char *argv[])
{
    if(argc<1) return(-1);
	// Deteclaunch : main class of tadaridaD (non graphic class)
    //QCoreApplication.addLibraryPath("imageformats");
    //QGuiApplication qga(argc, argv);
    DetecLaunch *dl= new DetecLaunch((QObject *)(&qga));
    //
    QString logDirPath = QDir::currentPath()+"/log";
    QDir logDir(logDirPath);
    if(!logDir.exists()) logDir.mkdir(logDirPath);
    QString logFilePath(logDirPath + QString("/tadaridaD.log"));
    dl->_logFile.setFileName(logFilePath);
    dl->_logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    dl->LogStream.setDevice(&(dl->_logFile));
    dl->LogStream << "deteclaunch 1" << endl;
    //

    // Treat : this method manages the whole treatment
	dl->Treat(argc,argv);
    delete dl;
    SLEEP(3000);
    exit(0);
}
*/

int main(int argc, char *argv[])
{
    if(argc<1) return(-1);
    // Deteclaunch : main class of tadaridaD (non graphic class)
    //QCoreApplication.addLibraryPath("imageformats");
    DetecLaunch *dl= new DetecLaunch();
    // Treat : this method manages the whole treatment
    dl->Treat(argc,argv);
    delete dl;
    SLEEP(3000);
    exit(0);
}
