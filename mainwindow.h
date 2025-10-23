#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QTranslator>

#include "AlarmALGVisual.h"

#include "deviceworker.h"
#include "settings.h"
#include "custompixmapitem.h"
#include "customgraphicsview.h"
#include "logform.h"
#include "depthdrawform.h"
#include "deviceargsetform.h"
#include "networksetform.h"

#include "vtkRenderer.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPoints.h"
#include "vtkBillboardTextActor3D.h"
#include "vtkPolyDataMapper.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkSelectEnclosedPoints.h"
#include "vtkCubeAxesActor.h"
#include "vtkImplicitPolyDataDistance.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class VertexInteractionStyle;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initVtkWindow();
    double doubleInputDialog(const QString &title, const QString &label, double value = 0, double min = -2147483647, double max = 2147483647, int decimals = 1, Qt::WindowFlags flags = Qt::WindowFlags(), double step = 1);
    void vertexActorUpdate(int);
    void restoreAreaInfo();
    bool isAreaInfoChange();
    bool is3DAreaChange();
    void dmUpdate();
    bool isQuadrilateral(vtkPoints* pots);
    int concaveVertex(vtkPoints* pots);


signals:
    void start();
    void stop();
    void stopStream();
    void linkSwitch(bool , int = 0);
    void setIRGammaValue(int );
    void setRotationAxis(int, float);
    void upgradeJsonFile();
    void exportJsonFile(QString);
    void importJsonFile(QStringList);
    void initJsonFile();
    void saveConfigFile();
    void setCalibrationARG();
    void readyMe();
    void pullLog(QString);
    void senToLogPanel(QString);
    void setPerspectiveMatrix();
    void setSecurityConfigInfo();
    void setSoundLightAlarm(bool );
    void setSecurity3DInfo();


private slots:
    void onFindDevice();
    void onDeviceList(QStringList);
    void onDeviceInfo(QString, QString, QString, QString, QString, LWSensorIntrinsicParam);
    void onLink();
    void onExecutionError(QString );
    void onSaveConfigureInfo();
    void onConnectStatus(bool, QString);
    void onExecute();
    void onRunStatus(bool);
    void onGraData(LWFrameData &, double);
    void onDepData(LWFrameData &, double);
    void onPotData(LWFrameData &, LWFrameData &, double);
    void onViewModeClicked(bool checked);
    void onEditModeClicked(bool checked);
    void onCalibrationModeClicked(bool checked);
    void onPointMoveEnable(bool checked);
    void onPointPlaneMoveEnable(bool checked);
    void onIndividualMoveEnable(bool checked);
    void onHeightMoveEnable(bool checked);
    void onIndividualAxialMoveEnable(bool checked);
    void onGroupNumberChanged(int index);
    void onAreaNumberChanged(int index);
    void onSaveAreaConfig();
    void onRemoveArea();
    void onRemoveGroup();
    void onDrawModelChanged(int index);
    void onUpdateFirmware();
    void onLogForm();
    void onDepthWindow();
    void onExpertModel();
    void onQProcessInfo();
    void onResetCamera();
    void onRecordLocation();
    void onPyramidsOpacity();
    void onOverlapOpacity();
    void onNonoverlapOpacity();
    void onPointSize();
    void onPerspectiveTransfer(int );
    void onPotTransfer(double );
    void onCalibration();
    void onUpdateAreaInfo();
    void onCalibrationStep(int);
    void onPullLog();
    void onRecoverInit();
    void onNetworkSet();
    void onRulerEnable();
    void onPotTextLabels();
    void onTextureMapEnable();
    void onSetSoundLightAlarm();
    void onSoundLightAlarmEnable(bool );
    void onImpressum();
    void onChangeText();
    void onSelectExportPath();
    void onSelectImportFile();
    void onProjectionTransform();
    void onPotBackgroundColor();
    void onLock3DView();
    void onArea3dDrawSave();
    void onEnter3dDrawMode();
    void onLeave3dDrawMode();
    void onSwitchLanguage();
    void onNote();


protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::MainWindow *ui;

    unsigned char                       *mIRPixels;

    QImage                              mIRImage{IR_PIX_COLS, IR_PIX_ROWS, QImage::Format_Grayscale8};
    CustomPixmapItem                    mIRItem;
    CustomGraphicsView                  *mIRView;

    QTimer                              mTimer;
    QThread                             mDeviceThread;
    QTranslator                         mTranslator;
    alarmVisualTofIntrinsicParam        mInParam;
    DeviceWorker                        mDeviceWorker;
    QProcess                            mProcess;
    LogForm                             mLogForm;
    DepthDrawForm                       mDepthDrawForm;
    DeviceArgSetForm                    mDeviceArgSetForm;
    NetworkSetForm                      mNetworkSetForm;

    bool                                mNotTransfer;
    bool                                mAreaConvert;
    bool                                mNonZeroArr[8];
    int                                 mFlag;
    int                                 mGroupNum;
    int                                 mAreaNum;
    float                               mFloatArr1[60];
    float                               mFloatArr2[320];
    vtkNew<vtkRenderer>                 mRenderer;
    vtkNew<vtkOrientationMarkerWidget>  mAxesWidget;
    vtkNew<vtkPoints>                   mPoints;
    vtkNew<vtkPoints>                   mPyramidPots;
    vtkNew<vtkPoints>                   mCubePots[8];
    vtkNew<vtkUnsignedCharArray>        mColors;
    vtkNew<vtkSelectEnclosedPoints>     mSelects[8];
    vtkNew<vtkSelectEnclosedPoints>     mPSelects[8];
    vtkNew<vtkImplicitPolyDataDistance> mSelect;
    vtkNew<vtkActor>                    mIntersectionActor[8];  /// 交集
    vtkNew<vtkActor>                    mCubeActor[8];          /// 棱柱
    vtkNew<vtkActor>                    mVertexActor;           /// 棱柱顶点
    vtkNew<vtkActor>                    mWireframeActor[8];     /// 线框集
    vtkNew<vtkActor>                    mPointActor;            /// 点云
    vtkNew<vtkActor>                    mPyramidActor;          /// 视锥
    vtkNew<vtkActor>                    mDMActor;               /// DM模型
    vtkNew<vtkCubeAxesActor>            mCubeAxesActor;         /// 标尺
    vtkNew<vtkBillboardTextActor3D>     mCubeTextActor[8];      /// 文字标签
    vtkNew<vtkEventQtSlotConnect>       mConnections;
    vtkNew<VertexInteractionStyle>      mStyle;

    AlarmVisual                         mAlarmVisual;

};
#endif // MAINWINDOW_H
