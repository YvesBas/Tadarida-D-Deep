#include "detec.h"
#include "detectreatment.h"

using namespace std;


Detec::Detec(DetecLaunch *pdl,int iThread,QString threadSuffixe,int modeDirFile,QString wavPath,QStringList wavFileList,QStringList wavRepList,int timeExpansion,bool withTimeCsv,int parVer,bool iDebug,bool withSox,bool mustCompress,int modeFreq,bool pol): QThread((QObject *)pdl)
{
    // detec class : deteclaunch treatment launches detec objects as threads which share sound files to treat
    // initialization of variables :
    PMainWindow = pdl;
    IThread = iThread;
    _threadSuffixe = threadSuffixe;
    _modeDirFile = modeDirFile;
    _wavPath = wavPath;
    _wavFileList = wavFileList;
    _wavRepList = wavRepList;
    _timeExpansion = timeExpansion;
    _withTimeCsv = withTimeCsv;
    _paramVersion = parVer;
    IDebug = iDebug;
    // this variable means case of live recordings with sox
    _withSox = withSox;
    MustCompress = mustCompress;
    _modeFreq =  modeFreq;
    Pol = pol;
    ReprocessingMode = false;
    if(_withSox) _txtPath = QDir::currentPath() + "/txt";
    else _txtPath = _wavPath + "/txt";
    _resultSuffix = QString("ta");
    _detectionThreshold = 26;
    _stopThreshold = 20;
    _freqMin = 0;
    _nbo = 4;
    _useValflag = true;
    _jumpThreshold = 30;
    _widthBigControl = 60;
    _widthLittleControl = 5;
    _highThreshold = 10;
    _lowThreshold = 8;
    _highThreshold2 = 0;
    _lowThreshold2 = 10;
    _qR = 5;
    _qN = 5;
    _freqCallMin=8.0f;
    //  call of initializeDetec : initialization of log files
    ImageData = true;
    DeepMode = true;
    initializeDetec();
    // creation of an object of the detectreatment class dedicated to the processing of sound files
    _detecTreatment = new DetecTreatment(this);
    LogStream << "LINWIN =  " << LINWIN << endl;
    LogStream << "VERQT =  " << VERQT << endl;
    LogStream << "_timeExpansion = " << _timeExpansion << endl;
    bool desactiveCorrectNoise = false;
    // assignment of settings for processing files
    CoefSpe = 1.09d;
    _detecTreatment->SetGlobalParameters(modeFreq,_timeExpansion,_timeExpansion,_detectionThreshold,_stopThreshold,
                                         _freqMin,_nbo,
                                         _useValflag,_jumpThreshold,_widthBigControl,_widthLittleControl,
                                         _highThreshold,_lowThreshold,_highThreshold2,_lowThreshold2,_qR,_qN,_paramVersion,desactiveCorrectNoise,
                                         CoefSpe);
    if(_modeDirFile==DIRECTORYMODE) _detecTreatment->SetDirParameters(_wavPath,_txtPath,ImageData,"","");
    LogStream << "_wathPath="   << _wavPath << "     _txtPath="   << _txtPath << endl;
    _detecTreatment->InitializeDetecTreatment();
}

Detec::~Detec()
{
    delete _detecTreatment;
}

bool Detec::initializeDetec()
{
    // initialization of log files
    QString logDirPath = QDir::currentPath()+"/log";
    QString logFilePath(logDirPath + QString("/detec")+_threadSuffixe+".log");
    _logFile.setFileName(logFilePath);
    _logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    LogStream.setDevice(&_logFile);
    LogStream.setRealNumberNotation(QTextStream::FixedNotation);
    LogStream.setRealNumberPrecision(22);

    LogStream << "Lancement Detec" <<_threadSuffixe << " : " << QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;
    QString errorFilePath(logDirPath + QString("/error")+_threadSuffixe+".log");
    _errorFile.setFileName(errorFilePath);
    if(_errorFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        ErrorStream.setDevice(&_errorFile);
        ErrorFileOpen = true;
    }
    else ErrorFileOpen = false;
    //
    TimeFileOpen = false;
    if(_withTimeCsv)
    {
        QString timePath(logDirPath + QString("/time")+_threadSuffixe+".csv");
        _timeFile.setFileName(timePath);
        if(_timeFile.open(QIODevice::WriteOnly | QIODevice::Text)==true)
        {
            TimeFileOpen = true;
            TimeStream.setDevice(&_timeFile);
            TimeStream.setRealNumberNotation(QTextStream::FixedNotation);
            TimeStream.setRealNumberPrecision(2);
            TimeStream << "filename" << '\t' << "computefft" << '\t' << "noisetreat" << '\t' << "shapesdetects" << '\t' << "parameters" << '\t'
                        << "save - end" << '\t' << "total time(ms)" << endl;
        }
    }
    LogStream << "Detec passe ici 1 " << endl;
    if(ImageData)
    {
        LogStream << "Detec passe ici 2 " << endl;
        /*
        _datPath = _wavPath+"/dat";
        QDir repdat(_datPath);
        if(!repdat.exists()) repdat.mkdir(_datPath);
        */
        LogStream << "Detec _wavPath = " << _wavPath << endl;
        _imagePath = _wavPath+"\\ima";
        LogStream << "Detec _imagePath = " << _imagePath << endl;
        QDir repima(_imagePath);
        if(!repima.exists())
        {
            LogStream << "creation de repima = " << _imagePath << endl;
            repima.mkdir(repima.absolutePath());
            if(!repima.exists()) LogStream << "repima = " << _imagePath << " n existe tj pas !" << endl;
        }
    }
    if(DeepMode)
    {
        LogStream << "Detec passe ici 3 " << endl;
        LogStream << "detec : deepmode = true" << endl;
        _ima2Path = _wavPath+"\\ima2";
        QDir repima2(_ima2Path);
        if(!repima2.exists())
        {
            LogStream << "creation de repima2 = " << _ima2Path << endl;
            repima2.mkdir(repima2.absolutePath());
            if(!repima2.exists()) LogStream << "repima2 = " << _ima2Path << " n existe tj pas !" << endl;
        }
    }

    return true;
}

void Detec::endDetec()
{
    if(ErrorFileOpen) _errorFile.close();
    if(TimeFileOpen) _timeFile.close();
    _detecTreatment->EndDetecTreatment();
    LogStream << "End of treatments by Detec object" << _threadSuffixe << " : " << QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;
    _logFile.close();
}

void Detec::run()
{
    // the run method manages the whole treatment by the thread
    QString dirPath,wavFile;
    if(_withSox) wavCut();
    for(int i=0;i<_wavFileList.size();i++)
    {
        if(_modeDirFile == DIRECTORYMODE) dirPath=_wavPath;
        else dirPath = _wavRepList.at(i);
        treatOneFile(_wavFileList.at(i),dirPath);
    }
    endDetec();
}

void Detec::treatOneFile(QString wavFile,QString dirPath)
{
    LogStream << "Begin:" << wavFile << " : " << " r:" << dirPath
                   << "   -   " << QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;
    QString pathFile = dirPath + '/' + wavFile;
    emit info1(wavFile,1);
    if(_modeDirFile == FILESMODE)
    {
        if(!createTxtDir(dirPath)) return;
        _detecTreatment->SetDirParameters(dirPath,_txtPath,ImageData,"","");
    }
    bool resu = _detecTreatment->CallTreatmentsForOneFile(wavFile,pathFile);
    //
    if(resu)
    {
        if(ImageData)
        {
            if(createImage(wavFile))
            {
                //saveDatFile(wavFile);
                if(DeepMode)
                {
                    int nbcris = (int)_detecTreatment->CallsArray.size();
                    for (int i = 0 ; i < nbcris ; i++)
                    {
                        createDeepImage(wavFile,i);
                    }
                }
            }
            else
            {
                LogStream << "Echec creation de image principale : pas d appel de cdi" << endl;
            }
        }
    }
    emit info1(wavFile,(int)resu);
    SLEEP(50);
    LogStream << "End:"<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;
}

bool Detec::createTxtDir(QString dirPath)
{
    // Creation of txt directory
    _txtPath = dirPath+"/txt";
    QDir reptxt(_txtPath);
    if(!reptxt.exists())
    {
        if(reptxt.mkdir(_txtPath))
        {
            LogStream << "created directory:"<< _txtPath << " !" << endl;
        }
        else
        {
            LogStream << "cannot create directory:"<< _txtPath << " !" << endl;
            return(false);
        }
    }
    return(true);
}

void Detec::wavCut()
{
    // method used in the case of live recordings
    // this method cuts the file of sounds recorded by sox
    // into files of duration of 5 seconds maximum
    QString adecouper = _wavFileList.at(0);
    QString grosFichierWav = _wavPath + "/" + adecouper;
    QString resuWav = _wavPath + "/r" + adecouper;
    QString program = "sox";
    QStringList  arguments;
    arguments << grosFichierWav << resuWav << "trim" << "0" << "5"  << ":" << "newfile" <<  "restart";
    LogStream << "decoupe par sox de fichierwav = " << grosFichierWav << " "  << QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << endl;
    QProcess p;
    p.execute(program,arguments);
    QDir sdir(_wavPath);
    if(!sdir.exists()) return;
    //
    /*
    try
    {
        QFile bigFile;
        bigFile.setFileName(grosFichierWav);
        bigFile.copy(QDir::currentPath()+"/log/"+adecouper);
    }
    catch(QException e)
    {

    }
    */
    sdir.remove(adecouper);
    _wavFileList = sdir.entryList(QStringList("*.wav"), QDir::Files);
}

bool Detec::createImage(QString wavFile)
{
    LogStream << "dci debut " << endl;
    if(_detecTreatment->SonogramWidth > 32767) {_xHalf = true; _imaWidth=(_detecTreatment->SonogramWidth+1)/2;}
    else  {_xHalf = false; _imaWidth=_detecTreatment->SonogramWidth;}
    int lyi = qMin(_detecTreatment->FftHeightHalf,_detecTreatment->LimY);

    QImage ima = QImage(_imaWidth, lyi,QImage::Format_RGB32);
    LogStream << "dci lyi = " << lyi << endl;
    LogStream << "dci _imaWidth = " << _imaWidth << endl;
    initBvrvb(_detecTreatment->EnergyMin,_detecTreatment->EnergyMax);
    int imax=((int)(_detecTreatment->EnergyMax-_detecTreatment->EnergyMin)*5)+1;
    uint crgb;
    for(int i=0;i<imax;i++)
    {
        _tvaleur[i]=calculateRGB((double)i/5.0f);
    }
    qint16 *ydl;
    int exceptions = 0;
    int blanc = qRgb(250,250,250);
    int noir = qRgb(5,5,5);
    int grisclair = qRgb(230,230,230);
    int grisfonce = qRgb(25,25,25);
    LogStream << "lyi=" << lyi << endl;
    for(int y = 0; y < lyi ; y++)
    {
        if(y<6 && _detecTreatment->WithSilence)
        {
            int xr = 0;
            for (int x = 0 ; x < _imaWidth ; x++)
            {
                crgb=0;
                if(y<3)
                {
                    if(_detecTreatment->FlagGoodColInitial[xr]==true) crgb = grisclair; else crgb = grisfonce;
                 }
                else
                {
                    if(_detecTreatment->FlagGoodCol[xr]==true) crgb = blanc; else crgb = noir;
                }
                ima.setPixel(x, lyi-y-1,crgb);
                xr+=1+_xHalf;
            }
        }
        else
        {
            ydl=_detecTreatment->SonogramArray[y];
            int digitPos = 0;
            char *pBoolChar = _detecTreatment->PointFlagsArray[y];;
            char boolChar = *pBoolChar;
            for (int x = 0 ; x < _imaWidth ; x++)
            {
                int valeur=(int)(((float)(*ydl)/20.0f)  -  _detecTreatment->EnergyMin*5.0f);
                if(valeur>=0 && valeur<imax)crgb=_tvaleur[valeur];
                else {crgb=0; exceptions++;}
                //if((boolChar & (1 << digitPos))!=0) crgb |= 224 << 16;
                ima.setPixel(x, lyi-y-1,crgb);
                for(int k=0;k<1+_xHalf;k++)
                {
                    ydl++;
                    digitPos++;
                    if(digitPos==8) {pBoolChar++; boolChar = *pBoolChar; digitPos=0;}
                }
            }
        }
    }
    //_imageFullName = _imagePath + '/' + wavFile.replace(QString(".wav"), QString(".jpg"), Qt::CaseInsensitive);
    _imageFullName = _imagePath + '/' + wavFile.replace(QString(".wav"), QString(".png"), Qt::CaseInsensitive);

    LogStream << "dci _imageFullName = " << _imageFullName << endl;

    LogStream << "dci avant enreg image " << endl;

    //bool retsave = ima.save(_imageFullName,JPG,100); // save image
    //bool retsave = ima.save(_imageFullName,0,100); // save image
    bool retsave = ima.save(_imageFullName); // save image

    if(retsave==true)
    {
        LogStream << "enreg image correct " << endl;
    }
    else
    {
        LogStream << "echec enreg image - retour = " << retsave << endl;
    }


    return(retsave);
    //return(false);
}



bool Detec::createDeepImage(QString wavFile,int callNumber)
{
    bool xHalf;
    int imaWidth;
    LogStream << "dcdi debut - callnumber=" << callNumber << endl;
    if(_detecTreatment->SonogramWidth > 32767) {xHalf = true; imaWidth=(_detecTreatment->SonogramWidth+1)/2;}
    else  {xHalf = false; imaWidth=_detecTreatment->SonogramWidth;}
    _imageHeight = qMin(_detecTreatment->FftHeightHalf,_detecTreatment->LimY);

    int refH = PMainWindow->_modeHeight;
    int refW = 1000;
    if(refH == 256) refW = 500;

    int lyi = _imageHeight;
    // if(lyi != 128)
    if(lyi != refH)
    {
        LogStream << "hauteur image : lyi = " << lyi << " # " << refH << " : non gere pour l'instant !" << endl;
        return(false);
    }
    //
    QPoint p =_detecTreatment-> MasterPoints[callNumber];
    int xmp = p.x(),ymp = p.y();
    float eCall = (float)(_detecTreatment->SonogramArray[ymp][xmp]/100.0f);
    int MpIn = (int)(eCall*100.0d);
    int MpHz = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][FreqMP]*1000.0d));
    int SMs = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][StTime]));
    int SDur = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][Dur]));
    int SFmin = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][Fmin]*1000.0d));
    int SFmax = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][Fmax]*1000.0d));
    int MpPos = (int)((double)(_detecTreatment->_paramsArray[callNumber][0][PosMP]*1000.0d));
    if(SDur < 9)
    {
        LogStream << "duree trop courte - image ima2 non enregistree !" << endl;
        return(false);
    }
    /*
    if(MpHz < 500 || MpHz > 10000)
    {
        LogStream << "frequence du point maitre hors limites ! MpHz=" << MpHz << endl;
        return(false);
    }
    */
    int Hzmax = 10000;
    if(Pol) Hzmax = 20000;

    if(MpHz < 100 || MpHz > Hzmax)
    {
        LogStream << "frequence du point maitre hors limites ! MpHz=" << MpHz << endl;
        return(false);
    }

    //
    QImage ima = QImage(imaWidth, lyi,QImage::Format_RGB32);
    LogStream << "dcdi ima width = " << imaWidth << "  lyi = " << lyi << endl;

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // chargement de l'image precedemment cr??e pour ne pas recr?er tout et parce que dans le fichier .dat
    // on n'a pas toute l'information de SonogramArray mais seulement celle des cris
    ima.load(_imageFullName); //
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // mise en valeur du cri
    int ycrimax = 0;
    int ycrimin = lyi;
    int xmin=6128,xmax=0;


    QVector<QPoint> unemat = _detecTreatment->CallsArray[callNumber];

    for(int j=0;j<unemat.size();j++)
    {
        int xr = unemat.at(j).x();
        int yr = unemat.at(j).y();
        if(yr > ycrimax) ycrimax = yr;
        if(yr < ycrimin) ycrimin = yr;
        if(xr > xmax) xmax = xr;
        if(xr < xmin) xmin = xr;

        //int ener = EnergyMatrix[callNumber][j];
        QRgb rgb = ima.pixel(xr,lyi-yr-1);
        int newRed = qRed(rgb);
        newRed = (int)((qRed(rgb) + 255)/2);
        //int newGreen = (int)((qGreen(rgb) + 255)/2);
        // int newGreen = (int)(((lyi-yr)*255)/lyi);
        int newGreen = (int)(((lyi-yr)*255)/lyi);
        //int newBlue = (int)( (qBlue(rgb) + 255) / 2 );
        //int newBlue = (int) qBlue(rgb);
        int newBlue = (int) ((qBlue(rgb)*3+255)/4);

        ima.setPixel(xr,lyi-yr-1,(uint)(qRgb(newRed,newGreen,newBlue)));
    }
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // mise en valeur du cri
    // mise en valeur du point maitre et des cretes
//
    LogStream << "dcdi milieu - callnumber = " << callNumber << endl;

    //----
    // cretes :

    for(int jcrete=0;jcrete<3;jcrete++)
    {
        LogStream << "dcdi boucle jcrete = " << jcrete << endl;
        QVector<QPoint> crestMat;
        if(jcrete==0) crestMat = _detecTreatment->CallMasterRidgeArray[callNumber];
        if(jcrete==1) crestMat = _detecTreatment->CallSouthArray[callNumber];
        if(jcrete==2) crestMat = _detecTreatment->CallNorthRidgeArray[callNumber];
        int ng = 80;
        if(jcrete==0) ng = 32;
        LogStream << "dcdi boucle crestMat.size = " << crestMat.size() << endl;
        for(int j=0;j<crestMat.size();j++)
        {
            int x=(int)(crestMat[j].x()/(1+xHalf));
            int y=(int)(lyi-crestMat[j].y()-1);
            if(jcrete==1 && y>0) y++;
            if(jcrete==2 && y<lyi-1) y--;
            //if(jcrete!=3) x+=0.5f; else x+=0.05f;
            //if(jcrete==1) y+=1;
            //if(jcrete==2 && crestMat.size()==674) LogStream << "    j = " << j << "  x = " << x << " y = " << y << endl;
            if(x>=0 && x<_imaWidth && y >=0 && y <lyi)
            ima.setPixel(x,y,(uint)(qRgb(ng,ng,ng)));
        }
        //
    } // next jcrete
    LogStream << "dcdi suite 1 - callnumber = " << callNumber << endl;
    //---
    // point maitre
    if(xmp > 0 && xmp < imaWidth-1 && ymp > 0 && ymp < lyi-1)
    {
        ima.setPixel(xmp-1,lyi-ymp-2,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp,lyi-ymp-2,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp+1,lyi-ymp-2,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp-1,lyi-ymp-1,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp+1,lyi-ymp-1,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp-1,lyi-ymp,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp,lyi-ymp,(uint)(qRgb(0,0,0)));
        ima.setPixel(xmp+1,lyi-ymp,(uint)(qRgb(0,0,0)));
    }
    ima.setPixel(xmp,lyi-ymp-1,(uint)(qRgb(255,255,255)));
    LogStream << "dcdi suite 2 - callnumber = " << callNumber << endl;
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // modification des autres cris
    for(int i=0;i<_detecTreatment->_callsNumber;i++)
    {
        if(i!= callNumber)
        {
            int ympc = _detecTreatment->MasterPoints[i].y();
            int xmpc = _detecTreatment->MasterPoints[i].x();
            bool afavoriser = false;
            if(ympc > ycrimin && ympc < ycrimax) afavoriser = true;
            QVector<QPoint> unemat = _detecTreatment->CallsArray[i];
            for(int j=0;j<unemat.size();j++)
            {
                int xr = unemat.at(j).x();
                int yr = unemat.at(j).y();
                //int ener = EnergyMatrix[callNumber][j];
                QRgb rgb = ima.pixel(xr,lyi-yr-1);
                int newRed = (int)(qRed(rgb)/5);
                if(afavoriser) newRed = (int)(qRed(rgb)/2);
                //int newGreen = (int)((qGreen(rgb) + 255)/2);
                int newGreen = (int)(((lyi-yr)*255)/(lyi*5));
                if(afavoriser) newGreen = (int)(((lyi-yr)*255)/(lyi*2));
                //int newBlue = (int)((qBlue(rgb) + 255)/2);
                int newBlue = (int) ( (qBlue(rgb)*2) / 3);
                if(afavoriser) newBlue = (int) ((qBlue(rgb)+255)/2);
                ima.setPixel(xr,lyi-yr-1,(uint)(qRgb(newRed,newGreen,newBlue)));
            }
            // ajouter aussi petite coloration du masterpoint
            ima.setPixel(xmpc,lyi-ympc-1,(uint)(qRgb(32,32,32)));

        }
    }
    LogStream << "dcdi suite 3 - callnumber = " << callNumber << endl;
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // int dlarp = 500;
    int dlarp = refW/2;
    int xcd = xmp - dlarp;
    if(xcd < 0) xcd = 0;
    int xcf = xmp + dlarp - 1;
    if(xcf > imaWidth - 1) xcf = imaWidth - 1;
    //
    QString EndName = QString("--") + QString::number(callNumber)
            + QString("--") + QString::number(MpIn)
            + QString("--") + QString::number(SDur)
            + QString("--") + QString::number(MpHz)
            + QString("--") + QString::number(SMs)
            + QString("--") + QString::number(SFmin)
            + QString("--") + QString::number(SFmax)
            + QString("--") + QString::number(MpPos)
            + ".png";
            //+ ".jpg";
    QString wavRadic = wavFile.replace(".wav","");
    QString callIma2Name = _ima2Path + "/" + wavRadic + EndName;
    LogStream << "dcdi suite 4 - callnumber = " << callNumber << endl;

    if(xmp > imaWidth-5 || xmp <5)
    {
        LogStream << callIma2Name << " : point maitre trop pres des bords" << endl;
        return(false);
    }
    LogStream << "dcdi proche fin" << callNumber << endl;

    QImage imac = ima.copy(xcd,0,xcf-xcd+1,lyi);

    // calcul de endname : a gerer dans une autre methode  :
    LogStream << "callIma2Name=" << callIma2Name << endl;
    imac.save(callIma2Name,0,100); // save image
    return(true);
}

void Detec::initBvrvb(double bornemin,double bornemax)
{
    _bRGB[0][0]=bornemin;
    _bRGB[1][0]=bornemin/2;
    _bRGB[2][0]=(double)_detecTreatment->EnergyStopThreshold;
    _bRGB[3][0]=(double)_detecTreatment->EnergyShapeThreshold;
    _bRGB[4][0]=bornemax+1;
    for(int i=0;i<5;i++) _bRGB[i][0]-=bornemin;
    _bRGB[0][1]=0;   _bRGB[0][2]=0;   _bRGB[0][3]=0;
    _bRGB[0][4]=0;   _bRGB[0][5]=0;   _bRGB[0][6]=32;

    _bRGB[1][1]=0;   _bRGB[1][2]=0;  _bRGB[1][3]=32;
    _bRGB[1][4]=0;   _bRGB[1][5]=0;  _bRGB[1][6]=64;

    _bRGB[2][1]=48;   _bRGB[2][2]=0; _bRGB[2][3]=80;
    _bRGB[2][4]=140;   _bRGB[2][5]=0; _bRGB[2][6]=92;

    _bRGB[3][1]=160;  _bRGB[3][2]=0;  _bRGB[3][3]=96;
    _bRGB[3][4]=255;  _bRGB[3][2]=0;  _bRGB[3][3]=192;
}

uint Detec::calculateRGB(double value)
{
    double rgb[3];
    for(int j=0;j<3;j++) rgb[j] =0.0f;
    for(int i=0;i<4;i++)
    {
        if(value>=_bRGB[i][0] && value<_bRGB[i+1][0])
        {
            for(int j=0;j<3;j++)
                rgb[j]=_bRGB[i][j+1]
                        +((_bRGB[i][j+4]-_bRGB[i][j+1])*(value-_bRGB[i][0]))/(_bRGB[i+1][0]-_bRGB[i][0]);
            break;
        }
    }
    uint urgb = qRgb(qRound(rgb[0]),qRound(rgb[1]),qRound(rgb[2]));
    return(urgb);
}
