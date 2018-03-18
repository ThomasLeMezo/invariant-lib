#include "room.h"
#include "taylor.h"

using namespace std;
using namespace ibex;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

ibex::IntervalVector convert_iv(const ibex::IntervalVector &iv){
    return iv;
}

ibex::IntervalVector convert_iv(const ppl::C_Polyhedron &p){
    return polyhedron_2_iv(p);
}


template<>
void Room<ibex::IntervalVector>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
    // Contraction with Taylor 1 order
    // ToDo : implement with several vector fields...
    if(in.size()==2 && m_vector_fields_d1.size()>0){
        ibex::IntervalMatrix jac=m_vector_fields_d1[0];
        ibex::IntervalVector vect_d(2);
        vect_d[0] = jac[0][0]*vect[0] + 0.5*jac[0][1]*vect[1];
        vect_d[1] = jac[1][1]*vect[1] + 0.5*jac[1][0]*vect[0];
        ibex::IntervalVector vect_in = m_maze->get_dynamics()->eval(in)[0];

        if(sens==FWD)
            out &= taylor_contrat_box(out, vect_d, vect_in, in);
        else if(sens==BWD)
            in &= taylor_contrat_box(in, vect_d, -vect_in, out);
    }

    // Contraction with Taylor 0 order
    // assert 0 not in v.

    // out = in + alpha*v => c=out-in
    // in = out - alpha*v => c=in-out
    ibex::IntervalVector c(out.size());
    ibex::Interval alpha(ibex::Interval::POS_REALS);
    if(sens==FWD)
        c=out-in;
    else if(sens==BWD)
        c=in-out;

    for(int i=0; i<vect.size(); i++){
        if(!(c[i]==ibex::Interval::ZERO && ibex::Interval::ZERO.is_subset(vect[i])))
            alpha &= ((c[i]/(vect[i] & ibex::Interval::POS_REALS)) & ibex::Interval::POS_REALS) | ((c[i]/(vect[i] & ibex::Interval::NEG_REALS)) & ibex::Interval::POS_REALS);
    }
    if(m_maze->get_domain()->get_init()==FULL_DOOR && alpha==ibex::Interval::ZERO) // To check ?
        alpha.set_empty();

    c &= alpha*vect;
    if(sens==FWD)
        out &= (c+in);
    else if(sens==BWD)
        in &= (c+out);
}

template <>
ibex::IntervalVector get_empty_door_container<ibex::IntervalVector>(int dim){
    return IntervalVector(dim, ibex::Interval::EMPTY_SET);
}

template <>
void set_empty<ibex::IntervalVector>(ibex::IntervalVector &T){
    T.set_empty();
}

template <>
ibex::IntervalVector get_diff_hull<ibex::IntervalVector>(const ibex::IntervalVector &a, const ibex::IntervalVector &b){
    ibex::IntervalVector *diff_list;
    int nb_boxes = a.diff(b, diff_list);

    ibex::IntervalVector union_of_diff(a.size(), ibex::Interval::EMPTY_SET);
    for(int i=0; i<nb_boxes; i++)
        union_of_diff |= diff_list[i];
    delete[] diff_list;
    return union_of_diff;
}

int get_nb_dim_flat(const ibex::IntervalVector &iv){
    int dim = iv.size();
    int flat=0;
    for(int i=0; i<dim; i++){
        if(iv[i].is_degenerated() && !iv[i].is_unbounded())
            flat++;
    }
    return flat;
}

template <>
void Room<ibex::IntervalVector>::compute_vector_field_typed(){
    m_vector_fields_typed_fwd = m_vector_fields;
    for(ibex::IntervalVector &vect:m_vector_fields){
        m_vector_fields_typed_bwd.push_back(-vect);
    }
}

template <>
const ibex::IntervalVector Room<ibex::IntervalVector>::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FaceIBEX *f:get_pave()->get_faces_vector()){
        DoorIBEX *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    return result;
}

template <>
const ibex::IntervalVector Room<ibex::IntervalVector>::get_hull_typed() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FaceIBEX *f:get_pave()->get_faces_vector()){
        DoorIBEX *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    if(m_is_initial_door_input)
        result |= *m_initial_door_input;
    if(m_is_initial_door_output)
        result |= *m_initial_door_output;
    return result;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
void Room<ppl::C_Polyhedron>::compute_vector_field_typed(){
    for(ibex::IntervalVector &vect:m_vector_fields){
        m_vector_fields_typed_fwd.push_back(iv_2_polyhedron(vect));
        m_vector_fields_typed_bwd.push_back(iv_2_polyhedron(-vect));
    }
}

template<>
void Room<ppl::C_Polyhedron>::contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ppl::C_Polyhedron &vect, const DYNAMICS_SENS &sens){
    if(sens == FWD){
        if(vect.is_empty() || in.is_empty())
            out = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        else{
            in.time_elapse_assign(vect);
            out &= in;
        }
    }
    else if(sens == BWD){
        if(vect.is_empty() || out.is_empty())
            in = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        else{
            out.time_elapse_assign(vect);
            in &= out;
        }
    }
}

template <>
ppl::C_Polyhedron get_empty_door_container<ppl::C_Polyhedron>(int dim){
    return ppl::C_Polyhedron(dim, Parma_Polyhedra_Library::EMPTY);
}

template <>
void set_empty<ppl::C_Polyhedron>(ppl::C_Polyhedron &T){
    T = ppl::C_Polyhedron(T.space_dimension(), ppl::EMPTY);
}

template <>
ppl::C_Polyhedron get_diff_hull<ppl::C_Polyhedron>(const ppl::C_Polyhedron &a, const ppl::C_Polyhedron &b){
    ppl::C_Polyhedron tmp(b);
    tmp.poly_difference_assign(a);
    return tmp;
}

int get_nb_dim_flat(const ppl::C_Polyhedron &p){
    return p.space_dimension() - p.affine_dimension();
}

template <>
const ibex::IntervalVector Room<ppl::C_Polyhedron>::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FacePPL *f:get_pave()->get_faces_vector()){
        DoorPPL *d = f->get_doors()[m_maze];
        result |= polyhedron_2_iv(d->get_hull());
    }
    return result;
}

template <>
const ppl::C_Polyhedron Room<ppl::C_Polyhedron>::get_hull_typed() const{
    if(m_contain_zero)
        return m_pave->get_position_typed();
    else{
        C_Polyhedron result(m_pave->get_position().size(), ppl::EMPTY);
        for(FacePPL *f:get_pave()->get_faces_vector()){
            DoorPPL *d = f->get_doors()[m_maze];
            result |= d->get_hull();
        }
        if(m_is_initial_door_input)
            result |= *m_initial_door_input;
        if(m_is_initial_door_output)
            result |= *m_initial_door_output;
        return result;
    }
}

/// ******************  Other functions ****************** ///

template<>
ibex::IntervalVector convert_vec_field<ibex::IntervalVector>(const ibex::IntervalVector &vect){
    return vect;
}

template<>
ppl::C_Polyhedron convert_vec_field<ppl::C_Polyhedron>(const ibex::IntervalVector &vect){
    return iv_2_polyhedron(vect);
}

}

