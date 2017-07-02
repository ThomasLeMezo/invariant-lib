#include "previmer.h"
#include <netcdf>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

PreviMer::PreviMer()
{
    NcFile dataFile("/home/lemezoth/Documents/ensta/flotteur/data/PREVIMER_L1-MARS2D-FINIS250_20081229T0000Z_MeteoMF.nc", NcFile::read);
    NcVar u=dataFile.getVar("U");
    NcDim nj_u_dim = dataFile.getDim("nj_u");
//    cout << "size nj_u_dim = " << nj_u_dim.getSize() << endl;

//    vector<NcDim> u_dims = u.getDims();
//    for(int i=0; i<u_dims.size(); i++){
//        cout << u_dims[i].getName() << " " << u_dims[i].getSize() << endl;
//    }

    double scale_factor;
    u.getAtt("scale_factor").getValues(&scale_factor);
    cout << scale_factor << endl;

//    int dataU[754][584];

//    if(u.isNull())
//        cout << "ERROR" << endl;
//    else{
//       cout << "U found" << endl;
//       u.getVar(dataU);
//    }

//    for(int i=0; i<5; i++){
//        cout << dataU[i][i] << endl;
//    }
}
