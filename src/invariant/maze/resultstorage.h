#ifndef RESULTSTORAGE_H
#define RESULTSTORAGE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <iostream>

#include <omp.h>

namespace invariant {

template <typename _TpD> class ResultStorage;
using ResultStoragePPL = ResultStorage<Parma_Polyhedra_Library::C_Polyhedron>;
using ResultStorageIBEX = ResultStorage<ibex::IntervalVector>;

class Dynamics; // declared only for friendship

template <typename _TpD>
class ResultStorage
{   
public:
    ResultStorage(const size_t &dim, const size_t &nb_vf);

//    void test();

    void push_back_input(const _TpD &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out, const size_t &n_vf);
    void push_back_output(const _TpD &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out, const size_t &n_vf);

    void push_back_input_initial(const _TpD &val, const size_t &face, const size_t &sens, const size_t &n_vf);
    void push_back_output_initial(const _TpD &val, const size_t &face, const size_t &sens, const size_t &n_vf);

    _TpD get_output(const size_t &face, const size_t &sens);
    _TpD get_output2(const size_t &face, const size_t &sens);
    _TpD get_output3(const size_t &face, const size_t &sens);
    _TpD get_input(const size_t &face, const size_t &sens);
    _TpD get_input2(const size_t &face, const size_t &sens);
    _TpD get_input3(const size_t &face, const size_t &sens);

private:
    //
    typedef std::vector< std::array<std::vector<_TpD>, 2>> sub_tab_type;
    typedef std::vector< std::array<sub_tab_type, 2>> tab_type;

    std::vector<std::vector<std::array<size_t, 2>>> m_intersection_list;

    tab_type m_input2output;
    tab_type m_output2input;

    sub_tab_type m_input_initial;
    sub_tab_type m_output_initial;

    size_t m_dim;
    size_t m_nb_vf;
};

}

#include "resultstorage.tpp"

#endif // RESULTSTORAGE_H
