#include "settings.h"
// #include <QStandardPaths>
#include <QRegularExpression>


Settings::Settings()
{
    // QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    devType = "dm";
    projName = "base";
    password = "dm-zgyfjch";

    isDHCP              = file.value("isDHCP", false).toBool();
    isDistCali          = file.value("isDistortionCalibration", true).toBool();
    isSpatialFilter     = file.value("isSpatialFilter", false).toBool();
    isTimeFilter        = file.value("isTimeFilter", false).toBool();
    isTimeFilter_       = file.value("isTimeFilter_", false).toBool();
    isFlyPixeFilter     = file.value("isFlyPixeFilter", false).toBool();
    isConfFilter        = file.value("isConfidenceFilter", false).toBool();

    isAutoReboot        = file.value("isAutoReboot", true).toBool();
    isTextureMapEnable  = file.value("isTextureMapEnable", true).toBool();

    timeFilterValue     = file.value("timeFilterValue", 3).toInt();
    timeFilterValue_    = file.value("timeFilterValue_", 3).toInt();
    spatialFilterValue  = file.value("spatialFilterValue", 5).toInt();
    flyPixeFilterValue  = file.value("flyPixeFilterValue", 5).toInt();
    confFilterValue     = file.value("confidenceFilterValue", 5).toInt();

    triggerMode     = file.value("triggerMode", 0).toInt();
    hdrMode         = file.value("hdrMode", 0).toInt();
    irValue         = file.value("irValue", 200).toInt();
    frameRate       = file.value("frameRate", 10).toInt();
    // exposure        = file.value("exposure", 1000).toInt();
    drawModel       = file.value("drawModel", 0).toInt();
    pointSize       = file.value("pointSize", 3).toInt();

    up[0] = file.value("up0", 0.0).toDouble();
    up[1] = file.value("up1", 0.0).toDouble();
    up[2] = file.value("up2", -1.0).toDouble();
    ps[0] = file.value("ps0", 0.0).toDouble();
    ps[1] = file.value("ps1", 22000.0).toDouble();
    ps[2] = file.value("ps2", 3000.0).toDouble();
    fp[0] = file.value("fp0", 0.0).toDouble();
    fp[1] = file.value("fp1", 0.0).toDouble();
    fp[2] = file.value("fp2", 3000.0).toDouble();
    dr[0] = file.value("dr0", 17000.0).toDouble();
    dr[1] = file.value("dr1", 35000.0).toDouble();
    overlapOpacity      = file.value("overlapOpacity", 0.3).toDouble();
    nonOverlapOpacity   = file.value("nonOverlapOpacity", 0.05).toDouble();
    pyramidOpacity      = file.value("pyramidOpacity", 0.2).toDouble();

    devIP       = file.value("devIP", "192.168.1.200").toString();
    devIPMask   = file.value("devIPMask", "255.255.255.0").toString();
    exportPath  = file.value("exportPath", "").toString();
    importPath  = file.value("importPath", "").toString();
    upgradePath = file.value("upgradePath", "").toString();
    logPath     = file.value("logPath", "").toString();

    auto colorStrings = file.value("colorStrings", (QStringList()<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00"<<"#00aa00")).toStringList();
    for (int i = 0; i < 9; ++i) colors[i] = QColor(colorStrings[i]);

    auto colorStr = file.value("backgroundColor", "#004d71").toString();
    backgroundColor = QColor(colorStr);

}

Settings::~Settings()
{
    file.setValue("isDHCP", isDHCP);

    file.setValue("isDistortionCalibration", isDistCali);
    file.setValue("isSpatialFilter", isSpatialFilter);
    file.setValue("isTimeFilter", isTimeFilter);
    file.setValue("isTimeFilter_", isTimeFilter_);
    file.setValue("isFlyPixeFilter", isFlyPixeFilter);
    file.setValue("isConfidenceFilter", isConfFilter);
    file.setValue("isAutoReboot", isAutoReboot);
    file.setValue("isTextureMapEnable", isTextureMapEnable);

    file.setValue("timeFilterValue", timeFilterValue);
    file.setValue("timeFilterValue_", timeFilterValue_);
    file.setValue("spatialFilterValue", spatialFilterValue);
    file.setValue("flyPixeFilterValue", flyPixeFilterValue);
    file.setValue("confidenceFilterValue", confFilterValue);

    file.setValue("triggerMode", triggerMode);
    file.setValue("hdrMode", hdrMode);
    file.setValue("irValue", irValue);
    file.setValue("frameRate", frameRate);
    // file.setValue("exposure", exposure);
    file.setValue("drawModel", drawModel);
    file.setValue("pointSize", pointSize);

    file.setValue("up0", up[0]);
    file.setValue("up1", up[1]);
    file.setValue("up2", up[2]);
    file.setValue("ps0", ps[0]);
    file.setValue("ps1", ps[1]);
    file.setValue("ps2", ps[2]);
    file.setValue("fp0", fp[0]);
    file.setValue("fp1", fp[1]);
    file.setValue("fp2", fp[2]);
    file.setValue("dr0", dr[0]);
    file.setValue("dr1", dr[1]);
    file.setValue("overlapOpacity", overlapOpacity);
    file.setValue("nonOverlapOpacity", nonOverlapOpacity);
    file.setValue("pyramidOpacity", pyramidOpacity);

    file.setValue("devIP", devIP);
    file.setValue("devIPMask", devIPMask);
    file.setValue("exportPath", exportPath);
    file.setValue("importPath", importPath);
    file.setValue("upgradePath", upgradePath);
    file.setValue("logPath", logPath);

    QStringList colorStrings;
    for (int i = 0; i < 9; ++i) colorStrings.append(colors[i].name(QColor::HexArgb));
    file.setValue("colorStrings", colorStrings);
    file.setValue("backgroundColor", backgroundColor.name(QColor::HexArgb));

    file.sync();
}

Settings &Settings::GetInstance()
{
    static Settings instance;

    return instance;
}

SecurityArg     ARG;
Settings&       config = Settings::GetInstance();
