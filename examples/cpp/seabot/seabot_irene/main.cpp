#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

#include "vtkMaze3D.h"

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>

#include <vtkPoints.h>
#include <vtkCubeSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPolyDataCollection.h>

#include <vtkFloatArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>

#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <omp.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
namespace pt = boost::property_tree;

using namespace std;
using namespace invariant;

int main(int argc, char *argv[]){
  // Load files
  if(argc<2){
    cout << "No file" << endl;
    return -1;
  }

  pt::ptree tree;
  string file_xml = argv[1];

  if(file_xml == ""){
    cout << "No file" << endl;
    return -1;
  }
  else
    cout << " READ file_xml = " << file_xml << endl;

  pt::read_xml(file_xml, tree);
  auto node = tree.get_child("RADAR");
  string file_name = node.get<string>("file");
  string file_output = node.get<string>("file_output");
  double x_init = node.get<double>("x");
  double y_init = node.get<double>("y");
  double t_init = node.get<double>("t_start");
  double t_end = node.get<double>("t_duration");
  double dt = node.get<double>("dt");

  double delta_x = node.get<double>("delta_x");
  double delta_y = node.get<double>("delta_y");
  double delta_t = node.get<double>("delta_t");
  double nb_x = node.get<double>("nb_x");
  double nb_y = node.get<double>("nb_y");
  double nb_t = node.get<double>("nb_t");

  cout << "file_name = " << file_name << endl;

  LambertGrid g(file_name);

  //    draw_map(g);

  double time_start = omp_get_wtime();

  // Compute trajectories
  cout << "time_min = " << std::setprecision(10) << g.get_time_min() << endl;
  cout << "time_max = " << g.get_time_max() << endl;

  bool plot_unfinished_trajectories = true;

  vector<array<double, 3>> init_conditions;
  for(double x=-nb_x; x<=nb_x; x++){
    for(double y=-nb_y; y<=nb_y; y++){
      for(double t=-nb_t; t<=nb_t; t++){
        init_conditions.push_back(array<double, 3>{x_init+delta_x*x, y_init+delta_y*y, t_init+delta_t*t});
      }
    }
  }

  vtkSmartPointer<vtkAppendPolyData> trajectories = vtkSmartPointer<vtkAppendPolyData>::New();

  const size_t nb_conditions = init_conditions.size();
#pragma omp parallel for
  for(size_t k=0; k<nb_conditions; k++){
    double x_tmp = init_conditions[k][0];
    double y_tmp = init_conditions[k][1];
    double t_tmp = init_conditions[k][2];
    bool valid = true;
#pragma omp critical
    {
      cout << "> Compute Traj " << k << endl;
    }

    vtkSmartPointer<vtkPolyData> trajectory = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for(double t=t_tmp; t<t_tmp+t_end; t+=dt){
      // RK2 scheme
      double u, v;
      g.eval(x_tmp, y_tmp, t, u, v);
      double x_r = x_tmp + (dt/2.0)*u;
      double y_r = y_tmp + (dt/2.0)*v;

      int err = g.eval(x_r, y_r, t+(dt/2.0), u, v);
      if(err !=0){
        valid = false;
        cout << "Error " << err << endl;
      }

      x_tmp+=u*dt;
      y_tmp+=v*dt;

      points->InsertNextPoint(x_tmp, y_tmp, t+dt-t_init);
      if(!valid)
        break;
    }
    trajectory->SetPoints(points);

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    for(unsigned int i = 0; i < points->GetNumberOfPoints()-1; i++){
      vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0,i);
      line->GetPointIds()->SetId(1,i+1);
      lines->InsertNextCell(line);
    }
    trajectory->SetLines(lines);

    //            vtkSmartPointer<vtkIntArray> intValue = vtkSmartPointer<vtkIntArray>::New();
    //            intValue->SetNumberOfComponents(1);
    //            intValue->SetName("TrajId");
    //            intValue->InsertNextValue(k);
    //            trajectory->GetFieldData()->AddArray(intValue);

#pragma omp critical
    {
      if(valid || plot_unfinished_trajectories){
        trajectories->AddInputData(trajectory);
      }
    }
  }

  trajectories->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  outputWriter->SetFileName(file_output.c_str());
  outputWriter->SetInputData(trajectories->GetOutput());
  outputWriter->Write();

  cout << "TIME = " << omp_get_wtime() - time_start << endl;
}
