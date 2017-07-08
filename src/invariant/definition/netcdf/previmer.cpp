#include "previmer.h"
#include <netcdf>
#include <omp.h>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

PreviMer::PreviMer(const std::string& file_name){
    NcFile dataFile(file_name, NcFile::read);

    NcVar u_var=dataFile.getVar("U");
    NcVar v_var=dataFile.getVar("V");

    // ******* Size U,V ******
    m_j_max = dataFile.getDim("nj_u").getSize();
    m_i_max = dataFile.getDim("ni_u").getSize();

    // ******* DATA U,V ******
    m_raw_u = new signed short[m_j_max*m_i_max];
    m_raw_v = new signed short[m_j_max*m_i_max];
    u_var.getVar(m_raw_u);
    v_var.getVar(m_raw_v);

    // ******* Params U,V ******
    u_var.getAtt("scale_factor").getValues(&m_scale_factor);
    u_var.getAtt("_FillValue").getValues(&m_fill_value);

    // ******* Node Current ******
    IntervalVector position(2);
    position[0] = Interval(0, m_j_max);
    position[1] = Interval(0, m_i_max);
    position[0] = Interval(100, 200);
    position[1] = Interval(400, 500);
    double epsilon_bisection = 0.51;

    double time_start_init = omp_get_wtime();
    m_node_current = new NodeCurrent(position, epsilon_bisection);
    cout << "TIME build tree= " << omp_get_wtime() - time_start_init << endl;
    m_dim = 2;

    time_start_init = omp_get_wtime();
    m_node_current->fill_leafs(m_raw_u, m_raw_v, m_i_max, m_scale_factor, m_fill_value);
    cout << "TIME compute leafs= " << omp_get_wtime() - time_start_init << endl;

    time_start_init = omp_get_wtime();
    m_node_current->compute_vector_field_tree();
    cout << "TIME compute tree = " << omp_get_wtime() - time_start_init << endl;
}

const vector<ibex::IntervalVector> PreviMer::eval(ibex::IntervalVector position){
    vector<ibex::IntervalVector> vector_fields;
    vector_fields.push_back(m_node_current->eval(position));
    return vector_fields;
}

const ibex::IntervalVector& PreviMer::get_search_space(){
    return m_node_current->get_position();
}

}
