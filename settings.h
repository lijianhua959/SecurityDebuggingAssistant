#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QJsonObject>
#include <QColor>
// #include <QPointF>


///< 透视变换后的IR像素数
#define IR_PIX_ROWS 1200
#define IR_PIX_COLS 1600
#define IR_PIX_NUMBER IR_PIX_ROWS*IR_PIX_COLS
///< The maximum value of the z-coordinate of the 3D point cloud.
#define PCD_MAX_VALUE 60000
///< The vertical pixel count of the TOF sensor.
#define TOF_PIX_ROWS 480/2
///< The horizontal pixel count of the TOF sensor.
#define TOF_PIX_COLS 640/2
///< The total number of pixels of the TOF sensor.
#define TOF_PIX_NUMBER  TOF_PIX_ROWS*TOF_PIX_COLS
///< The vertical pixel count of the TOF sensor.
#define TOF_MAX_PIX_ROWS 480
///< The horizontal pixel count of the TOF sensor.
#define TOF_MAX_PIX_COLS 640
///< The total number of pixels of the TOF sensor.
#define TOF_MAX_PIX_NUMBER  TOF_MAX_PIX_ROWS*TOF_MAX_PIX_COLS


struct ROIARG
{
    // int     is2DAreaChange;
    int     enable;
    int     attribute;

    struct
    {
        int d1;
        int d2;

    } depthRange;

    struct
    {
        int low;
        int high;

    } trsdSensitivityRate;

    // QPointF points[8];
    QColor  color;
    int     srcArea[8]; // IR图上绘制的区域
    float   dstArea[8]; // 3D图上绘制的区域
};

struct Group
{
    ROIARG ROI[8];
};

struct SecurityArg
{
    int         srcCalibrationParams[8];
    int         dstCalibrationParams[8];
    int         OutputFrameThreashold;
    int         Enable_Group_Number;
    int         MajorVersion;
    int         isCalibrated;
    float       Calibration_Matrix[9];
    double      Perspective_Matrix[9];

    Group       Group[8];

    QString     ProjectName;
    QString     JsonName;
};

class Settings
{

private:
    Settings();


public:
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    static Settings& GetInstance();


public:
    bool        isDHCP;
    bool        isDistCali;
    bool        isSpatialFilter;
    bool        isTimeFilter;
    bool        isTimeFilter_;
    bool        isFlyPixeFilter;
    bool        isConfFilter;

    bool        isSCalib;
    bool        isAutoReboot;
    bool        isTextureMapEnable;

    int         spatialFilterValue;
    int         timeFilterValue;
    int         timeFilterValue_;
    int         flyPixeFilterValue;
    int         confFilterValue;

    int         triggerMode;
    int         hdrMode;
    int         irValue;
    int         frameRate;
    int         exposure[5];
    int         drawModel;
    int         pointSize;
    int         fvMajor;

    double      up[3];
    double      ps[3];
    double      fp[3];
    double      dr[2];
    double      overlapOpacity;
    double      nonOverlapOpacity;
    double      pyramidOpacity;

    QString     projName;
    QString     projName_zh;

    QString     devIP;
    QString     devType;
    QString     devIPMask;
    QString     exportPath;
    QString     importPath;
    QString     upgradePath;
    QString     logPath;
    QString     password;

    QColor      colors[9];
    QColor      backgroundColor;


    QString getProjectName() const;
    void setProjectName(const QString &newProjectName);

private:
    QSettings       file{"config.ini", QSettings::IniFormat};

};

extern  SecurityArg     ARG;
extern  Settings        &config;

#endif // SETTINGS_H
