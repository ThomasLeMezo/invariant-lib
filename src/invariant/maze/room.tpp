#include "iostream"

#include "ibex_IntervalVector.h"
#include "../definition/dynamics.h"
#include "../definition/domain.h"
#include "pave.h"
#include "maze.h"
#include "door.h"
#include "../smartSubPaving/face.h"

namespace invariant {

template<typename _Tp, typename _V>
Room<_Tp, _V>::Room(Pave<_Tp, _V> *p, Maze<_Tp, _V> *m, Dynamics *dynamics)
{
    m_pave = p;
    m_maze = m;
    const ibex::IntervalVector position(p->get_position());
    this->compute_vector_field();

    // Eval Vector field d1
    m_vector_fields_d1 = dynamics->eval_d1(position);

    // Create Doors
    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door<_Tp, _V> *d = new Door<_Tp, _V>(m_pave->get_faces()[face][sens], this);
            Face<_Tp, _V> *f = m_pave->get_faces()[face][sens];
            f->add_door(d);
        }
    }

    m_full = true;
    m_empty = false;
    m_full_first_eval = true;
    m_empty_first_eval = true;
    m_removed = false;

    omp_init_lock(&m_lock_contraction);
    omp_init_lock(&m_lock_deque);
    omp_init_lock(&m_lock_vector_field);

    compute_vector_field_typed();

    //    for(int face_in=0; face_in<dim; face_in++){
    //        std::vector< std::vector<bool>> vect_temp;
    //        for(int sens_in = 0; sens_in < 2; sens_in++){
    //            Face<_Tp, _V>* f_in = m_pave->get_faces()[face_in][sens_in];
    //            Door<_Tp, _V>* door_in = f_in->get_doors()[m_maze];
    //            vect_temp.push_back(door_in->is_collinear());
    //        }
    //        m_door_collinearity.push_back(vect_temp);
    //    }

    ibex::Variable v(9);
    m_contract_function = new ibex::Function(v, ibex::Return(v[0]*pow(v[8],2)+v[2]*v[8]+v[4]-v[6], v[1]*pow(v[8],2)+v[3]*v[8]+v[5]-v[7]));
    m_ctc = new ibex::CtcFwdBwd(*m_contract_function);

}

template<typename _Tp, typename _V>
Room<_Tp, _V>::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
    omp_destroy_lock(&m_lock_vector_field);
    delete(m_ctc);
    delete(m_contract_function);
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::compute_vector_field(){
    m_vector_field_zero.clear();
    m_vector_fields.clear();
    const ibex::IntervalVector position(m_pave->get_position());
    std::vector<ibex::IntervalVector> vector_field_list = m_maze->get_dynamics()->eval(position);
    // Eval Vector field
    for(ibex::IntervalVector &vector_field:vector_field_list){
        // Test if 0 is inside the vector_field IV
        ibex::IntervalVector zero(m_pave->get_dim(), ibex::Interval::ZERO);
        if((zero.is_subset(vector_field))){
            m_vector_field_zero.push_back(true);
            m_contain_zero = true;
        }
        else{
            m_vector_field_zero.push_back(false);
        }
        m_vector_fields.push_back(vector_field);
        for(int i=0; i<vector_field.size(); i++){
            if(!(vector_field[i] & ibex::Interval::ZERO).is_empty())
                m_contain_zero_coordinate = true;
        }
    }
}


template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_empty_private_output(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_output();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_empty_private_input(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_input();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_full_private_output(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_output();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_full_private_input(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_input();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_empty_private(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_full_private(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::contract_vector_field(){
    int dim = m_pave->get_dim();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();
    ibex::IntervalVector zero(dim, ibex::Interval::ZERO);

    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];

        for(const ibex::IntervalVector &v:m_vector_fields){
            /// ************* Compute Vector possibilities *************
            // Construct the boolean interval vector of the vector_field
            ibex::IntervalVector v_bool_in = ibex::IntervalVector(dim, ibex::Interval::EMPTY_SET);
            ibex::IntervalVector v_bool_out = ibex::IntervalVector(dim, ibex::Interval::EMPTY_SET);

            for(int i=0; i<dim; i++){
                if(!(v[i] & ibex::Interval::POS_REALS).is_empty())
                    v_bool_out[i] |= ibex::Interval(1);
                if(!(v[i] & ibex::Interval::NEG_REALS).is_empty())
                    v_bool_out[i] |= ibex::Interval(0);
                if(!((-v[i]) & ibex::Interval::POS_REALS).is_empty())
                    v_bool_in[i] |= ibex::Interval(1);
                if(!((-v[i]) & ibex::Interval::NEG_REALS).is_empty())
                    v_bool_in[i] |= ibex::Interval(0);
            }

            if((f->get_orientation() & v_bool_in).is_empty()){
                d->push_back_possible_in(false);
                if(!zero.is_subset(v) && domain_init == FULL_DOOR)
                    d->set_empty_private_input();
            }
            else
                d->push_back_possible_in(true);

            if((f->get_orientation() & v_bool_out).is_empty()){
                d->push_back_possible_out(false);
                if(!zero.is_subset(v) && domain_init == FULL_DOOR)
                    d->set_empty_private_output();
            }
            else
                d->push_back_possible_out(true);

            /// ************* Compute Collinearity *************
            ibex::IntervalVector product = hadamard_product(v, f->get_normal());

            // Collinearity
            if(zero.is_subset(product))
                d->push_back_collinear_vector_field(true);
            else
                d->push_back_collinear_vector_field(false);

            // Composante collineaire
            std::vector<bool> where_zeros;
            for(int n_dim=0; n_dim<dim; n_dim++){
                if(ibex::Interval::ZERO.is_subset(v[n_dim]))
                    where_zeros.push_back(true);
                else
                    where_zeros.push_back(false);
            }
            d->push_back_zeros_in_vector_field(where_zeros);
        }

        // Note : synchronization will be proceed at the end of all contractors
        // to avoid unecessary lock
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::compute_sliding_mode(const int n_vf, std::vector<std::vector< std::array<_Tp, 2> > > &out_results, std::vector<std::vector<std::array<_Tp, 2> > > &in_results){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face=0; face<dim; face++){
        for(int sens = 0; sens < 2; sens++){
            Face<_Tp, _V>* f_in = m_pave->get_faces()[face][sens];
            Door<_Tp, _V>* door = f_in->get_doors()[m_maze];

            if(domain_init == FULL_DOOR && door->is_collinear()[n_vf]){
                /// INPUT
                _Tp out_return = get_empty_door_container<_Tp, _V>(dim);
                _Tp in_return = get_empty_door_container<_Tp, _V>(dim);
                contract_sliding_mode(n_vf, face, sens, out_return, in_return);

                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                    door->set_input_private(in_return); // Write input
                    in_results[n_vf][face][sens] = in_return;

                    /// Impact on other faces (OUT -> IN)
                    if(!out_return.is_empty()){
                        for(int face_in=0; face_in<dim; face_in++){
                            for(int sens_in = 0; sens_in < 2; sens_in++){
                                if(!(face_in == face && sens_in == sens)){
                                    Face<_Tp, _V>* f_out = m_pave->get_faces()[face_in][sens_in];
                                    Door<_Tp, _V>* door_in = f_out->get_doors()[m_maze];
                                    _Tp out_tmp(out_return);
                                    _Tp in_tmp(door_in->get_input_private());
                                    if(!in_tmp.is_empty() && door_in->is_possible_in()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_fwd(n_vf), BWD); // ToDo : check _fwd ?
                                        in_results[n_vf][face_in][sens_in] |= in_tmp;
                                    }
                                }
                            }
                        }
                    }
                }
                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                    /// Write output here to improve standard mode efficency
                    door->set_output_private(out_return); // Write output
                    out_results[n_vf][face][sens] = out_return;

                    /// Impact on other faces (IN -> OUT)
                    if(!in_return.is_empty()){
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){
                                if(!(face_out == face && sens_out == sens)){
                                    Face<_Tp, _V>* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door<_Tp, _V>* door_out = f_out->get_doors()[m_maze];
                                    _Tp out_tmp(door_out->get_output_private());
                                    _Tp in_tmp(in_return);
                                    if(!out_tmp.is_empty() && door_out->is_possible_out()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_fwd(n_vf), FWD);
                                        out_results[n_vf][face_out][sens_out] |= out_tmp;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::compute_standard_mode(const int n_vf, std::vector<std::vector<std::array<_Tp, 2> > > &out_results, std::vector<std::vector<std::array<_Tp, 2> > > &in_results){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face_in=0; face_in<dim; face_in++){
        for(int sens_in = 0; sens_in < 2; sens_in++){
            Face<_Tp, _V>* f_in = m_pave->get_faces()[face_in][sens_in];
            Door<_Tp, _V>* door_in = f_in->get_doors()[m_maze];

            if(!(domain_init == FULL_DOOR && door_in->is_collinear()[n_vf])){ // avoid sliding mode on face in
                const _Tp in(door_in->get_input_private());

                for(int face_out=0; face_out<dim; face_out++){
                    for(int sens_out = 0; sens_out < 2; sens_out++){
                        if(!(face_out == face_in && sens_out == sens_in)){
                            Face<_Tp, _V>* f_out = m_pave->get_faces()[face_out][sens_out];
                            Door<_Tp, _V>* door_out = f_out->get_doors()[m_maze];
                            if(!(domain_init == FULL_DOOR && door_out->is_collinear()[n_vf])){ // avoid sliding mode on face out
                                const _Tp out(door_out->get_output_private());

                                /// ************ IN -> OUT ************
                                if((dynamics_sens == FWD || dynamics_sens == FWD_BWD) && door_out->is_possible_out()[n_vf]){
                                    _Tp in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        out_tmp = f_out->get_position_typed();

                                    if(!out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_fwd(n_vf), FWD);
                                        out_results[n_vf][face_out][sens_out] |= out_tmp;
                                    }
                                }
                                /// ************ OUT -> IN ************
                                if((dynamics_sens == BWD || dynamics_sens == FWD_BWD) && door_in->is_possible_in()[n_vf]){
                                    _Tp in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        in_tmp = f_in->get_position_typed();

                                    if(!out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_bwd(n_vf), BWD);
                                        in_results[n_vf][face_in][sens_in] |= in_tmp;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::contract_consistency(){
    const int dim = m_pave->get_dim();

    /// Deal with netcdf empty data (set to empty vec_field)
    ibex::IntervalVector empty = ibex::IntervalVector(dim, ibex::Interval::EMPTY_SET);
    for(ibex::IntervalVector &vec_field:get_vector_fields()){
        if(vec_field == empty)
            return;
    }

    m_nb_contract++;

    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    /// ************ Create empty results vector ************ //
    std::vector<std::vector< std::array<_Tp, 2> > > out_results, in_results; // One per vec_field, dim, sens
    const int nb_vec = m_vector_fields.size();
    for(int vf=0; vf<nb_vec; vf++){
        std::vector< std::array<_Tp, 2>> out_result;
        for(int i=0; i<dim; i++){
            std::array<_Tp, 2> output = {get_empty_door_container<_Tp, _V>(dim), get_empty_door_container<_Tp, _V>(dim)};
            out_result.push_back(output);
        }
        out_results.push_back(out_result);
        in_results.push_back(out_result);
    }

    /// ************ Compute propagation ************ //
    bool global_compute = false;
    for(int n_vf=0; n_vf<nb_vec; n_vf++){
        if(m_vector_field_zero[n_vf]){ // Case Zero in f
            if(domain_init == FULL_WALL && !is_empty_private())
                this->set_full_possible();
        }
        else{
            global_compute |= true;
            compute_sliding_mode(n_vf, out_results, in_results);
            compute_standard_mode(n_vf, out_results, in_results);
        }
    }

    /// ************ Save propagation to private doors ************ //
    if(global_compute){
        for(int face = 0; face<dim; face++){
            for(int sens = 0; sens < 2; sens++){
                Face<_Tp, _V>* f= m_pave->get_faces()[face][sens];
                Door<_Tp, _V>* door = f->get_doors()[m_maze];

                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                    _Tp door_out_iv(door->get_face()->get_position_typed());
                    bool one_possible = false;
                    for(int n_vf=0; n_vf<nb_vec; n_vf++){
                        if(domain_init == FULL_DOOR || (domain_init == FULL_WALL && door->is_possible_out()[n_vf])){
                            one_possible = true;
                            door_out_iv &= out_results[n_vf][face][sens];
                        }
                    }
                    if(!one_possible) // For Kernel
                        set_empty<_Tp, _V>(door_out_iv);

                    if(domain_init == FULL_DOOR)
                        door->set_output_private(door_out_iv & door->get_output_private());
                    else
                        door->set_output_private(door->get_output_private() | door_out_iv);
                }
                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                    _Tp door_in_iv(door->get_face()->get_position_typed());
                    bool one_possible = false;
                    for(int n_vf=0; n_vf<nb_vec; n_vf++){
                        if(domain_init == FULL_DOOR || (domain_init == FULL_WALL && door->is_possible_in()[n_vf])){
                            one_possible = true;
                            door_in_iv &= in_results[n_vf][face][sens];
                        }
                    }
                    if(!one_possible) // For Kernel
                        set_empty<_Tp, _V>(door_in_iv);

                    if(domain_init == FULL_DOOR)
                        door->set_input_private(door_in_iv & door->get_input_private());
                    else
                        door->set_input_private(door->get_input_private() | door_in_iv);
                }
            }
        }
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::contract_sliding_mode(int n_vf, int face_in, int sens_in, _Tp &out_return, _Tp &in_return){
    if(m_debug_room)
        std::cout << "debug" << std::endl;

    Face<_Tp, _V>* f_in = m_pave->get_faces()[face_in][sens_in];
    Door<_Tp, _V>* door = f_in->get_doors()[m_maze];
    int dim = m_pave->get_dim();
    in_return = get_empty_door_container<_Tp, _V>(dim);
    out_return = get_empty_door_container<_Tp, _V>(dim);

    /// Compute IN_OUT door => Already done by the continuity contractor ??=> bug if removed
    _Tp output_global_door(door->get_output_private());
    _Tp input_global_door(door->get_input_private());
    bool input_full = door->is_full_private_input();
    bool output_full = door->is_full_private_output();
    for(Face<_Tp, _V> *f_n:f_in->get_neighbors()){
        Door<_Tp, _V>* d_n = f_n->get_doors()[m_maze];
        if(!output_full)
            output_global_door |= d_n->get_output();
        if(!input_full)
            input_global_door |= d_n->get_input();
    }
    if(!output_full)
        output_global_door &= f_in->get_position_typed();
    if(!input_full)
        input_global_door &= f_in->get_position_typed();

    /// ************* Find adjacent paves *************
    /// --> In the direction of the zeros

    std::vector<Pave<_Tp, _V> *> adjacent_paves;
    m_maze->get_subpaving()->get_tree()->get_intersection_pave_outer(adjacent_paves, f_in->get_position());

    // Remove pave not in the zero(s) direction
    ibex::IntervalVector vec_field_global(dim, ibex::Interval::EMPTY_SET);
    ibex::IntervalVector vec_field_neighbors(dim, ibex::Interval::EMPTY_SET);

    std::vector<Pave<_Tp, _V> *> adjacent_paves_valid;
    ibex::IntervalVector pave_extrude(f_in->get_position());
    std::vector<bool> where_zeros = door->get_where_zeros(n_vf);

    for(int id_zero=0; id_zero<dim; id_zero++){
        if(where_zeros[(size_t)id_zero])
            pave_extrude[id_zero] = ibex::Interval::ALL_REALS;
    }

    for(Pave<_Tp, _V> *pave_adj:adjacent_paves){
        // Find adjacent paves that extrude this pave in the directions of zeros
        if(pave_adj==this->m_pave){
            adjacent_paves_valid.push_back(pave_adj);
            vec_field_global |= this->get_one_vector_fields(n_vf);
        }
        else{
            ibex::IntervalVector inter_extrude = pave_adj->get_position() & pave_extrude;

            if(get_nb_dim_flat(inter_extrude)==get_nb_dim_flat(pave_extrude)){ // Key point : dim of the intersection equal to dim of the extrude pave
                adjacent_paves_valid.push_back(pave_adj);
                Room<_Tp, _V> *room_n= pave_adj->get_rooms()[m_maze];
                vec_field_global |= room_n->get_one_vector_fields(n_vf);
                vec_field_neighbors |= room_n->get_one_vector_fields(n_vf);
            }
        }
    }
//    if(vec_field_neighbors.is_empty())
//        std::cout << "ERROR" << std::endl;

    ibex::IntervalVector zero(dim, ibex::Interval::ZERO);
    if(zero.is_subset(vec_field_global)){ // Case no contraction (if there is a possible cycle) or border face
        in_return = door->get_input_private();
        out_return = door->get_output_private();
        return;
    }

    /// ************* Compute Consistency *************
    /// For each Pave, propagate OUT -> IN

    _V vec_field_typed_neighbors_fwd = convert_vec_field<_V>(vec_field_neighbors);

    for(Pave<_Tp, _V> *pave_adj:adjacent_paves_valid){
        bool local_pave = false;
        if(pave_adj == m_pave)
            local_pave = true;

        for(int face_out_adj=0; face_out_adj<dim; face_out_adj++){
            for(int sens_out_adj = 0; sens_out_adj < 2; sens_out_adj ++){
                Face<_Tp, _V> *f_out_adj = pave_adj->get_faces()[face_out_adj][sens_out_adj];
                Door<_Tp, _V> *d_out_adj = f_out_adj->get_doors()[m_maze];

                if(!(local_pave && face_out_adj==face_in && sens_out_adj==sens_in) || !local_pave){

                    if((!local_pave && !(d_out_adj->get_output().is_empty() && d_out_adj->get_input().is_empty()))
                            || (local_pave && !(d_out_adj->get_output_private().is_empty() && d_out_adj->get_input_private().is_empty()))){

                        /// ************* Determine if the face is part of the hull *************
                        /// Test if the Face intersect another face of the neighbours ?
                        /// By default, the "own surface" of the adjacent face is equal to its position
                        /// But in the case the face intersect an other face of the adjacents faces
                        /// it means that the intersected part is not on the hull

                        _Tp own_surface(f_out_adj->get_position_typed());
                        for(Face<_Tp, _V> *face_out_n:f_out_adj->get_neighbors()){
                            Pave<_Tp, _V> *pave_out_n = face_out_n->get_pave();

                            /// Find if this face is on the adjacent paves valid list ?
                            bool is_in_adj_pave_list = false;
                            bool is_same_face = false;
                            for(Pave<_Tp, _V> *pave_test:adjacent_paves_valid){
                                if(pave_test == pave_out_n){ // Pointer comparison (?working?)
                                    is_in_adj_pave_list = true;
                                    if(face_out_n == f_out_adj)
                                        is_same_face = true;
                                    break;
                                }
                            }

                            /// Compute the part of the face which is on the hull (reduce the size of own_surface)
                            /// There is an over approximation made because of the diff operator
                            if(is_same_face)
                                set_empty<_Tp, _V>(own_surface);
                            else if(is_in_adj_pave_list){
                                own_surface &= get_diff_hull<_Tp, _V>(f_out_adj->get_position_typed(), face_out_n->get_position_typed() & f_out_adj->get_position_typed());
                            }

                            /// Note : computation of the own_surface will remove the propagation of exact face_in to itself
                        }
                        if(get_nb_dim_flat(own_surface)==2)
                            set_empty<_Tp, _V>(own_surface);

                        /// ************* Compute the propagation *************
                        if(!own_surface.is_empty()){
                            // OUT -> IN
                            if(!input_global_door.is_empty()){
                                /// WARNING : do no set in_tmp_IN only to private door
                                /// because only half part is taken into account => take the union with the IN of the neighbour
                                _Tp in_tmp_IN(input_global_door);

                                _Tp out_tmp_IN(own_surface);
                                if(local_pave)
                                    out_tmp_IN &= d_out_adj->get_output_private();
                                else
                                    out_tmp_IN &= d_out_adj->get_output(); // Do not use the private door here !

                                // Avoid degenerated case of out_tmp_IN (when own_surface reduce out_tmp_IN to dim-2 => border)
                                if(get_nb_dim_flat(out_tmp_IN)==2)
                                    set_empty<_Tp, _V>(out_tmp_IN);

                                if(!out_tmp_IN.is_empty()){
                                    if(local_pave)
                                        contract_flow(in_tmp_IN, out_tmp_IN, get_one_vector_fields_fwd(n_vf), BWD); // The vector is fwd but we need to contract the input
                                    else
                                        contract_flow(in_tmp_IN, out_tmp_IN, vec_field_typed_neighbors_fwd, BWD);

                                    in_return |= in_tmp_IN ;
                                }
                            }

                            // IN -> OUT
                            if(!output_global_door.is_empty()){
                                _Tp out_tmp_OUT(output_global_door);
                                _Tp in_tmp_OUT(own_surface);
                                if(local_pave)
                                    in_tmp_OUT &= d_out_adj->get_input_private();
                                else
                                    in_tmp_OUT &= d_out_adj->get_input();

                                if(get_nb_dim_flat(in_tmp_OUT)==2)
                                    set_empty<_Tp, _V>(in_tmp_OUT);

                                if(!in_tmp_OUT.is_empty()){
                                    if(local_pave)
                                        contract_flow(in_tmp_OUT, out_tmp_OUT, get_one_vector_fields_fwd(n_vf), FWD);
                                    else
                                        contract_flow(in_tmp_OUT, out_tmp_OUT, vec_field_typed_neighbors_fwd, FWD);
                                    out_return |= out_tmp_OUT;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    out_return &= in_return;
    in_return &= out_return;

    in_return &= door->get_input_private();
    out_return &= door->get_output_private();
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_full_possible(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];
        d->set_full_possible_private();
    }
}

template<typename _Tp, typename _V>
bool Room<_Tp, _V>::contract_continuity(){
    bool change = false;
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        if(!f->is_border()){
            Door<_Tp, _V> *d = f->get_doors()[m_maze];
            change |= d->contract_continuity_private();
        }
    }
    /// In the case of sliding mode (only in case of contraction),
    /// a change on the door of an adjacent room which is not on
    /// the boundary can permit contraction (change in the hull)
    /// ie force to proceed to consistency contraction (because
    /// a change may have appended in a distant border).
    if(m_contain_zero_coordinate && m_maze->get_domain()->get_init() == FULL_DOOR)
        change = true;
    return change;
}

//template <typename _Tp, typename _V>
//void Room<_Tp, _V>::reset_update_neighbors(){
//    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
//            Door<_Tp, _V> *d = f->get_doors()[m_maze];
//            d->reset_update_neighbors();
//    }
//}

template<typename _Tp, typename _V>
bool Room<_Tp, _V>::contract(){
    // Do not synchronization in this function or sub-functions
    bool change = false;
    if(!is_removed()){
        DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();
        if(m_first_contract){
            contract_vector_field();
            change = true;
            m_first_contract = false;
        }
        else if((domain_init==FULL_WALL && is_full()) || (domain_init == FULL_DOOR && is_empty())){
            return false;
        }

        get_private_doors_info("before");
        change |= contract_continuity();
        get_private_doors_info("continuity");

        if(change){
            if(!m_first_contract || !is_empty_private())
                contract_consistency();
                get_private_doors_info("consistency");
        }
    }
    return change;
}

template<typename _Tp, typename _V>
bool Room<_Tp, _V>::get_private_doors_info(std::string message, bool cout_message){
    if(m_maze->get_domain()->get_init() != FULL_DOOR)
        return false;
    ibex::IntervalVector position(2);
    //
    position[0] = ibex::Interval(1.875, 2.25);
    position[1] = ibex::Interval(0, 0.375);
    //    ibex::IntervalVector position2(2);
    //    position2[0] = Interval(0.75, 1.5);
    //    position2[1] = Interval(1.5750000000000002, 3.1);
    ibex::IntervalVector position2(position);

    if((m_pave->get_position() == position || m_pave->get_position() == position2)){
        if(cout_message){
            std::cout << message << std::endl;
            if(m_pave->get_position() == position)
                std::cout << "position 1" << std::endl;
            else
                std::cout << "position 2" << std::endl;

            std::cout << "Room = " << m_pave->get_position() << " - " << m_pave->get_faces_vector().size() << " faces" << std::endl;
            for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
                Door<_Tp, _V> *d = f->get_doors()[m_maze];
                std::cout << " Face : ";
                std::ostringstream input, output;
                input << print(d->get_input_private());
                output << print(d->get_output_private());
                std::cout << std::left << "input = " << std::setw(46) << input.str() << " output = " << std::setw(46) << output.str() << std::endl;
            }
            std::cout << "----" << std::endl;
        }
        return true;
    }
    return false;
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::synchronize(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];
        d->synchronize();
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::analyze_change(std::vector<Room *>&list_rooms) const{
    bool change = false;
    for(Face<_Tp, _V>* f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];
        change |= d->analyze_change(list_rooms);
    }
    if(m_contain_zero_coordinate && change){
        get_all_active_neighbors(list_rooms);
    }
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::get_all_active_neighbors(std::vector<Room *> &list_rooms) const{
    for(Face<_Tp, _V>* f:m_pave->get_faces_vector()){
        for(Face<_Tp, _V> *f_n:f->get_neighbors()){
            Room<_Tp, _V> *r_n = f_n->get_pave()->get_rooms()[m_maze];
            if(!r_n->is_removed())
                list_rooms.push_back(r_n);
        }
    }
}

template<typename _Tp, typename _V>
const bool Room<_Tp, _V>::is_empty_private(){
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        if(!f->get_doors()[m_maze]->is_empty_private()){
            return false;
        }
    }
    return true;
}

template<typename _Tp, typename _V>
const bool Room<_Tp, _V>::is_empty(){
    if(m_empty && !m_empty_first_eval)
        return true;
    else{
        m_empty_first_eval = false;
        for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_empty()){
                return false;
            }
        }
        if(m_maze->get_domain()->get_init()!=FULL_WALL)
            m_empty = true;
        return true;
    }
}

template<typename _Tp, typename _V>
const bool Room<_Tp, _V>::is_full(){
    if(!m_full && !m_full_first_eval)
        return false;
    else{
        m_full_first_eval = false;
        for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_full()){
                if(m_maze->get_domain()->get_init()!=FULL_WALL)
                    m_full = false;
                return false;
            }
        }
        return true;
    }
}

template<typename _Tp, typename _V>
bool Room<_Tp, _V>::is_full_union() const{
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];
        if(!d->is_full_union()){
            return false;
        }
    }
    return true;
}

template<typename _Tp, typename _V>
bool Room<_Tp, _V>::request_bisection(){
    return !(is_empty());
}

template<typename _Tp, typename _V>
std::ostream& operator<< (std::ostream& stream, const Room<_Tp, _V>& r) {
    stream << "Room = " << r.get_pave()->get_position() << " - " << r.get_pave()->get_faces_vector().size() << " faces";
    stream << ", vector field = ";
    for(ibex::IntervalVector &v:r.get_vector_fields()){
        stream << v << " ";
    }
    stream << ", vector field d = ";
    for(ibex::IntervalMatrix &v:r.get_vector_fields_d()){
        stream << v << " ";
    }
    stream << std::endl;
    stream << " nb_contractions = " << r.get_nb_contractions();
    stream << ", removed = " << (r.is_removed()?"true":"false");
    stream << std::endl;
    for(Face<_Tp, _V> *f:r.get_pave()->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[r.get_maze()];
        stream << " Face : " << d->get_face()->get_orientation() << " - " << *d << std::endl;
    }
    stream << " contain_zero = " << r.get_contain_zero_coordinate() << std::endl;
    return stream;
}



//template<typename _Tp, typename _V>
//bool Room<_Tp, _V>::is_degenerated(const ibex::IntervalVector& iv){
//    int compt = 0;
//    int dim = m_maze->get_subpaving()->dim();
//    for(int i=0; i<dim; i++){
//        if(iv[i].is_degenerated() && !iv[i].is_unbounded()){
//            compt++;
//        }
//        if(compt == 2){
//            return true;
//        }
//    }
//    return false;
//}

template<typename _Tp, typename _V>
const bool Room<_Tp, _V>::get_one_vector_fields_zero(int n_vf) const{
    return m_vector_field_zero[n_vf];
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::set_removed(){
    omp_set_lock(&m_lock_deque);
    m_removed = true;
    omp_unset_lock(&m_lock_deque);
    // Free memory (private doors)
    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d = f->get_doors()[m_maze];
        d->set_removed();
    }
}

template<typename _Tp, typename _V>
const ibex::IntervalVector Room<_Tp, _V>::get_hull_complementary(){
    if(is_empty())
        return m_pave->get_position();
    ibex::IntervalVector hull = get_hull();
    ibex::IntervalVector *result;
    int nb_box = m_pave->get_position().diff(hull,result);

    ibex::IntervalVector complementary(hull.size(), ibex::Interval::EMPTY_SET);
    for(int i=0; i<nb_box; i++){
        complementary |= result[i];
    }
    return complementary;
}

template<typename _Tp, typename _V>
void Room<_Tp, _V>::contract_box(ibex::IntervalVector& virtual_door_out, ibex::Sep* sep, DOOR_SELECTOR doorSelector){
    ibex::IntervalVector in(m_pave->get_dim()), out(m_pave->get_dim());
    ibex::IntervalVector v_door(virtual_door_out & m_pave->get_position());
    DYNAMICS_SENS sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION init = m_maze->get_domain()->get_init();

    for(Face<_Tp, _V> *f:m_pave->get_faces_vector()){
        Door<_Tp, _V> *d_out = f->get_doors()[m_maze];

        // IN -> OUTPUT
        if(init!=FULL_DOOR){
            d_out->set_output_private(ibex::IntervalVector::empty(m_pave->get_dim()));
            d_out->set_input_private(ibex::IntervalVector::empty(m_pave->get_dim()));
            for(ibex::IntervalVector vect:m_vector_fields){
                in = v_door;
                out = f->get_position();
                if(sens != FWD_BWD){
                    contract_flow(in, out, ((sens==FWD)?1:-1)*vect, FWD);
                    out &= f->get_position();
                    if(doorSelector == DOOR_OUTPUT || doorSelector == DOOR_INPUT_OUTPUT)
                        d_out->set_output_private(d_out->get_output_private() | out);
                    if(doorSelector == DOOR_INPUT || doorSelector == DOOR_INPUT_OUTPUT)
                        d_out->set_input_private(d_out->get_input_private() | out);
                }
                else{
                    ibex::IntervalVector out2(out);
                    contract_flow(in, out, vect, FWD);
                    in = v_door;
                    contract_flow(in, out2, -vect, FWD);
                    out &= f->get_position();
                    out2 &= f->get_position();
                    if(doorSelector == DOOR_OUTPUT || doorSelector == DOOR_INPUT_OUTPUT)
                        d_out->set_output_private(d_out->get_output_private() | out | out2);
                    if(doorSelector == DOOR_INPUT || doorSelector == DOOR_INPUT_OUTPUT)
                        d_out->set_input_private(d_out->get_input_private() | out | out2);
                }
            }
        }
        else{
            ibex::IntervalVector x_in(f->get_position()), x_out(f->get_position());
            sep->separate(x_in, x_out);
            if(doorSelector == DOOR_OUTPUT || doorSelector == DOOR_INPUT_OUTPUT)
                d_out->set_output_private(x_out);
            if(doorSelector == DOOR_INPUT || doorSelector == DOOR_INPUT_OUTPUT)
                d_out->set_input_private(x_out);
        }
    }
}

}
