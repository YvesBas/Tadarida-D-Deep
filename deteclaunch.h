#ifndef DETECLAUNCH_H
#define DETECLAUNCH_H

#ifdef _WIN32
# define SLEEP(time) _sleep(time)
#else
# include <unistd.h>
# define SLEEP(time) usleep(time * 1000)
#endif
#define PARAMNO 0
#define PARAMEXPANSION 1
#define PARAMCOMPRESS 2
#define PARAMHELP 3
#define PARAMNTHREADS 4
#define PARAMNVERSION 5
#define PARAMRECORD 6
#define PARAMWAVSTOCK 7
#define PARAMAUDIO 8
#define PARAMFREQ 9
#define PARAMBEGIN 10
#define PARAMEND 11
#define PARAMLOOP 12
#define PARAMCALENDAR 13
#define PARAMHEIGHT 14

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include "fftw3.h"
#include <QGuiApplication>

// Deteclaunch : main class of tadaridaD (non graphic class)
class DetecLaunch : public QObject
{
Q_OBJECT
public:
    explicit DetecLaunch(QObject *parent=0);
    ~DetecLaunch();
    bool                                Treat(int,char **);

    bool                                 IDebug;
    fftwf_complex*	             ComplexInput[10];
    fftwf_complex*	             FftRes[10];
    fftwf_plan		                 Plan[10][6];
    QTextStream                  LogStream;
    //QGuiApplication*             Pga;
    QFile                                _logFile;
    int                                  _modeHeight;


private slots:
    void                                 detecInfoTreat(QString,int);

private :
    QDateTime                            calculateDate(QString);
    bool                                 calculateDates();
    int                                  calculateNRecords(QDateTime,QDateTime);
    bool                                 createTxtFile(QString);
    bool                                 lanceSox(int,int,int);
    void                                 showHelp();
    void                                 showInfo(QString s,bool b=false,bool e=true,bool l=true);
    void                                 waitForStartTime();

    QString                              _audioName;
    QDateTime                            _endTime;
    QString                              _endTimeString;
    bool                                 _helpShown;
    bool                                  _launchRecord;
    int                                  _modeFreq;
    int                                  _modeDirFile;
    bool                                 _mustCompress;
    int                                  _nbPec;
    int                                  _nbTreatedFiles;
    int                                  _nbErrorFiles;
    int                                  _nbThreads;
    int                                  _nFilesPerTreatment;
    int                                  _nRecords;
    int                                  _nSeries;
    int                                  _nwf;
    int                                  _paramVersion;
    bool                                 _pol;
    float                                _recordSize;
    QDateTime                            _startTime;
    QString                              _startTimeString;
    int                                  _timeExpansion;
    QDir                                 _wavTrav;
    QStringList                          _wavFileList;
    QStringList                          _wavFileListGen;
    QString                              _wavPath;
    QStringList                          _wavRepList;
    bool                                 _wavStock;
    bool                                 _withTimeCsv;
};

#endif // DETECLAUNCH_H
