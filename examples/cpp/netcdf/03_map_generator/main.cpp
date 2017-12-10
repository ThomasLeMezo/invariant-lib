#include <iostream>
#include <math.h>
#include <netcdf>

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkCubeSource.h>
#include <vtkXMLPolyDataWriter.h>

#include <ostream>

using namespace netCDF;
using namespace netCDF::exceptions;
using namespace std;

int main(int argc, char *argv[])
{
    string file_name = "/home/lemezoth/Documents/ensta/flotteur/data_ifremer/data/MARC_L1-MARS2D-FINIS250_20170709T0000Z_MeteoMF.nc";

    NcFile dataFile(file_name, NcFile::read);

    NcVar u_var=dataFile.getVar("U");

    // ******* Size U,V ******
    size_t j_max = dataFile.getDim("nj_u").getSize();
    size_t i_max = dataFile.getDim("ni_u").getSize();
    short int fill_value;
    u_var.getAtt("_FillValue").getValues(&fill_value);

    // ******* DATA U,V ******
    short int raw_u[j_max*i_max];
    u_var.getVar(&raw_u);

    vtkSmartPointer<vtkAppendPolyData> polyData = vtkSmartPointer<vtkAppendPolyData>::New();
    for(size_t j=1; j<j_max-1; j++){
        cout << j << endl;
        for(size_t i=1; i<i_max-1; i++){
            if(raw_u[j*i_max+i]==fill_value){
                vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
                cubedata->SetBounds(0.0,15*60,
                        i*250.0, (i+1)*250.0,
                        j*250.0, (j+1)*250.0);
                cubedata->Update();
                polyData->AddInputData(cubedata->GetOutput());
            }
        }
    }

    polyData->Update();
    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    outputWriter->SetFileName("map.vtp");
    outputWriter->SetInputData(polyData->GetOutput());
    outputWriter->Write();

    return 0;
}
