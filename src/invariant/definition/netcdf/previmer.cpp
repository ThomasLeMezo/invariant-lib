#include "previmer.h"
#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

PreviMer::PreviMer(const std::string& file_name):
    Dynamics(Dynamics::FWD)
{
    NcFile dataFile(file_name, NcFile::read);

    NcVar u_var=dataFile.getVar("U");
    NcVar v_var=dataFile.getVar("V");

    // ******* Size U,V ******
    m_j_max = dataFile.getDim("nj_u").getSize();
    m_i_max = dataFile.getDim("ni_u").getSize();

    // ******* DATA U,V ******
    short *raw_u = new short[m_j_max*m_i_max];
    short *raw_v = new short[m_j_max*m_i_max];
    u_var.getVar(raw_u);
    v_var.getVar(raw_v);

    for(size_t i=0; i<m_i_max; i++){
        vector<short> tmp_u, tmp_v;
        for(size_t j=0; j<m_j_max; j++){
            tmp_u.push_back(raw_u[m_i_max*j+i]);
            tmp_v.push_back(raw_v[m_i_max*j+i]);
        }
        m_raw_u.push_back(tmp_u);
        m_raw_v.push_back(tmp_v);
    }
    delete[] raw_u;
    delete[] raw_v;

    // ******* Params U,V ******
    u_var.getAtt("scale_factor").getValues(&m_scale_factor);
    u_var.getAtt("_FillValue").getValues(&m_fill_value);

    // ******* Node Current ******
    IntervalVector position(2);
    position[0] = Interval(0, m_j_max);
    position[1] = Interval(0, m_i_max);

    position[0] = Interval(75, 280);
    position[1] = Interval(400, 600);

    double epsilon_bisection = 0.51;

    double time_start_init = omp_get_wtime();
    m_node_current = new NodeCurrent(position, epsilon_bisection);
    cout << "TIME build tree= " << omp_get_wtime() - time_start_init << endl;
    m_dim = 2;

    time_start_init = omp_get_wtime();
    m_node_current->fill_leafs(m_raw_u, m_raw_v, m_scale_factor, m_fill_value);
    cout << "TIME compute leafs= " << omp_get_wtime() - time_start_init << endl;

    time_start_init = omp_get_wtime();
    m_node_current->compute_vector_field_tree();
    cout << "TIME compute tree = " << omp_get_wtime() - time_start_init << endl;
}

const vector<ibex::IntervalVector> PreviMer::eval(const ibex::IntervalVector& position){
    vector<ibex::IntervalVector> vector_fields;
    vector_fields.push_back(m_node_current->eval(position));
    return vector_fields;
}

const ibex::IntervalVector& PreviMer::get_search_space(){
    return m_node_current->get_position();
}

}
