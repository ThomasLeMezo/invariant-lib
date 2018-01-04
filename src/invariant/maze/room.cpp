#include "room.h"

using namespace std;
using namespace ibex;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

inline ibex::Interval root_zero(const ibex::Interval &a, const ibex::Interval &b, const ibex::Interval &c){
    return (-c/b) & ibex::Interval::POS_REALS;
}
inline ibex::Interval root_positive(const ibex::Interval &a, const ibex::Interval &b, const ibex::Interval &c){
    return (-b+sqrt(b*b-4*a*c))/(2*a) & ibex::Interval::POS_REALS;
}
inline ibex::Interval root_negative(const ibex::Interval &a, const ibex::Interval &b, const ibex::Interval &c){
    return (-b-sqrt(b*b-4*a*c))/(2*a) & ibex::Interval::POS_REALS;
}

inline ibex::Interval taylor(const ibex::Interval &t, const ibex::Interval &x_dot2, const ibex::Interval &x0_dot, const ibex::Interval &x0){
    return 0.5*x_dot2*t*t+x0_dot*t+x0;
}

template<>
void Room<ibex::IntervalVector>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
    // Contraction with Taylor 1 order
    if(m_pave->get_dim()==2 && m_vector_fields_d1.size()!=0){
        IntervalVector vect_d1(in.size());
        vect_d1[0] = m_vector_fields_d1[0][0][0]*vect[0] + m_vector_fields_d1[0][0][1]*vect[1];
        vect_d1[1] = m_vector_fields_d1[0][1][1]*vect[1] + m_vector_fields_d1[0][1][0]*vect[0];

        /// **************** FWD ****************
        if(sens==FWD || sens==FWD_BWD){
            IntervalVector vect_in = m_maze->get_dynamics()->eval(in)[0];
            IntervalVector out_result(2, ibex::Interval::EMPTY_SET);

            ibex::IntervalVector c(9);
            c[0] = vect_d1[0] & ibex::Interval::NEG_REALS;
            c[1] = vect_d1[1] & ibex::Interval::NEG_REALS;
            c[2] = vect_in[0];
            c[3] = vect_in[1];
            c[4] = in[0];
            c[5] = in[1];
            c[6] = out[0];
            c[7] = out[1];
            c[8] = ibex::Interval::POS_REALS;
            m_ctc->contract(c);
            out_result[0] |= c[6];
            out_result[1] |= c[7];

            c[0] = vect_d1[0] & ibex::Interval::POS_REALS;
            c[1] = vect_d1[1] & ibex::Interval::POS_REALS;
            c[2] = vect_in[0];
            c[3] = vect_in[1];
            c[4] = in[0];
            c[5] = in[1];
            c[6] = out[0];
            c[7] = out[1];
            c[8] = ibex::Interval::POS_REALS;
            m_ctc->contract(c);
            out_result[0] |= c[6];
            out_result[1] |= c[7];

            out &= out_result;
        }

        /// **************** BWD ****************
        if(sens==BWD || sens==FWD_BWD){
            IntervalVector vect_in = -m_maze->get_dynamics()->eval(out)[0];
            IntervalVector in_result(2, ibex::Interval::EMPTY_SET);

            ibex::IntervalVector c(9);
            c[0] = vect_d1[0] & ibex::Interval::NEG_REALS;
            c[1] = vect_d1[1] & ibex::Interval::NEG_REALS;
            c[2] = vect_in[0];
            c[3] = vect_in[1];
            c[4] = out[0];
            c[5] = out[1];
            c[6] = in[0];
            c[7] = in[1];
            c[8] = ibex::Interval::POS_REALS;
            m_ctc->contract(c);
            in_result[0] |= c[4];
            in_result[1] |= c[5];

            c[0] = vect_d1[0] & ibex::Interval::POS_REALS;
            c[1] = vect_d1[1] & ibex::Interval::POS_REALS;
            c[2] = vect_in[0];
            c[3] = vect_in[1];
            c[4] = out[0];
            c[5] = out[1];
            c[6] = in[0];
            c[7] = in[1];
            c[8] = ibex::Interval::POS_REALS;
            m_ctc->contract(c);
            in_result[0] |= c[6];
            in_result[1] |= c[7];

            in &= in_result;
        }
    }

    // Contraction with Taylor 0 order
    // assert 0 not in v.

    ibex::IntervalVector c(out-in);
    ibex::IntervalVector v(vect);
    ibex::Interval alpha(ibex::Interval::POS_REALS);

    for(int i=0; i<v.size(); i++){
        if(!(c[i]==ibex::Interval::ZERO && ibex::Interval::ZERO.is_subset(v[i])))
            alpha &= ((c[i]/(v[i] & ibex::Interval::POS_REALS)) & ibex::Interval::POS_REALS) | ((c[i]/(v[i] & ibex::Interval::NEG_REALS)) & ibex::Interval::POS_REALS);
    }
    if(alpha==ibex::Interval::ZERO && m_maze->get_domain()->get_init()==FULL_DOOR) // To check ?
        alpha.set_empty();

    c &= alpha*v;
    if(sens==FWD || sens==FWD_BWD)
        out &= (c+in);
    if(sens==BWD || sens==FWD_BWD)
        in &= (out-c);
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
    m_vector_fields_typed_bwd = m_vector_fields;
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
    std::vector<ppl::C_Polyhedron> p_list_fwd, p_list_bwd;
    for(ibex::IntervalVector &vect:m_vector_fields){
        p_list_fwd.push_back(iv_2_polyhedron(vect));
        p_list_bwd.push_back(iv_2_polyhedron(-vect));
    }
    m_vector_fields_typed_fwd = p_list_fwd;
    m_vector_fields_typed_bwd = p_list_bwd;
}

template<>
void Room<ppl::C_Polyhedron>::contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ppl::C_Polyhedron &vect, const DYNAMICS_SENS &sens){
    if(sens == FWD){
        if(vect.is_empty() || in.is_empty()){
            out = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        }
        else{
            in.time_elapse_assign(vect);
            out &= in;
        }
    }

    if(sens == BWD){
        if(vect.is_empty() || out.is_empty()){
            in = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        }
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

