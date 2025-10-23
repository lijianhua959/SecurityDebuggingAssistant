#include "deviceworker.h"

#include "settings.h"
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <thread>
#include <Windows.h>


void networkMonitoringCallback(LWDeviceHandle handle, const char* error, void* pUserData)
{
    auto obj = (DeviceWorker*)pUserData;

    obj->networkMonitoring();
}

DeviceWorker::DeviceWorker(QObject *parent)
    : QObject{parent}
{
    mRun = false;

    qRegisterMetaType<LWFrameData>("LWFrameData&");

    LWInitializeResources();
    LWRegisterNetworkMonitoringCallback(networkMonitoringCallback, this);
}

DeviceWorker::~DeviceWorker()
{
    mRun = false;
    if(config.isAutoReboot) LWRebootDevice(mHandle);
    LWCleanupResources();
}

void DeviceWorker::threadFunction()
{
    int64_t t_sec = 0;

    mT      = 0;
    mRun    = true;
    mIsConnect = true;
    while (mRun)
    {
        if(LWGetFrameReady(mHandle) != LW_RETURN_OK) continue;

        LWGetFrame(mHandle, &mDepData, LWFrameType::LW_DEPTH_FRAME);
        LWGetFrame(mHandle, &mGraData, LWFrameType::LW_IR_FRAME);
        LWGetFrame(mHandle, &mPotData, LWFrameType::LW_POINTCLOUD_FRAME);

        t_sec = mDepData.timestamp.tv_sec;
        if(mT != t_sec)
        {
            mFps = (double)mCount / (t_sec - mT);
            mT = t_sec;
            mCount = 0;
        }
        ++mCount;

        if(mIsConnect)
        {
            emit depData(mDepData, mFps);
            emit graData(mGraData, mFps);
            emit potData(mPotData, mGraData, mFps);

            mIsConnect = false;
        }
    }

    emit runStatus(false);
}

void DeviceWorker::loadJsonFile(const char* text, QJsonObject &json)
{
    QJsonParseError jError;
    auto Jdoc = QJsonDocument::fromJson(text, &jError);
    if(jError.error != QJsonParseError::NoError)
    {
        emit executionError(tr("配置文件加载失败（%1）！！！").arg(jError.errorString()));
        return;
    }
    if(!Jdoc.isObject())
    {
        emit executionError(tr("配置文件内容非json格式！！！"));
        return;
    }
    json = Jdoc.object();

    ARG.ProjectName.clear();
    if (const QJsonValue v = json["ProjectName"]; v.isString())
        ARG.ProjectName = v.toString();

    ARG.JsonName.clear();
    if (const QJsonValue v = json["JsonName"]; v.isString())
        ARG.JsonName = v.toString();

    ARG.MajorVersion = 0;
    if (const QJsonValue v = json["Version"]; v.isArray())
        ARG.MajorVersion = v.toArray().first().toInt();

    // 项目判断
    if(ARG.ProjectName != config.projName)
    {
        auto str = tr("\"%1\"配置文件非\"%2\"版！如果继续使用可能会出现未知BUG，请将其升级为\"%3\"版。")
                       .arg(ARG.JsonName, config.projName, config.projName);
        emit executionError(str);
    }
    // 版本判断
    if(ARG.MajorVersion < 3)
    {
        emit executionError(tr("\"%1\"配置文件低于3.0版本。").arg(ARG.JsonName));
        return;
    }

    // 数据内容解析
    bool    flag = false;
    int     step = 0;
    QString group, roi;
    if(ARG.JsonName == "perspective_region")
    {
        for (int i = 0; i < 8; ++i)
        {
            group = QString("Group%1").arg(i+1);

            if (const QJsonValue v = json[group]; v.isObject())
            {
                auto obj = v.toObject();

                for (int j = 0; j < 8; ++j)
                {
                    roi = QString("ROI%1").arg(j+1);
                    if (const QJsonValue v = obj[roi]; v.isObject())
                    {
                        auto obj = v.toObject();
                        if (const QJsonValue v = obj["rectangle"]; v.isArray())
                        {
                            auto arr = v.toArray();
                            if(arr.size() < 8) break;
                            ++step;
                            for (int k = 0; k < 8; k++)
                            {
                                ARG.Group[i].ROI[j].srcArea[k] = arr[k].toInt();
                            }
                        }
                    }
                }
            }
        }

        if(step == 64) flag = true;
    }
    else if(ARG.JsonName == "region")
    {
        if (const QJsonValue v = json["OutputFrameThreashold"]; v.isDouble())
            ARG.OutputFrameThreashold = v.toInt();
        if (const QJsonValue v = json["Enable_Group_Number"]; v.isDouble())
        {
            ARG.Enable_Group_Number = v.toInt();
            ++step;
        }
        for (int i = 0; i < 8; ++i)
        {
            group = QString("Group%1").arg(i+1);

            if (const QJsonValue v = json[group]; v.isObject())
            {
                auto obj = v.toObject();

                for (int j = 0; j < 8; ++j)
                {
                    roi = QString("ROI%1").arg(j+1);
                    if (const QJsonValue v = obj[roi]; v.isObject())
                    {
                        auto obj = v.toObject();
                        if (const QJsonValue v = obj["enable"]; v.isDouble())
                        {
                            ARG.Group[i].ROI[j].enable = v.toInt();
                            ++step;
                        }
                        if (const QJsonValue v = obj["attribute"]; v.isDouble())
                        {
                            ARG.Group[i].ROI[j].attribute = v.toInt();
                            ++step;
                        }
                        if (const QJsonValue v = obj["rectangle"]; v.isArray())
                        {
                            auto arr = v.toArray();
                            if(arr.size() < 8) break;
                            for (int k = 0; k < 8; k++)
                            {
                                ARG.Group[i].ROI[j].dstArea[k] = arr[k].toDouble();
                            }
                        }
                        if (const QJsonValue v = obj["depthRange"]; v.isObject())
                        {
                            auto obj = v.toObject();
                            if (const QJsonValue v = obj["d1"]; v.isDouble())
                            {
                                ARG.Group[i].ROI[j].depthRange.d1 = v.toInt();
                                ++step;
                            }
                            if (const QJsonValue v = obj["d2"]; v.isDouble())
                            {
                                ARG.Group[i].ROI[j].depthRange.d2 = v.toInt();
                                ++step;
                            }
                        }
                        if (const QJsonValue v = obj["trsdSensitivityRate"]; v.isObject())
                        {
                            auto obj = v.toObject();
                            if (const QJsonValue v = obj["low"]; v.isDouble())
                            {
                                ARG.Group[i].ROI[j].trsdSensitivityRate.low = v.toInt();
                                ++step;
                            }
                        }
                    }
                }
            }
        }

        if(step == 321) flag = true;
    }
    else if(ARG.JsonName == "perspective_mat")
    {
        if (const QJsonValue v = json["Perspective_Matrix"]; v.isArray())
        {
            const QJsonArray arr = v.toArray();
            if(arr.size() > 8)
            {
                for (int i = 0; i < 9; ++i)
                {
                    ARG.Perspective_Matrix[i] = arr[i].toDouble();
                }

                flag = true;
            }
        }
    }
    else if(ARG.JsonName == "calibrated")
    {
        if (const QJsonValue v = json["Calibrated"]; v.isDouble())
        {
            ARG.isCalibrated = v.toInt();
            ++step;
        }
        if (const QJsonValue v = json["Extrinsics_Calibrated_Tof2Plane"]; v.isArray())
        {
            const QJsonArray arr = v.toArray();
            if(arr.size() > 8)
            {
                ++step;
                for (int i = 0; i < 9; ++i)
                {
                    ARG.Calibration_Matrix[i] = arr[i].toDouble();
                }
            }
        }

        if(step == 2) flag = true;
    }
    else
    {
        emit executionError(tr("Json配置文件加载失败（未解析到相关文件格式）！！！"));
    }

    if(!flag)
    {
        mRun = false;

        emit connectStatus(false, tr("连接"));
        emit runStatus(false);

        return;
    }
}

bool DeviceWorker::loadProfile()
{
    char *text  = new char[65535*5];

    /// 获取安防区域绘制文件
    auto ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_PR_ARG, text, 65535*5);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        delete[] text;
        return false;
    }
    loadJsonFile(text, mJson_PRegion);

    /// 获取安防配置文件
    ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_ARG, text, 65535*5);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        delete[] text;
        return false;
    }
    loadJsonFile(text, mJson_Region);

    /// 获取透视矩阵
    ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_PR_MAT_ARG, text, 65535*5);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        delete[] text;
        return false;
    }
    loadJsonFile(text, mJson_PerspMatrix);

    /// 获取安防标定矩阵
    ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_CALIB, text, 65535*5);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        delete[] text;
        return false;
    }
    loadJsonFile(text, mJson_CalibMatrix);

    delete[] text;

    return true;
}

void DeviceWorker::networkMonitoring()
{
    mRun = false;

    emit connectStatus(false, tr("连接"));
    emit runStatus(false);
}

void DeviceWorker::onReadyMe()
{
    mIsConnect = true;
}

void DeviceWorker::onExportJsonFile(QString path)
{
    /// 获取安防区域绘制文件
    auto ret = LWGetSecurityConfigFile(mHandle, LWFileType::LW_SECURITY_PR_ARG, QString("%1/safety_perspective_region.json").arg(path).toLocal8Bit());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("safety_perspective_region 获取失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    /// 获取安防配置文件
    ret = LWGetSecurityConfigFile(mHandle, LWFileType::LW_SECURITY_ARG, QString("%1/safety_protection_region.json").arg(path).toLocal8Bit());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("safety_protection_region 获取失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    /// 获取透视矩阵
    ret = LWGetSecurityConfigFile(mHandle, LWFileType::LW_SECURITY_PR_MAT_ARG, QString("%1/safety_perspective_mat.json").arg(path).toLocal8Bit());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("safety_perspective_mat 获取失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    /// 获取安防标定矩阵
    ret = LWGetSecurityConfigFile(mHandle, LWFileType::LW_SECURITY_CALIB, QString("%1/safety_protection_calibrated.json").arg(path).toLocal8Bit());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("safety_protection_calibrated 获取失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("参数导出成功"));
}

void DeviceWorker::onImportJsonFile(QStringList names)
{
    int count = 0;
    for (const auto& name : names)
    {
        QFile file(name);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit executionError(name + tr("---文件读取失败！！！"));
            continue;
        }
        auto docData = file.readAll();
        file.close();

        QJsonParseError jError;
        auto Jdoc = QJsonDocument::fromJson(docData, &jError);
        if(jError.error != QJsonParseError::NoError)
        {
            emit executionError(tr("%1 配置文件加载失败（%2）！！！").arg(name, jError.errorString()));
            continue;
        }
        if(!Jdoc.isObject())
        {
            emit executionError(tr("%1 配置文件不是一个json对象！！！").arg(name));
            continue;
        }
        auto json = Jdoc.object();

        // 判断项目名
        if (const QJsonValue v = json["ProjectName"]; v.isString())
        {
            if(v.toString() != config.projName)
            {
                auto str = tr("%1 文件不属于本项目的配置文件！！！").arg(name);
                emit executionError(str);

                continue;
            }
        }
        // 判断版本号
        if (const QJsonValue v = json["Version"]; v.isArray())
        {
            if(v.toArray().first().toInt() != 3)
            {
                emit executionError(tr("%1 文件版本不匹配！！！").arg(name));
                continue;
            }
        }
        // 判断文件类型
        if (const QJsonValue v = json["JsonName"]; v.isString())
        {
            LWFileType type;
            if(v.toString() == QString("region"))
            {
                type = LWFileType::LW_SECURITY_ARG;
            }
            else if(v.toString() == QString("perspective_region"))
            {
                type = LWFileType::LW_SECURITY_PR_ARG;
            }
            else if(v.toString() == QString("perspective_mat"))
            {
                type = LWFileType::LW_SECURITY_PR_MAT_ARG;
            }
            else if(v.toString() == QString("calibrated"))
            {
                type = LWFileType::LW_SECURITY_CALIB;
            }
            else
            {
                emit executionError(tr("%1 文件类型不匹配！！！").arg(name));
                continue;
            }

            auto ret = LWSetSecurityConfigFileFromBuffer(mHandle, type, docData, docData.size());
            if(ret != LW_RETURN_OK)
            {
                emit executionError(tr("%1 文件下发失败: %2")
                                        .arg(name, QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret))));
                continue;
            }

            ++count;
        }
    }

    if(count == 0) return;

    emit replyText(tr("参数导入动作执行完成"));
    Sleep(200);
    if(!loadProfile())
    {
        emit executionError(tr("重新加载配置失败！"));
    }
    emit roiConfigUpdate();

}

void DeviceWorker::onReboot()
{
    mRun = false;

    emit connectStatus(false, tr("连接"));
    emit runStatus(false);

    auto ret = LWRebootDevice(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWRebootDevice 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("设备重启指令下发成功"));
}

void DeviceWorker::onSynchronizeTime()
{
    auto ret = LWSynchronizeDeviceSystemTime(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSynchronizeDeviceSystemTime 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("时间同步指令下发成功"));
}

void DeviceWorker::onCancelCalibration()
{
    auto ret = LWSecurityCancelCalibration(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityCancelCalibration 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isSCalib = false;
    emit calibrationStep(0);
    emit replyText(tr("取消标定指令下发成功"));
}

void DeviceWorker::onSetCalibrationARG()
{
    auto ret = LWSetSecurityCalibrationParams(mHandle, ARG.dstCalibrationParams, 8);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityCalibrationParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
    emit replyText(tr("标定参数下发成功"));
    emit calibrationStep(2);

    char *text  = new char[65535*5];
    ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_CALIB, text, 65535*5);
    if(ret == LW_RETURN_OK)
    {
        loadJsonFile(text, mJson_CalibMatrix);
        emit replyText(tr("获取标定矩阵参数成功"));
    }
    else
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    }
    delete[] text;
}

void DeviceWorker::onSetPerspectiveMatrix()
{
    if (const QJsonValue v = mJson_PerspMatrix["Perspective_Matrix"]; v.isArray())
    {
        QJsonArray arr = v.toArray();
        if(arr.size() > 8)
        {
            for (int i = 0; i < 9; ++i)
            {
                arr[i] = ARG.Perspective_Matrix[i];
            }
        }
        mJson_PerspMatrix["Perspective_Matrix"] = arr;
    }
    else
    {
        emit executionError(tr("无法加载透视矩阵文件，透视矩阵参数下发失败。"));
        return;
    }

    QJsonDocument Doc(mJson_PerspMatrix);
    auto text = Doc.toJson(QJsonDocument::Indented);
    auto ret = LWSetSecurityConfigFileFromBuffer(
        mHandle,
        LWFileType::LW_SECURITY_PR_MAT_ARG,
        text,
        text.size());

    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityConfigFileFromBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("透视矩阵参数下发成功"));
}

void DeviceWorker::onPullLog(QString fileName)
{
    /// 设置低帧率，用以减少带宽，更快的获取日志信息
    auto ret = LWSetFrameRate(mHandle, 1);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetFrameRate 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
    Sleep(1500);
    /// 获取日志信息
    ret = LWGetSecurityConfigFile(mHandle, LWFileType::LW_SECURITY_LOG, fileName.toLocal8Bit());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWGetSecurityLog 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    }
    else
    {
        emit replyText(tr("日志导出成功"));
    }
    /// 设回原来的帧率
    ret = LWSetFrameRate(mHandle, config.frameRate);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetFrameRate 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
}

void DeviceWorker::onSetNetwork(bool type, QString ip, QString netmask)
{
    LWNetworkInfo mode;
    memcpy(mode.ip, ip.toStdString().c_str(), netmask.size());
    memcpy(mode.netmask, netmask.toStdString().c_str(), netmask.size());
    mode.type = !type;

    auto ret = LWSetNetworkInfo(mHandle, mode);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetNetworkInfo 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    mRun = false;
    emit connectStatus(false, tr("连接"));
    emit runStatus(false);

    emit replyText(tr("网络配置下发成功"));
}

void DeviceWorker::onSetExposure(int val)
{
    auto ret = LWSetExposureTime(mHandle, LW_TOF_SENSOR, &val, 1);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetExposureTime 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.exposure[0] = val;
    emit replyText(tr("积分时间下发成功"));
}

void DeviceWorker::onSetExposureLevel(int val)
{
    val *= 40;
    auto ret = LWSetExposureTime(mHandle, LW_TOF_SENSOR, &val, 1);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetExposureTime 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.exposure[0] = val;
    emit replyText(tr("曝光等级设置成功"));
}

void DeviceWorker::onSetFrameRate(int val)
{
    auto ret = LWSetFrameRate(mHandle, val);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetFrameRate 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.frameRate = val;
    emit replyText(tr("帧率指令下发成功"));
}

void DeviceWorker::onSetIRGammaValue(int arg)
{
    auto ret = LWSetIRGMMGain(mHandle, arg);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetFrameRate 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.irValue = arg;
    emit replyText(tr("IR伽马值设置成功"));
}

void DeviceWorker::onSetSpatialFilter(bool enable, int val)
{
    auto ret = LWSetSpatialFilterParams(mHandle, LWFilterParam{enable, val});
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSpatialFilterParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isSpatialFilter = enable;
    config.spatialFilterValue = val;
    emit replyText(tr("空间滤波参数下发成功"));
}

void DeviceWorker::onSetTimeFilter(bool enable, int val)
{
    auto ret = LWSetTimeFilterParams(mHandle, LWFilterParam{enable, val, 100});
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetTimeFilterParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isTimeFilter = enable;
    config.timeFilterValue = val;
    emit replyText(tr("时间均值滤波参数下发成功"));
}

void DeviceWorker::onSetTimeFilter_(bool enable, int val)
{
    auto ret = LWSetTimeMedianFilterParams(mHandle, LWFilterParam{enable, val, 100});
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetTimeMedianFilterParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isTimeFilter_ = enable;
    config.timeFilterValue_ = val;
    emit replyText(tr("时间中值滤波参数下发成功"));
}

void DeviceWorker::onSetFlyPixeFilter(bool enable, int val)
{
    auto ret = LWSetFlyingPixelsFilterParams(mHandle, LWFilterParam{enable, val});
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetFlyingPixelsFilterParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isFlyPixeFilter = enable;
    config.flyPixeFilterValue = val;
    emit replyText(tr("飞点滤波参数下发成功"));
}

void DeviceWorker::onSetConfidenceFilter(bool enable, int val)
{
    auto ret = LWSetConfidenceFilterParams(mHandle, LWFilterParam{enable, val});
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetConfidenceFilterParams 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    config.isConfFilter = enable;
    config.confFilterValue = val;
    emit replyText(tr("置信度滤波参数下发成功"));
}

void DeviceWorker::onSetSoundLightAlarm(bool enable)
{
    auto ret = LWSetSecurityZHSAEnable(mHandle, enable);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityZHSAEnable 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit soundLightAlarmEnable(enable);
    emit replyText(tr("声光报警器使能参数下发成功"));
}

void DeviceWorker::onInitJsonFile()
{
    /// 清理安防配置文件
    auto ret = LWResetSecurityConfigure(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWResetSecurityConfigure 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
    emit replyText(tr("恢复出厂配置成功"));
    config.isSCalib = 1;
    emit calibrationStep(1);

    Sleep(200);
    /// 获取安防最新得配置文件
    if(!loadProfile())
    {
        emit executionError(tr("重新加载配置失败！"));
    }
    emit roiConfigUpdate();
}

void DeviceWorker::onUpgradeJsonFile()
{
    // std::string group, roi;
    // QFile file("safety_protection_region.json");
    // if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    // {
    //     emit executionError("safety_protection_region.json---文件读取失败！！！");
    //     return;
    // }
    // auto data = file.readAll();
    // file.close();

    // try
    // {
    //     mJson.parse(data.data());
    //     mJson["OutputFrameThreashold"] = ARG.OutputFrameThreashold;
    //     mJson["Enable_Group_Number"] = ARG.Enable_Group_Number;

    //     for (int i = 0; i < 8; ++i)
    //     {
    //         group = "Group" + std::to_string(i+1);

    //         for (int j = 0; j < 8; ++j)
    //         {
    //             roi = "ROI" + std::to_string(j+1);

    //             mJson[group][roi]["enable"] = ARG.Group[i].ROI[j].enable;

    //             for (int var = 0, k = 0; var < 4; ++var)
    //             {
    //                 mJson[group][roi]["rectangle"][k++] = ARG.Group[i].ROI[j].points[var].toPoint().rx();
    //                 mJson[group][roi]["rectangle"][k++] = ARG.Group[i].ROI[j].points[var].toPoint().ry();
    //             }

    //             mJson[group][roi]["depthRange"]["d1"] = ARG.Group[i].ROI[j].depthRange.d1;
    //             mJson[group][roi]["depthRange"]["d2"] = ARG.Group[i].ROI[j].depthRange.d2;

    //             mJson[group][roi]["trsdSensitivityRate"]["low"] = ARG.Group[i].ROI[j].trsdSensitivityRate.low;
    //         }
    //     }

    //     const auto &_data = mJson.str();
    //     auto ret = LWSetSecurityConfigFileFromBuffer(mHandle, _data.c_str(), _data.size());
    //     if(ret != LW_RETURN_OK)
    //     {
    //         emit executionError("LWSetSecurityConfigFileFromBuffer 函数调用失败: " + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    //         return;
    //     }
    //     emit replyText("配置文件升级成功");
    // }
    // catch (std::exception e) {
    //     emit executionError("safety_protection_region.json 配置文件加载失败！");
    //     return;
    // }
}

void DeviceWorker::onSetRotationAxis(int ax, float ds)
{
    auto ret = LWSetSecurityAxialAdjustment(mHandle, ax, ds);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityAxialAdjustment 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
    emit replyText(tr("旋转参数下发成功"));

    char *text  = new char[65535*5];
    ret = LWGetSecurityConfigFileToBuffer(mHandle, LWFileType::LW_SECURITY_CALIB, text, 65535*5);
    if(ret == LW_RETURN_OK)
    {
        loadJsonFile(text, mJson_CalibMatrix);
        emit replyText(tr("获取标定矩阵参数成功"));
    }
    else
    {
        emit executionError(tr("LWGetSecurityConfigFileToBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    }
    delete[] text;
}

void DeviceWorker::onSetSecurityConfigInfo()
{
    QString  group, roi;

    // 安防绘制区域
    for (int i = 0; i < 8; ++i)
    {
        group = QString("Group%1").arg(i+1);
        auto obj1 = mJson_PRegion[group].toObject();
        for (int j = 0; j < 8; ++j)
        {
            roi = QString("ROI%1").arg(j+1);
            auto obj2 = obj1[roi].toObject();
            QJsonArray arr;
            for (int k = 0; k < 8; k++)
            {
                arr.append(ARG.Group[i].ROI[j].srcArea[k]);
            }
            obj2["rectangle"] = arr;
            obj1[roi] = obj2;
        }
        mJson_PRegion[group] = obj1;
    }
    auto text = QJsonDocument(mJson_PRegion).toJson(QJsonDocument::Indented).trimmed();
    auto ret = LWSetSecurityConfigFileFromBuffer(mHandle, LWFileType::LW_SECURITY_PR_ARG, text, text.size());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityConfigFileFromBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    // 安防配置信息
    mJson_Region["OutputFrameThreashold"] = ARG.OutputFrameThreashold;
    mJson_Region["Enable_Group_Number"] = ARG.Enable_Group_Number;
    for (int i = 0; i < 8; ++i)
    {
        group = QString("Group%1").arg(i+1);
        auto obj1 = mJson_Region[group].toObject();
        for (int j = 0; j < 8; ++j)
        {
            roi = QString("ROI%1").arg(j+1);
            auto obj2 = obj1[roi].toObject();

            obj2["enable"] = ARG.Group[i].ROI[j].enable;
            obj2["attribute"] = ARG.Group[i].ROI[j].attribute;

            QJsonArray arr;
            for (int k = 0; k < 8; ++k)
            {
                arr.append(ARG.Group[i].ROI[j].dstArea[k]);
            }
            obj2["rectangle"] = arr;

            auto obj3_1 = obj2["depthRange"].toObject();
            obj3_1["d1"] = ARG.Group[i].ROI[j].depthRange.d1;
            obj3_1["d2"] = ARG.Group[i].ROI[j].depthRange.d2;

            auto obj3_2 = obj2["trsdSensitivityRate"].toObject();
            obj3_2["low"] = ARG.Group[i].ROI[j].trsdSensitivityRate.low;

            obj2["depthRange"] = obj3_1;
            obj2["trsdSensitivityRate"] = obj3_2;

            obj1[roi] = obj2;
        }
        mJson_Region[group] = obj1;
    }
    text = QJsonDocument(mJson_Region).toJson(QJsonDocument::Indented).trimmed();
    ret = LWSetSecurityConfigFileFromBuffer(mHandle, LWFileType::LW_SECURITY_ARG, text, text.size());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityConfigFileFromBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("区域参数下发成功"));
}

void DeviceWorker::onSetSecurity3DInfo()
{
    QString  group, roi;

    // 安防配置信息
    for (int i = 0; i < 8; ++i)
    {
        group = QString("Group%1").arg(i+1);
        auto obj1 = mJson_Region[group].toObject();
        for (int j = 0; j < 8; ++j)
        {
            roi = QString("ROI%1").arg(j+1);
            auto obj2 = obj1[roi].toObject();

            QJsonArray arr;
            for (int k = 0; k < 8; ++k)
            {
                arr.append(ARG.Group[i].ROI[j].dstArea[k]);
            }
            obj2["rectangle"] = arr;

            auto obj3_1 = obj2["depthRange"].toObject();
            obj3_1["d1"] = ARG.Group[i].ROI[j].depthRange.d1;
            obj3_1["d2"] = ARG.Group[i].ROI[j].depthRange.d2;
            obj2["depthRange"] = obj3_1;

            obj1[roi] = obj2;
        }
        mJson_Region[group] = obj1;

    }
    auto text = QJsonDocument(mJson_Region).toJson(QJsonDocument::Indented);
    auto ret = LWSetSecurityConfigFileFromBuffer(mHandle, LWFileType::LW_SECURITY_ARG, text, text.size());
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetSecurityConfigFileFromBuffer 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("3D区域参数下发成功"));
}

void DeviceWorker::onSaveConfigureInfo()
{
    auto ret = LWSaveConfigureInfo(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSaveConfigureInfo 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    emit replyText(tr("滤波配置保存成功"));
}

void DeviceWorker::onDeleteConfigureInfo()
{
    auto ret = LWRemoveConfigureInfo(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWRemoveConfigureInfo 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
}

void DeviceWorker::onResetConfigureInfo()
{
    auto ret = LWRestoreFactoryConfigureInfo(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWRestoreFactoryConfigureInfo 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
}

void DeviceWorker::onStart()
{
    // 设置数据接收类型
    auto ret = LWSetDataReceiveType(mHandle, LWDataRecvType::LW_POINTCLOUD_DEPTH_IR_RTY);
    if (ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetDataReceiveType 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }

    ret = LWSetIRGMMGain(mHandle, config.irValue);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("LWSetIRGMMGain 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    }

    // LWSetSpatialFilterParams(mHandle, LWFilterParam{config.isSpatialFilter, 5});
    // LWSetTimeFilterParams(mHandle, LWFilterParam{config.isTimeFilter, config.timeFilterValue, 100});
    // LWSetFlyingPixelsFilterParams(mHandle, LWFilterParam{config.isFlyPixeFilter, config.flyPixeFilterValue});
    // LWSetConfidenceFilterParams(mHandle, LWFilterParam{config.isConfFilter, config.confFilterValue});

    // auto ret = LWStartStream(mHandle);
    // if(ret != LW_RETURN_OK)
    // {
    //     emit executionError("StartStream 函数调用失败: " + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    //     return;
    // }

    std::thread(&DeviceWorker::threadFunction, this).detach();

    emit runStatus(true);
}

void DeviceWorker::onStop()
{
    mRun = false;
    // auto ret = LWStopStream(mHandle);
    // if(ret != LW_RETURN_OK)
    // {
    //     emit executionError("StopStream 函数调用失败: " + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
    //     return;
    // }
}

void DeviceWorker::onStopStream()
{
    mRun = false;

    auto ret = LWStopStream(mHandle);
    if(ret != LW_RETURN_OK)
    {
        emit executionError(tr("StopStream 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
        return;
    }
}

void DeviceWorker::onFindDevice()
{
    QStringList list;

    int counts;
    LWFindDevices(mHandleList, 10, &counts);
    for(int i = 0; i < counts; ++i)
    {
        auto idPtr = (unsigned char *)&mHandleList[i];
        list.append(QString("%1.%2.%3.%4").arg(idPtr[0]).arg(idPtr[1]).arg(idPtr[2]).arg(idPtr[3]));
    }

    emit deviceList(list);
}

void DeviceWorker::onConnectionEnable(bool flag, int index)
{
    if(flag)
    {
        /// 打开设备
        auto ret = LWOpenDevice(mHandleList[index]);
        if(ret != LW_RETURN_OK) {
            emit connectStatus(false, tr("连接"));
            emit executionError(tr("LWOpenDevice 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
            return;
        }
        mHandle = mHandleList[index];

        /// 获取设备型号
        char type[32];
        LWGetDeviceType(mHandle, type, 32);
        if(QString(type).contains("D345", Qt::CaseInsensitive))
        {
            config.devType  = "se";
            if(config.projName == "base") config.projName = "dmse-base";
            config.password = "dm-se-zgyfjch";
        }
        else
        {
            config.devType  = "dm";
            config.password = "dm-zgyfjch";
        }

        /// 加载配置文件
        if(!loadProfile())  return;

        /// 获取设备版本信息
        LWVersionInfo fv;
        LWVersionInfo dv;
        LWGetDeviceVersion(mHandle, &fv, &dv);
        if(ret != LW_RETURN_OK)
        {
            emit executionError(tr("LWGetDeviceVersion 函数调用失败: ") + QString::fromLocal8Bit(LWGetReturnCodeDescriptor(ret)));
            return;
        }
        config.fvMajor = fv.major;
        /// 获取设备参数信息
        LWFilterParam Param;
        LWGetSpatialFilterParams(mHandle, &Param);
        config.isSpatialFilter = Param.enable;
        config.spatialFilterValue = Param.threshold;
        LWGetTimeFilterParams(mHandle, &Param);
        config.isTimeFilter = Param.enable;
        config.timeFilterValue = Param.threshold;
        LWGetTimeMedianFilterParams(mHandle, &Param);
        config.isTimeFilter_ = Param.enable;
        config.timeFilterValue_ = Param.threshold;
        LWGetFlyingPixelsFilterParams(mHandle, &Param);
        config.isFlyPixeFilter = Param.enable;
        config.flyPixeFilterValue = Param.threshold;
        LWGetConfidenceFilterParams(mHandle, &Param);
        config.isConfFilter = Param.enable;
        config.confFilterValue = Param.threshold;
        LWGetFrameRate(mHandle, &config.frameRate);
        LWGetExposureTime(mHandle, LW_TOF_SENSOR, config.exposure, 5, &index);
        /// 获取设备是否进行了标定
        config.isSCalib = false;
        LWGetSecurityCalibrationEnable(mHandle, &config.isSCalib);
        emit calibrationStep(config.isSCalib);
        /// 获取设备内参
        LWSensorIntrinsicParam inParam;
        LWGetIntrinsicParam(mHandle, LWSensorType::LW_TOF_SENSOR, &inParam);
        ///
        /// \brief LWGetSecurityZHSAEnable
        bool enable = true;
        LWGetSecurityZHSAEnable(mHandle, &enable);
        emit soundLightAlarmEnable(enable);
        /// 设置设备帧率
        ret = LWSetFrameRate(mHandle, 5);
        if(ret == LW_RETURN_OK) config.frameRate = 5;
        /// 获取设备
        char sn[32];
        LWNetworkInfo net;
        LWGetDeviceSN(mHandle, sn, 32);
        LWGetNetworkInfo(mHandle, &net);
        emit deviceInfo(QString("%1.%2.%3.%4").arg(fv.major).arg(fv.minor).arg(fv.patch).arg(fv.reserved),
                        QString("%1.%2.%3.%4").arg(dv.major).arg(dv.minor).arg(dv.patch).arg(dv.reserved),
                        QString(sn),
                        QString(net.ip),
                        QString(type),
                        inParam
                        );

        emit roiConfigUpdate();
        emit connectStatus(true, tr("断开"));
    }
    else
    {
        if(config.isAutoReboot)
            LWRebootDevice(mHandle);
        else
            LWCloseDevice(mHandle);

        emit connectStatus(false, tr("连接"));
    }

}
