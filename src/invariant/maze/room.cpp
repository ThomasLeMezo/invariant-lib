#include "room.h"
#include "ibex_IntervalVector.h"
#include "ibex_CtcFwdBwd.h"
#include "ibex_Function.h"
#include "door.h"

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
void Room<ibex::IntervalVector, ibex::IntervalVector>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
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
    if(alpha==ibex::Interval::ZERO)
        alpha.set_empty();

    c &= alpha*v;
    if(sens==FWD || sens==FWD_BWD)
        out &= (c+in);
    if(sens==BWD || sens==FWD_BWD)
        in &= (out-c);
}

template <>
ibex::IntervalVector get_empty_door_container<ibex::IntervalVector, ibex::IntervalVector>(int dim){
    return IntervalVector(dim, ibex::Interval::EMPTY_SET);
}

template <>
void set_empty<ibex::IntervalVector, ibex::IntervalVector>(ibex::IntervalVector &T){
    T.set_empty();
}

template <>
ibex::IntervalVector get_diff_hull<ibex::IntervalVector, ibex::IntervalVector>(const ibex::IntervalVector &a, const ibex::IntervalVector &b){
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
void Room<ibex::IntervalVector, ibex::IntervalVector>::compute_vector_field_typed(){
    m_vector_fields_typed_fwd = m_vector_fields;
    m_vector_fields_typed_bwd = m_vector_fields;// No negative bc of the way the contractor is coded
}

template <>
const ibex::IntervalVector Room<ibex::IntervalVector, ibex::IntervalVector>::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FaceIBEX *f:get_pave()->get_faces_vector()){
        DoorIBEX *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    return result;
}

/// ******************  ppl::C_Polyhedron ****************** ///

void recursive_linear_expression_from_iv(const ibex::IntervalVector &vect_field,
                                         int dim,
                                         ppl::Generator_System &gs,
                                         Linear_Expression &local_linear_expression){
    if(dim > 0){
        ppl::Variable x(dim-1);
        Linear_Expression linear_expression_lb = local_linear_expression;
        Linear_Expression linear_expression_ub = local_linear_expression;

        // ToDo: case theta[dim] -> lb=+oo | ub=-oo
        if(std::isinf(vect_field[dim-1].ub()))
            gs.insert(ppl::ray(Linear_Expression(x)));
        else
            linear_expression_ub += x*ceil(vect_field[dim-1].ub()*IBEX_PPL_PRECISION);

        if(std::isinf(vect_field[dim-1].lb()))
            gs.insert(ppl::ray(Linear_Expression(-x)));
        else
            linear_expression_lb += x*floor(vect_field[dim-1].lb()*IBEX_PPL_PRECISION);

        recursive_linear_expression_from_iv(vect_field, dim-1, gs, linear_expression_ub);
        recursive_linear_expression_from_iv(vect_field, dim-1, gs, linear_expression_lb);
    }
    else{
        if(!local_linear_expression.all_homogeneous_terms_are_zero()) // ie cannot add ray 0
            gs.insert(ppl::ray(local_linear_expression));
    }
}

template <>
void Room<ppl::C_Polyhedron, ppl::Generator_System>::compute_vector_field_typed(){
    std::vector<ppl::Generator_System> gs_list_fwd;
    for(ibex::IntervalVector &vect:m_vector_fields){
        Linear_Expression l = Linear_Expression(0);
        ppl::Generator_System gs;
        if(!vect.is_empty())
            recursive_linear_expression_from_iv(vect, vect.size(), gs,l);
        gs_list_fwd.push_back(gs);
    }
    m_vector_fields_typed_fwd = gs_list_fwd;

    std::vector<ppl::Generator_System> gs_list_bwd;
    for(ibex::IntervalVector &vect:m_vector_fields){
        Linear_Expression l = Linear_Expression(0);
        ppl::Generator_System gs;
        if(!vect.is_empty())
            recursive_linear_expression_from_iv(-vect, vect.size(), gs,l);
        gs_list_bwd.push_back(gs);
    }
    m_vector_fields_typed_bwd = gs_list_bwd;
}

template<>
void Room<ppl::C_Polyhedron, ppl::Generator_System>::contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ppl::Generator_System &gs, const DYNAMICS_SENS &sens){
    if(sens == FWD){
        if(gs.empty() || in.is_empty()){
            out = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        }
        else{
            in.add_generators(gs);
            out &= in;
        }
    }

    if(sens == BWD){
        if(gs.empty() || out.is_empty()){
            in = ppl::C_Polyhedron(out.space_dimension(), ppl::EMPTY);
        }
        else{
            out.add_generators(gs);
            in &= out;
        }
    }
}

template <>
ppl::C_Polyhedron get_empty_door_container<ppl::C_Polyhedron, ppl::Generator_System>(int dim){
    return ppl::C_Polyhedron(dim, Parma_Polyhedra_Library::EMPTY);
}

template <>
void set_empty<ppl::C_Polyhedron, ppl::Generator_System>(ppl::C_Polyhedron &T){
    T = ppl::C_Polyhedron(T.space_dimension(), ppl::EMPTY);
}

template <>
ppl::C_Polyhedron get_diff_hull<ppl::C_Polyhedron, ppl::Generator_System>(const ppl::C_Polyhedron &a, const ppl::C_Polyhedron &b){
    ppl::C_Polyhedron tmp(b);
    tmp.poly_difference_assign(a);
    return tmp;
}

int get_nb_dim_flat(const ppl::C_Polyhedron &p){
    return p.space_dimension() - p.affine_dimension();
}

template <>
const ibex::IntervalVector Room<ppl::C_Polyhedron, ppl::Generator_System>::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FacePPL *f:get_pave()->get_faces_vector()){
        DoorPPL *d = f->get_doors()[m_maze];
        result |= polyhedron_2_iv(d->get_hull());
    }
    return result;
}

/// ******************  Other functions ****************** ///

template<>
ibex::IntervalVector convert_vec_field<ibex::IntervalVector>(const ibex::IntervalVector &vect){
    return vect;
}

template<>
ppl::Generator_System convert_vec_field<ppl::Generator_System>(const ibex::IntervalVector &vect){
    Linear_Expression l = Linear_Expression(0);
    ppl::Generator_System gs;
    recursive_linear_expression_from_iv(vect, vect.size(), gs,l);
    return gs;
}

}

