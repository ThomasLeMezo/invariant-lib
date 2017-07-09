#include "previmer3d.h"
#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

PreviMer3D::PreviMer3D(const std::string& file_directory, const IntervalVector &search_space, double epsilon_bisection_tree){

    if(search_space.size()!=3){
        throw std::runtime_error("in [previmer3d.cpp/PreviMer3D()] dim of search_space is not equal to 3");
    }
    m_dim = 3;

    vector<string> file_list;
    get_file_list(file_directory, file_list);
    int nb_files = file_list.size();
    int max_t = min(nb_files, (int)(search_space[0].ub()));
    bool first_read = true;
    int cpt = 0;

    for(int file_id = 0; file_id<max_t; file_id++){
        const string file_name = file_list[file_id];

        cout << "load file : " << ++cpt << "/" << max_t << endl;
        NcFile dataFile(file_directory+file_name, NcFile::read);

        NcVar u_var=dataFile.getVar("U");
        NcVar v_var=dataFile.getVar("V");

        // ******* Size U,V ******
        size_t j_max = dataFile.getDim("nj_u").getSize();
        size_t i_max = dataFile.getDim("ni_u").getSize();

        // ******* DATA U,V ******
        short *raw_u = new short[j_max*i_max];
        short *raw_v = new short[j_max*i_max];
        u_var.getVar(raw_u);
        v_var.getVar(raw_v);

        vector<vector<short>> tab_u, tab_v;

        for(size_t i=0; i<i_max; i++){
            vector<short> line_u, line_v;
            for(size_t j=0; j<j_max; j++){
                line_u.push_back(raw_u[i_max*j+i]);
                line_v.push_back(raw_v[i_max*j+i]);
            }
            tab_u.push_back(line_u);
            tab_v.push_back(line_v);
        }

        delete[] raw_u;
        delete[] raw_v;

        m_raw_u.push_back(tab_u);
        m_raw_v.push_back(tab_v);

        if(first_read){
            u_var.getAtt("scale_factor").getValues(&m_scale_factor);
            u_var.getAtt("_FillValue").getValues(&m_fill_value);
            first_read = false;
            m_size.push_back(nb_files);
            m_size.push_back(i_max);
            m_size.push_back(j_max);
        }
    }

    // ******* Node Current ******
    IntervalVector position(search_space);
    if(search_space.is_unbounded()){
        position[0] = Interval(0, m_size[0]);
        position[1] = Interval(0, m_size[1]);
        position[2] = Interval(0, m_size[2]);
        cout << "unbound search space set to : " << position << endl;
    }

    double time_start_init = omp_get_wtime();
    cout << "TIME build tree = ";
    m_node_current = new NodeCurrent3D(position, epsilon_bisection_tree);
    cout << omp_get_wtime() - time_start_init << endl;

    time_start_init = omp_get_wtime();
    cout << "TIME compute leafs = ";
    m_node_current->fill_leafs(m_raw_u, m_raw_v, m_scale_factor, m_fill_value);
    cout << omp_get_wtime() - time_start_init << endl;

    m_raw_u.clear();
    m_raw_v.clear();

    time_start_init = omp_get_wtime();
    m_node_current->compute_vector_field_tree();
    cout << "TIME compute tree = " << omp_get_wtime() - time_start_init << endl;
}

const vector<ibex::IntervalVector> PreviMer3D::eval(const ibex::IntervalVector& position){
    vector<ibex::IntervalVector> vector_fields;
    vector_fields.push_back(m_node_current->eval(position));
    return vector_fields;
}

const ibex::IntervalVector& PreviMer3D::get_search_space(){
    return m_node_current->get_position();
}

int PreviMer3D::get_file_list(string dir, vector<string> &files){
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        string file_name = string(dirp->d_name);
        if(!(file_name == "." || file_name == ".."))
            files.push_back(file_name);
    }
    closedir(dp);
    return 0;
}

}
