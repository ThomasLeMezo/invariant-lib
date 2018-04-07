#include "room.h" // only for ide

namespace invariant {

template<typename _Tp>
Room<_Tp>::Room(Pave<_Tp> *p, Maze<_Tp> *m, Dynamics *dynamics):
    m_vector_fields_union(p->get_dim(), ibex::Interval::EMPTY_SET)
{
    m_pave = p;
    m_maze = m;
    const ibex::IntervalVector position(p->get_position());

    // Eval Vector field d1
    m_vector_fields_d1 = dynamics->eval_d1(position);

    // Create Doors
    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door<_Tp> *d = new Door<_Tp>(m_pave->get_faces()[face][sens], this);
            Face<_Tp> *f = m_pave->get_faces()[face][sens];
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

    compute_vector_field();
    compute_vector_field_typed();
}

template<typename _Tp>
inline void Room<_Tp>::reset(){
    m_full = true;
    m_empty = false;
    m_full_first_eval = true;
    m_empty_first_eval = true;
    m_removed = false;
    m_in_deque = false;
    m_first_contract = true;

    m_is_initial_door_input = false;
    m_is_initial_door_output = false;
    if(m_initial_door_input!=nullptr){
        delete(m_initial_door_input);
        m_initial_door_input=nullptr;
    }
    if(m_initial_door_output!=nullptr){
        delete(m_initial_door_output);
        m_initial_door_output=nullptr;
    }
    m_nb_contract = 0;

    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->reset();
    }

    // Vector field reset
    m_vector_fields.clear();
    m_vector_fields_d1.clear();
    m_vector_fields_typed_bwd.clear();
    m_vector_fields_typed_fwd.clear();
    m_vector_field_zero.clear();
    m_zero_component_in_vector_field.clear();
    m_zero_component_in_vector_fields_union.clear();

    m_contract_vector_field = true;
    compute_vector_field();
    compute_vector_field_typed();


}

template<typename _Tp>
Room<_Tp>::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
    omp_destroy_lock(&m_lock_vector_field);

    if(m_initial_door_output != nullptr)
        delete(m_initial_door_output);
    if(m_initial_door_input != nullptr)
        delete(m_initial_door_input);
    if(m_father_hull != nullptr)
        delete(m_father_hull);
}

template<typename _Tp>
void Room<_Tp>::compute_vector_field(){
    m_vector_field_zero.clear();
    m_vector_fields.clear();
    m_contain_zero = false;
    ibex::IntervalVector position(m_pave->get_position());

    // Contract position according to father_hull to get a better vector field approximation
    if(m_maze->get_contract_vector_field() && m_is_father_hull){
        position &= convert_iv(*m_father_hull);
    }

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
        m_vector_fields_union |= vector_field;

    }

    /// ************* Compute Collinearity ************* ///

    for(size_t face=0; face<m_pave->get_dim(); face++){
        for(size_t sens=0; sens<2; sens++){
            Face<_Tp> *f = m_pave->get_faces()[face][sens];
            Door<_Tp> *d = f->get_doors()[m_maze];

            if(ibex::Interval::ZERO.is_subset(m_vector_fields_union[face]))
                d->set_collinear_vector_field_union(true);
            else
                d->set_collinear_vector_field_union(false);
        }
    }

    for(size_t n_vf=0; n_vf<m_vector_fields.size(); n_vf++){
        std::vector<bool> where_zero;
        for(size_t n_dim=0; n_dim<m_pave->get_dim(); n_dim++){
            if(ibex::Interval::ZERO.is_subset(m_vector_fields[n_vf][n_dim])) // v instead of vector_fields_union
                where_zero.push_back(true);
            else
                where_zero.push_back(false);
        }
        m_zero_component_in_vector_field.push_back(where_zero);

        for(size_t face=0; face<m_pave->get_dim(); face++){
            for(size_t sens=0; sens<2; sens++){
                Face<_Tp> *f = m_pave->get_faces()[face][sens];
                Door<_Tp> *d = f->get_doors()[m_maze];

                if(ibex::Interval::ZERO.is_subset(m_vector_fields[n_vf][face]))
                    d->push_back_collinear_vector_field(true);
                else
                    d->push_back_collinear_vector_field(false);
            }
        }
    }

    // Composante collineaire
    for(size_t n_dim=0; n_dim<m_pave->get_dim(); n_dim++){
        if(ibex::Interval::ZERO.is_subset(m_vector_fields_union[n_dim])){ // v instead of vector_fields_union
            m_zero_component_in_vector_fields_union.push_back(true);
            m_contain_zero_coordinate=true;
        }
        else
            m_zero_component_in_vector_fields_union.push_back(false);
    }
}


template<typename _Tp>
void Room<_Tp>::set_empty_private_output(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_output();
    }
}

template<typename _Tp>
void Room<_Tp>::set_empty_private_input(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_input();
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_private_output(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_output();
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_private_input(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_input();
    }
}

template<typename _Tp>
void Room<_Tp>::set_empty_private(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private();
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_private(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private();
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_private_with_father(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->set_full_private_with_father();
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_possible(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->set_full_possible_private();
    }
}

template<typename _Tp>
void Room<_Tp>::contract_according_to_vector_field(){
    if(m_maze->get_domain()->get_init() == FULL_DOOR){
        int dim = m_pave->get_dim();
        ibex::IntervalVector zero(dim, ibex::Interval::ZERO);

        for(int face=0; face<dim; face++){
            for(size_t sens=0; sens<2; sens++){
                Face<_Tp> *f = m_pave->get_faces()[face][sens];
                Door<_Tp> *d = f->get_doors()[m_maze];
                bool input_empty=true;
                bool output_empty=true;
                for(size_t n_vf=0; n_vf<m_vector_fields.size(); n_vf++){
                    if(zero.is_subset(m_vector_fields[n_vf]) || d->is_possible_in()[n_vf])
                        input_empty=false;
                    if(zero.is_subset(m_vector_fields[n_vf]) || d->is_possible_out()[n_vf])
                        output_empty=false;
                }
                if(input_empty)
                    d->set_empty_private_input();
                if(output_empty)
                    d->set_empty_private_output();
            }
        }
    }
}

template<typename _Tp>
void Room<_Tp>::contract_vector_field(){
    int dim = m_pave->get_dim();
    //    get_private_doors_info("contract_vector_field");

    for(int face=0; face<dim; face++){
        for(size_t sens=0; sens<2; sens++){
            Face<_Tp> *f = m_pave->get_faces()[face][sens];
            Door<_Tp> *d = f->get_doors()[m_maze];

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

                // Possible IN
                if((f->get_orientation() & v_bool_in).is_empty()){
                    d->push_back_possible_in(false);
                }
                else
                    d->push_back_possible_in(true);

                // Possible OUT
                if((f->get_orientation() & v_bool_out).is_empty()){
                    d->push_back_possible_out(false);
                }
                else
                    d->push_back_possible_out(true);
            }
        }
    }

    contract_according_to_vector_field();
    // Note : synchronization will be proceed at the end of all contractors
    // to avoid unecessary lock

}

template<typename _Tp>
void Room<_Tp>::compute_sliding_mode(const int n_vf, ResultStorage<_Tp> &result_storage){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face=0; face<dim; face++){
        for(int sens = 0; sens < 2; sens++){
            Face<_Tp>* f_in = m_pave->get_faces()[face][sens];
            Door<_Tp>* door = f_in->get_doors()[m_maze];

            if(door->is_collinear_union()){ // To TEST allowing FULL_WALL Domain instead of just FULL_DOOR!
                /// INPUT
                _Tp out_return = get_empty_door_container<_Tp>(dim);
                _Tp in_return = get_empty_door_container<_Tp>(dim);
                contract_sliding_mode(n_vf, face, sens, out_return, in_return);

                if(f_in->is_border()){
                    if(m_maze->get_domain()->get_border_path_in())
                        in_return |= f_in->get_position_typed();
                    if(m_maze->get_domain()->get_border_path_out())
                        out_return |= f_in->get_position_typed();
                }

                if(domain_init==FULL_WALL){
                    in_return |= door->get_input_private(); // In case of border ?
                    out_return |= door->get_output_private();
                }

                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                    result_storage.push_back_input(in_return, face, sens, face, sens, n_vf);

                    /// Impact : IN (others) <- OUT (self)
                    if(!out_return.is_empty()){
                        for(int face_in=0; face_in<dim; face_in++){
                            for(int sens_in = 0; sens_in < 2; sens_in++){
                                if(!(face_in == face && sens_in == sens)){
                                    Face<_Tp>* f_in = m_pave->get_faces()[face_in][sens_in];
                                    Door<_Tp>* door_in = f_in->get_doors()[m_maze];
                                    _Tp out_tmp(out_return);
                                    _Tp in_tmp(door_in->get_input_private());
                                    if(domain_init == FULL_WALL)
                                        in_tmp = f_in->get_position_typed();

                                    if(!in_tmp.is_empty() && door_in->is_possible_in()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_bwd(n_vf), BWD); // ToDo : check _fwd ?
                                        result_storage.push_back_input(in_tmp, face_in, sens_in, face, sens, n_vf);
                                    }
                                }
                            }
                        }
                    }
                    /// Impact : IN (self) <- OUT (others)
                    if(!in_return.is_empty()){
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){
                                if(!(face_out == face && sens_out == sens)){
                                    //                                    result_storage.push_back_input(in_return, face, sens, face_out, sens_out, n_vf); // To be tested
                                    Face<_Tp>* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door<_Tp>* door_out = f_out->get_doors()[m_maze];

                                    _Tp out_tmp(door_out->get_output_private());
                                    if(domain_init == FULL_WALL)
                                        out_tmp = f_out->get_position_typed();
                                    _Tp in_tmp(in_return);

                                    if(!out_tmp.is_empty() && door_out->is_possible_out()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_bwd(n_vf), BWD);
                                        result_storage.push_back_input(in_tmp, face, sens, face_out, sens_out, n_vf);
                                    }
                                }
                            }
                        }
                    }

                }

                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                    result_storage.push_back_output(out_return, face, sens, face, sens, n_vf);

                    /// Impact : IN (self) -> OUT (others)
                    if(!in_return.is_empty()){
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){
                                if(!(face_out == face && sens_out == sens)){
                                    Face<_Tp>* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door<_Tp>* door_out = f_out->get_doors()[m_maze];

                                    _Tp out_tmp(door_out->get_output_private());
                                    if(domain_init == FULL_WALL)
                                        out_tmp = f_out->get_position_typed();
                                    _Tp in_tmp(in_return);

                                    if(!out_tmp.is_empty() && door_out->is_possible_out()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_fwd(n_vf), FWD);
                                        result_storage.push_back_output(out_tmp, face, sens, face_out, sens_out, n_vf);
                                    }
                                }
                            }
                        }
                    }
                    /// Impact : IN (others) -> OUT (self)
                    if(!out_return.is_empty()){
                        for(int face_in=0; face_in<dim; face_in++){
                            for(int sens_in = 0; sens_in < 2; sens_in++){
                                if(!(face_in == face && sens_in == sens)){
                                    //                                    result_storage.push_back_output(out_return, face_in, sens_in, face, sens, n_vf); // Test improved version
                                    Face<_Tp>* f_in = m_pave->get_faces()[face_in][sens_in];
                                    Door<_Tp>* door_in = f_in->get_doors()[m_maze];
                                    _Tp out_tmp(out_return);
                                    _Tp in_tmp(door_in->get_input_private());

                                    if(!in_tmp.is_empty() && door_in->is_possible_in()[n_vf]){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_fwd(n_vf), FWD); // ToDo : check _fwd ?
                                        result_storage.push_back_output(out_tmp, face_in, sens_in, face, sens, n_vf);
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

template<typename _Tp>
void Room<_Tp>::contract_sliding_mode(int n_vf, int face, int sens, _Tp &out_return, _Tp &in_return){
    Face<_Tp>* f_in = m_pave->get_faces()[face][sens];
    Door<_Tp>* door = f_in->get_doors()[m_maze];
    int dim = m_pave->get_dim();
    in_return = get_empty_door_container<_Tp>(dim);
    out_return = get_empty_door_container<_Tp>(dim);
    _Tp position_typed = f_in->get_position_typed();

    /// Compute IN_OUT door => Already done by the continuity contractor ??=> bug if removed
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();
    _Tp output_global_door(door->get_output_private());
    _Tp input_global_door(door->get_input_private());
    if(domain_init==FULL_WALL){
        output_global_door = position_typed;
        input_global_door = position_typed;
    }

    bool input_full = door->is_full_private_input();
    bool output_full = door->is_full_private_output();
    if((!input_full || !output_full) && domain_init==FULL_DOOR){
        for(Face<_Tp> *f_n:f_in->get_neighbors()){
            Door<_Tp>* d_n = f_n->get_doors()[m_maze];
            if(!output_full)
                output_global_door |= d_n->get_output();
            if(!input_full)
                input_global_door |= d_n->get_input();
        }
        if(!output_full)
            output_global_door &= position_typed;
        if(!input_full)
            input_global_door &= position_typed;
    }

    if(output_global_door.is_empty() || input_global_door.is_empty()){
        in_return = in_return; // i.e. empty container (cf. before)
        out_return = out_return;
        return;
    }

    /// ************* Find adjacent paves *************
    /// --> In the direction of the zeros

    std::vector<Pave<_Tp> *> adjacent_paves;
    m_maze->get_subpaving()->get_tree()->get_intersection_pave_outer(adjacent_paves, f_in->get_position());

    // Remove pave not in the zero(s) direction
    ibex::IntervalVector vec_field_global(dim, ibex::Interval::EMPTY_SET);
    ibex::IntervalVector vec_field_neighbors(dim, ibex::Interval::EMPTY_SET);

    std::vector<Pave<_Tp> *> adjacent_paves_valid;
    ibex::IntervalVector pave_extrude(f_in->get_position());

    for(int id_zero=0; id_zero<dim; id_zero++){
        // ToDo : source of bug : select more than required pave (issue difficult to solve...)
        // Should not be the union but required when there is command (otherwise it might won't proceed to the contraction)
        // It then selects more pave than required
        // One solution : add a constraint to remove "point" intersection (see get_nb_dim_flat(inter) : a point has "dim" flat dimension)
        if(m_zero_component_in_vector_fields_union[(size_t)id_zero])
            pave_extrude[id_zero] = ibex::Interval::ALL_REALS;
    }

    for(Pave<_Tp> *pave_adj:adjacent_paves){
        // Find adjacent paves that extrude this pave in the directions of zeros
        if(pave_adj==this->m_pave){
            adjacent_paves_valid.push_back(pave_adj);
            vec_field_global |= this->get_one_vector_fields(n_vf);
        }
        else{
            ibex::IntervalVector inter_extrude = pave_adj->get_position() & pave_extrude;
            ibex::IntervalVector inter = pave_adj->get_position() & f_in->get_position();

            if(get_nb_dim_flat(inter_extrude)==get_nb_dim_flat(pave_extrude) && get_nb_dim_flat(inter)<dim){ // Key point : dim of the intersection equal to dim of the extrude pave
                adjacent_paves_valid.push_back(pave_adj);
                Room<_Tp> *room_n= pave_adj->get_rooms()[m_maze];
                vec_field_global |= room_n->get_one_vector_fields(n_vf);
                vec_field_neighbors |= room_n->get_one_vector_fields(n_vf);
            }
        }
    }
    //    if(vec_field_neighbors.is_empty())
    //        std::cout << "ERROR" << std::endl;

    ibex::IntervalVector zero(dim, ibex::Interval::ZERO);
    if(zero.is_subset(vec_field_global)){ // Case no contraction (if there is a possible cycle) or border face
        in_return = input_global_door;
        out_return = output_global_door;
        return;
    }

    /// ************* Compute Consistency *************
    /// For each Pave, propagate OUT -> IN

    _Tp vec_field_typed_neighbors_fwd = convert_vec_field<_Tp>(vec_field_neighbors);
    _Tp vec_field_typed_neighbors_bwd = convert_vec_field<_Tp>(-vec_field_neighbors);

    for(Pave<_Tp> *pave_adj:adjacent_paves_valid){
        bool local_pave = false;
        if(pave_adj == m_pave)
            local_pave = true;

        // ******** INITIAL CONDITION ******** //
        Room<_Tp> *r_adj = pave_adj->get_rooms()[m_maze];
        if(r_adj->is_initial_door_output()){
            _Tp in_tmp_IN(input_global_door);
            _Tp out_tmp_IN(r_adj->get_initial_door_output());
            if(!out_tmp_IN.is_empty()){
                if(local_pave)
                    contract_flow(in_tmp_IN, out_tmp_IN, get_one_vector_fields_typed_bwd(n_vf), BWD);
                else
                    contract_flow(in_tmp_IN, out_tmp_IN, vec_field_typed_neighbors_bwd, BWD);
                in_return |= in_tmp_IN ;
            }
        }
        if(r_adj->is_initial_door_input()){
            _Tp out_tmp_OUT(output_global_door);
            _Tp in_tmp_OUT(r_adj->get_initial_door_input());
            if(!in_tmp_OUT.is_empty()){
                if(local_pave)
                    contract_flow(in_tmp_OUT, out_tmp_OUT, get_one_vector_fields_typed_fwd(n_vf), FWD);
                else
                    contract_flow(in_tmp_OUT, out_tmp_OUT, vec_field_typed_neighbors_fwd, FWD);
                out_return |= out_tmp_OUT;
            }
        }

        // ******** STANDRAD ROOM ******** //
        for(int face_adj=0; face_adj<dim; face_adj++){
            for(int sens_adj = 0; sens_adj < 2; sens_adj ++){
                Face<_Tp> *f_adj = pave_adj->get_faces()[face_adj][sens_adj];
                Door<_Tp> *d_adj = f_adj->get_doors()[m_maze];

                if(!(local_pave && face_adj==face && sens_adj==sens) || !local_pave){ // Do not consider own [face, sens],  but consider the neighbor same door (that might be larger)

                    if((!local_pave && !(d_adj->get_output().is_empty() && d_adj->get_input().is_empty()))
                       || (local_pave && !(d_adj->get_output_private().is_empty() && d_adj->get_input_private().is_empty()))){

                        /// ************* Determine if the face is part of the hull *************
                        /// Test if the Face intersect another face of the neighbours ?
                        /// By default, the "own surface" of the adjacent face is equal to its position
                        /// But in the case the face intersect an other face of the adjacents faces
                        /// it means that the intersected part is not on the hull

                        _Tp own_surface(f_adj->get_position_typed());
                        for(Face<_Tp> *face_out_n:f_adj->get_neighbors()){
                            Pave<_Tp> *pave_out_n = face_out_n->get_pave();

                            /// Find if this face is on the adjacent paves valid list ?
                            bool is_in_adj_pave_list = false;
                            bool is_same_face = false;
                            for(Pave<_Tp> *pave_test:adjacent_paves_valid){
                                if(pave_test == pave_out_n){ // Pointer comparison (?working?)
                                    is_in_adj_pave_list = true;
                                    if(face_out_n == f_adj)
                                        is_same_face = true;
                                    break;
                                }
                            }

                            /// Compute the part of the face which is on the hull (reduce the size of own_surface)
                            /// There is an over approximation made because of the diff operator
                            if(is_same_face)
                                set_empty<_Tp>(own_surface);
                            else if(is_in_adj_pave_list){
                                own_surface &= get_diff_hull<_Tp>(own_surface, face_out_n->get_position_typed() & own_surface); // Optimization (bug with PPL?)
                            }

                            /// Note : computation of the own_surface will remove the propagation of exact face_in to itself
                        }

                        /// ************* Compute the propagation *************
                        if(!own_surface.is_empty() && get_nb_dim_flat(own_surface)<2){
                            ///*************** OUT -> IN ***************///
                            /// WARNING : do no set in_tmp_IN only to private door
                            /// because only half part is taken into account => take the union with the IN of the neighbour
                            _Tp in_tmp_IN(input_global_door);

                            _Tp out_tmp_IN(own_surface);
                            /// Note : do not use output door for Wall (in case of propagation...) // ToDo: to be validated
                            if(domain_init!=FULL_WALL){
                                if(local_pave)
                                    out_tmp_IN &= d_adj->get_output_private();
                                else
                                    out_tmp_IN &= d_adj->get_output(); // Do not use the private door here !
                            }

                            // Avoid degenerated case of out_tmp_IN (when own_surface reduce out_tmp_IN to dim-2 => border)
                            if(!out_tmp_IN.is_empty() && get_nb_dim_flat(out_tmp_IN)<2){
                                if(local_pave)
                                    contract_flow(in_tmp_IN, out_tmp_IN, get_one_vector_fields_typed_bwd(n_vf), BWD); // The vector is fwd but we need to contract the input
                                else
                                    contract_flow(in_tmp_IN, out_tmp_IN, vec_field_typed_neighbors_bwd, BWD);
                                in_return |= in_tmp_IN ;
                            }

                            ///*************** IN -> OUT ***************///
                            _Tp out_tmp_OUT(output_global_door);
                            _Tp in_tmp_OUT(own_surface);

                            /// Note : Use input door in both WALL & DOOR case
                            if(local_pave)
                                in_tmp_OUT &= d_adj->get_input_private(); // Fwd case (?)
                            else
                                in_tmp_OUT &= d_adj->get_input();

                            if(!in_tmp_OUT.is_empty() && (get_nb_dim_flat(in_tmp_OUT)<2)){
                                if(local_pave)
                                    contract_flow(in_tmp_OUT, out_tmp_OUT, get_one_vector_fields_typed_fwd(n_vf), FWD);
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

    if(domain_init==FULL_DOOR){ // Only for DOOR
//        out_return &= in_return;
//        in_return &= out_return;

        in_return &= door->get_input_private();
        out_return &= door->get_output_private();
    }
    else{
        // Order seems to be different if FULL_WALL
        in_return &= out_return;
        out_return &= in_return;
    }
}

template<typename _Tp>
void Room<_Tp>::compute_standard_mode(const int n_vf, ResultStorage<_Tp> &result_storage){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face_in=0; face_in<dim; face_in++){
        for(int sens_in = 0; sens_in < 2; sens_in++){
            Face<_Tp>* f_in = m_pave->get_faces()[face_in][sens_in];
            Door<_Tp>* door_in = f_in->get_doors()[m_maze];

            if(!door_in->is_collinear_union()){ // avoid sliding mode on face in
                const _Tp in(door_in->get_input_private());

                for(int face_out=0; face_out<dim; face_out++){
                    for(int sens_out = 0; sens_out < 2; sens_out++){
                        if(!(face_out == face_in && sens_out == sens_in)){ // Done with sliding mode if necessary
                            Face<_Tp>* f_out = m_pave->get_faces()[face_out][sens_out];
                            Door<_Tp>* door_out = f_out->get_doors()[m_maze];
                            if(!door_out->is_collinear_union()){ // avoid sliding mode on face out
                                const _Tp out(door_out->get_output_private());

                                /// ************ IN -> OUT ************
                                if((dynamics_sens == FWD || dynamics_sens == FWD_BWD)){
                                    _Tp in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        out_tmp = f_out->get_position_typed();

                                    if(door_out->is_possible_out()[n_vf] && !out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_fwd(n_vf), FWD);
                                        result_storage.push_back_output(out_tmp, face_in, sens_in, face_out, sens_out, n_vf);
                                    }
                                }

                                /// ************ OUT -> IN ************
                                if((dynamics_sens == BWD || dynamics_sens == FWD_BWD)){
                                    _Tp in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        in_tmp = f_in->get_position_typed();

                                    if(door_in->is_possible_in()[n_vf] && !out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_bwd(n_vf), BWD);
                                        result_storage.push_back_input(in_tmp, face_in, sens_in, face_out, sens_out, n_vf);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Initial condition
    if(m_is_initial_door_input || m_is_initial_door_output){
        for(int face=0; face<dim; face++){
            for(int sens =0 ; sens <2; sens++){
                Face<_Tp>* f = m_pave->get_faces()[face][sens];
                Door<_Tp>* door = f->get_doors()[m_maze];

                if(m_is_initial_door_input && (dynamics_sens == FWD || dynamics_sens == FWD_BWD) && !m_initial_door_input->is_empty() && door->is_possible_out()[n_vf]){
                    _Tp in_tmp(*m_initial_door_input);
                    _Tp out_tmp(door->get_output_private());
                    if(domain_init == FULL_WALL)
                        out_tmp = f->get_position_typed();

                    if(!out_tmp.is_empty()){
                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_fwd(n_vf), FWD);
                        result_storage.push_back_output_initial(out_tmp, face, sens, n_vf);
                    }
                }

                if(m_is_initial_door_output && (dynamics_sens == BWD || dynamics_sens == FWD_BWD) && !m_initial_door_output->is_empty() && door->is_possible_in()[n_vf]){
                    _Tp out_tmp(*m_initial_door_output);
                    _Tp in_tmp(door->get_input_private());
                    if(domain_init == FULL_WALL)
                        in_tmp = f->get_position_typed();

                    if(!in_tmp.is_empty()){
                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields_typed_bwd(n_vf), BWD);
                        result_storage.push_back_input_initial(in_tmp, face, sens, n_vf);
                    }

                }
            }
        }
    }
}

template<typename _Tp>
void Room<_Tp>::set_full_result(const int n_vf, ResultStorage<_Tp> &result_storage){
    for(size_t face = 0; face < m_pave->get_dim(); face++){
        for(size_t sens = 0; sens < 2; sens++){
            Face<_Tp> *f = m_pave->get_faces()[face][sens];
            _Tp tmp = f->get_position_typed(); // IN -> OUT [tmp]
            for(size_t face2 = 0; face2 < m_pave->get_dim(); face2++){
                for(size_t sens2 = 0; sens2 < 2; sens2++){
                    result_storage.push_back_input(tmp, face, sens, face2, sens2, n_vf);
                    result_storage.push_back_output(tmp, face2, sens2, face, sens, n_vf);
                }
            }
        }
    }
}

template<typename _Tp>
void Room<_Tp>::contract_consistency(){
    const size_t dim = m_pave->get_dim();

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
    const size_t nb_vec = m_vector_fields.size();
    ResultStorage<_Tp> result_storage(dim, nb_vec);

    /// ************ Compute propagation ************ //
    for(size_t n_vf=0; n_vf<nb_vec; n_vf++){
        if(m_vector_field_zero[n_vf]){ // Case Zero in f --- ToDo : Check working seems WRONG ! ???
            if(!is_empty_private() || (m_is_initial_door_input || m_is_initial_door_output)){
                this->set_full_result(n_vf, result_storage);
                if(domain_init==FULL_WALL){
                    if(dynamics_sens == FWD)
                        this->set_full_private_input();
                    if(dynamics_sens == BWD)
                        this->set_full_private_output();
                }
            }
        }
        else{
            compute_sliding_mode(n_vf, result_storage);
            compute_standard_mode(n_vf, result_storage);
        }
    }

    /// ************ Save propagation to private doors ************ //
    for(size_t face = 0; face<(size_t)dim; face++){
        for(size_t sens = 0; sens < 2; sens++){
            Face<_Tp>* f= m_pave->get_faces()[face][sens];
            Door<_Tp>* door = f->get_doors()[m_maze];

            if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                _Tp door_out_iv = result_storage.get_output(face, sens);

                if(domain_init == FULL_DOOR)
                    door_out_iv &= door->get_output_private();
                else
                    door_out_iv |= door->get_output_private();

                if(m_is_father_hull) // For father hull
                    door_out_iv &= *m_father_hull;

                //                if(get_nb_dim_flat(door_out_iv)==(int)dim){
                //                    door_out_iv = get_empty_door_container<_Tp>(dim);
                //                }
                door->set_output_private(door_out_iv);
            }
            if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                _Tp door_in_iv = result_storage.get_input(face, sens);

                if(domain_init == FULL_DOOR)
                    door_in_iv &= door->get_input_private();
                else
                    door_in_iv |= door->get_input_private();

                if(m_is_father_hull) // For father hull
                    door_in_iv &= *m_father_hull;
                //                if(get_nb_dim_flat(door_in_iv)==(int)dim){
                //                    door_in_iv = get_empty_door_container<_Tp>(dim);
                //                }
                door->set_input_private(door_in_iv);
            }
        }
    }

    /// **** ZERO CONTRACTION *****
    if(m_zero_contraction){
        m_zero_contraction = m_maze->get_domain()->contract_zero_door(this);
    }
}

template<typename _Tp>
bool Room<_Tp>::contract_continuity(){
    bool change = false;
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        if(!f->is_border()){
            Door<_Tp> *d = f->get_doors()[m_maze];
            change |= d->contract_continuity_private();
        }
    }
    /// In the case of sliding mode (only in case of contraction),
    /// a change on the door of an adjacent room which is not on
    /// the boundary can allow contraction (change in the hull)
    /// ie force to proceed to consistency contraction (because
    /// a change may have appended in a distant border).
    if(m_contain_zero_coordinate /*&& m_maze->get_domain()->get_init() == FULL_DOOR*/) // Test if FULL_DOOR is really necessary ?
        change = true;
    return change;
}

template<typename _Tp>
bool Room<_Tp>::contract(){
    // Do not synchronization in this function or sub-functions
    bool change = false;
//    get_private_doors_info("before removed");
    if(!is_removed()){
        DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();
        if(m_contract_vector_field){
            contract_vector_field();
            m_contract_vector_field = false;
        }
        else if(!m_contract_vector_field && m_first_contract){ // 2nd bisection
            contract_according_to_vector_field();
        }
        else if((domain_init==FULL_WALL && is_full()) || (domain_init == FULL_DOOR && is_empty()))
            return false;

        //        get_private_doors_info("before");
        change |= contract_continuity();
//        get_private_doors_info("continuity");

        if((change || m_first_contract)
           && ((m_is_initial_door_input || m_is_initial_door_output) || !is_empty_private())){
            //            get_private_doors_info("before consistency");
            contract_consistency();
            change = true;
            get_private_doors_info("consistency");
        }
    }
    m_first_contract = false;
    return change;
}

template<typename _Tp>
bool Room<_Tp>::get_private_doors_info(std::string message, bool cout_message){
    if(m_maze->get_domain()->get_init() == FULL_DOOR){

        ibex::IntervalVector position(2);
        position[0] = ibex::Interval(2.296875, 2.34375);
        position[1] = ibex::Interval(0, 0.09375);

        //    ibex::IntervalVector position(3);
        //    position[0] = ibex::Interval(0, 450);
        //    position[1] = ibex::Interval(45976.5625, 46074.21875);
        //    position[2] = ibex::Interval(125566.40625, 125664.0625);

        if(m_pave->get_position() == position){
            m_debug_cpt++;
            if(cout_message){
                std::cout << m_debug_cpt << " " << message << std::endl;

                if(m_maze->get_domain()->get_init()==FULL_WALL)
                    std::cout << "FULL_WALL" << std::endl;
                else
                    std::cout << "FULL_DOOR" << std::endl;

                std::cout << "Room = " << m_pave->get_position() << " - " << m_pave->get_faces_vector().size() << " faces" << " - removed = ";
                if(m_removed)
                    std::cout  << "true" << std::endl;
                else
                    std::cout  << "false" << std::endl;

                for(Face<_Tp> *f:m_pave->get_faces_vector()){
                    Door<_Tp> *d = f->get_doors()[m_maze];
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
        else
            return false;
    }
    else
        return false;
}

template<typename _Tp>
void Room<_Tp>::synchronize(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->synchronize();
    }
}

template<typename _Tp>
void Room<_Tp>::analyze_change(std::vector<Room *>&list_rooms) const{
    bool change = false;
    for(Face<_Tp>* f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        change |= d->analyze_change(list_rooms);
    }
    if(change && m_contain_zero_coordinate){ // Because of sliding mode
        get_all_active_neighbors(list_rooms);
    }
    //    if(change){ // Improve of commented function above - To be tested (seems not working well with car on the hill)
    //        for(size_t face = 0; face<m_pave->get_dim(); face++){
    //            for(size_t sens=0; sens<2; sens++){
    //                if(m_zero_component_in_vector_fields_union[face]){
    //                    Face<_Tp> *f = m_pave->get_faces()[face][sens];
    //                    for(Face<_Tp>*f_n:f->get_neighbors()){
    //                        Room<_Tp> *r_n = f_n->get_pave()->get_rooms()[m_maze];
    //                        if(!r_n->is_removed())
    //                            list_rooms.push_back(r_n);
    //                    }
    //                }
    //            }
    //        }
    //    }
}

template<typename _Tp>
void Room<_Tp>::get_all_active_neighbors(std::vector<Room *> &list_rooms) const{
    for(Face<_Tp>* f:m_pave->get_faces_vector()){
        for(Face<_Tp> *f_n:f->get_neighbors()){
            Room<_Tp> *r_n = f_n->get_pave()->get_rooms()[m_maze];
            if(!r_n->is_removed())
                list_rooms.push_back(r_n);
        }
    }
}

template<typename _Tp>
const bool Room<_Tp>::is_empty_private(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        if(!f->get_doors()[m_maze]->is_empty_private()){
            return false;
        }
    }
    return true;
}

template<typename _Tp>
const bool Room<_Tp>::is_empty(){
    if(m_empty && !m_empty_first_eval)
        return true;
    else{
        m_empty_first_eval = false;
        if(m_is_initial_door_input || m_is_initial_door_output)
            return false;
        for(Face<_Tp> *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_empty()){
                return false;
            }
        }
        if(m_maze->get_domain()->get_init()!=FULL_WALL)
            m_empty = true;
        return true;
    }
}

template<typename _Tp>
const bool Room<_Tp>::is_full_private(){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        if(!f->get_doors()[m_maze]->is_full_private()){
            return false;
        }
    }
    return true;
}

template<typename _Tp>
const bool Room<_Tp>::is_full(){
    if(!m_full && !m_full_first_eval)
        return false;
    else{
        m_full_first_eval = false;
        for(Face<_Tp> *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_full()){
                if(m_maze->get_domain()->get_init()!=FULL_WALL)
                    m_full = false;
                return false;
            }
        }
        return true;
    }
}

template<typename _Tp>
bool Room<_Tp>::is_full_union() const{
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        if(!d->is_full_union()){
            return false;
        }
    }
    return true;
}

template<typename _Tp>
bool Room<_Tp>::request_bisection(){
    return !is_removed() && (!is_empty() || (m_first_contract && m_maze->get_domain()->get_init()==FULL_WALL));
}

template<typename _Tp>
std::ostream& operator<< (std::ostream& stream, const Room<_Tp>& r) {
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
    stream << " => nb_contractions = " << r.get_nb_contractions() << std::endl;
    stream << " => removed = " << (r.is_removed()?"true":"false") << std::endl;
    stream << " => border = "  << (r.get_pave()->is_border()?"true":"false") << std::endl;
    stream << " => zero = " << (r.get_contain_zero()?"true":"false") << std::endl;
    stream << " => initial door (input/output) = " << (r.is_initial_door_input()?"true":"false") << "/" << (r.is_initial_door_output()?"true":"false") << std::endl;

    if(r.is_initial_door_input()){
        stream << "    |=> input = " << print(r.get_initial_door_input()) << std::endl;
    }
    if(r.is_initial_door_output()){
        stream << "    |=> output = " << print(r.get_initial_door_output()) << std::endl;
    }

    if(r.is_father_hull())
        stream << " => father hull (true) =" << print(r.get_father_hull()) << std::endl;
    else
        stream << " => father hull (false)" << std::endl;

    for(Face<_Tp> *f:r.get_pave()->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[r.get_maze()];
        stream << " Face : " << d->get_face()->get_orientation() << " - " << *d << std::endl;
    }
    stream << " contain_zero = " << r.get_contain_zero_coordinate() << std::endl;
    return stream;
}

template<typename _Tp>
const bool Room<_Tp>::get_one_vector_fields_zero(int n_vf) const{
    return m_vector_field_zero[n_vf];
}

template<typename _Tp>
void Room<_Tp>::set_removed(){
    omp_set_lock(&m_lock_deque);
    m_removed = true;
    omp_unset_lock(&m_lock_deque);
    // Free memory (private doors)
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->set_removed();
    }

    if(m_is_father_hull){
        delete(m_father_hull);
        m_father_hull = nullptr;
        m_is_father_hull = false;
    }

    // Do not remove initial condition => needed for hull when bisecting (?? ToDo: test ?)
    if(m_is_initial_door_input){
        delete(m_initial_door_input);
        m_initial_door_input=nullptr;
        m_is_initial_door_input=false;
    }
    if(m_is_initial_door_output){
        delete(m_initial_door_output);
        m_initial_door_output=nullptr;
        m_is_initial_door_output=false;
    }
}

template<typename _Tp>
const ibex::IntervalVector Room<_Tp>::get_hull_complementary(){
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

}
