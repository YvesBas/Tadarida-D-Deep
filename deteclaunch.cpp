#include "deteclaunch.h"
#include "detec.h"
using namespace std;
#include <iostream>


DetecLaunch::DetecLaunch(QObject *parent) : QObject(parent)
{
	// Deteclaunch : main class of tadaridaD (non graphic class)
    //Pga = (QGuiApplication *)parent;
}

DetecLaunch::~DetecLaunch()
{

}

bool DetecLaunch::Treat(int argc, char *argv[])
{
    // Treat : this method manages the whole treatment
	// Initializations
    _timeExpansion = 10;
    _nbThreads = 1;
    _withTimeCsv = false;
    _paramVersion = 1;  
    IDebug = false;
    _mustCompress = false;
    _modeFreq = 1;
    _modeHeight = 128;
    _pol = false;
    _launchRecord = false; 
	_wavStock = false;
    _helpShown = false;
    _audioName = "";
    _startTimeString = "";
    _endTimeString = "";
    _modeDirFile = NODETERMINED;
    _recordSize = 5;
    _nRecords = 0;
    _nFilesPerTreatment=50;
    Detec **pdetec;
    QStringList   *pWavFileList;
    bool *threadRunning;
    bool *processRunning;
    QString logDirPath;
    _nbTreatedFiles = 0;
    _nbErrorFiles = 0;
    // Use of parameters
    bool waitValue=false;
    int paramParam=PARAMNO;
    bool on_a_determine = false;
    QStringList   firstList;
    QString determine;
    int nbLoop = 1;
    QString calendarName="";
    int nbDates = 0;
    QVector<QDateTime>  listStartDates;
    QVector<QDateTime>  listEndDates;
    // ---------------
    for(int i=1;i<argc;i++)
    {
        QString alire = QString(argv[i]);
        if(alire.left(1)=="-")
        {
            paramParam = PARAMNO;
            waitValue = false;
            if(alire.length()==2)
            {
                if(alire.right(1) == "x") {paramParam = PARAMEXPANSION; waitValue=true;}
                if(alire.right(1) == "c") _mustCompress = true;
                if(alire.right(1) == "h") showHelp();
                if(alire.right(1) == "t") {paramParam = PARAMNTHREADS; waitValue=true;}
                if(alire.right(1) == "s") _withTimeCsv = true;
                if(alire.right(1) == "v") {paramParam = PARAMNVERSION; waitValue=true;}
                if(alire.right(1) == "d") IDebug = true;
                if(alire.right(1) == "r") {_launchRecord = true; paramParam = PARAMRECORD; waitValue=true;}
                if(alire.right(1) == "a") {paramParam = PARAMAUDIO; waitValue=true;}
                if(alire.right(1) == "w") _wavStock = true;
                if(alire.right(1) == "f") {paramParam = PARAMFREQ; waitValue=true;}
                if(alire.right(1) == "u") {paramParam = PARAMHEIGHT; waitValue=true;}
                if(alire.right(1) == "b") {paramParam = PARAMBEGIN; waitValue=true;}
                if(alire.right(1) == "e") {paramParam = PARAMEND; waitValue=true;}
                if(alire.right(1) == "l") {paramParam = PARAMLOOP; waitValue=true;}
                if(alire.right(1) == "g") {paramParam = PARAMCALENDAR; waitValue=true;}
                if(alire.right(1) == "p") _pol = true;
            }
        }
        else
        {
            if(waitValue)
            {
                bool ok;
                int value = alire.toInt(&ok,10) ;
                if(paramParam==PARAMEXPANSION)
                {
                    if(ok==true && (value == 1 || value == 10)) _timeExpansion = value;
                }
                if(paramParam==PARAMNTHREADS)
                {
                    if(ok==true && (value > 1 && value <= 8)) _nbThreads = value;
                }
                if(paramParam==PARAMNVERSION)
                {
                    if(ok==true && value > 0) _paramVersion = value;
                }
                if(paramParam==PARAMRECORD)
                {
                    if(ok==true && value > 0) _nRecords = value;
                }
                if(paramParam==PARAMAUDIO)
                {
                    _audioName = alire;
                }
                if(paramParam==PARAMFREQ)
                {
                    if(ok==true && (value ==1 || value == 2)) _modeFreq = value;
                }
                if(paramParam==PARAMHEIGHT)
                {
                    if(ok==true && (value ==128 || value == 256 || value == 512)) _modeHeight = value;
                }
                if(paramParam==PARAMBEGIN)
                {
                    _startTimeString = alire;
                }
                if(paramParam==PARAMEND)
                {
                    _endTimeString = alire;
                }
                if(paramParam==PARAMLOOP)
                {
                    if(ok==true && value > 1 && value < 500) nbLoop = value;
                }
                if(paramParam==PARAMCALENDAR)
                {
                    calendarName = alire;
                }
            }
            else
            {
                if(!on_a_determine)
                {
                    determine = "";
                    if(alire.length()>4) determine = alire.right(4).toLower();
                    if(determine == ".wav" ) _modeDirFile = FILESMODE;
                    else
                    {
                        _modeDirFile = DIRECTORYMODE;
                        _wavPath = alire;
                    }
                }
                if(_modeDirFile == FILESMODE) firstList.append(alire);
            }
            waitValue=false;
        }
    }
    // -----------------------------------------------------------------
    // initializations
    logDirPath = QDir::currentPath()+"/log";
    QDir logDir(logDirPath);
    if(!logDir.exists()) logDir.mkdir(logDirPath);
    QString logFilePath(logDirPath + QString("/tadaridaD.log"));
    _logFile.setFileName(logFilePath);
    _logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    LogStream.setDevice(&_logFile);
    LogStream << "deteclaunch 1" << endl;
    // ------------------------------------
    if(!calendarName.isEmpty())
    {
        QFile calFile;
        QTextStream calStream;
        calFile.setFileName(calendarName);
        if(calFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            calStream.setDevice(&calFile);
            while(!calStream.atEnd())
            {
                QString line = (calStream.readLine());
                if(line.isNull() or line.isEmpty()) break;
                QStringList ld = line.split(" ");
                if(ld.size()==2)
                {
                    QDateTime d1 = calculateDate(ld.at(0));
                    QDateTime d2 = calculateDate(ld.at(1));
                    if(d1<d2)
                    {
                        listStartDates.push_back(d1);
                        listEndDates.push_back(d2);
                        nbDates++;
                    }
                }
            }
        }
        else
        {
            showInfo(QString("Calendar file doesn't exist : ")+calendarName,false,true,true);
            exit(0);
        }
        if(nbDates==0)
        {
            showInfo(QString("Calendar file doesn't contains dates"),false,true,true);
            exit(0);
        }
        else
        {
            nbLoop = nbDates;
            _launchRecord = true;
        }
    }
    // -----------------------------------------------------------------
    if(nbDates==0)
    {
        if(!_endTimeString.isEmpty() || !_startTimeString.isEmpty())
        {
            _launchRecord = true;
            calculateDates();
        }
        if(!_endTimeString.isEmpty())
        {
            _nRecords = calculateNRecords(_startTime,_endTime);
            if(_nRecords > 0) _launchRecord = true;
        }
        else
        {
            if(!_startTimeString.isEmpty() && _nRecords==0)
            {
                 showInfo("Time Start is defined, you must define -r or -e parameter !",false,true,false);
                 exit(0);
            }
        }
    }
    // -----------------------------------------------------------------
    // _launchRecord = true : tadaridad launches sox to record sound files to process
    if(_launchRecord)
    {
        _nbThreads = 1;
        _nSeries = (_nRecords + _nFilesPerTreatment - 1)/ _nFilesPerTreatment;
        _modeDirFile = DIRECTORYMODE;
        if(!createTxtFile(QDir::currentPath()))
        {
            showInfo("Unable to create txt directory !",false,true,true);
            _logFile.close();
            exit(0);
        }

    }
    else _nSeries = 0;
    // -----------------------------------------------------------------
    if(!_helpShown) showInfo("Launch TadaridaD",false,true,true);
    // the files to be processed are not entered: program stopped
    if(_modeDirFile == NODETERMINED)
    {
        if(!_helpShown)
        showInfo(QString("\nNo file or folder to treat ! You must enter :\n   >TadaridaD [optional setttings] [folder name or .wav files list to treat]\n"),false,true,true);
        _logFile.close();
        return(false);
    }
    // ---------------------------------------------------------------------
    if(!_launchRecord) nbLoop=1;
    else
    {
        if(_nRecords > 16000) nbLoop=1;
    }
    int totSeries = nbLoop * _nSeries;
    int itSerie = 0;
// ---------------------------------------------------------------------
// Main loops - single pass in standard mode (without using sox)
for(int iLoop = 0;iLoop < nbLoop;iLoop++)
{
    // ----------------------------------
    if(nbDates>0)
    {
        _startTime = listStartDates.at(iLoop);
        _endTime = listEndDates.at(iLoop);
        _nRecords = calculateNRecords(_startTime,_endTime);
        if(_nRecords < 1) continue;
        _nSeries = (_nRecords + _nFilesPerTreatment - 1)/ _nFilesPerTreatment;
        showInfo(QString("calendar mode - line ")+QString::number(iLoop+1)+" sur "+QString::number(nbLoop),false,true,true);
        showInfo(QString("   startime : ")+_startTime.toString("dd-MM  hh:mm"),false,true,true);
        showInfo(QString("   endtime  : ")+_endTime.toString("dd-MM  hh:mm"),false,true,true);
    }
    // ----------------------------------
    // waiting for the start time
    if(_launchRecord && (!_startTimeString.isEmpty() || iLoop>0) || nbDates>0)
    {
        waitForStartTime();
    }
    // ----------------------------------
    for(int iSerie = 0;iSerie<_nSeries+1;iSerie++)
    {
        itSerie = _nSeries*iLoop+iSerie;
        if(_launchRecord && iSerie>0)
        {
            QString wavtravPath(QDir::current().path()+"/wavtrav");
            if((iSerie&1)==1) wavtravPath += "1"; else  wavtravPath += "2";
            _wavPath = wavtravPath;
        }

        if(!_launchRecord || iSerie>0)
        {
            // creation of the list of files to be processed
            // First case: a directory has been entered
            if(_modeDirFile == DIRECTORYMODE)
            {
                QDir sdir(_wavPath);
                if(!sdir.exists())
                {
                    showInfo(QString("Folder ")+_wavPath+" does not exist !",false,true,true);
                    _logFile.close();
                    return(false);
                }
                _wavFileList = sdir.entryList(QStringList("*.wav"), QDir::Files);

                if(!_launchRecord)
                {
                    if(!createTxtFile(sdir.absolutePath()))
                    {
                        showInfo("Unable to create txt directory !",false,true,true);
                        _logFile.close();
                        return(false);
                    }
                }
            }
            else
            {
                // Second case: file names have been entered
                QFile f;
                QDir d;
                QString determine;
                _nwf=0;
                foreach(QString wf,firstList)
                {
                    if(wf.length()>4) determine = wf.right(4).toLower();
                    else determine = "";
                    if(determine == ".wav" )
                    {
                        f.setFileName(wf);
                        if(f.exists())
                        {
                            QFileInfo finf(wf);
                            d = finf.dir();
                            if(d.exists())
                            {
                                _wavFileList.append(finf.fileName());
                                _wavRepList.append(d.absolutePath());
                                _nwf++;
                            }
                        }
                    }
                }
            }
            _nwf = _wavFileList.size();
            if(_nwf==0)
            {
                showInfo("No wav file to treat ",false,true,true);
                _logFile.close();
                return(false);
            }
            // -----------------------------------------------------------------
            // Distribution of files between threads
            if(_modeDirFile!=DIRECTORYMODE || _nwf <2) _nbThreads = 1;
            else
            {
                if(_nwf < _nbThreads) _nbThreads = _nwf;
            }
            showInfo(QString::number(_nbThreads)+" thread(s) launched",false,true,true);
            showInfo(QString("See log files in log folder"),false,true,true);
            pdetec = new Detec*[_nbThreads];
            pWavFileList = new QStringList[_nbThreads];
            threadRunning = new bool[_nbThreads];
            if(_nbThreads>0)
            {
                int c=0;
                for(int j=0;j<_nwf;j++)
                {
                    pWavFileList[c].append(_wavFileList.at(j));
                    c++;
                    if(c>=_nbThreads) c=0;
                }
            }
            // -----------------------------------------------------------------
            // Initializing shared fftw variables
            int fh;
            for(int j=0;j<_nbThreads;j++)
            {
                FftRes[j] 		= ( fftwf_complex* ) fftwf_malloc( sizeof( fftwf_complex ) * FFT_HEIGHT_MAX );
                ComplexInput[j]        = ( fftwf_complex* ) fftwf_malloc( sizeof( fftwf_complex ) * FFT_HEIGHT_MAX );
                for(int i=0;i<6;i++)
                {
                    fh = pow(2,7+i);
                    Plan[j][i] = fftwf_plan_dft_1d(fh, ComplexInput[j], FftRes[j], FFTW_FORWARD, FFTW_ESTIMATE );
                }
            }
            // -----------------------------------------------------------------
            // Launching threads
            QString threadSuffixe = "";
            for(int i=0;i<_nbThreads;i++)
            {
                // creating a thread
                if(_nbThreads>1) threadSuffixe = QString("_") + QString::number(i+1);
                if(_nbThreads==1) pdetec[i] = new Detec(this,i,threadSuffixe,_modeDirFile,_wavPath,_wavFileList,_wavRepList,_timeExpansion,_withTimeCsv,_paramVersion,IDebug,_launchRecord,_mustCompress,_modeFreq,_pol);
                else  pdetec[i] = new Detec(this,i,threadSuffixe,_modeDirFile,_wavPath,pWavFileList[i],_wavRepList,_timeExpansion,_withTimeCsv,_paramVersion,IDebug,_launchRecord,_mustCompress,_modeFreq,_pol);
                connect(pdetec[i], SIGNAL(info1(QString,int)),this, SLOT(detecInfoTreat(QString,int)));
                // launching this thread
                pdetec[i]->start();
                threadRunning[i]=true;
            }
        }
        // -----------------------------------------------------------------
        // Mode of operation with sound recording: launch of sox
        if(_launchRecord)
        {
            if(iSerie < _nSeries)
            {
                int nfi = _nFilesPerTreatment;
                if(iSerie == _nSeries-1) nfi = _nRecords - (_nSeries - 1) * _nFilesPerTreatment;
                LogStream << QString("Lance sox :  ")+QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;

                if(!lanceSox(iSerie,nfi,itSerie))
                {
                    showInfo("Unable to launch sox !",false,true,true);
                    _logFile.close();
                    return(false);
                }
                if(iSerie==0) continue;
            }
        }
        // -----------------------------------------------------------------
        // Waiting for the end of threads
        int nbtr = _nbThreads;
        while(nbtr>0)
        {
            for(int i=0;i<_nbThreads;i++)
            {
                if(threadRunning[i]) if(!pdetec[i]->isRunning())
                {
                    threadRunning[i] = false;
                    nbtr--;
                    delete pdetec[i];
                    }
            }
            SLEEP(20);
        }
        delete[] pdetec;
        delete[] pWavFileList;
        delete[] threadRunning;
        // -----------------------------------------------------------------
        // Release of thread memory allocations
        for(int j=0;j<_nbThreads;j++)
        {
            fftwf_free(FftRes[j]);
            fftwf_free(ComplexInput[j]);
        }
        // -----------------------------------------------------------------
        if(_launchRecord && iSerie>0)
        {
            // Copy of files in the case of live recordings
            QString taStockPath(QDir::current().path()+"/txt");
            QDir taStock(taStockPath);
            if(!taStock.exists()) if(!taStock.mkdir(taStockPath))
            {
                showInfo("Unable to create txt directory !",false,true,true);
                _logFile.close();
                return(false);
            }
            // conservation of .wav files (-w option)
            if(_wavStock)
            {
                QString wavStockPath(QDir::current().path()+"/wav");
                QDir wavStock(wavStockPath);
                if(!wavStock.exists()) if(!wavStock.mkdir(wavStockPath))
                {
                    showInfo("Unable to create wav directory !",false,true,true);
                    _logFile.close();
                    return(false);
                }
                // list re-created after cutting of files by thread
                QDir sdir(_wavPath);
                if(sdir.exists()) _wavFileList = sdir.entryList(QStringList("*.wav"), QDir::Files);
                foreach(QString wavFileName,_wavFileList)
                {
                    QFile wavFile(_wavPath + "/" + wavFileName);
                    if(wavFile.exists()) wavFile.copy(wavStockPath+"/"+wavFileName);
                }
            }
        }
    } // next iSeries
    if(iLoop < nbLoop-1)
    {
        // modify start and end time for next loop
        //_startTime.addDays(1);
        //_endTime.addDays(1);
        // only for tests
        if(nbDates==0)
        {
            _startTime=_startTime.addDays(1);
            _endTime=_endTime.addDays(1);
            //showInfo(QString("nouveau calcul de startime : ")+_startTime.toString("dd-MM  hh:mm"),false,true,true);
        }
        //showInfo(QString("nouveau calcul de endtime : ")+_endTime.toString("dd-MM  hh:mm"),false,true,true);
    }
} // next iLoop
    // -----------------------------------------------------------------
    showInfo(QString("\nEnd of TadaridaD"),true,true,false);
    _logFile.close();
    return(true);
}

bool DetecLaunch::createTxtFile(QString dirPath)
{
    QString txtPath = dirPath+"/txt";
    QDir reptxt(txtPath);
    if(!reptxt.exists())
    {
        if(!reptxt.mkdir(txtPath)) return(false);
    }
    return(true);
}

bool DetecLaunch::lanceSox(int iserie,int nfi,int itserie)
{
    // launching sox to record the sound files to be processed
    QString wavtravPath(QDir::current().path()+"/wavtrav");
    if((iserie&1)==0) wavtravPath += "1"; else  wavtravPath += "2";
    QDir wavtrav(wavtravPath);
    if(!wavtrav.exists())
    {
        if(!wavtrav.mkdir(wavtravPath)) return(false);
    }
    else
    {
        QStringList listBefore = wavtrav.entryList(QStringList("*.wav"), QDir::Files);
        foreach(QString f,listBefore) wavtrav.remove(f);
        QString taTravPath(wavtravPath+"/txt");
        QDir taTrav(taTravPath);
        if(taTrav.exists())
        {
            QString taDirPath =  wavtravPath+"/txt";
            QDir taDir(taDirPath);
            QStringList listTaBefore = taDir.entryList(QStringList("*.ta"), QDir::Files);
            foreach(QString f,listTaBefore) taDir.remove(f);
        }

		}
    _wavTrav = wavtrav;
    QString program = "sox";
    // a big file has to be cut off in the thread
    QString fichierWav = wavtravPath + "/f" + QString::number(itserie+1) + ".wav";
    QStringList  arguments;
    // arguments << "-c" << "1" << "-d" << fichierWav << "trim" << "0" << QString::number(_recordSize*nfi) ;
    QString paraudio = "hw:1,0";
    if(_audioName == "tw")
    {
        arguments << "-c" << "1" << "-d" << fichierWav << "trim" << "0" << QString::number(_recordSize*nfi) ;
    }
    else
    {
        if(!_audioName.isEmpty()) paraudio = QString("hw:") + _audioName;
        arguments << "-c" << "1" << "-t" << "alsa" << paraudio << fichierWav << "trim" << "0" << QString::number(_recordSize*nfi) ;
    }
    QProcess p;
    p.execute(program,arguments);
    return(true);
}

void DetecLaunch::showInfo(QString s,bool showTime,bool e,bool l)
{
    QString s2 = "";
    if(showTime) s2 = QString("  -   ")+QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    if(l) LogStream << s << s2 << endl;
    if(e) cout << s.toStdString() << s2.toStdString() << endl;
}

void DetecLaunch::detecInfoTreat(QString wavFile,int resu)
{
    //LogStream << "detecInfoTreat " << endl;
    if(resu)_nbTreatedFiles++; else _nbErrorFiles++;
    QString sresu;
    if(resu==1) sresu = "ok"; else sresu = "error";
    showInfo(QString("Treatment of ")+wavFile+" : "+sresu,false,true,true);
}

void DetecLaunch::showHelp()
{
    QString helpInfo = "\n-t [n] allows to execute n parallel threads (1 by default)\n";
    helpInfo += "-x [n] is the time expansion factor,\n   either 10 (default) for 10-times expanded .wav files\n   or 1 for direct recordings\n";
    helpInfo += "-v [n] sets the list of features to be extracted on each detected sound event\n   (2 by default)\n";
    helpInfo += "_f [n] sets the frequency bands to be used;\n   n = 2 allows to treat low frequencies (0.8 to 25 kHz)\n   whereas n=1 (default) treats high frequencies (8 to 250 kHz)\n";
    helpInfo += "-c gives compressed version of .ta output files\n";
    helpInfo += "\?nAfter optional settings, must be mentioned :\n   - either a directory path containing .wav files \n   - or a list of .wav files, to be processed.\n   Relative or absolute paths can be used.";
    showInfo(helpInfo,false,true,false);
    _helpShown = true;
}

void DetecLaunch::waitForStartTime()
{
    //LogStream << "detecInfoTreat " << endl;
    showInfo(QString("Waiting start time : ")+_startTime.toString("dd-MM  hh:mm"),false,true,true);
   QDateTime now = QDateTime::currentDateTime();
   // int cs=0;
   while(now < _startTime)
   {
       SLEEP(1000);
       /*
       cs++;
       if(cs==60)
       {
           cs=0;
           showInfo(QString("attente - il est ")+now.toString("dd-MM  hh:mm:ss"),false,true,true);;
       }
       */
       now = QDateTime::currentDateTime();
   }
   //showInfo(QString("attente terminee - il est ")+now.toString("dd-MM  hh:mm"),false,true,true);
}

int DetecLaunch::calculateNRecords(QDateTime sT,QDateTime eT)
{
   int nsec = sT.secsTo(eT);
   if(nsec < 1)
   {
       showInfo("Time parameter is incorrect",false,true,true);
       exit(0);
   }
   int nrec = (int)(((float)nsec + _recordSize - 1)/_recordSize);
   showInfo(QString("Calculation ot the number of records  : ")+QString::number(nrec),false,true,true);
   return(nrec);
}

bool DetecLaunch::calculateDates()
{
    //showInfo("calculateDates",false,true,true);
    if(!_endTimeString.isEmpty())
    {
        _endTime = calculateDate(_endTimeString);
        showInfo(QString("...endtime : ")+_endTime.toString("dd-MM  hh:mm"),false,true,true);
    }
    if(!_startTimeString.isEmpty())
    {
        _startTime = calculateDate(_startTimeString);
        showInfo(QString("...startime : ")+_startTime.toString("dd-MM  hh:mm"),false,true,true);
    }
    else
    {
        _startTime = QDateTime::currentDateTime();
        //showInfo("starttime = now ",false,true,true);
    }
    if(_startTime > _endTime) _endTime = _endTime.addDays(1);
}

QDateTime DetecLaunch::calculateDate(QString s)
{
   QDate today =  QDate::currentDate();
   QDateTime now = QDateTime::currentDateTime();
   int sh,smn,sy,sm,sd;
   bool b1,b2,b3;
   bool withDate = false;
   QString sdate="",stime;
   QDate d;
   QDateTime resu;
   QString cme = "0";
   QString complement = "   Format : hh:mm ou dd-mm-yyyy_hh:mm";
   // --------------------------------
   // date
   if(s.count("-")==2)
   {
       bool dok = true;
       withDate = true;
       int ppt = s.indexOf("-");
       if(ppt>=2)
       {
           sdate=s.mid(ppt-2,10);
           stime = s.mid(ppt+9,5);
           QStringList lv = sdate.split("-");
           if(lv.size()==3)
           {
              sd=lv.at(0).toInt(&b1);
              sm=lv.at(1).toInt(&b2);
              sy=lv.at(2).toInt(&b3);
              QString sv = lv.at(0)+","+lv.at(1)+","+lv.at(2);

               if(b1 && b2 && b3)
               {
                   d = QDate(sy,sm,sd);
                   if(!d.isValid()) { cme = QString("1 : ")+QString::number(sd)+","+QString::number(sm)+","+QString::number(sy); dok = false;}
                   else
                   {
                       if(d<today)
                       {
                           showInfo(QString("This date is outdated : ")+sdate+complement,false,true,true);
                           dok = false;
                       }
                   }
               }
               else { cme = QString("3   s=")+s+"   sdate="+sdate+"   sv="+sv; dok=false; }
           }
           else { cme = QString("4"); dok=false; }
       }
       else { dok = false; cme = QString("5")+" : ppt="+QString::number(ppt);}
       if(dok==false)
       {

           //showInfo(QString("Date is incorrect : ")+s+" ("+cme+")",false,true,true);
           showInfo(QString("Date is incorrect : ")+s+complement,false,true,true);
           exit(0);
       }
   }
   else
   {
       withDate = false;
       stime =s;
       d = today;
   }

   // --------------------------------
   // time
   bool tok = true;
   QStringList lv = stime.split(":");
   if(lv.size()==2)
   {
      sh=lv.at(0).toInt(&b1);
      smn=lv.at(1).toInt(&b2);
      QString sv = lv.at(0)+","+lv.at(1);

      if(b1 && b2)
      {
          QTime t = QTime(sh,smn);
          if(!t.isValid())
          {
              showInfo(QString("Time is incorrect : ")+stime+complement,false,true,true);
              exit(0);
          }
          resu = QDateTime(d,t);
          if(!resu.isValid()) {tok = false; cme=QString("1 stime=")+stime;}
          else
          {
             if(now > resu && !withDate) resu = resu.addDays(1);
          }
      }
      else {tok = false; cme=QString("2 stime=")+stime+" sv="+sv;}
   }
   else {tok = false; cme=QString("3 stime=")+stime;}

   if(tok == false)
   {
       //showInfo(QString("Time is incorrect : ")+s+" ("+cme+")",false,true,true);
       showInfo(QString("Time is incorrect : ")+s+complement,false,true,true);
       exit(0);
   }
   // --------------------------------
   return(resu);
}
