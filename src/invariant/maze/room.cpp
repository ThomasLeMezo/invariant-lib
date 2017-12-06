#include "room.h"
#include "ibex/ibex_IntervalVector.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
void Room<ibex::IntervalVector, std::vector<ibex::IntervalVector>>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
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

template <>
ibex::IntervalVector get_empty_door_container<ibex::IntervalVector, std::vector<ibex::IntervalVector>>(int dim){
    return IntervalVector(dim, ibex::Interval::EMPTY_SET);
}

template <>
void set_empty<ibex::IntervalVector, std::vector<ibex::IntervalVector>>(ibex::IntervalVector &T){
    T.set_empty();
}

template <>
ibex::IntervalVector get_diff_hull<ibex::IntervalVector, std::vector<ibex::IntervalVector>>(const ibex::IntervalVector &a, const ibex::IntervalVector &b){
    ibex::IntervalVector *diff_list;
    int nb_boxes = a.diff(b, diff_list);

    ibex::IntervalVector union_of_diff(a.size(), ibex::Interval::EMPTY_SET);
    for(int i=0; i<nb_boxes; i++)
        union_of_diff |= diff_list[i];
    delete[] diff_list;
    return union_of_diff;
}

template<>
int get_nb_dim_flat<ibex::IntervalVector, std::vector<ibex::IntervalVector>>(const ibex::IntervalVector &iv){
    int dim = iv.size();
    int flat=0;
    for(int i=0; i<dim; i++){
        if(iv[i].is_degenerated() && !iv[i].is_unbounded())
            flat++;
    }
    return flat;
}

/// ******************  ppl::C_Polyhedron ****************** ///

void recursive_linear_expression_from_iv(const ibex::IntervalVector &vect_field,
                                         int dim,
                                         std::vector<Linear_Expression> &linear_expression_list,
                                         Linear_Expression &local_linear_expression){
    if(dim > 0){
        ppl::Variable x(dim-1);
        Linear_Expression linear_expression_lb = local_linear_expression;
        Linear_Expression linear_expression_ub = local_linear_expression;

        // ToDo: case theta[dim] -> lb=+oo | ub=-oo
        if(std::isinf(vect_field[dim-1].ub()))
            linear_expression_list.push_back(Linear_Expression(x));
        else
            linear_expression_ub += x*ceil(vect_field[dim-1].ub()*IBEX_PPL_PRECISION);

        if(std::isinf(vect_field[dim-1].lb()))
            linear_expression_list.push_back(Linear_Expression(-x));
        else
            linear_expression_lb += x*floor(vect_field[dim-1].lb()*IBEX_PPL_PRECISION);

        recursive_linear_expression_from_iv(vect_field, dim-1, linear_expression_list, linear_expression_ub);
        recursive_linear_expression_from_iv(vect_field, dim-1, linear_expression_list, linear_expression_lb);
    }
    else{
        linear_expression_list.push_back(local_linear_expression);
    }
}

template<>
void Room<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>::contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
    if(vect.is_empty()){
        out = ppl::C_Polyhedron(in.space_dimension(), ppl::EMPTY);
        in = ppl::C_Polyhedron(in.space_dimension(), ppl::EMPTY);
        return;
    }

    if(sens == FWD || sens == FWD_BWD){
        ppl::C_Polyhedron in_tmp(in);
        Linear_Expression e_in = Linear_Expression(0);
        std::vector<Linear_Expression> linear_expression_list_in;
        recursive_linear_expression_from_iv(vect, vect.size(), linear_expression_list_in,e_in);
        for(auto &l:linear_expression_list_in){
            in_tmp.add_generator(ray(l));
        }
        out &= in_tmp;
    }

    if(sens == FWD || sens == FWD_BWD){
        ppl::C_Polyhedron out_tmp(out);
        Linear_Expression e_out = Linear_Expression(0);
        std::vector<Linear_Expression> linear_expression_list_out;
        recursive_linear_expression_from_iv(-vect, vect.size(), linear_expression_list_out,e_out);
        for(auto &l:linear_expression_list_out){
            out_tmp.add_generator(ray(l));
        }
        in &= out_tmp;
    }
}

template <>
Parma_Polyhedra_Library::C_Polyhedron get_empty_door_container<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>(int dim){
    return Parma_Polyhedra_Library::C_Polyhedron(dim, Parma_Polyhedra_Library::EMPTY);
}

template <>
void set_empty<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>(ppl::C_Polyhedron &T){
    T = get_empty_door_container<Parma_Polyhedra_Library::C_Polyhedron,std::vector<Parma_Polyhedra_Library::Generator_System>>(T.space_dimension());
}

template <>
Parma_Polyhedra_Library::C_Polyhedron get_diff_hull<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>(const ppl::C_Polyhedron &a, const ppl::C_Polyhedron &b){
    Parma_Polyhedra_Library::C_Polyhedron tmp(b);
    tmp.poly_difference_assign(a);

    //    Parma_Polyhedra_Library::C_Polyhedron tmp(a);
    //    tmp.poly_difference_assign(b);

    return tmp;
}

template<>
int get_nb_dim_flat<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>(const Parma_Polyhedra_Library::C_Polyhedron &p){
    return p.space_dimension() - p.affine_dimension();
}

/// ******************  Other functions ****************** ///


}
