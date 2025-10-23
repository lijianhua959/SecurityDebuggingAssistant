#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QInputDialog>
#include <QToolTip>

#include "vtkRenderWindow.h"
#include "vtkAxesActor.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkScalarBarActor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkBillboardTextActor3D.h"
#include "vtkTextProperty.h"
#include "vtkSTLReader.h"
#include "vtkPolygon.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkCollisionDetectionFilter.h"
#include "vtkTriangleFilter.h"
#include "vtkBooleanOperationPolyDataFilter.h"
#include "vtkSelectEnclosedPoints.h"
#include "vtkCleanPolyData.h"
#include "vtkDelaunay3D.h"
#include "vtkConnectivityFilter.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkPointPicker.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkAppendPolyData.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTriangle.h"
#include "vtkQuad.h"
#include "vtkGeometryFilter.h"

#include "dialogform.h"


vtkIdType pyramIdType[5][4] = {
    {0, 2, 1},  // 侧面0
    {0, 3, 2},  // 侧面1
    {0, 4, 3},  // 侧面2
    {0, 1, 4},  // 侧面3
    {1, 2, 3, 4}// 底面4
};
vtkIdType polyIdType[6][4] = {
    {0, 3, 2, 1},   // 底面
    {0, 1, 5, 4},   // 侧面1
    {1, 2, 6, 5},   // 侧面2
    {2, 3, 7, 6},   // 侧面3
    {3, 0, 4, 7},   // 侧面4
    {4, 5, 6, 7}    // 顶面
};

class VertexInteractionStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static VertexInteractionStyle* New();
    vtkTypeMacro(VertexInteractionStyle, vtkInteractorStyleTrackballCamera);

    void SetVertexActor(vtkActor* actor)
    {
        mPicker->SetTolerance(0.003);
        mPicker->PickFromListOn();
        mPicker->AddPickList(actor);
        mVertexActor = actor;

        mAxesActor_X->GetProperty()->SetColor(1, 0, 0);
        mAxesActor_Y->GetProperty()->SetColor(0, 1, 1);
        mAxesActor_Z->GetProperty()->SetColor(0, 0, 1);

        mPickerActor->SetTolerance(0.001);
        mPickerActor->PickFromListOn();
        mPickerActor->AddPickList(mAxesActor_X);
        mPickerActor->AddPickList(mAxesActor_Y);
        mPickerActor->AddPickList(mAxesActor_Z);
    }

    virtual void Rotate() override
    {
        //
        if(mIsRotate) vtkInteractorStyleTrackballCamera::Rotate();
    }

    virtual void OnLeftButtonDown() override
    {
        if(mIsDraw)
        {
            mIsLeftButton = true;
            // 获取鼠标位置
            auto clickPos = this->GetInteractor()->GetEventPosition();
            // 根据屏幕坐标获取世界坐标
            vtkNew<vtkCoordinate> coordinate;
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(clickPos[0], clickPos[1], 0);
            double* worldPos = coordinate->GetComputedWorldValue(this->DefaultRenderer);
            mPickPos[0] = worldPos[0];
            mPickPos[1] = worldPos[1];
            mPickPos[2] = worldPos[2];
            //
            if(mIsIndividualAxialMoveEnable)
            {
                // 抓取轴向目标
                mPickerActor->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
                auto actor = mPickerActor->GetActor();
                if(actor == mAxesActor_X)
                {
                    mAxesActor_X->GetProperty()->SetColor(1, 1, 0);
                    mAxialFlag = 1;
                    return;
                }
                else if(actor == mAxesActor_Y)
                {
                    mAxesActor_Y->GetProperty()->SetColor(1, 1, 0);
                    mAxialFlag = 2;
                    return;
                }
                else if(actor == mAxesActor_Z)
                {
                    mAxesActor_Z->GetProperty()->SetColor(1, 1, 0);
                    mAxialFlag = 3;
                    return;
                }
            }
            else
            {
                // 获取拾取点的索引
                mPicker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
                mSelectedPointId = mPicker->GetPointId();
                if (mSelectedPointId >= 0 && mSelectedPointId < 8)
                {
                    // 获取抓对象的数据
                    auto* polyData = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput());
                    //
                    if(mIsPointMove || mIsPointPlaneMoveEnable )
                    {
                        polyData->GetPointData()->GetScalars()->SetTuple3(mSelectedPointId, 255, 255, 0);
                        if(mSelectedPointId<4)
                            polyData->GetPointData()->GetScalars()->SetTuple3(mSelectedPointId+4, 255, 255, 0);
                        else
                            polyData->GetPointData()->GetScalars()->SetTuple3(mSelectedPointId-4, 255, 255, 0);

                        // 更新几何体
                        polyData->Modified();
                        this->Interactor->Render();
                        return;
                    }
                    else if(mIsIndividualMoveEnable)
                    {
                        for (int i = 0; i < 8; ++i)
                            polyData->GetPointData()->GetScalars()->SetTuple3(i, 255, 255, 0);
                        // 更新几何体
                        polyData->Modified();
                        this->Interactor->Render();
                        return;
                    }
                    else if(mIsHeightMove)
                    {
                        if(mSelectedPointId<4)
                        {
                            polyData->GetPointData()->GetScalars()->SetTuple3(0, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(1, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(2, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(3, 255, 255, 0);
                        }
                        else
                        {
                            polyData->GetPointData()->GetScalars()->SetTuple3(4, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(5, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(6, 255, 255, 0);
                            polyData->GetPointData()->GetScalars()->SetTuple3(7, 255, 255, 0);
                        }
                        // 更新几何体
                        polyData->Modified();
                        this->Interactor->Render();
                        return;
                    }
                }
            }
        }

        // 未选中顶点则调用默认行为
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    virtual void OnLeftButtonUp() override
    {
        if(mIsDraw)
        {
            mIsLeftButton = false;

            if(mIsIndividualAxialMoveEnable && (mAxialFlag != 0))
            {
                mAxesActor_X->GetProperty()->SetColor(1, 0, 0);
                mAxesActor_Y->GetProperty()->SetColor(0, 1, 1);
                mAxesActor_Z->GetProperty()->SetColor(0, 0, 1);
                mAxialFlag = 0;
            }

            if (mSelectedPointId != -1)
            {
                mSelectedPointId = -1;

                auto* polyData = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput());
                for (int i = 0; i < 8; ++i) polyData->GetPointData()->GetScalars()->SetTuple3(i, 0, 115, 157);
                polyData->Modified();
            }

            this->Interactor->Render();
        }

        //
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }

    virtual void OnMouseMove() override
    {
        if (mIsDraw && mIsLeftButton && (mSelectedPointId != -1 || mAxialFlag > 0))
        {
            // 移动选中的顶点
            auto posptr = this->Interactor->GetEventPosition();
            // 根据屏幕坐标获取世界坐标
            vtkNew<vtkCoordinate> coordinate;
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(posptr[0], posptr[1], 0);
            double* worldPos = coordinate->GetComputedWorldValue(this->DefaultRenderer);
            // 计算偏移量
            mOffset[0] = worldPos[0] - mPickPos[0];
            mOffset[1] = worldPos[1] - mPickPos[1];
            mOffset[2] = worldPos[2] - mPickPos[2];
            // 记录当前位置信息，以便下次计算
            mPickPos[0] = worldPos[0];
            mPickPos[1] = worldPos[1];
            mPickPos[2] = worldPos[2];
            //
            double pointv1[3];
            double pointv2[3];
            //
            if(mIsPointMove)
            {
                // 更新顶点位置
                vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();
                points->GetPoint(mSelectedPointId, pointv1);
                pointv1[0] += mOffset[0];
                pointv1[1] += mOffset[1];
                pointv1[2] += mOffset[2];
                if(mSelectedPointId < 4)
                {
                    if(pointv1[0] < mMinX) pointv1[0] = mMinX;
                    if(pointv1[0] > mMaxX) pointv1[0] = mMaxX;
                    if(pointv1[1] < mMinY) pointv1[1] = mMinY;
                    if(pointv1[1] > mMaxY) pointv1[1] = mMaxY;
                    if(pointv1[2] < mMinZ) pointv1[2] = mMinZ;
                    if(pointv1[2] > mMaxZ) pointv1[2] = mMaxZ;
                    points->SetPoint(mSelectedPointId, pointv1);

                    points->GetPoint(mSelectedPointId+4, pointv2);
                    pointv2[0] = pointv1[0];
                    pointv2[1] = pointv1[1];
                    points->SetPoint(mSelectedPointId+4, pointv2);

                    for (int var = 0; var < 4; ++var)
                    {
                        if(var == mSelectedPointId) continue;
                        points->GetPoint(var, pointv2);
                        pointv2[2] = pointv1[2];
                        points->SetPoint(var, pointv2);
                    }
                }
                else
                {
                    if(pointv1[0] < mMinX) pointv1[0] = mMinX;
                    if(pointv1[0] > mMaxX) pointv1[0] = mMaxX;
                    if(pointv1[1] < mMinY) pointv1[1] = mMinY;
                    if(pointv1[1] > mMaxY) pointv1[1] = mMaxY;
                    if(pointv1[2] < mMinZ) pointv1[2] = mMinZ;
                    if(pointv1[2] > mMaxZ) pointv1[2] = mMaxZ;
                    points->SetPoint(mSelectedPointId, pointv1);

                    points->GetPoint(mSelectedPointId-4, pointv2);
                    pointv2[0] = pointv1[0];
                    pointv2[1] = pointv1[1];
                    points->SetPoint(mSelectedPointId-4, pointv2);

                    for (int var = 4; var < 8; ++var)
                    {
                        if(var == mSelectedPointId) continue;
                        points->GetPoint(var, pointv2);
                        pointv2[2] = pointv1[2];
                        points->SetPoint(var, pointv2);
                    }
                }
                // 更新几何体
                points->Modified();
                this->Interactor->Render();

                return;
            }
            else if(mIsPointPlaneMoveEnable)
            {
                // 更新顶点位置（Z轴不变）
                vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();
                points->GetPoint(mSelectedPointId, pointv1);
                pointv1[0] += mOffset[0];
                pointv1[1] += mOffset[1];

                if(pointv1[0] < mMinX) pointv1[0] = mMinX;
                if(pointv1[0] > mMaxX) pointv1[0] = mMaxX;
                if(pointv1[1] < mMinY) pointv1[1] = mMinY;
                if(pointv1[1] > mMaxY) pointv1[1] = mMaxY;
                points->SetPoint(mSelectedPointId, pointv1);

                if(mSelectedPointId < 4)
                {
                    points->GetPoint(mSelectedPointId+4, pointv2);
                    pointv2[0] = pointv1[0];
                    pointv2[1] = pointv1[1];
                    points->SetPoint(mSelectedPointId+4, pointv2);
                }
                else
                {
                    points->GetPoint(mSelectedPointId-4, pointv2);
                    pointv2[0] = pointv1[0];
                    pointv2[1] = pointv1[1];
                    points->SetPoint(mSelectedPointId-4, pointv2);
                }
                // 更新几何体
                points->Modified();
                this->Interactor->Render();

                return;
            }
            else if(mIsIndividualMoveEnable)
            {
                vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();
                //
                auto minX = std::min({points->GetPoint(0)[0], points->GetPoint(1)[0], points->GetPoint(2)[0], points->GetPoint(3)[0]});
                auto maxX = std::max({points->GetPoint(0)[0], points->GetPoint(1)[0], points->GetPoint(2)[0], points->GetPoint(3)[0]});
                auto minY = std::min({points->GetPoint(0)[1], points->GetPoint(1)[1], points->GetPoint(2)[1], points->GetPoint(3)[1]});
                auto maxY = std::max({points->GetPoint(0)[1], points->GetPoint(1)[1], points->GetPoint(2)[1], points->GetPoint(3)[1]});
                auto minZ = std::min({points->GetPoint(0)[2], points->GetPoint(4)[2]});
                auto maxZ = std::max({points->GetPoint(0)[2], points->GetPoint(4)[2]});
                //
                if(minX + mOffset[0] < mMinX) mOffset[0] = mMinX - minX;
                if(maxX + mOffset[0] > mMaxX) mOffset[0] = mMaxX - maxX;
                if(minY + mOffset[1] < mMinY) mOffset[1] = mMinY - minY;
                if(maxY + mOffset[1] > mMaxY) mOffset[1] = mMaxY - maxY;
                if(minZ + mOffset[2] < mMinZ) mOffset[2] = mMinZ - minZ;
                if(maxZ + mOffset[2] > mMaxZ) mOffset[2] = mMaxZ - maxZ;
                //
                for (int i = 0; i < 8; ++i)
                {
                    points->GetPoint(i, pointv1);
                    pointv1[0] += mOffset[0];
                    pointv1[1] += mOffset[1];
                    pointv1[2] += mOffset[2];
                    points->SetPoint(i, pointv1);
                }
                // 更新几何体
                points->Modified();
                this->Interactor->Render();

                return;
            }
            else if(mIsHeightMove)
            {
                // 更新顶点Z-轴位置（X,Y轴不变）
                vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();
                //
                auto h = points->GetPoint(mSelectedPointId)[2] + mOffset[2];
                //
                if(h < mMinZ)
                    mOffset[2] = mMinZ - points->GetPoint(mSelectedPointId)[2];
                else if(h > mMaxZ)
                    mOffset[2] = mMaxZ - points->GetPoint(mSelectedPointId)[2];
                //
                for (int i = 0, k = mSelectedPointId<4?0:4; i < 4; ++i, ++k)
                {
                    points->GetPoint(k, pointv1);
                    pointv1[2] += mOffset[2];
                    points->SetPoint(k, pointv1);
                }
                // 更新几何体
                points->Modified();
                this->Interactor->Render();

            }
            else if(mIsIndividualAxialMoveEnable)
            {
                // 更新顶点位置
                vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();
                //
                if(mAxialFlag == 1)
                {
                    auto minX = std::min({points->GetPoint(0)[0], points->GetPoint(1)[0], points->GetPoint(2)[0], points->GetPoint(3)[0]});
                    auto maxX = std::max({points->GetPoint(0)[0], points->GetPoint(1)[0], points->GetPoint(2)[0], points->GetPoint(3)[0]});
                    if(minX + mOffset[0] < mMinX) mOffset[0] = mMinX - minX;
                    if(maxX + mOffset[0] > mMaxX) mOffset[0] = mMaxX - maxX;
                }
                if(mAxialFlag == 2)
                {
                    auto minY = std::min({points->GetPoint(0)[1], points->GetPoint(1)[1], points->GetPoint(2)[1], points->GetPoint(3)[1]});
                    auto maxY = std::max({points->GetPoint(0)[1], points->GetPoint(1)[1], points->GetPoint(2)[1], points->GetPoint(3)[1]});
                    if(minY + mOffset[1] < mMinY) mOffset[1] = mMinY - minY;
                    if(maxY + mOffset[1] > mMaxY) mOffset[1] = mMaxY - maxY;
                }
                if(mAxialFlag == 3)
                {
                    auto minZ = std::min({points->GetPoint(0)[2], points->GetPoint(4)[2]});
                    auto maxZ = std::max({points->GetPoint(0)[2], points->GetPoint(4)[2]});
                    if(minZ + mOffset[2] < mMinZ) mOffset[2] = mMinZ - minZ;
                    if(maxZ + mOffset[2] > mMaxZ) mOffset[2] = mMaxZ - maxZ;
                }
                //
                for (int i = 0; i < 8; ++i)
                {
                    points->GetPoint(i, pointv1);
                    if(mAxialFlag == 1 ) pointv1[0] +=mOffset[0];
                    if(mAxialFlag == 2 ) pointv1[1] +=mOffset[1];
                    if(mAxialFlag == 3 ) pointv1[2] +=mOffset[2];
                    points->SetPoint(i, pointv1);
                }
                //
                mAxesActor_X->GetPosition(pointv1);
                if(mAxialFlag == 1 ) pointv1[0] +=mOffset[0];
                if(mAxialFlag == 2 ) pointv1[1] +=mOffset[1];
                if(mAxialFlag == 3 ) pointv1[2] +=mOffset[2];
                mAxesActor_X->SetPosition(pointv1);
                //
                mAxesActor_Y->GetPosition(pointv1);
                if(mAxialFlag == 1 ) pointv1[0] +=mOffset[0];
                if(mAxialFlag == 2 ) pointv1[1] +=mOffset[1];
                if(mAxialFlag == 3 ) pointv1[2] +=mOffset[2];
                mAxesActor_Y->SetPosition(pointv1);
                //
                mAxesActor_Z->GetPosition(pointv1);
                if(mAxialFlag == 1 ) pointv1[0] +=mOffset[0];
                if(mAxialFlag == 2 ) pointv1[1] +=mOffset[1];
                if(mAxialFlag == 3 ) pointv1[2] +=mOffset[2];
                mAxesActor_Z->SetPosition(pointv1);
                // 更新几何体
                points->Modified();
                this->Interactor->Render();

                return;
            }
        }

        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }

    void drawEnable(bool enable)
    {
        mIsDraw = enable;

        if(enable)
        {
            if(this->DefaultRenderer->HasViewProp(mVertexActor) == 0)
                this->DefaultRenderer->AddActor(mVertexActor);

            if(mIsIndividualAxialMoveEnable) drawAxialArrow();
        }
        else
        {
            this->DefaultRenderer->RemoveActor(mVertexActor);

            if(this->DefaultRenderer->HasViewProp(mAxesActor_X) != 0)
            {
                this->DefaultRenderer->RemoveActor(mAxesActor_X);
                this->DefaultRenderer->RemoveActor(mAxesActor_Y);
                this->DefaultRenderer->RemoveActor(mAxesActor_Z);
            }
        }
        this->Interactor->Render();
    }

    void pointMoveEnable(bool enable)
    {
        if(mIsDraw)
        {
            mIsPointMove = enable;
        }
    }

    void pointPlaneMoveEnable(bool enable)
    {
        if(mIsDraw)
        {
            mIsPointPlaneMoveEnable = enable;
        }
    }

    void individualMoveEnable(bool enable)
    {
        if(mIsDraw)
        {
            mIsIndividualMoveEnable = enable;
        }
    }

    void heightMoveEnable(bool enable)
    {
        if(mIsDraw)
        {
            mIsHeightMove = enable;
        }
    }

    void individualAxialMoveEnable(bool enable)
    {
        if(mIsDraw)
        {
            mIsIndividualAxialMoveEnable = enable;

            if(enable)
            {
                drawAxialArrow();
            }
            else
            {
                this->DefaultRenderer->RemoveActor(mAxesActor_X);
                this->DefaultRenderer->RemoveActor(mAxesActor_Y);
                this->DefaultRenderer->RemoveActor(mAxesActor_Z);
            }
        }
    }

    void rotateEnable(bool enable)
    {
        mIsRotate = enable;
    }

    void drawAxialArrow()
    {
        vtkPoints* points = vtkPolyData::SafeDownCast(mVertexActor->GetMapper()->GetInput())->GetPoints();

        // 创建锥形箭头头部
        vtkNew<vtkConeSource> tipSource;
        tipSource->SetHeight(100);
        tipSource->SetRadius(60);
        tipSource->SetResolution(20);
        // 创建圆柱形箭杆
        vtkNew<vtkCylinderSource> shaftSource;
        shaftSource->SetHeight(150);
        shaftSource->SetRadius(10);
        shaftSource->SetResolution(20);
        shaftSource->SetCenter(0, shaftSource->GetHeight()/2, 0);
        vtkNew<vtkTransform> customTransform;
        customTransform->RotateZ(90);
        vtkNew<vtkTransformPolyDataFilter> customFilter;
        customFilter->SetInputConnection(shaftSource->GetOutputPort());
        customFilter->SetTransform(customTransform);
        // 组合锥体和圆柱体
        vtkNew<vtkAppendPolyData> append;
        append->AddInputConnection(tipSource->GetOutputPort());
        append->AddInputConnection(customFilter->GetOutputPort());
        // X轴向
        vtkNew<vtkPolyDataMapper> xMapper;
        xMapper->SetInputConnection(append->GetOutputPort());
        mAxesActor_X->SetMapper(xMapper);
        mAxesActor_X->SetPosition(//mVertexActor->GetCenter()[0] +150,
            std::max({points->GetPoint(0)[0], points->GetPoint(1)[0], points->GetPoint(2)[0], points->GetPoint(3)[0]})+150,
            mVertexActor->GetCenter()[1],
            mVertexActor->GetCenter()[2]);
        // Y轴向
        vtkNew<vtkTransform> customTransform_y;
        customTransform_y->RotateZ(90);
        vtkNew<vtkTransformPolyDataFilter> customFilter_y;
        customFilter_y->SetInputConnection(append->GetOutputPort());
        customFilter_y->SetTransform(customTransform_y);
        vtkNew<vtkPolyDataMapper> yMapper;
        yMapper->SetInputConnection(customFilter_y->GetOutputPort());
        mAxesActor_Y->SetMapper(yMapper);
        mAxesActor_Y->SetPosition(mVertexActor->GetCenter()[0],
                                  // mVertexActor->GetCenter()[1] +150,
                                  std::max({points->GetPoint(0)[1], points->GetPoint(1)[1], points->GetPoint(2)[1], points->GetPoint(3)[1]})+150,
                                  mVertexActor->GetCenter()[2]);
        // Z轴向
        vtkNew<vtkTransform> customTransform_z;
        customTransform_z->RotateY(-90);
        vtkNew<vtkTransformPolyDataFilter> customFilter_z;
        customFilter_z->SetInputConnection(append->GetOutputPort());
        customFilter_z->SetTransform(customTransform_z);
        vtkNew<vtkPolyDataMapper> zMapper;
        zMapper->SetInputConnection(customFilter_z->GetOutputPort());
        mAxesActor_Z->SetMapper(zMapper);
        mAxesActor_Z->SetPosition(mVertexActor->GetCenter()[0],
                                  mVertexActor->GetCenter()[1],
                                  // mVertexActor->GetCenter()[2] +150
                                  std::max({points->GetPoint(0)[2], points->GetPoint(4)[2]})+150
                                  );

        if(this->DefaultRenderer->HasViewProp(mAxesActor_X) == 0)
        {
            this->DefaultRenderer->AddActor(mAxesActor_X);
            this->DefaultRenderer->AddActor(mAxesActor_Y);
            this->DefaultRenderer->AddActor(mAxesActor_Z);
        }
    }


private:
    bool                    mIsRotate = true;
    bool                    mIsLeftButton = false;
    bool                    mIsIndividualMoveEnable = false;
    bool                    mIsPointMove = true;
    bool                    mIsPointPlaneMoveEnable = false;
    bool                    mIsHeightMove = false;
    bool                    mIsIndividualAxialMoveEnable = false;
    bool                    mIsDraw = false;
    int                     mAxialFlag = 0;
    double                  mPickPos[3];
    double                  mOffset[3];
    double                  mMinX = -6000;
    double                  mMaxX =  6000;
    double                  mMinY = -6000;
    double                  mMaxY =  6000;
    double                  mMinZ =  150;
    double                  mMaxZ =  6000;
    vtkNew<vtkPointPicker>  mPicker;
    vtkNew<vtkPicker>       mPickerActor;
    vtkNew<vtkActor>        mAxesActor_X;
    vtkNew<vtkActor>        mAxesActor_Y;
    vtkNew<vtkActor>        mAxesActor_Z;
    vtkIdType               mSelectedPointId=-1;
    vtkActor*               mVertexActor;

};
vtkStandardNewMacro(VertexInteractionStyle);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // auto ret = mTranslator.load("D:/B_ProjectQt/SecurityDebuggingAssistant/APP_en_US.qm");
    auto ret = mTranslator.load("APP_en_US.qm");
    if(!ret) qDebug()<<"加载失败！！！";

    mIRView = new CustomGraphicsView(this);
    mIRView->setItem(mIRItem);
    ui->irLayout->addWidget(mIRView);
    ui->irGammaValue->setValue(config.irValue);
    if(!config.isTextureMapEnable) ui->textureMap->setText(tr("开启点云纹理映射"));

    mIRPixels = reinterpret_cast<unsigned char*>(mIRImage.bits());
    for (int i = 0; i < IR_PIX_NUMBER; ++i) mIRPixels[i] = 50;
    mIRItem.setPixmap(QPixmap::fromImage(mIRImage));

    mGroupNum = 0;
    mAreaNum = 0;
    mNotTransfer = true;
    mAreaConvert = false;
    ui->widget_1->setEnabled(false);

    DialogForm::icon = this->windowIcon();
    mLogForm.setWindowIcon(this->windowIcon());
    mDepthDrawForm.setWindowIcon(this->windowIcon());
    mDeviceArgSetForm.setWindowIcon(this->windowIcon());
    mNetworkSetForm.setWindowIcon(this->windowIcon());

    // 设置纯色背景
    ui->container1->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                    .arg(config.backgroundColor.red())
                                    .arg(config.backgroundColor.green())
                                    .arg(config.backgroundColor.blue()));

    LWVersionInfo v;
    LWGetLibVersion(&v);
    ui->sdk->setText(QString("%1.%2.%3.%4").arg(v.major).arg(v.minor).arg(v.patch).arg(v.reserved));
    mProcess.setWorkingDirectory(QApplication::applicationDirPath());

    // 开启设备管理子线程
    mDeviceWorker.moveToThread(&mDeviceThread);
    mDeviceThread.start();

    // 关联信号槽
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onQProcessInfo);
    connect(ui->search, &QPushButton::clicked, this, &MainWindow::onFindDevice);
    connect(ui->search, &QPushButton::clicked, &mDeviceWorker, &DeviceWorker::onFindDevice);
    connect(ui->link, &QPushButton::clicked, this, &MainWindow::onLink);
    connect(ui->execute, &QPushButton::clicked, this, &MainWindow::onExecute);
    connect(ui->adjustView, &QPushButton::clicked, this, &MainWindow::onResetCamera);
    connect(ui->overlookView, &QPushButton::clicked, this, &MainWindow::onResetCamera);
    connect(ui->leftView, &QPushButton::clicked, this, &MainWindow::onResetCamera);
    connect(ui->frontView, &QPushButton::clicked, this, &MainWindow::onResetCamera);
    connect(ui->recordLocation, &QPushButton::clicked, this, &MainWindow::onRecordLocation);
    connect(ui->calibrateArea, &QPushButton::clicked, this, &MainWindow::onCalibration);
    connect(ui->calibrationCompleted, &QPushButton::clicked, this, [this]{ onCalibrationStep(1); });
    connect(ui->saveAreaConfig, &QPushButton::clicked, this, &MainWindow::onSaveAreaConfig);
    connect(ui->removeArea, &QPushButton::clicked, this, &MainWindow::onRemoveArea);
    connect(ui->removeGroup, &QPushButton::clicked, this, &MainWindow::onRemoveGroup);
    connect(ui->lockView, &QPushButton::clicked, this, &MainWindow::onLock3DView);
    connect(ui->area3dDrawSave, &QPushButton::clicked, this, &MainWindow::onArea3dDrawSave);
    connect(ui->enter3dDrawMode, &QPushButton::clicked, this, &MainWindow::onEnter3dDrawMode);
    connect(ui->leave3dDrawMode, &QPushButton::clicked, this, &MainWindow::onLeave3dDrawMode);
    connect(ui->calibrationCancel, &QPushButton::clicked, &mDeviceWorker, &DeviceWorker::onCancelCalibration);
    connect(ui->note1, &QPushButton::clicked, this, &MainWindow::onNote);
    connect(ui->note2, &QPushButton::clicked, this, &MainWindow::onNote);
    connect(ui->viewMode, &QRadioButton::toggled, this, &MainWindow::onViewModeClicked);
    connect(ui->editMode, &QRadioButton::toggled, this, &MainWindow::onEditModeClicked);
    connect(ui->calibrationMode, &QRadioButton::toggled, this, &MainWindow::onCalibrationModeClicked);
    connect(ui->pointMoveEnable, &QRadioButton::toggled, this, &MainWindow::onPointMoveEnable);
    connect(ui->pointPlaneMoveEnable, &QRadioButton::toggled, this, &MainWindow::onPointPlaneMoveEnable);
    connect(ui->individualMoveEnable, &QRadioButton::toggled, this, &MainWindow::onIndividualMoveEnable);
    connect(ui->heightSetEnable, &QRadioButton::toggled, this, &MainWindow::onHeightMoveEnable);
    connect(ui->individualAxialMoveEnable, &QRadioButton::toggled, this, &MainWindow::onIndividualAxialMoveEnable);
    connect(ui->groupNumber, &QComboBox::currentIndexChanged, this, &MainWindow::onGroupNumberChanged);
    connect(ui->areaNumber, &QComboBox::currentIndexChanged, this, &MainWindow::onAreaNumberChanged);
    connect(ui->drawModel, &QComboBox::currentIndexChanged, this, &MainWindow::onDrawModelChanged);
    connect(ui->drawModel1, &QComboBox::currentIndexChanged, this, &MainWindow::onDrawModelChanged);
    connect(ui->axisDirec, &QComboBox::currentIndexChanged, this, &MainWindow::onPerspectiveTransfer);
    connect(ui->axial, &QComboBox::currentIndexChanged, this, &MainWindow::onPotTransfer);
    connect(ui->levelDirec, &QSpinBox::valueChanged, this, &MainWindow::onPerspectiveTransfer);
    connect(ui->degrees, &QDoubleSpinBox::valueChanged, this, &MainWindow::onPotTransfer);
    connect(ui->updateFirmware, &QAction::triggered, this, &MainWindow::onUpdateFirmware);
    connect(ui->logWindow, &QAction::triggered, this, &MainWindow::onLogForm);
    connect(ui->depthWindow, &QAction::triggered, this, &MainWindow::onDepthWindow);
    connect(ui->pyramidsOpacity, &QAction::triggered, this, &MainWindow::onPyramidsOpacity);
    connect(ui->overlapOpacity, &QAction::triggered, this, &MainWindow::onOverlapOpacity);
    connect(ui->non_overlapOpacity, &QAction::triggered, this, &MainWindow::onNonoverlapOpacity);
    connect(ui->pointSize, &QAction::triggered, this, &MainWindow::onPointSize);
    connect(ui->expertModel, &QAction::triggered, this, &MainWindow::onExpertModel);
    connect(ui->pullLog, &QAction::triggered, this, &MainWindow::onPullLog);
    connect(ui->recover, &QAction::triggered, this, &MainWindow::onRecoverInit);
    connect(ui->netSet, &QAction::triggered, this, &MainWindow::onNetworkSet);
    connect(ui->rulerEnable, &QAction::triggered, this, &MainWindow::onRulerEnable);
    connect(ui->vtkTextLabels, &QAction::triggered, this, &MainWindow::onPotTextLabels);
    connect(ui->soundLightAlarm, &QAction::triggered, this, &MainWindow::onSetSoundLightAlarm);
    connect(ui->textureMap, &QAction::triggered, this, &MainWindow::onTextureMapEnable);
    connect(ui->impressum, &QAction::triggered, this, &MainWindow::onImpressum);
    connect(ui->changeText, &QAction::triggered, this, &MainWindow::onChangeText);
    connect(ui->reboot, &QAction::triggered, &mDeviceWorker, &DeviceWorker::onReboot);
    connect(ui->timeSync, &QAction::triggered, &mDeviceWorker, &DeviceWorker::onSynchronizeTime);
    connect(ui->paraExport, &QAction::triggered, this, &MainWindow::onSelectExportPath);
    connect(ui->paraImport, &QAction::triggered, this, &MainWindow::onSelectImportFile);
    connect(ui->projectionTransform, &QAction::triggered, this, &MainWindow::onProjectionTransform);
    connect(ui->potBackgroundColor, &QAction::triggered, this, &MainWindow::onPotBackgroundColor);
    connect(ui->chinese, &QAction::triggered, this, &MainWindow::onSwitchLanguage);
    connect(ui->english, &QAction::triggered, this, &MainWindow::onSwitchLanguage);
    connect(ui->irGammaValue, &QSlider::sliderReleased, this, [this]{emit setIRGammaValue(ui->irGammaValue->value());});
    connect(ui->exposureLevel, &QSlider::sliderReleased, this, [this]{mDeviceArgSetForm.setExposureLevel(ui->exposureLevel->value());});
    connect(ui->irGammaValue,  &QSlider::valueChanged, this, [this](int v){ ui->irGammaValue->setToolTip(QString::number(v)); });
    connect(ui->exposureLevel, &QSlider::valueChanged, this, [this](int v){ ui->exposureLevel->setToolTip(QString::number(v));});
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::saveDeviceConfig, this, &MainWindow::onSaveConfigureInfo);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setExposure, &mDeviceWorker, &DeviceWorker::onSetExposure);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setExposure, this, [this](int v){ui->exposureLevel->setValue(v/40);});
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setFrameRate, &mDeviceWorker, &DeviceWorker::onSetFrameRate);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setSpatialFilter, &mDeviceWorker, &DeviceWorker::onSetSpatialFilter);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setTimeFilter, &mDeviceWorker, &DeviceWorker::onSetTimeFilter);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setTimeFilter_, &mDeviceWorker, &DeviceWorker::onSetTimeFilter_);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setFlyPixeFilter, &mDeviceWorker, &DeviceWorker::onSetFlyPixeFilter);
    connect(&mDeviceArgSetForm, &DeviceArgSetForm::setConfidenceFilter, &mDeviceWorker, &DeviceWorker::onSetConfidenceFilter);
    connect(&mNetworkSetForm, &NetworkSetForm::setNetwork, &mDeviceWorker, &DeviceWorker::onSetNetwork);
    connect(&mDeviceWorker, &DeviceWorker::deviceList, this, &MainWindow::onDeviceList);
    connect(&mDeviceWorker, &DeviceWorker::deviceInfo, this, &MainWindow::onDeviceInfo);
    connect(&mDeviceWorker, &DeviceWorker::connectStatus, this, &MainWindow::onConnectStatus);
    connect(&mDeviceWorker, &DeviceWorker::runStatus, this, &MainWindow::onRunStatus);
    connect(&mDeviceWorker, &DeviceWorker::graData, this, &MainWindow::onGraData);
    connect(&mDeviceWorker, &DeviceWorker::depData, this, &MainWindow::onDepData);
    connect(&mDeviceWorker, &DeviceWorker::potData, this, &MainWindow::onPotData);
    connect(&mDeviceWorker, &DeviceWorker::roiConfigUpdate, this, &MainWindow::onUpdateAreaInfo);
    connect(&mDeviceWorker, &DeviceWorker::executionError, this, &MainWindow::onExecutionError);
    connect(&mDeviceWorker, &DeviceWorker::calibrationStep, this, &MainWindow::onCalibrationStep);
    connect(&mDeviceWorker, &DeviceWorker::soundLightAlarmEnable, this, &MainWindow::onSoundLightAlarmEnable);
    connect(&mDeviceWorker, &DeviceWorker::replyText, &mLogForm, &LogForm::onLogInfo);
    connect(&mDeviceWorker, &DeviceWorker::executionError, &mLogForm, &LogForm::onLogInfo);
    connect(this, &MainWindow::initJsonFile, &mDeviceWorker, &DeviceWorker::onInitJsonFile);
    connect(this, &MainWindow::setPerspectiveMatrix, &mDeviceWorker, &DeviceWorker::onSetPerspectiveMatrix);
    connect(this, &MainWindow::setCalibrationARG, &mDeviceWorker, &DeviceWorker::onSetCalibrationARG);
    connect(this, &MainWindow::linkSwitch, &mDeviceWorker, &DeviceWorker::onConnectionEnable);
    connect(this, &MainWindow::start, &mDeviceWorker, &DeviceWorker::onStart);
    connect(this, &MainWindow::stop, &mDeviceWorker, &DeviceWorker::onStop);
    connect(this, &MainWindow::stopStream, &mDeviceWorker, &DeviceWorker::onStopStream);
    connect(this, &MainWindow::readyMe, &mDeviceWorker, &DeviceWorker::onReadyMe);
    connect(this, &MainWindow::pullLog, &mDeviceWorker, &DeviceWorker::onPullLog);
    connect(this, &MainWindow::setIRGammaValue, &mDeviceWorker, &DeviceWorker::onSetIRGammaValue);
    connect(this, &MainWindow::setRotationAxis, &mDeviceWorker, &DeviceWorker::onSetRotationAxis);
    connect(this, &MainWindow::setSoundLightAlarm, &mDeviceWorker, &DeviceWorker::onSetSoundLightAlarm);
    connect(this, &MainWindow::setSecurityConfigInfo, &mDeviceWorker, &DeviceWorker::onSetSecurityConfigInfo);
    connect(this, &MainWindow::exportJsonFile, &mDeviceWorker, &DeviceWorker::onExportJsonFile);
    connect(this, &MainWindow::importJsonFile, &mDeviceWorker, &DeviceWorker::onImportJsonFile);
    connect(this, &MainWindow::saveConfigFile, &mDeviceWorker, &DeviceWorker::onSaveConfigureInfo);
    connect(this, &MainWindow::setSecurity3DInfo, &mDeviceWorker, &DeviceWorker::onSetSecurity3DInfo);
    connect(this, &MainWindow::senToLogPanel, &mLogForm, &LogForm::onLogInfo);


    enableAlarmVisualPrintDetails(false);
    initVtkWindow();
}

MainWindow::~MainWindow()
{
    mDeviceThread.quit();
    mDeviceThread.wait();

    delete ui;
}

void MainWindow::initVtkWindow()
{
    // 全局关闭 VTK 的警告和错误信息
    vtkObject::SetGlobalWarningDisplay(0);
    // 向渲染窗口添加场景
    ui->potWidget->renderWindow()->AddRenderer(mRenderer);
    // 设置VTK交互样式，用于3D绘制
    mStyle->SetVertexActor(mVertexActor);
    mStyle->SetDefaultRenderer(mRenderer);
    ui->potWidget->interactor()->SetInteractorStyle(mStyle);
    // 绘制坐标轴，用于表示场景的当前的呈现视角
    vtkNew<vtkAxesActor> axes;
    mAxesWidget->SetOrientationMarker(axes);
    mAxesWidget->SetInteractor(ui->potWidget->interactor());
    mAxesWidget->SetViewport(0, 0, 0.1, 0.2);
    mAxesWidget->EnabledOn();
    mAxesWidget->InteractiveOff();
    // 创建点云的相关信息
    mColors->SetNumberOfComponents(3);
    mColors->SetNumberOfTuples(TOF_PIX_NUMBER);
    vtkNew<vtkPolyData> potPolydata;
    mPoints->SetNumberOfPoints(TOF_PIX_NUMBER);
    potPolydata->SetPoints(mPoints);
    potPolydata->GetPointData()->SetScalars(mColors);
    vtkNew<vtkVertexGlyphFilter> vertex;
    vertex->SetInputData(potPolydata);
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(vertex->GetOutputPort());
    mPointActor->SetMapper(mapper);
    mPointActor->GetProperty()->SetPointSize(config.pointSize);
    // 创建视锥点集
    mPyramidPots->SetNumberOfPoints(5);
    mPyramidPots->SetPoint(0, 0, 0, 0);
    mPyramidPots->SetPoint(1,  6000*0.70021,  6000*0.46631, 6000);
    mPyramidPots->SetPoint(2, -6000*0.70021,  6000*0.46631, 6000);
    mPyramidPots->SetPoint(3, -6000*0.70021, -6000*0.46631, 6000);
    mPyramidPots->SetPoint(4,  6000*0.70021, -6000*0.46631, 6000);
    // 创建视锥单元
    vtkNew<vtkCellArray> pyramidCells;
    for (int i = 0; i < 4; ++i) pyramidCells->InsertNextCell(3, pyramIdType[i]);
    pyramidCells->InsertNextCell(4, pyramIdType[4]);
    vtkNew<vtkPolyData> pyramidPolydata;
    pyramidPolydata->SetPoints(mPyramidPots);
    pyramidPolydata->SetPolys(pyramidCells);
    // 用于后续判定点是否在视锥里面
    mSelect->SetInput(pyramidPolydata);
    // 将视锥区域三角形面片化
    vtkNew<vtkTriangleFilter> ptriangleFilter;
    ptriangleFilter->SetInputData(pyramidPolydata);
    ptriangleFilter->Update();
    // 创建视锥演员管线
    vtkNew<vtkPolyDataMapper> pmapper;
    pmapper->SetInputConnection(ptriangleFilter->GetOutputPort());
    mPyramidActor->SetMapper(pmapper);
    mPyramidActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    mPyramidActor->GetProperty()->SetOpacity(config.pyramidOpacity);
    mPyramidActor->Modified();
    mRenderer->AddActor(mPyramidActor);
    // 创建动态包围盒坐标轴，用于估测演员尺寸
    mCubeAxesActor->SetBounds(mPyramidActor->GetBounds()); // 自动匹配物体包围盒
    mCubeAxesActor->SetCamera(mRenderer->GetActiveCamera()); // 绑定相机视角
    mCubeAxesActor->GetTitleTextProperty(0)->SetColor(1, 1, 0); // X 轴标题颜色
    mCubeAxesActor->GetLabelTextProperty(0)->SetColor(1, 1, 0); // X 轴标签颜色
    mCubeAxesActor->GetTitleTextProperty(1)->SetColor(1, 1, 0); // Y 轴标题颜色
    mCubeAxesActor->GetLabelTextProperty(1)->SetColor(1, 1, 0); // Y 轴标签颜色
    mCubeAxesActor->GetTitleTextProperty(2)->SetColor(1, 1, 0); // Z 轴标题颜色
    mCubeAxesActor->GetLabelTextProperty(2)->SetColor(1, 1, 0); // Z 轴标签颜色
    // 创建8个绘制区域
    for (int i = 0; i < 8; ++i)
    {
        // 创建点、单元
        vtkNew<vtkCellArray> cells;
        for (int j = 0; j < 8; ++j) mCubePots[i]->InsertPoint(j, 0, 0, 0);
        for (int i = 0; i < 6; ++i) cells->InsertNextCell(4, polyIdType[i]);
        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(mCubePots[i]);
        polyData->SetPolys(cells);
        // 用于判定点云是否在立方体里面
        mSelects[i]->SetInputData(potPolydata);
        mSelects[i]->SetSurfaceData(polyData);
        // 将绘制区域三角形面片化
        vtkNew<vtkTriangleFilter> ctriangleFilter;
        ctriangleFilter->SetInputData(polyData);
        ctriangleFilter->Update();
        // 判定安防区域的8个顶点与视锥的包含关系
        vtkNew<vtkPolyData> pointSet;
        pointSet->SetPoints(mCubePots[i]);
        mPSelects[i]->SetInputData(pointSet);
        mPSelects[i]->SetSurfaceData(pyramidPolydata);
        mPSelects[i]->SetTolerance(0.0001);
        mPSelects[i]->Update();
        // 设置视锥与安防区域的交集实体属性
        mIntersectionActor[i]->GetProperty()->SetColor(1.0, 1.0, 1.0);
        mIntersectionActor[i]->GetProperty()->SetOpacity(config.overlapOpacity);
        // 创建安防区域实体数据管线
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->ScalarVisibilityOff();
        mapper->SetInputConnection(ctriangleFilter->GetOutputPort());
        mCubeActor[i]->SetMapper(mapper);
        mCubeActor[i]->GetProperty()->SetColor(0.8, 0.8, 0.8);
        mCubeActor[i]->GetProperty()->SetOpacity(config.nonOverlapOpacity);
        mNonZeroArr[i] = false;
        // 视设置（安防区域的）文字标签相关信息
        mCubeTextActor[i]->GetTextProperty()->SetFontSize(12);
        mCubeTextActor[i]->SetInput("");
        mCubeTextActor[i]->SetPosition(0, 0, 0);
        mRenderer->AddActor(mCubeTextActor[i]);
        // 建立设置（安防区域的）线框实体数据管线及其相关属性
        vtkNew<vtkCellArray> lines;
        lines->InsertNextCell({0, 1});
        lines->InsertNextCell({1, 2});
        lines->InsertNextCell({2, 3});
        lines->InsertNextCell({3, 0});
        lines->InsertNextCell({4, 5});
        lines->InsertNextCell({5, 6});
        lines->InsertNextCell({6, 7});
        lines->InsertNextCell({7, 4});
        lines->InsertNextCell({0, 4});
        lines->InsertNextCell({1, 5});
        lines->InsertNextCell({2, 6});
        lines->InsertNextCell({3, 7});
        vtkNew<vtkPolyData> wpolyData;
        wpolyData->SetPoints(mCubePots[i]);
        wpolyData->SetLines(lines);
        vtkNew<vtkPolyDataMapper> wmapper;
        wmapper->SetInputData(wpolyData);
        mWireframeActor[i]->SetMapper(wmapper);
        mWireframeActor[i]->GetProperty()->SetColor(1.0, 1.0, 1.0);
        mWireframeActor[i]->GetProperty()->SetLineWidth(2.0);
        mWireframeActor[i]->GetProperty()->SetOpacity(0);
    }
    // 与3D绘制相关的初始化
    mFlag = 0;
    vertexActorUpdate(mFlag);
    mVertexActor->GetProperty()->SetPointSize(10);
    // 加载DM相机模型
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName("dm.stl");
    reader->Update();
    vtkNew<vtkPolyDataMapper> dm_mapper;
    dm_mapper->SetInputConnection(reader->GetOutputPort());
    mDMActor->SetPosition(-54, -30, -40);
    mDMActor->SetMapper(dm_mapper);
    mDMActor->GetProperty()->SetColor(0.6, 0.6, 0.6);
    mRenderer->AddActor(mDMActor);
    // 初始化相机位置
    auto camera = mRenderer->GetActiveCamera();
    camera->SetParallelProjection(true);
    mRenderer->ResetCamera();
    camera->SetViewUp(config.up);
    camera->SetFocalPoint(config.fp);
    camera->SetPosition(config.ps);
    camera->SetClippingRange(config.dr);
    // 设置背景颜色
    mRenderer->SetBackground(config.backgroundColor.redF(), config.backgroundColor.greenF(), config.backgroundColor.blueF());
    // 刷新VTK窗口
    ui->potWidget->renderWindow()->Render();

}

double MainWindow::doubleInputDialog(const QString &title, const QString &label, double value, double min, double max, int decimals, Qt::WindowFlags flags, double step)
{
    QInputDialog dialog(nullptr);
    dialog.setWindowIcon(this->windowIcon());
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setDoubleMinimum(min);
    dialog.setDoubleMaximum(max);
    dialog.setDoubleValue(value);
    dialog.setDoubleDecimals(decimals);
    dialog.setDoubleStep(step);

    auto retvalue = value;
    if(dialog.exec()) retvalue = dialog.doubleValue();

    return retvalue;
}

void MainWindow::vertexActorUpdate(int index)
{
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(3);
    colors->SetName("PointColors");
    colors->SetNumberOfTuples(8);
    for(int i = 0; i < 8; ++i) colors->SetTuple3(i, 0, 115, 157);
    vtkNew<vtkPolyData> polydata;
    polydata->SetPoints(mCubePots[index]);
    polydata->GetPointData()->SetScalars(colors);
    vtkNew<vtkVertexGlyphFilter> vertex;
    vertex->SetInputData(polydata);
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(vertex->GetOutputPort());
    mVertexActor->SetMapper(mapper);
}

void MainWindow::restoreAreaInfo()
{
    ui->areaEnable->setChecked(ARG.Group[mGroupNum].ROI[mAreaNum].enable);
    ui->areaAttribute->setCurrentIndex(ARG.Group[mGroupNum].ROI[mAreaNum].attribute-1);
    ui->sensitivity->setValue(ARG.Group[mGroupNum].ROI[mAreaNum].trsdSensitivityRate.low);
}

bool MainWindow::isAreaInfoChange()
{
    return (mIRItem.isAreaChange()
            || (ARG.Group[mGroupNum].ROI[mAreaNum].enable != (int)ui->areaEnable->isChecked())
            || (ARG.Group[mGroupNum].ROI[mAreaNum].attribute != (ui->areaAttribute->currentIndex()+1))
            || (ARG.Group[mGroupNum].ROI[mAreaNum].trsdSensitivityRate.low != ui->sensitivity->value())
            );
}

bool MainWindow::is3DAreaChange()
{
    int h1, h2;
    double pos[3];
    for (int k = 0, l = 0; k < 4; ++k, l += 2)
    {
        mCubePots[mAreaNum]->GetPoint(k, pos);
        if(((int)ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l] != (int)pos[0])
            || ((int)ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l+1] != (int)pos[1]))
            return true;
    }

    h1 = pos[2];
    mCubePots[mAreaNum]->GetPoint(5, pos);
    h2 = pos[2];

    return h1 < h2
               ? ((ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d1 != h1)
                  || (ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d2 != h2))
               : ((ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d1 != h2)
                  || (ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d2 != h1));
}

void MainWindow::dmUpdate()
{
    double rotationMatrix4x4[16] = {
        ARG.Calibration_Matrix[0], ARG.Calibration_Matrix[1], ARG.Calibration_Matrix[2], 0,
        ARG.Calibration_Matrix[3], ARG.Calibration_Matrix[4], ARG.Calibration_Matrix[5], 0,
        ARG.Calibration_Matrix[6], ARG.Calibration_Matrix[7], ARG.Calibration_Matrix[8], 0,
        0, 0, 0, 1
    };
    vtkNew<vtkTransform> tr;
    tr->SetMatrix(rotationMatrix4x4);

    mDMActor->SetUserTransform(tr);
    mDMActor->Modified();
}

bool MainWindow::isQuadrilateral(vtkPoints *pots)
{
    QPointF p1{pots->GetPoint(0)[0], pots->GetPoint(0)[1]};
    QPointF p2{pots->GetPoint(1)[0], pots->GetPoint(1)[1]};
    QPointF p3{pots->GetPoint(2)[0], pots->GetPoint(2)[1]};
    QPointF p4{pots->GetPoint(3)[0], pots->GetPoint(3)[1]};

    // 计算四个三点组的叉积
    auto cross1 = (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x());
    auto cross2 = (p3.x() - p2.x()) * (p4.y() - p2.y()) - (p3.y() - p2.y()) * (p4.x() - p2.x());
    auto cross3 = (p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x());
    auto cross4 = (p1.x() - p4.x()) * (p2.y() - p4.y()) - (p1.y() - p4.y()) * (p2.x() - p4.x());

    // 检查是否四点共线、交叉
    if ((cross1 == 0 && cross2 == 0)
        || (((cross1 > 0 && cross2 < 0) || (cross1 < 0 && cross2 > 0)) &&
            ((cross3 > 0 && cross4 < 0) || (cross3 < 0 && cross4 > 0)))
        )
        return false;

    // 计算四个三点组的叉积
    cross1 = (p3.x() - p2.x()) * (p4.y() - p2.y()) - (p3.y() - p2.y()) * (p4.x() - p2.x());
    cross2 = (p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x());
    cross3 = (p1.x() - p4.x()) * (p2.y() - p4.y()) - (p1.y() - p4.y()) * (p2.x() - p4.x());
    cross4 = (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x());

    // 判断交叉
    return !(((cross1 > 0 && cross2 < 0) || (cross1 < 0 && cross2 > 0)) &&
             ((cross3 > 0 && cross4 < 0) || (cross3 < 0 && cross4 > 0)));
}

int MainWindow::concaveVertex(vtkPoints *pots)
{
    QPointF p1{pots->GetPoint(0)[0], pots->GetPoint(0)[1]};
    QPointF p2{pots->GetPoint(1)[0], pots->GetPoint(1)[1]};
    QPointF p3{pots->GetPoint(2)[0], pots->GetPoint(2)[1]};
    QPointF p4{pots->GetPoint(3)[0], pots->GetPoint(3)[1]};
    // 计算四个三点组的叉积
    auto cross1 = (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x());
    auto cross2 = (p3.x() - p2.x()) * (p4.y() - p2.y()) - (p3.y() - p2.y()) * (p4.x() - p2.x());
    auto cross3 = (p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x());
    auto cross4 = (p1.x() - p4.x()) * (p2.y() - p4.y()) - (p1.y() - p4.y()) * (p2.x() - p4.x());
    // 获取叉积符号
    int sign0 = (cross1 > 0) ? 1 : (cross1 < 0) ? -1 : 0;
    int sign1 = (cross2 > 0) ? 1 : (cross2 < 0) ? -1 : 0;
    int sign2 = (cross3 > 0) ? 1 : (cross3 < 0) ? -1 : 0;
    int sign3 = (cross4 > 0) ? 1 : (cross4 < 0) ? -1 : 0;
    // 凹点判定
    if (sign0 != sign1 && sign0 != sign2 && sign0 != sign3) return 1;
    else if (sign1 != sign0 && sign1 != sign2 && sign1 != sign3) return 2;
    else if (sign2 != sign0 && sign2 != sign1 && sign2 != sign3) return 3;
    else if (sign3 != sign0 && sign3 != sign1 && sign3 != sign2) return 0;
    else  return -1;
}

void MainWindow::onFindDevice()
{
    ui->search->setEnabled(false);
    ui->deviceList->clear();
}

void MainWindow::onDeviceList(QStringList arg)
{
    ui->deviceList->addItems(arg);
    ui->search->setEnabled(true);
}

void MainWindow::onDeviceInfo(QString fw, QString dv, QString sn, QString ip, QString type, LWSensorIntrinsicParam para)
{
    ui->fw->setText(fw);
    ui->dv->setText(dv);
    ui->sn->setText(QString(sn.trimmed()));
    ui->type->setText(QString(type.trimmed()));

    mInParam.cx = para.cx;
    mInParam.cy = para.cy;
    mInParam.fx = para.fx;
    mInParam.fy = para.fy;
    mInParam.k1 = para.k1;
    mInParam.k2 = para.k2;
    mInParam.k3 = para.k3;

    mDeviceArgSetForm.setArgUI();
}

void MainWindow::onLink()
{
    if(ui->deviceList->count() < 1)
    {
        DialogForm().warning(tr("设备列表为空！请点击“搜索”按钮查找可链接设备"), tr("警告"));
        return;
    }

    if(ui->link->text() == tr("连接"))
    {
        emit linkSwitch(true, ui->deviceList->currentIndex());
    }
    else
    {
        emit linkSwitch(false);
    }
}

void MainWindow::onExecutionError(QString arg)
{
    DialogForm().critical(arg, tr("错误"));
}

void MainWindow::onSaveConfigureInfo()
{
    if (DialogForm().question(
            tr("确定要执行此操作吗？执行后会将界面参数写入设备，设备每次启动时均会按此参数运行！"),
            tr("待确认")) == QMessageBox::Ok)
    {
        emit saveConfigFile();
    }
}

void MainWindow::onConnectStatus(bool arg1, QString arg2)
{
    if(arg1)
    {
        ui->link->setText(arg2);
        ui->search->setEnabled(false);
        ui->deviceList->setEnabled(false);
        ui->widget_1->setEnabled(true);

        ui->irGammaValue->setEnabled(true);
        ui->exposureLevel->setEnabled(true);
        ui->exposureLevel->blockSignals(true);
        ui->exposureLevel->setValue(config.exposure[0]/40);
        ui->exposureLevel->setToolTip(QString::number(config.exposure[0]/40));
        ui->exposureLevel->blockSignals(false);
    }
    else
    {
        ui->search->setEnabled(true);
        ui->deviceList->setEnabled(true);
        ui->link->setText(arg2);
        ui->type->clear();
        ui->fw->clear();
        ui->dv->clear();
        ui->sn->clear();
        ui->groupLabel->clear();
        ui->areaDrawLabel->clear();
        ui->groupEnableLabel->clear();
        ui->fps->setValue(0);
        ui->execute->setText(tr("开始"));
        ui->widget_1->setEnabled(false);
        ui->irGammaValue->setEnabled(false);
        ui->exposureLevel->setEnabled(false);
    }

    ui->alarmICO->setStyleSheet("image: url(:/image/alarm_off.svg);");
    ui->warningICO->setStyleSheet("image: url(:/image/warning_off.svg);");
}

void MainWindow::onExecute()
{
    if(ui->link->text() != tr("断开"))
    {
        DialogForm().warning(tr("请连接设备！"), tr("警告"));
        return;
    }

    if(ui->execute->text() == tr("开始"))
    {
        /// 判断是否支持当前固件版
        if(config.fvMajor != 3) {
            DialogForm().warning(
                tr("上位机不支持当前固件版本<%1>(只支持主版本为3的固件)，请升级到对应固件版本").arg(ui->fw->text()),
                tr("警告"));
            return;
        }

        emit start();
    }
    else
    {
        emit stop();
    }
}

void MainWindow::onRunStatus(bool arg)
{
    if(arg)
    {
        ui->link->setEnabled(false);
        ui->execute->setText(tr("停止"));
    }
    else
    {
        ui->link->setEnabled(true);
        ui->execute->setText(tr("开始"));
    }

    ui->alarmICO->setStyleSheet("image: url(:/image/alarm_off.svg);");
    ui->warningICO->setStyleSheet("image: url(:/image/warning_off.svg);");
}

void MainWindow::onGraData(LWFrameData &data, double arg)
{
    if(mNotTransfer)
    {
        mAlarmVisual.perspectiveTransformationIR_useMat(
            (uint8_t*)data.pFrameData,
            mIRPixels,
            ARG.Perspective_Matrix
            );

        // LWSaveDataAsCSVFile("C:\\Users\\12267\\Desktop\\DATA\\tempf\\gra.csv", &data);
        mIRItem.alarmZone(&data.pVariant->SecurityDetection.alarmAreaNumber[0], &data.pVariant->SecurityDetection.areaAttribute[0]);
    }
    else
    {
        auto ret = mAlarmVisual.perspectiveTransformationIR_useParams(
            (uint8_t*)data.pFrameData,
            mIRPixels,
            ui->axisDirec->currentIndex(),
            ui->levelDirec->value(),
            ARG.Perspective_Matrix
            );

        if(ret == alarmVisualReturnStatus::AlarmALGVisual_SUCCESS)
        {
            emit setPerspectiveMatrix();
        }
        else
        {
            DialogForm().warning(tr("透视变换失败！！！"), tr("警告"));
            emit senToLogPanel(getDescriptionOfAlarmVisualReturnStatus(ret));
        }

        mNotTransfer = true;
    }

    mIRItem.setPixmap(QPixmap::fromImage(mIRImage));
    mIRView->update();
}

void MainWindow::onDepData(LWFrameData &data, double)
{
    if(mDepthDrawForm.isVisible()) mDepthDrawForm.setData((uint16_t*)data.pFrameData);

    if(mAreaConvert)
    {
        mAreaConvert = false;
        float   dst[12];
        std::vector<int> ep;
        auto ret = mAlarmVisual.convertPerspectiveIRCornersTo3DPoints(
            (uint16_t*)data.pFrameData,
            ARG.Calibration_Matrix,
            mInParam,
            ARG.Group[mGroupNum].ROI[mAreaNum].srcArea,
            nullptr,
            dst,
            ep
            );

        mIRItem.anomalyPoint(ep);

        //
        // LWSaveDataAsCSVFile("C:\\Users\\12267\\Desktop\\DATA\\tempf\\depth.csv", &data);

        if((ret == alarmVisualReturnStatus::AlarmALGVisual_SUCCESS)
            || (ret == AlarmALGVisual_INVALID_POLYGON_OR_PIXEL_AUTO_DRAWED))
        {
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[0] = dst[0];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[1] = dst[1];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[2] = dst[3];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[3] = dst[4];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[4] = dst[6];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[5] = dst[7];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[6] = dst[9];
            ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[7] = dst[10];

            ARG.Group[mGroupNum].ROI[mAreaNum].enable = ui->areaEnable->isChecked();
            ARG.Group[mGroupNum].ROI[mAreaNum].attribute = ui->areaAttribute->currentIndex() + 1;
            ARG.Group[mGroupNum].ROI[mAreaNum].trsdSensitivityRate.low = ui->sensitivity->value();

            emit setSecurityConfigInfo();
            emit senToLogPanel(getDescriptionOfAlarmVisualReturnStatus(ret));

            if(ret == AlarmALGVisual_INVALID_POLYGON_OR_PIXEL_AUTO_DRAWED)
            {
                DialogForm().warning(
                    tr("所选区域存在缺陷(已用“×”标注出异常的选点)，虽已自适应绘制，可能不准确，建议重新绘制选区！！！"),
                    tr("警告"));
                return;
            }
        }
        else
        {
            DialogForm().warning(tr("绘制区域转换出现错误！！！"), tr("警告"));
            emit senToLogPanel(getDescriptionOfAlarmVisualReturnStatus(ret));
        }
    }
}

void MainWindow::onPotData(LWFrameData &data, LWFrameData &gra, double fps)
{
    //点云数据更新
    auto ptr = (float *)data.pFrameData;
    for (int i = 0, k = 0; i < TOF_PIX_NUMBER; ++i, k += 3)
    {
        mPoints->SetPoint(i, ptr[k], ptr[k+1], ptr[k+2]);
    }
    mPoints->Modified();
    // 视锥区域更新
    bool uflag = false;
    if (memcmp(data.pVariant->SecurityDetection.pyramid, mFloatArr1, 60) != 0)
    {
        // 修复BUG,因此点云的添加放在此处
        if(mRenderer->HasViewProp(mPointActor) == 0)
            mRenderer->AddActor(mPointActor);
        // *********
        auto ptr = &data.pVariant->SecurityDetection.pyramid[0];
        memcpy(mFloatArr1, ptr, 60);
        for (int i = 0, n = 0; i < 5; ++i, n+=3)
        {
            mPyramidPots->SetPoint(i, ptr[n], ptr[n+1], ptr[n+2]);
        }
        mPyramidPots->Modified();
        dmUpdate();
        // 更新标志量
        uflag = true;
    }
    // 报警区域绘制更新
    if (memcmp(data.pVariant->SecurityDetection.detectionArea, mFloatArr2, 320) != 0)
    {
        auto ptr = &data.pVariant->SecurityDetection.detectionArea[0];
        memcpy(mFloatArr2, ptr, 320);
        for (int i = 0; i < 8; ++i, ptr += 10)
        {
            auto z_min = ptr[8] < 150 ? 150 : ptr[8];
            auto z_max = ptr[9] < z_min ? z_min + 1 : ptr[9];
            // 判定是逆时针还是顺时针并设置正确的点数据
            double sum = 0.0;
            sum += (ptr[0] * ptr[3] - ptr[2] * ptr[1]);
            sum += (ptr[2] * ptr[5] - ptr[4] * ptr[3]);
            sum += (ptr[4] * ptr[7] - ptr[6] * ptr[5]);
            sum += (ptr[6] * ptr[1] - ptr[0] * ptr[7]);
            if(sum > 0)
            {
                mCubePots[i]->SetPoint(1, ptr[2], ptr[3], z_min);
                mCubePots[i]->SetPoint(3, ptr[6], ptr[7], z_min);
                mCubePots[i]->SetPoint(5, ptr[2], ptr[3], z_max);
                mCubePots[i]->SetPoint(7, ptr[6], ptr[7], z_max);
            }
            else
            {
                mCubePots[i]->SetPoint(3, ptr[2], ptr[3], z_min);
                mCubePots[i]->SetPoint(1, ptr[6], ptr[7], z_min);
                mCubePots[i]->SetPoint(7, ptr[2], ptr[3], z_max);
                mCubePots[i]->SetPoint(5, ptr[6], ptr[7], z_max);
            }
            mCubePots[i]->SetPoint(0, ptr[0], ptr[1], z_min);
            mCubePots[i]->SetPoint(2, ptr[4], ptr[5], z_min);
            mCubePots[i]->SetPoint(4, ptr[0], ptr[1], z_max);
            mCubePots[i]->SetPoint(6, ptr[4], ptr[5], z_max);
            mCubePots[i]->Modified();
            // 判定区域是否存在
            mNonZeroArr[i] = !std::all_of(ptr, ptr + 8, [](int x) { return x == 0; });
        }
        uflag = true;
    }
    if(uflag)
    {
        for (int i = 0; i < 8; ++i)
        {
            if(!mNonZeroArr[i])
            {
                // 移除相关演员
                if(mRenderer->HasViewProp(mIntersectionActor[i]) != 0)
                    mRenderer->RemoveActor(mIntersectionActor[i]);
                if(mRenderer->HasViewProp(mCubeActor[i]) != 0)
                    mRenderer->RemoveActor(mCubeActor[i]);
                if((i == mFlag) && (mRenderer->HasViewProp(mWireframeActor[mFlag]) != 0))
                    mRenderer->RemoveActor(mWireframeActor[mFlag]);

                continue;
            }
            // 添加相关演员
            if(mRenderer->HasViewProp(mCubeActor[i]) == 0)
                mRenderer->AddActor(mCubeActor[i]);
            if((i == mFlag) && (mRenderer->HasViewProp(mWireframeActor[mFlag]) == 0))
                mRenderer->AddActor(mWireframeActor[mFlag]);
            // 刷新判定区域（安防绘制顶点与视锥的包含关系）
            int flag = 0;
            mPSelects[i]->Update();
            for (int var = 0; var < 8; ++var) if(mPSelects[i]->IsInside(var)) ++flag;
            // 判断安防区域的凹凸性
            auto concaveIdx = concaveVertex(mCubePots[i]);
            if(concaveIdx  < 0)
            {
                // 创建安防区域点、单元
                vtkNew<vtkCellArray> cells;
                for (int i = 0; i < 6; ++i) cells->InsertNextCell(4, polyIdType[i]);
                vtkNew<vtkPolyData> polyData;
                polyData->SetPoints(mCubePots[i]);
                polyData->SetPolys(cells);
                // 将绘制区域三角形面片化
                vtkNew<vtkTriangleFilter> ctriangleFilter;
                ctriangleFilter->SetInputData(polyData);
                ctriangleFilter->Update();
                // 计算视锥与安防区域的交集
                vtkNew<vtkBooleanOperationPolyDataFilter> intersection;
                intersection->SetOperation(vtkBooleanOperationPolyDataFilter::VTK_INTERSECTION);
                intersection->SetInputConnection(0, mPyramidActor->GetMapper()->GetInputAlgorithm()->GetOutputPort());
                intersection->SetInputConnection(1, ctriangleFilter->GetOutputPort());
                intersection->Update();
                // 判定交集
                if((intersection->GetOutput()->GetNumberOfPoints() < 4) && (flag < 5))
                {
                    if(mRenderer->HasViewProp(mIntersectionActor[i]) != 0)
                        mRenderer->RemoveActor(mIntersectionActor[i]);
                    continue;
                }
                // 创建映射器
                vtkNew<vtkDataSetMapper> mapper;
                mapper->ScalarVisibilityOff();
                mapper->SetInputConnection(ctriangleFilter->GetOutputPort());
                // 用于判定点的包含关系
                vtkNew<vtkImplicitPolyDataDistance> select;
                select->SetInput(polyData);
                // 清理重复点与无效单元
                vtkNew<vtkCleanPolyData> cleanFilter;
                cleanFilter->SetInputData(intersection->GetOutput());
                cleanFilter->SetTolerance(0.005);
                cleanFilter->Update();
                // 处理“异常”点
                auto srcPots =  cleanFilter->GetOutput()->GetPoints();
                if(srcPots != nullptr)
                {
                    // 移除异常点
                    vtkNew<vtkPoints>   newPots;
                    auto potnmu =  srcPots->GetNumberOfPoints();
                    for (int var = 0, k = 0; var < potnmu; ++var)
                    {
                        auto vp = srcPots->GetPoint(var);
                        if((mSelect->FunctionValue(vp) < 0.5) && (fabs(select->FunctionValue(vp)) < 1))
                        {
                            newPots->InsertPoint(k++, vp);
                        }
                    }
                    if(newPots->GetNumberOfPoints() > 3)
                    {
                        // 将点集包装为 PolyData（Delaunay3D 的输入需要 vtkPointSet 类型）
                        vtkNew<vtkPolyData> inputPolyData;
                        inputPolyData->SetPoints(newPots);
                        // 执行 Delaunay 3D 三角剖分 ----------
                        vtkNew<vtkDelaunay3D> delaunay3D;
                        delaunay3D->SetInputData(inputPolyData);
                        delaunay3D->Update();
                        // 创建映射器（直接映射四面体网格）
                        mapper->SetInputData(delaunay3D->GetOutput());
                    }
                }

                mIntersectionActor[i]->SetMapper(mapper);
                if(mRenderer->HasViewProp(mIntersectionActor[i]) == 0)
                    mRenderer->AddActor(mIntersectionActor[i]);
            }
            else
            {
                vtkNew<vtkPoints> points0;
                vtkNew<vtkAppendPolyData> appendFilter;

                {
                    vtkNew<vtkPoints> points;
                    vtkNew<vtkCellArray> polys;
                    // 创建相关点集
                    if (concaveIdx == 0 || concaveIdx == 2)
                    {
                        points->InsertNextPoint(mCubePots[i]->GetPoint(0));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(1));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(2));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(4));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(5));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(6));

                        points0->InsertNextPoint(mCubePots[i]->GetPoint(0));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(2));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(6));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(4));
                    }
                    else
                    {
                        points->InsertNextPoint(mCubePots[i]->GetPoint(0));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(1));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(3));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(4));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(5));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(7));

                        points0->InsertNextPoint(mCubePots[i]->GetPoint(1));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(3));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(7));
                        points0->InsertNextPoint(mCubePots[i]->GetPoint(5));
                    }
                    // 创建所有面（2个三角形 + 3个四边形）
                    // 底面三角形
                    vtkNew<vtkTriangle> bottom;
                    bottom->GetPointIds()->SetId(0, 0);
                    bottom->GetPointIds()->SetId(1, 1);
                    bottom->GetPointIds()->SetId(2, 2);
                    // 顶面三角形
                    vtkNew<vtkTriangle> top;
                    top->GetPointIds()->SetId(0, 3);
                    top->GetPointIds()->SetId(1, 4);
                    top->GetPointIds()->SetId(2, 5);
                    // 侧面四边形1 (0-1-4-3)
                    vtkNew<vtkQuad> quad1;
                    quad1->GetPointIds()->SetId(0, 0);
                    quad1->GetPointIds()->SetId(1, 1);
                    quad1->GetPointIds()->SetId(2, 4);
                    quad1->GetPointIds()->SetId(3, 3);
                    // 侧面四边形2 (1-2-5-4)
                    vtkNew<vtkQuad> quad2;
                    quad2->GetPointIds()->SetId(0, 1);
                    quad2->GetPointIds()->SetId(1, 2);
                    quad2->GetPointIds()->SetId(2, 5);
                    quad2->GetPointIds()->SetId(3, 4);
                    // 侧面四边形3 (2-0-3-5)
                    vtkNew<vtkQuad> quad3;
                    quad3->GetPointIds()->SetId(0, 2);
                    quad3->GetPointIds()->SetId(1, 0);
                    quad3->GetPointIds()->SetId(2, 3);
                    quad3->GetPointIds()->SetId(3, 5);
                    // 添加到多边形集合
                    polys->InsertNextCell(bottom);
                    polys->InsertNextCell(top);
                    polys->InsertNextCell(quad1);
                    polys->InsertNextCell(quad2);
                    polys->InsertNextCell(quad3);
                    // 创建子区域数据
                    vtkNew<vtkPolyData> polyData;
                    polyData->SetPoints(points);
                    polyData->SetPolys(polys);
                    // 将绘制区域三角形面片化
                    vtkNew<vtkTriangleFilter> ctriangleFilter;
                    ctriangleFilter->SetInputData(polyData);
                    ctriangleFilter->Update();
                    // 计算交集
                    vtkNew<vtkBooleanOperationPolyDataFilter> intersection;
                    intersection->SetOperation(vtkBooleanOperationPolyDataFilter::VTK_INTERSECTION);
                    intersection->SetInputConnection(0, mPyramidActor->GetMapper()->GetInputAlgorithm()->GetOutputPort());
                    intersection->SetInputConnection(1, ctriangleFilter->GetOutputPort());
                    intersection->Update();
                    // 判定交集
                    if((intersection->GetOutput()->GetNumberOfPoints() < 1) && (flag < 5))
                    {
                        // 相离情形
                    }
                    else
                    {
                        // 用于判定点的包含关系
                        vtkNew<vtkImplicitPolyDataDistance> select;
                        select->SetInput(polyData);
                        // 清理重复点与无效单元
                        vtkNew<vtkCleanPolyData> cleanFilter;
                        cleanFilter->SetInputData(intersection->GetOutput());
                        cleanFilter->Update();
                        // 处理“异常”点
                        auto srcPots =  cleanFilter->GetOutput()->GetPoints();
                        if(srcPots != nullptr)
                        {
                            // 移除异常点
                            vtkNew<vtkPoints>   newPots;
                            auto potnmu =  srcPots->GetNumberOfPoints();
                            for (int var = 0, k = 0; var < potnmu; ++var)
                            {
                                auto vp = srcPots->GetPoint(var);
                                if((mSelect->FunctionValue(vp) < 0.5) && (fabs(select->FunctionValue(vp)) < 1))
                                {
                                    newPots->InsertPoint(k++, vp);
                                }
                            }
                            if(newPots->GetNumberOfPoints() > 0)
                            {
                                // 将点集包装为 PolyData（Delaunay3D 的输入需要 vtkPointSet 类型）
                                vtkNew<vtkPolyData> inputPolyData;
                                inputPolyData->SetPoints(newPots);
                                // 执行 Delaunay 3D 三角剖分 ----------
                                vtkNew<vtkDelaunay3D> delaunay3D;
                                delaunay3D->SetInputData(inputPolyData);
                                delaunay3D->Update();
                                // 提取面
                                vtkNew<vtkGeometryFilter> surfaceFilter1;
                                surfaceFilter1->SetInputConnection(delaunay3D->GetOutputPort());
                                surfaceFilter1->Update();
                                appendFilter->AddInputData(surfaceFilter1->GetOutput());
                            }
                        }
                        else
                        {
                            appendFilter->AddInputData(ctriangleFilter->GetOutput());
                        }
                    }
                }

                {
                    vtkNew<vtkPoints>   points;
                    vtkNew<vtkCellArray> polys;
                    // 创建点集
                    if (concaveIdx == 0 || concaveIdx == 2)
                    {
                        points->InsertNextPoint(mCubePots[i]->GetPoint(0));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(2));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(3));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(4));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(6));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(7));
                    }
                    else
                    {
                        points->InsertNextPoint(mCubePots[i]->GetPoint(1));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(2));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(3));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(5));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(6));
                        points->InsertNextPoint(mCubePots[i]->GetPoint(7));
                    }
                    // 创建所有面（2个三角形 + 3个四边形）
                    // 底面三角形
                    vtkNew<vtkTriangle> bottom;
                    bottom->GetPointIds()->SetId(0, 0);
                    bottom->GetPointIds()->SetId(1, 1);
                    bottom->GetPointIds()->SetId(2, 2);
                    // 顶面三角形
                    vtkNew<vtkTriangle> top;
                    top->GetPointIds()->SetId(0, 3);
                    top->GetPointIds()->SetId(1, 4);
                    top->GetPointIds()->SetId(2, 5);
                    // 侧面四边形1 (0-1-4-3)
                    vtkNew<vtkQuad> quad1;
                    quad1->GetPointIds()->SetId(0, 0);
                    quad1->GetPointIds()->SetId(1, 1);
                    quad1->GetPointIds()->SetId(2, 4);
                    quad1->GetPointIds()->SetId(3, 3);
                    // 侧面四边形2 (1-2-5-4)
                    vtkNew<vtkQuad> quad2;
                    quad2->GetPointIds()->SetId(0, 1);
                    quad2->GetPointIds()->SetId(1, 2);
                    quad2->GetPointIds()->SetId(2, 5);
                    quad2->GetPointIds()->SetId(3, 4);
                    // 侧面四边形3 (2-0-3-5)
                    vtkNew<vtkQuad> quad3;
                    quad3->GetPointIds()->SetId(0, 2);
                    quad3->GetPointIds()->SetId(1, 0);
                    quad3->GetPointIds()->SetId(2, 3);
                    quad3->GetPointIds()->SetId(3, 5);
                    // 添加到多边形集合
                    polys->InsertNextCell(bottom);
                    polys->InsertNextCell(top);
                    polys->InsertNextCell(quad1);
                    polys->InsertNextCell(quad2);
                    polys->InsertNextCell(quad3);
                    vtkNew<vtkPolyData> polyData;
                    polyData->SetPoints(points);
                    polyData->SetPolys(polys);
                    // 将绘制区域三角形面片化
                    vtkNew<vtkTriangleFilter> ctriangleFilter;
                    ctriangleFilter->SetInputData(polyData);
                    ctriangleFilter->Update();
                    // 计算交集
                    vtkNew<vtkBooleanOperationPolyDataFilter> intersection;
                    intersection->SetOperation(vtkBooleanOperationPolyDataFilter::VTK_INTERSECTION); // 设置为交集模式
                    intersection->SetInputConnection(0, mPyramidActor->GetMapper()->GetInputAlgorithm()->GetOutputPort());
                    intersection->SetInputConnection(1, ctriangleFilter->GetOutputPort());
                    intersection->Update();
                    // 判定交集
                    if((intersection->GetOutput()->GetNumberOfPoints() < 1) && (flag < 5))
                    {
                        // 相离情形
                    }
                    else
                    {
                        // 用于判定点的包含关系
                        vtkNew<vtkImplicitPolyDataDistance> select;
                        select->SetInput(polyData);
                        // 清理重复点与无效单元
                        vtkNew<vtkCleanPolyData> cleanFilter;
                        cleanFilter->SetInputData(intersection->GetOutput());
                        cleanFilter->Update();
                        // 处理“异常”点
                        auto srcPots =  cleanFilter->GetOutput()->GetPoints();
                        if(srcPots != nullptr)
                        {
                            // 移除异常点
                            vtkNew<vtkPoints>   newPots;
                            auto potnmu =  srcPots->GetNumberOfPoints();
                            for (int var = 0, k = 0; var < potnmu; ++var)
                            {
                                auto vp = srcPots->GetPoint(var);
                                if((mSelect->FunctionValue(vp) < 0.5) && (fabs(select->FunctionValue(vp)) < 1))
                                {
                                    newPots->InsertPoint(k++, vp);
                                }
                            }
                            if(newPots->GetNumberOfPoints() > 0)
                            {
                                // 将点集包装为 PolyData（Delaunay3D 的输入需要 vtkPointSet 类型）
                                vtkNew<vtkPolyData> inputPolyData;
                                inputPolyData->SetPoints(newPots);
                                // 执行 Delaunay 3D 三角剖分 ----------
                                vtkNew<vtkDelaunay3D> delaunay3D;
                                delaunay3D->SetInputData(inputPolyData);
                                delaunay3D->Update();
                                // 提取面
                                vtkNew<vtkGeometryFilter> surfaceFilter2;
                                surfaceFilter2->SetInputConnection(delaunay3D->GetOutputPort());
                                surfaceFilter2->Update();
                                appendFilter->AddInputData(surfaceFilter2->GetOutput());
                            }
                        }
                        else
                        {
                            appendFilter->AddInputData(ctriangleFilter->GetOutput());
                        }
                    }
                }

                // 移除多余面片
                double normal1[3];
                double normal2[3];
                vtkPolygon::ComputeNormal(points0, normal1);
                appendFilter->Update();
                int numCells = appendFilter->GetOutput()->GetNumberOfCells();
                if(numCells > 0)
                {
                    auto shell = appendFilter->GetOutput();
                    shell->BuildLinks();
                    for (int var = 0; var < numCells; ++var)
                    {
                        vtkPolygon::ComputeNormal(shell->GetCell(var)->GetPoints(), normal2);
                        auto v = vtkMath::Dot(normal1, normal2);
                        if((std::abs(std::abs(v) - 1.0) < 1e-6)) shell->DeleteCell(var);
                    }
                    shell->RemoveDeletedCells();
                }
                // 刷新数据管线并显示
                vtkNew<vtkDataSetMapper> mapper;
                mapper->SetInputConnection(appendFilter->GetOutputPort());
                mIntersectionActor[i]->SetMapper(mapper);
                if(mRenderer->HasViewProp(mIntersectionActor[i]) == 0)
                    mRenderer->AddActor(mIntersectionActor[i]);
            }
        }
    }
    // 更新区域警报、文字标识
    ptr = &data.pVariant->SecurityDetection.detectionArea[0];
    for (int i = 0; i < 8; ++i, ptr += 10)
    {
        if(mNonZeroArr[i])
        {
            mSelects[i]->Update();

            //
            mCubeTextActor[i]->SetInput(QString("(%1, %2)").arg(i+1).arg(data.pVariant->SecurityDetection.pots[i]).toLocal8Bit());
            mCubeTextActor[i]->SetPosition((ptr[0]+ptr[4])/2, (ptr[1]+ptr[5])/2, ptr[8]);

            // 重置警报标识
            if(ARG.Group[mGroupNum].ROI[i].enable)
            {
                auto color = config.colors[i];
                mIntersectionActor[i]->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
            }
            else
            {
                mIntersectionActor[i]->GetProperty()->SetColor(0.4, 0.4, 0.4);
            }
        }
        else
        {
            mCubeTextActor[i]->SetInput("");
        }

        mCubeTextActor[i]->Modified();
    }
    // 报警信号标识刷新
    ui->alarmICO->setStyleSheet("image: url(:/image/alarm_on.svg);");
    ui->warningICO->setStyleSheet("image: url(:/image/warning_on.svg);");
    for (uint16_t i = 0; i < data.pVariant->SecurityDetection.alarmAreaNumber[0]; ++i)
    {
        if(data.pVariant->SecurityDetection.areaAttribute[i+1] == 1)
        {
            ui->alarmICO->setStyleSheet("image: url(:/image/alarm.svg);");
            mIntersectionActor[data.pVariant->SecurityDetection.alarmAreaNumber[i+1]-1]->GetProperty()->SetColor(1.0, 0.0, 0.0);
        }
        else if(data.pVariant->SecurityDetection.areaAttribute[i+1] == 2)
        {
            ui->warningICO->setStyleSheet("image: url(:/image/warning.svg);");
            mIntersectionActor[data.pVariant->SecurityDetection.alarmAreaNumber[i+1]-1]->GetProperty()->SetColor(170/255.0, 0, 1.0);
        }
    }
    // 点云颜色更新
    if(!config.isTextureMapEnable)
    {
        for (int i = 0; i < TOF_PIX_NUMBER; ++i)
        {
            if ((mNonZeroArr[0] && mSelects[0]->IsInside(i))
                || (mNonZeroArr[1] && mSelects[1]->IsInside(i))
                || (mNonZeroArr[2] && mSelects[2]->IsInside(i))
                || (mNonZeroArr[3] && mSelects[3]->IsInside(i))
                || (mNonZeroArr[4] && mSelects[4]->IsInside(i))
                || (mNonZeroArr[5] && mSelects[5]->IsInside(i))
                || (mNonZeroArr[6] && mSelects[6]->IsInside(i))
                || (mNonZeroArr[7] && mSelects[7]->IsInside(i))
                )
                mColors->SetTuple3(i, 255, 0, 0);
            else
            {
                mColors->SetTuple3(i, 255, 255, 255);
            }
        }
    }
    else
    {
        auto graPtr = (unsigned char*)gra.pFrameData;
        for (int i = 0; i < TOF_PIX_NUMBER; ++i)
        {
            if ((mNonZeroArr[0] && mSelects[0]->IsInside(i))
                || (mNonZeroArr[1] && mSelects[1]->IsInside(i))
                || (mNonZeroArr[2] && mSelects[2]->IsInside(i))
                || (mNonZeroArr[3] && mSelects[3]->IsInside(i))
                || (mNonZeroArr[4] && mSelects[4]->IsInside(i))
                || (mNonZeroArr[5] && mSelects[5]->IsInside(i))
                || (mNonZeroArr[6] && mSelects[6]->IsInside(i))
                || (mNonZeroArr[7] && mSelects[7]->IsInside(i))
                )
                mColors->SetTuple3(i, 255, 0, 0);
            else
            {
                mColors->SetTuple3(i, graPtr[i], graPtr[i], graPtr[i]);
            }
        }
    }

    mColors->Modified();
    // VTK画面更新
    ui->potWidget->renderWindow()->Render();

    ui->fps->setValue(fps);

    QTimer::singleShot(15, this, [this]{ emit readyMe(); });
}

void MainWindow::onViewModeClicked(bool checked)
{
    mIRItem.setEditModel(!checked);

    if(checked)
    {
        mIRItem.setAreaNumber(mAreaNum);
        mIRItem.setDrawModel(ui->drawModel->currentIndex());
        ui->stackedWidget->setCurrentIndex(0);
        ui->page_1->setEnabled(false);
    }
}

void MainWindow::onEditModeClicked(bool checked)
{
    for (int var = 0; var < 8; ++var)
    {
        mWireframeActor[var]->GetProperty()->SetOpacity(checked);
    }

    if(checked)
    {
        mIRItem.setAreaNumber(mAreaNum);
        mIRItem.setDrawModel(ui->drawModel->currentIndex());
        ui->stackedWidget->setCurrentIndex(0);
        ui->page_1->setEnabled(true);
    }
}

void MainWindow::onCalibrationModeClicked(bool checked)
{
    mIRItem.setCalibrationModel(checked);

    if(checked)
    {
        mIRItem.setAreaNumber(8);
        mIRItem.setDrawModel(ui->drawModel1->currentIndex());
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::onPointMoveEnable(bool checked)
{
    mStyle->pointMoveEnable(checked);
}

void MainWindow::onPointPlaneMoveEnable(bool checked)
{
    mStyle->pointPlaneMoveEnable(checked);
}

void MainWindow::onIndividualMoveEnable(bool checked)
{
    mStyle->individualMoveEnable(checked);
}

void MainWindow::onHeightMoveEnable(bool checked)
{
    mStyle->heightMoveEnable(checked);
}

void MainWindow::onIndividualAxialMoveEnable(bool checked)
{
    mStyle->individualAxialMoveEnable(checked);
}

void MainWindow::onGroupNumberChanged(int index)
{
    if(mGroupNum == index) return;
    if (isAreaInfoChange())
    {
        if(DialogForm().question(
                tr("检测到你对区域 %1 进行了更改！按“确认”键则忽略此更改继续往下执行！").arg(mAreaNum+1),
                tr("待确认")) != QMessageBox::Ok)
        {
            ui->groupNumber->setCurrentIndex(mGroupNum);
            return;
        }
        mIRItem.restoreArea();
    }

    ARG.Enable_Group_Number = index + 1;
    mGroupNum = index;
    mIRItem.setGroupNumber(index);

    emit setSecurityConfigInfo();

    auto &GRP = ARG.Group[index];
    QString str;
    for (int i = 0; i < 8; ++i) if(GRP.ROI[i].enable) str += QString("%1 ").arg(i+1);
    ui->areaDrawLabel->setText(str);
    ui->groupLabel->setText(QString::number(index+1));
    ui->groupEnableLabel->setText(QString::number(index+1));
    ui->areaEnable->setChecked(GRP.ROI[mAreaNum].enable);
    ui->areaAttribute->setCurrentIndex(GRP.ROI[mAreaNum].attribute-1);
    ui->sensitivity->setValue(GRP.ROI[mAreaNum].trsdSensitivityRate.low);
}

void MainWindow::onAreaNumberChanged(int num)
{
    if(mAreaNum == num) return;
    if (isAreaInfoChange())
    {
        if(DialogForm().question(
                tr("检测到你对区域 %1 进行了更改！按“确认”键则忽略此更改继续往下执行！").arg(mAreaNum+1),
                tr("待确认")) != QMessageBox::Ok)
        {
            ui->areaNumber->setCurrentIndex(mAreaNum);
            return;
        }
        mIRItem.restoreArea();
    }

    mAreaNum = num;
    mIRItem.setAreaNumber(num);
    // 更新线框
    if(mFlag != num)
    {
        if(mRenderer->HasViewProp(mWireframeActor[mFlag]) != 0)
            mRenderer->RemoveActor(mWireframeActor[mFlag]);

        mFlag = num;
        if(mNonZeroArr[mFlag] && (mRenderer->HasViewProp(mWireframeActor[mFlag]) == 0))
            mRenderer->AddActor(mWireframeActor[mFlag]);

        ui->potWidget->renderWindow()->Render();
    }

    auto &GRP = ARG.Group[mGroupNum];

    ui->areaEnable->setChecked(GRP.ROI[num].enable);
    ui->areaAttribute->setCurrentIndex(GRP.ROI[num].attribute-1);
    ui->sensitivity->setValue(GRP.ROI[num].trsdSensitivityRate.low);
}

void MainWindow::onSaveAreaConfig()
{
    if(ui->execute->text() == tr("开始"))
    {
        DialogForm().warning(tr("请先点击\"开始\"按钮，开启数据流！！！"), tr("警告"));
        return;
    }

    ARG.Group[mGroupNum].ROI[mAreaNum].enable = ui->areaEnable->isChecked();
    ARG.Group[mGroupNum].ROI[mAreaNum].attribute = ui->areaAttribute->currentIndex() + 1;
    ARG.Group[mGroupNum].ROI[mAreaNum].trsdSensitivityRate.low = ui->sensitivity->value();

    QString str;
    for (int i = 0; i < 8; ++i)
        if(ARG.Group[mGroupNum].ROI[i].enable) str += QString("%1 ").arg(i+1);
    ui->areaDrawLabel->setText(str);

    if(mIRItem.isAreaChange())
    {
        mIRItem.loadDrawArea();
        mAreaConvert = true;
    }
    else
    {
        emit setSecurityConfigInfo();
    }
}

void MainWindow::onRemoveArea()
{
    auto &GRP    = ARG.Group[mGroupNum];

    for (int i = 0; i < 8; ++i)
    {
        GRP.ROI[mAreaNum].srcArea[i] = 0;
        GRP.ROI[mAreaNum].dstArea[i] = 0;
    }
    GRP.ROI[mAreaNum].enable = true;
    GRP.ROI[mAreaNum].attribute = 1;
    GRP.ROI[mAreaNum].depthRange.d1 = 150;
    GRP.ROI[mAreaNum].depthRange.d2 = 4500;
    GRP.ROI[mAreaNum].trsdSensitivityRate.low = 50;

    ui->areaEnable->setChecked(true);
    ui->areaAttribute->setCurrentIndex(0);
    ui->sensitivity->setValue(50);

    mIRItem.restoreArea();

    QString str;
    for (int i = 0; i < 8; ++i)
        if(ARG.Group[mGroupNum].ROI[i].enable) str += QString("%1 ").arg(i+1);
    ui->areaDrawLabel->setText(str);

    emit setSecurityConfigInfo();
}

void MainWindow::onRemoveGroup()
{
    auto &GRP    = ARG.Group[mGroupNum];

    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            GRP.ROI[i].srcArea[j] = 0;
            GRP.ROI[i].dstArea[j] = 0;
        }
        GRP.ROI[i].enable = true;
        GRP.ROI[i].attribute = 1;
        GRP.ROI[i].depthRange.d1 = 150;
        GRP.ROI[i].depthRange.d2 = 4500;
        GRP.ROI[i].trsdSensitivityRate.low = 50;
    }
    ui->areaDrawLabel->setText("1 2 3 4 5 6 7 8");

    ui->areaEnable->setChecked(true);
    ui->areaAttribute->setCurrentIndex(0);
    ui->sensitivity->setValue(50);

    mIRItem.setGroupNumber(mGroupNum);
    emit setSecurityConfigInfo();
}

void MainWindow::onDrawModelChanged(int index)
{
    mIRItem.setDrawModel(index);
}

void MainWindow::onUpdateFirmware()
{
    if(ui->deviceList->count() == 0)
    {
        DialogForm().warning(tr("无可操作设备(设备列表为空)！"), tr("警告"));
        return;
    }
    if (mProcess.state() != QProcess::NotRunning) mProcess.kill();
    if(config.upgradePath.isEmpty()) config.upgradePath = QApplication::applicationDirPath();
    auto fileName = QFileDialog::getOpenFileName(nullptr, tr("请选择固件升级文件"), config.upgradePath);
    if(!fileName.isEmpty())
    {
        config.upgradePath = fileName;

        if(!fileName.contains("_V3", Qt::CaseInsensitive))
        {
            QFileInfo info(fileName);
            DialogForm().warning(
                tr("不支持\"%1\"更新文件的更新，请选择主版本号为3的更新文件！").arg(info.fileName()),
                tr("警告"));
            return;
        }
        /// 关闭数据流以节省网络带宽，提升成功率
        if(ui->link->text() == tr("断开")) emit stopStream();
        /// 判定主版本号，对配置文件进行相应升级
        // if(config.fvMajor < 3) emit upgradeJsonFile();
        /// 调用升级程序对设备固件进行升级
        if(ui->link->text() == tr("连接"))
        {
            config.password = QString(QFileInfo(fileName).baseName()).contains("se", Qt::CaseInsensitive)
                                  ? "dm-se-zgyfjch"
                                  : "dm-zgyfjch";
        }

        QStringList arguments;
        arguments<<"-h"<<ui->deviceList->currentText();
        arguments<<"-p"<<config.password;
        arguments<<"-f"<<fileName;
        arguments<<"-r";
        arguments<<"-e";
        arguments<<"-w";
        mProcess.start("firmwareUpdateTool.exe", arguments);
        QTimer::singleShot(120000, this, [this]{ if (mProcess.state() != QProcess::NotRunning) mProcess.kill(); });
    }
}

void MainWindow::onLogForm()
{
    mLogForm.show();
}

void MainWindow::onDepthWindow()
{
    mDepthDrawForm.show();
}

void MainWindow::onExpertModel()
{
    mDeviceArgSetForm.setArgUI();
    mDeviceArgSetForm.show();
}

void MainWindow::onQProcessInfo()
{
    auto str = mProcess.readAllStandardOutput();
    if(!str.startsWith("progress")) emit senToLogPanel(str);
}

void MainWindow::onResetCamera()
{
    double          up[3];
    double          ps[3];
    double          fp[3];

    auto camera = mRenderer->GetActiveCamera();
    camera->GetFocalPoint(fp);
    camera->GetPosition(ps);
    camera->GetViewUp(up);

    if(sender() == ui->overlookView)
    {
        up[0] = 0;
        up[1] = -1;
        up[2] = 0;

        ps[0] = 0;
        ps[1] = 0;
        ps[2] = -1;

        fp[0] = 0;
        fp[1] = 0;
        fp[2] = 0;

        camera->SetFocalPoint(fp);
        camera->SetClippingRange(1, -1);
    }
    else if(sender() == ui->leftView)
    {
        up[0] = 0;
        up[1] = 0;
        up[2] = -1;

        ps[0] = -1;
        ps[1] = 0;
        ps[2] = 0;

        fp[0] = 0;
        fp[1] = 0;
        fp[2] = 0;

        camera->SetFocalPoint(fp);
        camera->SetClippingRange(1, -1);
    }
    else if(sender() == ui->frontView)
    {
        up[0] = 0;
        up[1] = 0;
        up[2] = -1;

        ps[0] = 0;
        ps[1] = 1;
        ps[2] = 0;

        fp[0] = 0;
        fp[1] = 0;
        fp[2] = 0;

        camera->SetFocalPoint(fp);
        camera->SetClippingRange(1, -1);
    }
    else
    {
        if(abs(up[0])>abs(up[1]) && abs(up[0])>abs(up[2])) {
            up[0] = (up[0] > 0) ? 1 : -1;
            up[1] = 0;
            up[2] = 0;

            ps[0] = fp[0];
            abs(ps[1])>abs(ps[2]) ? (ps[2] = fp[2]) : (ps[1] = fp[1]);
        }
        else if(abs(up[1])>abs(up[2])) {
            up[0] = 0;
            up[1] = (up[1]> 0) ? 1 : -1;
            up[2] = 0;

            ps[1] = fp[1];
            abs(ps[0])>abs(ps[2]) ? (ps[2] = fp[2]) : (ps[0] = fp[0]);
        }
        else {
            up[0] = 0;
            up[1] = 0;
            up[2] = (up[2]> 0) ? 1 : -1;

            ps[2] = fp[2];
            abs(ps[0])>abs(ps[1]) ? (ps[1] = fp[1]) : (ps[0] = fp[0]);
        }
    }

    camera->SetViewUp(up);
    camera->SetPosition(ps);
    mRenderer->ResetCamera();

    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onRecordLocation()
{
    auto camera = mRenderer->GetActiveCamera();
    camera->GetViewUp(config.up);
    camera->GetFocalPoint(config.fp);
    camera->GetPosition(config.ps);
    camera->GetClippingRange(config.dr);
}

void MainWindow::onPyramidsOpacity()
{
    auto value = doubleInputDialog(tr("请输入视锥的透明度"), tr("数值:"), config.pyramidOpacity, 0, 1, 2, Qt::WindowFlags(), 0.05);
    config.pyramidOpacity = value;
    mPyramidActor->GetProperty()->SetOpacity(value);
    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onOverlapOpacity()
{
    auto value = doubleInputDialog(tr("请输入有效区域的透明度"), tr("数值:"), config.overlapOpacity, 0, 1, 2, Qt::WindowFlags(),0.05);
    config.overlapOpacity = value;
    for (int i = 0; i < 8; ++i) {
        mIntersectionActor[i]->GetProperty()->SetOpacity(value);
    }

    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onNonoverlapOpacity()
{
    auto value = doubleInputDialog(tr("请输入绘制区域的透明度"), tr("数值:"), config.nonOverlapOpacity, 0, 1, 2, Qt::WindowFlags(),0.05);
    config.nonOverlapOpacity = value;
    for (int i = 0; i < 8; ++i) {
        mCubeActor[i]->GetProperty()->SetOpacity(value);
        // mDifferenceActor[i]->GetProperty()->SetOpacity(value);
    }

    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onPointSize()
{
    auto value = doubleInputDialog(tr("请输入要设置的点云大小"), tr("数值:"), config.pointSize, 1, 10, 1, Qt::WindowFlags(), 0.5);
    config.pointSize = value;
    mPointActor->GetProperty()->SetPointSize(value);
    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onPerspectiveTransfer(int)
{
    if(ui->execute->text() == tr("开始"))
    {
        DialogForm().warning(tr("请先点击\"开始\"按钮，开启数据流！！！"), tr("警告"));
    }

    mNotTransfer = false;
}

void MainWindow::onPotTransfer(double )
{
    emit setRotationAxis(ui->axial->currentIndex()+1, ui->degrees->value());
}

void MainWindow::onCalibration()
{
    mIRItem.loadCalibrationArea();
    auto ret = mAlarmVisual.convertPerspectiveIRCornersToIRCorners(ARG.srcCalibrationParams, ARG.dstCalibrationParams);

    if(ret == alarmVisualReturnStatus::AlarmALGVisual_SUCCESS)
    {
        emit setCalibrationARG();
    }
    else
    {
        DialogForm().warning(tr("区域转换出现错误，标定失败！！！"), tr("警告"));
        emit senToLogPanel(getDescriptionOfAlarmVisualReturnStatus(ret));
    }
}

void MainWindow::onUpdateAreaInfo()
{
    int  index = ARG.Enable_Group_Number-1;
    auto num    = ui->areaNumber->currentIndex();
    auto &GRP   = ARG.Group[index];

    mIRItem.setGroupNumber(index);

    QString str;
    for (int i = 0; i < 8; ++i) if(GRP.ROI[i].enable) str += QString("%1 ").arg(i+1);
    ui->areaDrawLabel->setText(str);
    ui->groupNumber->blockSignals(true);
    mGroupNum = index;
    ui->groupNumber->setCurrentIndex(index);
    ui->groupNumber->blockSignals(false);
    ui->groupLabel->setText(QString::number(ARG.Enable_Group_Number));
    ui->groupEnableLabel->setText(QString::number(ARG.Enable_Group_Number));
    ui->areaEnable->setChecked(GRP.ROI[num].enable);
    ui->areaAttribute->setCurrentIndex(GRP.ROI[num].attribute-1);
    ui->sensitivity->setValue(GRP.ROI[num].trsdSensitivityRate.low);
}

void MainWindow::onCalibrationStep(int FLAG)
{
    if(FLAG == 0)
    {
        ui->axisDirec->setCurrentIndex(0);
        ui->levelDirec->setValue(0);
        ui->axial->setCurrentIndex(0);
        ui->degrees->setValue(0);

        ui->axisDirec->setEnabled(true);
        ui->levelDirec->setEnabled(true);
        ui->drawModel1->setEnabled(true);
        ui->calibrateArea->setEnabled(true);
        ui->axial->setEnabled(false);
        ui->degrees->setEnabled(false);
        ui->calibrationCompleted->setEnabled(false);
        ui->calibrationCancel->setEnabled(false);

        mNotTransfer = false;
    }
    else if(FLAG == 1)
    {
        ui->axisDirec->setEnabled(false);
        ui->levelDirec->setEnabled(false);
        ui->drawModel1->setEnabled(false);
        ui->calibrateArea->setEnabled(false);
        ui->axial->setEnabled(false);
        ui->degrees->setEnabled(false);
        ui->calibrationCompleted->setEnabled(false);
        ui->calibrationCancel->setEnabled(true);
    }
    else if(FLAG == 2)
    {
        ui->axisDirec->setEnabled(false);
        ui->levelDirec->setEnabled(false);
        ui->drawModel1->setEnabled(false);
        ui->calibrateArea->setEnabled(false);
        ui->axial->setEnabled(true);
        ui->degrees->setEnabled(true);
        ui->calibrationCompleted->setEnabled(true);
        ui->calibrationCancel->setEnabled(true);
    }
}

void MainWindow::onPullLog()
{
    if(config.logPath.isEmpty()) config.logPath = QApplication::applicationDirPath();
    auto filename = QFileDialog::getSaveFileName(nullptr, tr("请选择日志导出路径"), config.logPath, "Text files (*.txt);");
    if(filename.isEmpty()) return;
    config.logPath = filename;
    emit pullLog(filename);
}

void MainWindow::onRecoverInit()
{
    if (DialogForm().question(tr("确定将所有组别的绘制区域初始化吗？"), tr("待确认")) == QMessageBox::Ok)
    {
        emit initJsonFile();
    }
}

void MainWindow::onNetworkSet()
{
    mNetworkSetForm.show();
}

void MainWindow::onRulerEnable()
{
    if(ui->rulerEnable->text() == tr("显示标尺"))
    {
        ui->rulerEnable->setText(tr("隐藏标尺"));
        mRenderer->AddActor(mCubeAxesActor);
    }
    else
    {
        ui->rulerEnable->setText(tr("显示标尺"));
        mRenderer->RemoveActor(mCubeAxesActor);
    }
    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onPotTextLabels()
{
    if(ui->vtkTextLabels->text() == tr("隐藏文字标签"))
    {
        ui->vtkTextLabels->setText(tr("显示文字标签"));
        for (int i = 0; i < 8; ++i) mRenderer->RemoveActor(mCubeTextActor[i]);
    }
    else
    {
        ui->vtkTextLabels->setText(tr("隐藏文字标签"));
        for (int i = 0; i < 8; ++i) mRenderer->AddActor(mCubeTextActor[i]);
    }
    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onTextureMapEnable()
{
    if(ui->textureMap->text() == tr("开启点云纹理映射"))
    {
        ui->textureMap->setText(tr("关闭点云纹理映射"));
        config.isTextureMapEnable = true;
    }
    else
    {
        ui->textureMap->setText(tr("开启点云纹理映射"));
        config.isTextureMapEnable = false;
    }
}

void MainWindow::onSoundLightAlarmEnable(bool arg)
{
    if(arg)
    {
        ui->soundLightAlarm->setText(tr("关闭声光报警器"));
    }
    else
    {
        ui->soundLightAlarm->setText(tr("开启声光报警器"));
    }
}

void MainWindow::onImpressum()
{
    DialogForm(800, 500).information(tr(
        "\n"
        "1. 该程序只能运行固件主版本号为3的设备。\n"
        "2. 为了降低误操作, 只有在连接好设备开启数据流的时候才能进行相关绘制操作。\n"
        "3. \"绘制区域\"是指划定好的安全防护区域。\n"
        "4. \"视锥\"是指设备能够探测到物体的区域。\n"
        "5. \"有效区域\"是指绘制区域与视锥的重叠部分。\n"
        "6. 按下键盘\"V\"键3D点云图和防区绘制图会对调位置。\n"
        "7. 在编辑模式下, 先点击一下绘制区域(主要是为了触发鼠标焦点), 再按下键盘\"Control\"键, 这时鼠标会变成十字箭头, 就可以随意拖动至任意位置。\n"
        "8. 在编辑模式下, 先点击一下绘制区域(主要是为了触发鼠标焦点), 再按下键盘\"Shift\"键, 这时绘制区域的四个角点会出现圆点, 当鼠标放上去就会变成十字架, 然后可以将其随意拖动至任意位置。\n"
        "9. 只要对区域进行了重新绘制并未对其保存(按下保存按钮), 当切换至其它组别、区域或进入3D绘制时, 会弹出对话框, 以确定该如何执行接下来的操作。\n"
        "10. 由于算法只能对直四棱柱区域进行相应判定, 因此在进行3D绘制会有些许限制, 目前提供如下5项子操作: \n"
        "    ① \"单点自由移动\"选项: 选中某一顶点可进行任意方向自由拖动, 由于上下底面必须平行, 因此在移动该顶点时处于同一底面的其它三个顶点的Z-轴数值也会同步更新。\n"
        "    ② \"单点XOY平面移动\"选项: 选中某一顶点可在XOY平面内自由拖动, 由于上下底面必须平行, 因此在移动该顶点时处于另一底面的对应顶点的X-轴、Y-轴数值也会同步更新。\n"
        "    ③ \"基于Z-轴方向的高度拉伸\"选项: 选中某一顶点可在Z-轴方向自由拖动, 由于上下底面必须平行, 因此在移动该顶点时处于同一底面的其它三个顶点的Z-轴数值也会同步更新。\n"
        "    ④ \"单体自由移动\"选项: 选中某一顶点可进行任意方向整体拖动。\n"
        "    ⑤ \"单体轴向移动\"选项: 选中某一顶点可进行X轴或Y-轴或Z-轴方向整体拖动。\n"
        "11. 未在\"防区绘制图\"界面上对区域进行改动的情况下, 当点击编辑界面的\"保存\"按钮时, 不会执行2D->3D区域的重绘动作。\n"
        "12. 每当\"防区绘制图\"界面的窗口大小发生变化时, 其界面的图像显示内容会自动自适应显示。\n"
        )
        ,
        tr("简要说明")
        );
}

void MainWindow::onChangeText()
{
    DialogForm(600, 400).information(tr(
        "V3.0.8: \n"
        "1. 安防植入算法版本: V3.0.3.0 。\n"
        "2. 将主界面上的\"深度-投影图\"和\"操作日志\"窗口迁移至新增的\"窗口\"菜单栏里。\n"
        "3. 小幅度调整界面布局和组件样式。\n"
        "4. 防区绘制图新增\"IR图伽马值\"和\"点云曝光等级\"设置选项, 其中\"点云曝光等级\"主要用于对点云的呈现效果施加影响。\n"
        "5. \"设备管理\"菜单栏新增声光报警器的启禁功能。\n"
        "6. 移除\"设备标定\"菜单, 将其改为以模式的形式呈现(右下角的\"标定\"选项)。\n"
        "7. \"设置\"菜单栏新增点云纹理映射(以IR图颜色对其进行着色)的启禁功能。\n"
        "8. \"设置\"菜单栏新增点云投影模式的切换, 默认投影模式为: \"正交投影\"。\n"
        "9. \"设置\"菜单栏新增点云图的背景色选择。\n"
        "10. 新增了3D绘制功能\n"
        "11. 绘制模式里的\"凸多边形\"选项改为\"四边形\", 可绘制凹四边形。\n"
        "12. 优化了若干交互逻辑, 使之操作更加合理化。\n"
        "13. 在每次与设备断开连接时, 会向设备发送指令使其重启(默认行为), 可在\"设置\"菜单栏里的\"专家模式\"菜单项里进行设置。\n"
        "\nV3.0.9: \n"
        "1. 解决弹框出现的内容显示不全的问题。\n"
        "2. 解决\"专家模式\"界面里帧率值与实际值对应不上的问题。\n"
        "3. 更新\"标定\"模式界面里点云旋转的文字描述, 解决其歧义性。\n"
        "\nV3.0.10: \n"
        "1. 解决编辑模式下\"区域使能\"选项发生改变并保存时, 界面的显示信息未及时更新的问题。\n"
        "2. 解决3D点云图与防区绘制图的区域颜色不一致的问题。\n"
        "3. 解决点云曝光等级与曝光时间不对应的问题。\n"
        "\nV3.0.11: \n"
        "1. 解决在设置点云曝光等级和曝光时间的时候出现重复触发设置动作。\n"
        "2. 解决在设置IR伽马值的时候滑块每移动一个步长便会触发一次设置动作(点云曝光等级的设置也同步解决)。\n"
        "3. 程序界面小幅调整。\n"
        "\nV3.0.12: \n"
        "1. 解决退出标定模式时标定区域未隐藏。\n"
        "\nV3.0.13: \n"
        "1. 解决程序界面在特定操作时出现的显示比例不协调问题。\n"
        "2. 新增工具提示(toolTip)窗口的样式, 使其在不同背景色下能够清晰显示。\n"
        "3. 解决多个消息弹框同时触发时出现的无信息内容的BUG。\n"
        "4. 解决绘制凹多面体时, 其显示的\"有效区域\"会出现异常面片。\n"
        "\nV3.0.14: \n"
        "1. 适配基线版。\n"
        "2. 解决显示凹多面体时出现裂缝的问题。\n"
        "3. 解决当绘制区域面积很小时无法显示的问题。\n"
        "\nV3.0.15: \n"
        "1. 算法版本更新至: V3.0.4.0 。\n"
        "2. 修复算法模块---交叉四边形自适应调整后依旧输出原坐标问题。\n"
        "3. 修复安防区域文字标签在切换组别时出现与菜单栏的设置项不匹配问题。\n"
        "4. 解决显示凹多面体时在其内部多出的一个面片。\n"
        "5. 在设备连接后IR图伽马值和点云曝光等级才能进行设置。\n"
        "6. 新增语言切换功能。\n"
        "\nV3.0.16: \n"
        "1. 解决程序无法在某些Windows10系统上运行。\n"
        "2. 解决部分对话框的标题在切换语言时未做相应更改。\n"
        )
        ,
        tr("版本变更说明")
        );
}

void MainWindow::onSelectExportPath()
{
    if(config.exportPath.isEmpty()) config.exportPath = QApplication::applicationDirPath();
    auto path = QFileDialog::getExistingDirectory(nullptr, tr("请选择导出路径"), config.exportPath);
    if(!path.isEmpty())
    {
        config.exportPath = path;
        emit exportJsonFile(path);
    }
}

void MainWindow::onSelectImportFile()
{
    if(config.importPath.isEmpty()) config.importPath = QApplication::applicationDirPath();
    auto Names = QFileDialog::getOpenFileNames(nullptr, tr("请选择导入文件"), config.importPath, "Json files (*.json)");
    if(!Names.isEmpty())
    {
        config.importPath = QFileInfo(Names.first()).path();
        emit importJsonFile(Names);
    }
}

void MainWindow::onProjectionTransform()
{
    auto camera = mRenderer->GetActiveCamera();

    if(ui->projectionTransform->text() == tr("(点云)正交投影"))
    {
        ui->projectionTransform->setText(tr("(点云)透视投影"));
        camera->SetParallelProjection(true); //开启正交投影
    }
    else
    {
        ui->projectionTransform->setText(tr("(点云)正交投影"));
        camera->SetParallelProjection(false); //开启透视投影
    }

    ui->potWidget->renderWindow()->Render();
}

void MainWindow::onPotBackgroundColor()
{
    QColorDialog colorDialog{config.backgroundColor, nullptr};
    colorDialog.setWindowIcon(this->windowIcon());
    colorDialog.setWindowTitle(tr("点云图背景色选择"));
    colorDialog.exec();
    auto color = colorDialog.selectedColor();
    if(color.isValid())
    {
        config.backgroundColor = color;

        mRenderer->SetBackground(color.redF(), color.greenF(), color.blueF());
        ui->potWidget->renderWindow()->Render();

        ui->container1->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                          .arg(color.red())
                                          .arg(color.green())
                                          .arg(color.blue()));
    }
}

void MainWindow::onLock3DView()
{
    if(ui->lockView->toolTip() == tr("3D旋转未锁定"))
    {
        mStyle->rotateEnable(false);
        ui->lockView->setToolTip(tr("3D旋转已锁定"));
        ui->lockView->setStyleSheet("QPushButton { border: 0px solid #ffffff; "
                                    "background-color: rgba(255, 255, 255,0); "
                                    "image: url(:/image/thumbtack_p.svg);} "
                                    );
    }
    else
    {
        mStyle->rotateEnable(true);
        ui->lockView->setToolTip(tr("3D旋转未锁定"));
        ui->lockView->setStyleSheet("QPushButton { border: 0px solid #ffffff; background-color: rgba(255, 255, 255,0); image: url(:/image/thumbtack.svg);} "
                                    "QPushButton:pressed { image: url(:/image/thumbtack_p.svg); }"
                                    "QPushButton:hover { image: url(:/image/thumbtack_h.svg); } "
                                    );
    }
}

void MainWindow::onArea3dDrawSave()
{
    if(!isQuadrilateral(mCubePots[mAreaNum]))
    {
        DialogForm().critical(tr("绘制的3D区域，出现了边交叉，请重新绘制！！！"), tr("错误"));
        return;
    }

    int h1, h2;
    double pos[3];

    for (int k = 0, l = 0; k < 4; ++k, l += 2)
    {
        mCubePots[mAreaNum]->GetPoint(k, pos);
        ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l  ] = pos[0];
        ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l+1] = pos[1];
    }

    h1 = pos[2];
    mCubePots[mAreaNum]->GetPoint(5, pos);
    h2 = pos[2];

    if(h1 == h2) h2 = h2+1;
    ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d1 = h1 < h2 ? h1 : h2;
    ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d2 = h1 < h2 ? h2 : h1;

    emit setSecurity3DInfo();
}

void MainWindow::onEnter3dDrawMode()
{
    if(!mNonZeroArr[mFlag])
    {
        DialogForm().warning(tr("请先绘制防区！！！"), tr("警告"));
        return;
    }

    if (isAreaInfoChange())
    {
        if(DialogForm().question(
                tr("检测到你对区域 %1 进行了更改！按“确认”键则忽略此更改继续往下执行！").arg(mAreaNum+1),
                tr("待确认")) != QMessageBox::Ok)
        {
            return;
        }

        restoreAreaInfo();
    }

    mIRItem.restoreArea();
    mIRItem.setEditModel(false);
    ui->modeWidget->setEnabled(false);
    mStyle->drawEnable(true);
    vertexActorUpdate(mAreaNum);
    mDMActor->GetProperty()->SetOpacity(0.1);
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::onLeave3dDrawMode()
{
    if(is3DAreaChange())
    {
        if(DialogForm().question(
                tr("检测到你对3D区域 %1 进行了更改！按“确认”键则忽略此更改继续往下执行！").arg(mAreaNum+1),
                tr("待确认")) != QMessageBox::Ok)
        {
            return;
        }

        double pos[3];
        double h1 = ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d1;
        double h2 = ARG.Group[mGroupNum].ROI[mAreaNum].depthRange.d2;
        for (int k = 0, l = 0; k < 8; ++k, l += 2)
        {
            if(l > 7) l = 0;
            pos[0] = ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l  ];
            pos[1] = ARG.Group[mGroupNum].ROI[mAreaNum].dstArea[l+1];
            pos[2] = k < 4 ? h1 : h2;
            mCubePots[mAreaNum]->SetPoint(k, pos);
        }
        mCubePots[mAreaNum]->Modified();
        ui->potWidget->renderWindow()->Render();
    }

    mIRItem.setEditModel(true);
    ui->modeWidget->setEnabled(true);
    mStyle->drawEnable(false);
    ui->stackedWidget->setCurrentIndex(0);
    mDMActor->GetProperty()->SetOpacity(1);
}

void MainWindow::onSwitchLanguage()
{
    if(ui->link->text() == tr("断开"))
    {
        DialogForm().warning(tr("请先断开连接设备！"), tr("警告"));
        return;
    }

    auto str = ui->sdk->text();

    if(sender() == ui->english)
        qApp->installTranslator(&mTranslator);
    else
        qApp->removeTranslator(&mTranslator);

    QTimer::singleShot(20, this, [this, str]{ ui->sdk->setText(str); });
}

void MainWindow::onNote()
{
    if(sender() == ui->note1)
    {
        QToolTip::showText(
            this->cursor().pos(),
            ui->note1->toolTip(),
            ui->note1,
            ui->note1->rect()
            );
    }
    else if(sender() == ui->note2)
    {
        QToolTip::showText(
            this->cursor().pos(),
            ui->note2->toolTip(),
            ui->note2,
            ui->note2->rect()
            );
    }
}

void MainWindow::onSetSoundLightAlarm()
{
    if(ui->soundLightAlarm->text() == tr("开启声光报警器"))
    {
        emit setSoundLightAlarm(true);
    }
    else
    {
        emit setSoundLightAlarm(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat())
    {
        if(event->key() == Qt::Key_Control)
        {
            mIRItem.setPolygonMoveEnable(true);
        }
        else if(event->key() == Qt::Key_Shift)
        {
            mIRItem.setPointMoveEnable(true);
        }
        else if(event->key() == Qt::Key_V)
        {
            static bool flag = true;

            ui->container1->hide();
            ui->container2->hide();
            if(flag)
            {
                ui->IRLayout->addWidget(ui->container1);
                ui->PotLayout->addWidget(ui->container2);
            }
            else
            {
                ui->IRLayout->addWidget(ui->container2);
                ui->PotLayout->addWidget(ui->container1);
            }
            ui->container1->show();
            ui->container2->show();
            flag = !flag;
        }
        // else if(event->key() == Qt::Key_Z)
        // {
        //     mStyle->pointMoveEnable(true);
        // }
        // else if(event->key() == Qt::Key_X)
        // {
        //     mStyle->planeMoveEnable(true);
        // }
        // else if(event->key() == Qt::Key_A)
        // {
        //     mStyle->heightMoveEnable(true);
        // }
        // else if(event->key() == Qt::Key_S)
        // {
        //     mStyle->heightTranslationEnable(true);
        // }
        else if(event->key() == Qt::Key_Delete)
        {

        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat())
    {
        if(event->key() == Qt::Key_Control)
        {
            mIRItem.setPolygonMoveEnable(false);
        }
        else if(event->key() == Qt::Key_Shift)
        {
            mIRItem.setPointMoveEnable(false);
        }
        // else if(event->key() == Qt::Key_Z)
        // {
        //     mStyle->pointMoveEnable(false);
        // }
        // else if(event->key() == Qt::Key_X)
        // {
        //     mStyle->planeMoveEnable(false);
        // }
        // else if(event->key() == Qt::Key_A)
        // {
        //     mStyle->heightMoveEnable(false);
        // }
        // else if(event->key() == Qt::Key_S)
        // {
        //     mStyle->heightTranslationEnable(false);
        // }
        else if(event->key() == Qt::Key_Q)
        {

        }
        else if(event->key() == Qt::Key_Delete)
        {

        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mLogForm.close();
    mDepthDrawForm.close();
    mDeviceArgSetForm.close();
    mNetworkSetForm.close();

    event->accept();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本

    }

    QMainWindow::changeEvent(event); // 调用基类处理
}

