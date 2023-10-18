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

ibex::IntervalVector convert_iv(const ExpPoly &q){
    return q.getBox();
}

template<>
void RoomIBEX::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect, const DYNAMICS_SENS &sens){
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


    // Contraction with Taylor 1 order
    // ToDo : implement with several vector fields...
//    if(in.size()==2 && !in.is_empty() && !out.is_empty() && m_vector_fields_d1.size()>0){
//        ibex::IntervalMatrix jac=m_vector_fields_d1[0];
//        ibex::IntervalVector vect_d(2);
//        vect_d[0] = jac[0][0]*vect[0] + 0.5*jac[0][1]*vect[1];
//        vect_d[1] = jac[1][1]*vect[1] + 0.5*jac[1][0]*vect[0];
//        ibex::IntervalVector vect_in = m_maze->get_dynamics()->eval(in)[0];

//        if(sens==FWD)
//            out &= taylor_contrat_box(out, vect_d, vect_in, in);
//        else if(sens==BWD)
//            in &= taylor_contrat_box(in, vect_d, -vect_in, out);
//    }
}

template <>
ibex::IntervalVector get_empty_door_container<ibex::IntervalVector>(int dim){
    return IntervalVector(dim, ibex::Interval::EMPTY_SET);
}

template <>
ibex::IntervalVector get_full_door_container<ibex::IntervalVector>(int dim){
    return IntervalVector(dim, ibex::Interval::ALL_REALS);
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
void RoomIBEX::compute_vector_field_typed(){
    m_vector_fields_typed_fwd = m_vector_fields;
    for(ibex::IntervalVector &vect:m_vector_fields){
        m_vector_fields_typed_bwd.push_back(-vect);
    }
}

template <>
const ibex::IntervalVector RoomIBEX::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FaceIBEX *f:get_pave()->get_faces_vector()){
        DoorIBEX *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    return result;
}

template <>
const ibex::IntervalVector RoomIBEX::get_hull_typed() const{
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


/// ******************  invariant::ExpBox ****************** ///

int get_nb_dim_flat(const ExpPoly &Q){
    int r = Q.get_not_flat_dim();
    if (r<0) r=0;
    return Q.get_dim()-r;
}

template <>
ExpPoly get_empty_door_container<ExpPoly>(int dim){
    return ExpPoly(dim, true);
}

template <>
ExpPoly get_full_door_container<ExpPoly>(int dim){
    return ExpPoly(dim, false);
}

template <>
void set_empty<ExpPoly>(ExpPoly &T){
    T.set_empty();
}

#if 0
template <>
ExpPoly get_diff_hull<ExpPoly>(const ExpPoly &a, const ExpPoly &b){
    return diff_hull(a,b);
}
#endif

template <>
void RoomEXP::compute_vector_field_typed(){
    ibex::IntervalVector position(m_pave->get_position());
    // Contract position according to father_hull to get a better vector field approximation
    if(m_maze->get_contract_vector_field() && m_is_father_hull){
        position &= convert_iv(*m_father_hull);
    }

    const std::vector<std::pair<ibex::Matrix,ibex::IntervalVector>> vecJac = m_maze->get_dynamics()->eval_jac(position); /* TODO : have eval_jac with a polyhedron ? */

    int nb = m_vector_fields.size();
    for (int i=0;i<nb;i++) {
       if (vecJac.size()>i) {
          const std::pair<ibex::Matrix,ibex::IntervalVector> &p  = vecJac[i];
          ExpVF vfield(position,m_vector_fields[i],p);
          m_vector_fields_typed_fwd.push_back(vfield);
          m_vector_fields_typed_bwd.push_back(ExpVF(vfield,-1.0));
       } else {
          ExpVF vfield(position,m_vector_fields[i]);
          m_vector_fields_typed_fwd.push_back(vfield);
          m_vector_fields_typed_bwd.push_back(ExpVF(vfield,-1.0));
       }
   }
}

template <>
const ibex::IntervalVector RoomEXP::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FaceEXP *f:get_pave()->get_faces_vector()){
        DoorEXP *d = f->get_doors()[m_maze];
        result |= d->get_hull().getBox();
    }
    return result;
}

template <>
const ExpPoly RoomEXP::get_hull_typed() const{
    ExpPoly result(m_pave->get_position().size(), true);
    for(FaceEXP *f:get_pave()->get_faces_vector()){
        DoorEXP *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    if(m_is_initial_door_input)
        result |= *m_initial_door_input;
    if(m_is_initial_door_output)
        result |= *m_initial_door_output;
    return result;
}

template<>
void RoomEXP::contract_flow(ExpPoly &in, ExpPoly &out, const ExpVF &vect, const DYNAMICS_SENS &sens){
    // Contraction with Taylor 0 order
    // assert 0 not in v.

    // out = in + alpha*v => c=out-in
    // in = out - alpha*v => c=in-out
    if (sens==FWD) {
        vect.contract_flow(in, out);
    } else {
        vect.contract_flow(out, in);
    }

}


/// ******************  ppl::C_Polyhedron ****************** ///

template <>
void RoomPPL::compute_vector_field_typed(){
    for(ibex::IntervalVector &vect:m_vector_fields){
        m_vector_fields_typed_fwd.push_back(iv_2_polyhedron(vect));
        m_vector_fields_typed_bwd.push_back(iv_2_polyhedron(-vect));
    }
}

template<>
void RoomPPL::contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ppl::C_Polyhedron &vect, const DYNAMICS_SENS &sens){
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
ppl::C_Polyhedron get_full_door_container<ppl::C_Polyhedron>(int dim){
    return ppl::C_Polyhedron(dim, Parma_Polyhedra_Library::UNIVERSE);
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
const ibex::IntervalVector RoomPPL::get_hull() const{
    ibex::IntervalVector result(m_pave->get_position().size(), ibex::Interval::EMPTY_SET);
    for(FacePPL *f:get_pave()->get_faces_vector()){
        DoorPPL *d = f->get_doors()[m_maze];
        result |= polyhedron_2_iv(d->get_hull());
    }
    return result;
}

template <>
const ppl::C_Polyhedron RoomPPL::get_hull_typed() const{
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

template<>
ExpVF convert_vec_field<ExpVF>(const ibex::IntervalVector &vect){
    return ExpVF(vect); 
}

template<>
bool contains_v<ibex::IntervalVector>(const ibex::IntervalVector &iv, const ibex::Vector &vect){
    return iv.contains(vect);
}

template<>
bool contains_v<ppl::C_Polyhedron> (const  ppl::C_Polyhedron &iv, const ibex::Vector &vect){
    return polyhedron_2_iv(iv).contains(vect);
}

template<>
bool contains_v<ExpVF> (const ExpVF &iv, const ibex::Vector &vect){
    return iv.getVF().contains(vect);
}

/* FIXME : très "lourd"... est-ce qu'on considère les VF de PPL comme des
   IntervalVector et on fait la transformation au dernier moment ? */
ppl::C_Polyhedron operator-(const ppl::C_Polyhedron &p) {
   ppl::C_Polyhedron p1(p);
   for (int i=0;i<p.space_dimension();i++) {
      ppl::Linear_Expression le;
      le -= ppl::Variable(i);
      p1.affine_image(ppl::Variable(i),le);
   }
   return p1;
}

}

