
// 测试opencascade环境
#include <cassert>
#include <iostream>

#include "opencascade.h"

using namespace std;

TopoDS_Compound construt_bottle() {
  float myWidth = 1.0f;
  float myThickness = 0.4f;
  float myHeight = 1.5f;

  gp_Pnt aPnt1(-myWidth / 2.0f, 0, 0);
  gp_Pnt aPnt2(-myWidth / 2.0f, -myThickness / 4.0f, 0);
  gp_Pnt aPnt3(0, -myThickness / 2.0f, 0);
  gp_Pnt aPnt4(myWidth / 2.0f, -myThickness / 4.0f, 0);
  gp_Pnt aPnt5(myWidth / 2.0f, 0, 0);

  Handle(Geom_TrimmedCurve) aArcOfCircle = GC_MakeArcOfCircle(aPnt2, aPnt3, aPnt4);
  Handle(Geom_TrimmedCurve) aSegment1 = GC_MakeSegment(aPnt1, aPnt2);
  Handle(Geom_TrimmedCurve) aSegment2 = GC_MakeSegment(aPnt4, aPnt5);

  TopoDS_Edge anEdge1 = BRepBuilderAPI_MakeEdge(aSegment1);
  TopoDS_Edge anEdge2 = BRepBuilderAPI_MakeEdge(aArcOfCircle);
  TopoDS_Edge anEdge3 = BRepBuilderAPI_MakeEdge(aSegment2);

  TopoDS_Wire aWire = BRepBuilderAPI_MakeWire(anEdge1, anEdge2, anEdge3);

  gp_Ax1 xAxis(gp_Pnt(0.0f, 0.0f, 0.0f), gp_Dir(1.0f, 0.0f, 0.0f));
  gp_Trsf aTrsf;
  aTrsf.SetMirror(xAxis);

  BRepBuilderAPI_Transform aBRepTrsf(aWire, aTrsf);
  TopoDS_Shape aMirroredShape = aBRepTrsf.Shape();
  TopoDS_Wire aMirroredWire = TopoDS::Wire(aMirroredShape);

  BRepBuilderAPI_MakeWire mkWire;
  mkWire.Add(aWire);
  mkWire.Add(aMirroredWire);
  TopoDS_Wire myWireProfile = mkWire.Wire();

  TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(myWireProfile);

  // 朝着z轴方向挤出height高度，生成
  gp_Vec aPrismVec(0.0f, 0.0f, myHeight);
  TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec);

  // 为myBody导圆角
  BRepFilletAPI_MakeFillet mkFillet(myBody);

  TopExp_Explorer anEdgeExplorer(myBody, TopAbs_EDGE); // 从myBody中遍历所有Edge对象
  while (anEdgeExplorer.More()) {
    TopoDS_Edge anEdge = TopoDS::Edge(anEdgeExplorer.Current());
    mkFillet.Add(myThickness / 12.0f, anEdge); // 对anEdge边导圆角，弧长为myThickness/12
    anEdgeExplorer.Next();
  }
  myBody = mkFillet.Shape();

  // 创建圆柱瓶口，半径为myThickness/4，高度myHeight/10
  gp_Pnt neckLocation(0.0f, 0.0f, myHeight); // 圆柱起始位置
  gp_Dir neckAxis = gp::DZ();                // 延伸方向
  gp_Ax2 neckAxis2(neckLocation, neckAxis);  // 给定一个主方向，自动计算另外两个方向以构造局部坐标系

  Standard_Real myNeckRadius = myThickness / 4.0f;
  Standard_Real myNeckHeight = myHeight / 10.0f;

  BRepPrimAPI_MakeCylinder MKCylinder(neckAxis2, myNeckRadius, myNeckHeight);
  TopoDS_Shape myNeck = MKCylinder.Shape();

  // 拼接瓶身和瓶口
  myBody = BRepAlgoAPI_Fuse(myBody, myNeck); // 这个操作应该会将两部分重叠的面移除，两个实体焊接为一个实体

  // 镂空瓶子内部
  // 1. 找到待移除的顶部Face
  TopoDS_Face faceToRemove;
  Standard_Real zMax = -1.0f;
  for (TopExp_Explorer aFaceExplorer(myBody, TopAbs_FACE); aFaceExplorer.More(); aFaceExplorer.Next()) {
    TopoDS_Face aFace = TopoDS::Face(aFaceExplorer.Current());
    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
    if (aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane)) {
      Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
      Standard_Real zLoc = aPlane->Location().Z();
      if (zLoc > zMax) {
        faceToRemove = aFace;
        zMax = zLoc;
      }
    }
  }
  // 2. 移除顶部Face，并构造新的Solid
  TopTools_ListOfShape facesToRemove;
  facesToRemove.Append(faceToRemove);
  BRepOffsetAPI_MakeThickSolid aSolidMaker;
  aSolidMaker.MakeThickSolidByJoin(myBody, facesToRemove, -myThickness / 50.0f, 0.001f);
  myBody = aSolidMaker.Shape();

  // 创建瓶口圆柱曲面上的螺纹
  // 1. 创建内外两个圆柱面，螺纹会在两者之间生成，以嵌入到瓶口圆环柱面中
  Handle(Geom_CylindricalSurface) aCyl1 = new Geom_CylindricalSurface(neckAxis2, myNeckRadius * 0.99f);
  Handle(Geom_CylindricalSurface) aCyl2 = new Geom_CylindricalSurface(neckAxis2, myNeckRadius * 1.05f);
  // 2. 建立圆柱面参数方程，以计算：面上任意位置、任意阶向量导数
  // 参数方程：P(u,v) = O + R*(xDir*cos(u) + yDir*sin(u)) + v*zDir
  // 3. 定义圆柱面上的关键参数
  gp_Pnt2d aPnt(2.0f * M_PI, myNeckHeight / 2.0f); // 圆柱面上的中心位置
  gp_Dir2d aDir(2.0f * M_PI, myNeckHeight / 4.0f); // 圆柱面中心位置的方向向量
  gp_Ax2d anAx2d(aPnt, aDir);                      // 建立圆柱面坐标系
  // 4. 生成圆柱面上的螺纹曲线
  Standard_Real aMajor = 2.0f * M_PI;
  Standard_Real aMinor = myNeckHeight / 10.0f;
  Handle(Geom2d_Ellipse) anEllipse1 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor);
  Handle(Geom2d_Ellipse) anEllipse2 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor / 4.0f);

  Handle(Geom2d_TrimmedCurve) anArc1 = new Geom2d_TrimmedCurve(anEllipse1, 0.0f, M_PI); // 限制为上半椭圆弧
  Handle(Geom2d_TrimmedCurve) anArc2 = new Geom2d_TrimmedCurve(anEllipse2, 0.0f, M_PI);

  // 5. 取出anEllipse1的起点和终点位置
  gp_Pnt2d anEllipsePnt1 = anEllipse1->Value(0);
  gp_Pnt2d anEllipsePnt2;
  anEllipse1->D0(M_PI, anEllipsePnt2);

  Handle(Geom2d_TrimmedCurve) aSegment = GCE2d_MakeSegment(anEllipsePnt1, anEllipsePnt2); // 创建由两点构成的二维线段

  // 6. 从曲面的二维参数空间中构建Edge，并连接为闭合的Wire
  TopoDS_Edge anEdge10nSurf1 = BRepBuilderAPI_MakeEdge(anArc1, aCyl1);   // 内侧，椭圆上半圆弧
  TopoDS_Edge anEdge20nSurf1 = BRepBuilderAPI_MakeEdge(aSegment, aCyl1); // 内侧，椭圆长轴连线
  TopoDS_Edge anEdge10nSurf2 = BRepBuilderAPI_MakeEdge(anArc2, aCyl2);
  TopoDS_Edge anEdge20nSurf2 = BRepBuilderAPI_MakeEdge(aSegment, aCyl2);

  TopoDS_Wire threadingWire1 = BRepBuilderAPI_MakeWire(anEdge10nSurf1, anEdge20nSurf1); // 内测螺纹
  TopoDS_Wire threadingWire2 = BRepBuilderAPI_MakeWire(anEdge10nSurf2, anEdge20nSurf2); // 外侧螺纹

  // 7. 构建三维曲线（不太好理解这一步）
  BRepLib::BuildCurves3d(threadingWire1);
  BRepLib::BuildCurves3d(threadingWire2);

  // 两条闭合螺纹之间放样生成Solid
  BRepOffsetAPI_ThruSections aTool(Standard_True); // true表示生成Solid，否则默认生成Shell
  aTool.AddWire(threadingWire1);
  aTool.AddWire(threadingWire2);
  TopoDS_Shape myThreading = aTool.Shape();

  // 生成最终的Compound
  TopoDS_Compound aRes;
  BRep_Builder aBuilder;
  aBuilder.MakeCompound(aRes);
  aBuilder.Add(aRes, myBody);
  aBuilder.Add(aRes, myThreading);
  return aRes;
}

int main(void) {
  TopoDS_Compound bottle = construt_bottle();

  // 导出
  STEPControl_Writer writer;
  if (!Interface_Static::SetIVal("write.precision.mode", 1)) { // 设置精度模式为最大容限
    cerr << "set write.precision.mode failed!" << endl;
    exit(0);
  }
  if (!Interface_Static::SetRVal("write.precision.val", 0.001f)) { // 设置精度值为0.001
    cerr << "set write.precision.val failed!" << endl;
    exit(0);
  }
  if (!Interface_Static::SetIVal("write.step.assembly", 1)) { // ？？
    cerr << "set write.step.assembly failed!" << endl;
    exit(0);
  }
  // 选择转换模型，开始转换
  STEPControl_StepModelType mode = STEPControl_ManifoldSolidBrep;
  IFSelect_ReturnStatus stat = writer.Transfer(bottle, mode);
  cout << "transfer return status: " << stat << endl;
  stat = writer.Write("aa.stp");
  cout << "write return status: " << stat << endl;

  return 0;
}

// using namespace std;
// int main(int argc, char **argv) {

//   Scene scene;

//   scene.showAxis();
//   scene.camera.setPosition(glm::vec3(0.0f, 10.0f, 15.0f));
//   scene.camera.lookAt({0.0f, 2.0f, 0.0f});
//   scene.light.color = {0.6f, 0.6f, 0.6f};
//   scene.light.position = {1.0f, 7.0f, 2.0f};

//   scene.add("TruncatedConeEx",
//             make_shared<TruncatedConeEx>(2.0f, 0.5f, 6.0f, 0.0f, 0.0f),
//             {-4.0f, 0.0f, -6.0f});
//   scene.add("Sphere", make_shared<Sphere>(2.0f, 18, 36), {0.0f, 2.0f, 0.0f});
//   scene.add("Cone", make_shared<Cone>(2.0f, 3.0f), {0.0f, 0.0f, 6.0f});
//   scene.add("Cylinder", make_shared<Cylinder>(2.5f, 5.0f), {5.0f, 0.0f, 0.0f});

//   // 创建Sphere对象的层次包围盒
//   for (auto &obj : scene.objs)
//     obj.second->constructBvhTree();

//   scene.mainloop();
//   return 0;
// }
