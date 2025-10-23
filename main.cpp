#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Windows"));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "APP_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}


// #include "vtkRenderWindow.h"
// #include "vtkAxesActor.h"
// #include "vtkPolyData.h"
// #include "vtkScalarBarActor.h"
// #include "vtkProperty.h"
// #include "vtkCamera.h"
// #include "vtkBillboardTextActor3D.h"
// #include "vtkRenderWindowInteractor.h"
// #include "vtkSTLReader.h"
// #include "vtkPolygon.h"
// #include "vtkUnstructuredGrid.h"
// #include "vtkDataSetMapper.h"
// #include "vtkCollisionDetectionFilter.h"
// #include "vtkBooleanOperationPolyDataFilter.h"
// #include "vtkDelaunay3D.h"
// #include "vtkConnectivityFilter.h"
// #include "vtkAppendPolyData.h"
// #include "vtkDataSetSurfaceFilter.h"

// int main() {

//     vtkSmartPointer<vtkPoints> points1 = vtkSmartPointer<vtkPoints>::New();
//     for (int i = 0; i < 20; ++i) {
//         points1->InsertNextPoint(vtkMath::Random(0, 10),
//                                  vtkMath::Random(0, 10),
//                                  vtkMath::Random(0, 10));
//     }
//     vtkSmartPointer<vtkPolyData> polydata1 = vtkSmartPointer<vtkPolyData>::New();
//     polydata1->SetPoints(points1);

//     vtkSmartPointer<vtkDelaunay3D> delaunay1 = vtkSmartPointer<vtkDelaunay3D>::New();
//     delaunay1->SetInputData(polydata1);
//     delaunay1->Update();


//     vtkSmartPointer<vtkPoints> points2 = vtkSmartPointer<vtkPoints>::New();
//     for (int i = 0; i < 20; ++i) {
//         points2->InsertNextPoint(vtkMath::Random(5, 15),
//                                  vtkMath::Random(5, 15),
//                                  vtkMath::Random(5, 15));
//     }
//     vtkSmartPointer<vtkPolyData> polydata2 = vtkSmartPointer<vtkPolyData>::New();
//     polydata2->SetPoints(points2);

//     vtkSmartPointer<vtkDelaunay3D> delaunay2 = vtkSmartPointer<vtkDelaunay3D>::New();
//     delaunay2->SetInputData(polydata2);
//     delaunay2->Update();


//     vtkNew<vtkDataSetSurfaceFilter> surfaceFilter1;
//     surfaceFilter1->SetInputData(delaunay1->GetOutput());
//     surfaceFilter1->Update();
//     vtkPolyData* surface1 = surfaceFilter1->GetOutput();

//     vtkNew<vtkDataSetSurfaceFilter> surfaceFilter2;
//     surfaceFilter2->SetInputData(delaunay2->GetOutput());
//     surfaceFilter2->Update();
//     vtkPolyData* surface2 = surfaceFilter2->GetOutput();

//     // 4. 对两个表面网格进行布尔并集操作
//     vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperation =
//         vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
//     booleanOperation->SetOperationToUnion(); // 设置操作类型为并集
//     booleanOperation->SetInputData(0, surface1);
//     booleanOperation->SetInputData(1, surface2);

//     try {
//         booleanOperation->Update();
//     } catch (...) {
//         std::cerr << "布尔运算失败！可能的原因：网格不封闭或存在自相交" << std::endl;
//         return EXIT_FAILURE;
//     }

//     // 5. 可视化结果
//     vtkSmartPointer<vtkPolyDataMapper> originalMapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
//     originalMapper1->SetInputData(surface1);
//     originalMapper1->ScalarVisibilityOff();

//     vtkSmartPointer<vtkActor> originalActor1 = vtkSmartPointer<vtkActor>::New();
//     originalActor1->SetMapper(originalMapper1);
//     originalActor1->GetProperty()->SetColor(1, 0, 0); // 红色
//     originalActor1->GetProperty()->SetOpacity(0.5);

//     vtkSmartPointer<vtkPolyDataMapper> originalMapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
//     originalMapper2->SetInputData(surface2);
//     originalMapper2->ScalarVisibilityOff();

//     vtkSmartPointer<vtkActor> originalActor2 = vtkSmartPointer<vtkActor>::New();
//     originalActor2->SetMapper(originalMapper2);
//     originalActor2->GetProperty()->SetColor(0, 1, 0); // 绿色
//     originalActor2->GetProperty()->SetOpacity(0.5);

//     vtkSmartPointer<vtkPolyDataMapper> booleanMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//     booleanMapper->SetInputData(booleanOperation->GetOutput());
//     booleanMapper->ScalarVisibilityOff();

//     vtkSmartPointer<vtkActor> booleanActor = vtkSmartPointer<vtkActor>::New();
//     booleanActor->SetMapper(booleanMapper);
//     booleanActor->GetProperty()->SetColor(0, 1, 1); // 蓝色
//     booleanActor->GetProperty()->SetOpacity(0.3);
//     // booleanActor->GetProperty()->SetEdgeVisibility(1);
//     // booleanActor->GetProperty()->SetEdgeColor(0, 0, 0);

//     // 创建渲染器和窗口
//     vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
//     vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//     renderWindow->AddRenderer(renderer);
//     vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//         vtkSmartPointer<vtkRenderWindowInteractor>::New();
//     interactor->SetRenderWindow(renderWindow);

//     // 添加 actor 到渲染器
//     // renderer->AddActor(originalActor1);
//     // renderer->AddActor(originalActor2);
//     renderer->AddActor(booleanActor);
//     renderer->SetBackground(1, 1, 1); // 白色背景

//     renderWindow->SetSize(800, 600);
//     renderWindow->SetWindowName("Delaunay 3D Boolean Union");
//     renderWindow->Render();
//     interactor->Start();

//     return EXIT_SUCCESS;
// }


// {
//     // 创建点云的相关信息
//     vtkNew<vtkPolyData> potPolydata;
//     // mPoints->SetNumberOfPoints(intersection->GetOutput()->GetNumberOfPoints());
//     potPolydata->SetPoints(intersection->GetOutput()->GetPoints());
//     vtkNew<vtkVertexGlyphFilter> vertex;
//     vertex->SetInputData(potPolydata);
//     vtkNew<vtkPolyDataMapper> mapper;
//     mapper->SetInputConnection(vertex->GetOutputPort());
//     vtkNew<vtkActor> potActor;
//     potActor->SetMapper(mapper);
//     potActor->GetProperty()->SetPointSize(10);
//     potActor->GetProperty()->SetColor(0, 0, 1); // 红色边
//     mRenderer->AddActor(potActor);
// }


// {
//     vtkNew<vtkFeatureEdges> edges;
//     edges->SetInputData(surface1);
//     edges->NonManifoldEdgesOn();
//     edges->BoundaryEdgesOn();
//     edges->FeatureEdgesOff();
//     edges->ManifoldEdgesOff();
//     edges->Update();
//     int nOpen = edges->GetOutput()->GetNumberOfCells();
//     if (nOpen == 0)
//         qDebug()<<"模型1表面是流形";
//     else
//         qDebug() << "模型1存在 " << nOpen << " 条非流形边";
// }
