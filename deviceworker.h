#ifndef DEVICEWORKER_H
#define DEVICEWORKER_H

#include <QObject>
#include <QJsonObject>
#include "LWDMApi.h"


class DeviceWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeviceWorker(QObject *parent = nullptr);
    ~DeviceWorker();

    void threadFunction();
    void loadJsonFile(const char *, QJsonObject &);
    void networkMonitoring();


private:
    bool loadProfile();


signals:
    void executionError(QString error);
    void deviceList(QStringList);
    void deviceInfo(QString, QString, QString, QString, QString, LWSensorIntrinsicParam);
    void connectStatus(bool, QString);
    void runStatus(bool);
    void calibrationStep(int);
    void temperature(LWTemperature);
    void roiConfigUpdate();

    void logText(QString);
    void replyText(QString);
    void depData(LWFrameData &, double);
    void graData(LWFrameData &, double);
    void potData(LWFrameData &, LWFrameData &, double);
    void soundLightAlarmEnable(bool );


public slots:
    void onReadyMe();
    void onReboot();
    void onSynchronizeTime();
    void onCancelCalibration();
    void onSetCalibrationARG();
    void onSetPerspectiveMatrix();
    void onPullLog(QString);
    void onSetNetwork(bool, QString, QString);
    void onSetExposure(int );
    void onSetExposureLevel(int );
    void onSetFrameRate(int );
    void onSetIRGammaValue(int );
    void onSetSpatialFilter(bool, int);
    void onSetTimeFilter(bool, int);
    void onSetTimeFilter_(bool, int);
    void onSetFlyPixeFilter(bool, int);
    void onSetConfidenceFilter(bool, int);
    void onSetSoundLightAlarm(bool );

    void onInitJsonFile();
    void onUpgradeJsonFile();
    void onExportJsonFile(QString);
    void onImportJsonFile(QStringList);
    void onSetRotationAxis(int, float);
    void onSetSecurityConfigInfo();
    void onSetSecurity3DInfo();

    void onSaveConfigureInfo();
    void onDeleteConfigureInfo();
    void onResetConfigureInfo();

    void onStart();
    void onStop();
    void onStopStream();
    void onFindDevice();
    void onConnectionEnable(bool , int = 0);


public:
    bool                        mRun;


private:
    bool                        mIsConnect;
    bool                        mIsRGB = false;
    int                         mCount = 0;
    double                      mFps = 0;
    int64_t                     mT = 0;
    std::string                 mSN;

    QJsonObject                 mJson_PerspMatrix;
    QJsonObject                 mJson_CalibMatrix;
    QJsonObject                 mJson_Region;
    QJsonObject                 mJson_PRegion;

    LWFrameData                 mDepData;
    LWFrameData                 mGraData;
    LWFrameData                 mPotData;
    LWDeviceHandle              mHandle;
    LWDeviceHandle              mHandleList[10];

};

#endif // DEVICEWORKER_H
