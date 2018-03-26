#ifndef RESULTSTORAGE_H
#define RESULTSTORAGE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <iostream>

#include <omp.h>

namespace invariant {

template <typename _Tp> class ResultStorage;
using ResultStoragePPL = ResultStorage<Parma_Polyhedra_Library::C_Polyhedron>;
using ResultStorageIBEX = ResultStorage<ibex::IntervalVector>;

class Dynamics; // declared only for friendship

template <typename _Tp=ibex::IntervalVector>
class ResultStorage
{   
public:
    ResultStorage(size_t dim);

    void test();

    void push_back_input(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out);
    void push_back_output(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out);

    void push_back_input_initial(const _Tp &val, const size_t &face, const size_t &sens);
    void push_back_output_initial(const _Tp &val, const size_t &face, const size_t &sens);

    _Tp get_output(const size_t &face, const size_t &sens);
    _Tp get_input(const size_t &face, const size_t &sens);

private:
    typedef std::vector< std::array<std::vector<_Tp>, 2>> sub_tab_type;
    typedef std::vector< std::array<sub_tab_type, 2>> tab_type;

    tab_type m_input2output;
    tab_type m_output2input;

    sub_tab_type m_input_initial;
    sub_tab_type m_output_initial;

    size_t m_dim;
};

}

#include "resultstorage.tpp"

#endif // RESULTSTORAGE_H
