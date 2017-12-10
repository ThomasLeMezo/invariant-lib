#include "room.h"
#include "ibex_IntervalVector.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
void Room<ibex::IntervalVector, ibex::IntervalVector>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){

    if(in.size()==2 && m_vector_fields_d1.size()!=0){
        IntervalVector vect_d1 = m_vector_fields_d1[0];
        if(sens==FWD || sens==FWD_BWD){
            int out_comp = 0;
            if(out[1].is_degenerated())
                out_comp=1;

            ibex::Interval y(ibex::Interval::EMPTY_SET);
            ibex::Interval t;

            IntervalVector in0_lb = IntervalVector(in.lb());
            IntervalVector vec_in0_lb = m_maze->get_dynamics()->eval(in0_lb)[0];
            ibex::Interval a1=0.5*vect_d1[out_comp];
            ibex::Interval b1=vec_in0_lb[out_comp];
            ibex::Interval c1=in0_lb[out_comp]-out[out_comp].lb();

            if(a1 == ibex::Interval(0)){
                t = -c1/b1 & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_lb[1-out_comp]*t+in0_lb[1-out_comp];
            }
            else{
                t = ((-b1+sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1)) & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_lb[1-out_comp]*t+in0_lb[1-out_comp];

                t = ((-b1-sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1)) & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_lb[1-out_comp]*t+in0_lb[1-out_comp];
            }

            IntervalVector in0_ub = IntervalVector(in.ub());
            IntervalVector vec_in0_ub = m_maze->get_dynamics()->eval(in0_ub)[0];
            a1=0.5*vect_d1[out_comp];
            b1=vec_in0_ub[out_comp];
            c1=in0_ub[out_comp]-out[out_comp].lb();

            if(a1 == ibex::Interval(0)){
                t = -c1/b1 & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_ub[1-out_comp]*t+in0_ub[1-out_comp];
            }
            else{
                t = ((-b1+sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1)) & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_ub[1-out_comp]*t+in0_ub[1-out_comp];

                t = ((-b1-sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1)) & ibex::Interval::POS_REALS;
                y |= 0.5*vect_d1[1-out_comp]*(pow(t,2))+vec_in0_ub[1-out_comp]*t+in0_ub[1-out_comp];
            }

            out[1-out_comp] &= y;
        }

        if(sens==BWD || sens==FWD_BWD){
            int in_comp = 0;
            if(in[1].is_degenerated())
                in_comp=1;

            ibex::Interval x(ibex::Interval::EMPTY_SET);
            ibex::Interval t;

            IntervalVector out0_lb = IntervalVector(out.lb());
            IntervalVector vec_out0_lb = -m_maze->get_dynamics()->eval(out0_lb)[0];
            ibex::Interval a1=-0.5*vect_d1[in_comp];
            ibex::Interval b1=vec_out0_lb[in_comp];
            ibex::Interval c1=out0_lb[in_comp]-in[in_comp].lb();

            if(a1 == ibex::Interval(0)){
                t = -c1/b1 & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_lb[1-in_comp];
            }
            else{
                t = (-b1+sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1) & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_lb[1-in_comp];

                t = (-b1-sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1) & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_lb[1-in_comp];
            }

            IntervalVector out0_ub = IntervalVector(out.ub());
            IntervalVector vec_out0_ub = -m_maze->get_dynamics()->eval(out0_ub)[0];
            a1=-0.5*vect_d1[in_comp];
            b1=vec_out0_ub[in_comp];
            c1=out0_ub[in_comp]-in[in_comp].lb();

            if(a1 == ibex::Interval(0)){
                t = -c1/b1 & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_ub[1-in_comp];
            }
            else{
                t = (-b1+sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1) & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_ub[1-in_comp];

                t = (-b1-sqrt((pow(b1, 2)-4*a1*c1) & ibex::Interval::POS_REALS))/(2*a1) & ibex::Interval::POS_REALS;
                x |= -0.5*vect_d1[1-in_comp]*(pow(t,2))+vec_out0_lb[1-in_comp]*t+out0_ub[1-in_comp];
            }

            in[1-in_comp] &= x;
        }
    }
    else{
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
            out &= c+in;
        if(sens==BWD || sens==FWD_BWD)
            in &= out-c;
    }

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

template<>
int get_nb_dim_flat<ibex::IntervalVector, ibex::IntervalVector>(const ibex::IntervalVector &iv){
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

template<>
int get_nb_dim_flat<ppl::C_Polyhedron, ppl::Generator_System>(const Parma_Polyhedra_Library::C_Polyhedron &p){
    return p.space_dimension() - p.affine_dimension();
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

