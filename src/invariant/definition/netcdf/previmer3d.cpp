#include "previmer3d.h"
#include "nodecurrent3d.h"
#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

#include <utility>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

PreviMer3D::PreviMer3D(const std::string& file_directory, const IntervalVector &search_space, std::vector<double> grid_size, const std::vector<double> &limit_bisection, int stop_level){

    if(search_space.size()!=3){
        throw std::runtime_error("in [previmer3d.cpp/PreviMer3D()] dim of search_space is not equal to 3");
    }
    m_dim = 3;
    m_grid_size = grid_size;
    m_limit_bisection = limit_bisection;

    // Compute ratio dimension
    for(int dim=0; dim<m_dim; dim++){
        double diam = search_space[dim].diam();
        m_ratio_dimension.push_back(1.0/diam);
    }


    vector<string> file_list;
    get_file_list(file_directory, file_list);
    int nb_files = file_list.size();
    int t_max = min(nb_files, (int)(search_space[0].ub()/grid_size[0]));
    int t_min = max(0, (int)(search_space[0].lb()/grid_size[0]));
    bool first_read = true;
    int cpt = 0;

    for(int file_id = t_min; file_id<t_max; file_id++){
        const string file_name = file_list[file_id];

        cout << "load file : " << ++cpt << "/" << t_max << " T+" << floor(cpt*0.25) << "h" << (cpt*15)%60 << "min" << endl;
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
    cout << "Data properties" << endl;
    cout << " Scale factor = " << m_scale_factor << endl;
    cout << " Fill Value = " << m_fill_value << endl;

    // ******* Node Current ******
    IntervalVector position(search_space);
    if(search_space.is_unbounded()){
        for(int i=0; i<3; i++)
            position[i] = Interval(0, m_size[i]*m_grid_size[i]);
        cout << "unbound search space set to : " << position << endl;
    }

    double time_start_init = omp_get_wtime();
    cout << "TIME build tree = ";
    m_node_current = new NodeCurrent3D(position, limit_bisection, this, 0, stop_level);
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

std::pair<IntervalVector, IntervalVector> PreviMer3D::bisect_largest_first(const IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    vector<bool> possible_dim;
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() > m_limit_bisection[dim]){
            possible_dim.push_back(true);
            one_possible = true;
        }
        else{
            possible_dim.push_back(false);
        }
    }
    if(!one_possible){ // If no-one possible make all possible
        for(int dim=0; dim<m_dim; dim++)
            possible_dim[dim] = true;
    }

    // Find largest dimension
    Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i])){
            max = test;
            dim_max = i;
        }
    }
    IntervalVector p1(position);
    IntervalVector p2(position);

    p1[dim_max] = Interval(position[dim_max].lb(), position[dim_max].mid());
    p2[dim_max] = Interval(position[dim_max].mid(), position[dim_max].ub());

    return std::make_pair(p1, p2);
}

}
