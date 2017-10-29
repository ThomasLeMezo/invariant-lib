#include "room.h"
#include "face.h"
#include "iostream"

using namespace ibex;
using namespace std;
namespace invariant {

Room::Room(Pave *p, Maze *m, Dynamics *dynamics)
{
    m_pave = p;
    m_maze = m;
    const IntervalVector position(p->get_position());
    vector<IntervalVector> vector_field_list = dynamics->eval(position);

    // Eval Vector field
    for(IntervalVector &vector_field:vector_field_list){
        // Test if 0 is inside the vector_field IV
        IntervalVector zero(m_pave->get_dim(), Interval::ZERO);
        if((zero.is_subset(vector_field))){
            m_vector_field_zero.push_back(true);
            m_contain_zero = true;
        }
        else{
            m_vector_field_zero.push_back(false);
        }
        m_vector_fields.push_back(vector_field);
        for(int i=0; i<vector_field.size(); i++){
            if(!(vector_field[i] & Interval::ZERO).is_empty())
                m_contain_zero_coordinate = true;
        }
    }

    // Create Doors
    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door *d = new Door(m_pave->get_faces()[face][sens], this);
            Face *f = m_pave->get_faces()[face][sens];
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

    for(int face_in=0; face_in<dim; face_in++){
        std::vector< std::vector<bool>> vect_temp;
        for(int sens_in = 0; sens_in < 2; sens_in++){
            Face* f_in = m_pave->get_faces()[face_in][sens_in];
            Door* door_in = f_in->get_doors()[m_maze];
            vect_temp.push_back(door_in->is_collinear());
        }
        m_door_collinearity.push_back(vect_temp);
    }
}

Room::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
    omp_destroy_lock(&m_lock_vector_field);
}

void Room::set_empty_private_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_output();
    }
}

void Room::set_empty_private_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_input();
    }
}

void Room::set_full_private_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_output();
    }
}

void Room::set_full_private_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_input();
    }
}

void Room::set_empty_private(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private();
    }
}

void Room::set_full_private(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private();
    }
}

void Room::contract_vector_field(){
    // Test
    //    IntervalVector test(2);
    //    test[0] = Interval(-2.25, -1.5);
    //    test[1] = Interval(-3, -1.5);
    //    if(m_pave->get_position() == test)
    //        cout << "TEST" << endl;

    int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    IntervalVector zero(dim, Interval::ZERO);

    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        vector<IntervalVector> vector_fields_face = m_maze->get_dynamics()->eval(f->get_position());
        //        vector<IntervalVector> vector_fields_face = this->get_vector_fields();

        for(const IntervalVector &v:vector_fields_face){
            if(!zero.is_subset(v)){
                // Construct the boolean interval vector of the vector_field
                IntervalVector v_bool_in = IntervalVector(dim, Interval::EMPTY_SET);
                IntervalVector v_bool_out = IntervalVector(dim, Interval::EMPTY_SET);

                for(int i=0; i<dim; i++){
                    if(!(v[i] & Interval::POS_REALS).is_empty())
                        v_bool_out[i] |= Interval(1);
                    if(!(v[i] & Interval::NEG_REALS).is_empty())
                        v_bool_out[i] |= Interval(0);
                    if(!((-v[i]) & Interval::POS_REALS).is_empty())
                        v_bool_in[i] |= Interval(1);
                    if(!((-v[i]) & Interval::NEG_REALS).is_empty())
                        v_bool_in[i] |= Interval(0);
                }

                if((f->get_orientation() & v_bool_in).is_empty()){
                    if(dynamics_sens == BWD || dynamics_sens == FWD_BWD)
                        d->set_empty_private_input();
                }
                if((f->get_orientation() & v_bool_out).is_empty()){
                    if(dynamics_sens == FWD || dynamics_sens == FWD_BWD)
                        d->set_empty_private_output();
                }
            }

            // Note : synchronization will be proceed at the end of all contractors
            // to avoid unecessary lock
        }
    }
}

void Room::eval_vector_field_possibility(){
    int dim = m_pave->get_dim();
    IntervalVector zero = IntervalVector(dim, Interval::ZERO);

    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        vector<IntervalVector> vector_fields_face = m_maze->get_dynamics()->eval(f->get_position());

        for(IntervalVector &v:vector_fields_face){
            // Construct the boolean interval vector of the vector_field
            IntervalVector v_bool_in = IntervalVector(dim, Interval::EMPTY_SET);
            IntervalVector v_bool_out = IntervalVector(dim, Interval::EMPTY_SET);

            for(int i=0; i<dim; i++){
                if(!(v[i] & Interval::POS_REALS).is_empty())
                    v_bool_out[i] |= Interval(1);
                if(!(v[i] & Interval::NEG_REALS).is_empty())
                    v_bool_out[i] |= Interval(0);
                if(!((-v[i]) & Interval::POS_REALS).is_empty())
                    v_bool_in[i] |= Interval(1);
                if(!((-v[i]) & Interval::NEG_REALS).is_empty())
                    v_bool_in[i] |= Interval(0);
            }

            if((f->get_orientation() & v_bool_in).is_empty())
                d->push_back_possible_in(false);
            else
                d->push_back_possible_in(true);

            if((f->get_orientation() & v_bool_out).is_empty())
                d->push_back_possible_out(false);
            else
                d->push_back_possible_out(true);

            // Note : synchronization will be proceed at the end of all contractors
            // to avoid unecessary lock

            for(const IntervalVector&v:get_vector_fields()){
                IntervalVector product = hadamard_product(v, f->get_normal());
                vector<bool> where_zeros;

                if(zero.is_subset(product)){
                    d->push_back_collinear_vector_field(true);
                    for(int n_dim=0; n_dim<dim; n_dim++){
                        if(Interval::ZERO.is_subset(v[n_dim]))
                            where_zeros.push_back(true);
                        else
                            where_zeros.push_back(false);
                    }
                }
                else
                    d->push_back_collinear_vector_field(false);
                d->push_back_zeros_in_vector_field(where_zeros);
            }
        }
    }
}

void Room::compute_sliding_mode(const int n_vf, vector<vector< array<IntervalVector, 2>>> &out_results, vector<vector<array<IntervalVector, 2>>> &in_results){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face_in=0; face_in<dim; face_in++){
        for(int sens_in = 0; sens_in < 2; sens_in++){
            if(domain_init == FULL_DOOR && m_door_collinearity[face_in][sens_in][n_vf]){
                Face* f_in = m_pave->get_faces()[face_in][sens_in];
                Door* door_in = f_in->get_doors()[m_maze];

                /// INPUT
                IntervalVector out_return(dim, Interval::EMPTY_SET);
                IntervalVector in_return(dim, Interval::EMPTY_SET);
                contract_sliding_mode(n_vf, face_in, sens_in, out_return, in_return);

                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                    door_in->set_input_private(in_return); // Write input
                    in_results[n_vf][face_in][sens_in] = in_return;
                }
                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                    /// Write output here to improve standard mode efficency
                    door_in->set_output_private(out_return); // Write output
                    out_results[n_vf][face_in][sens_in] = out_return;

                    /// Impact on other faces (IN -> OUT)
                    if(!in_return.is_empty()){
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){
                                if(!(face_out == face_in && sens_in == sens_out)){
                                    Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door* door_out = f_out->get_doors()[m_maze];
                                    IntervalVector out_tmp(door_out->get_output_private());
                                    IntervalVector in_tmp(in_return);
                                    if(!out_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields(n_vf));
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

void Room::compute_standard_mode(const int n_vf, vector<vector<array<IntervalVector, 2>>> &out_results, vector<vector<array<IntervalVector, 2>>> &in_results){
    const int dim = m_pave->get_dim();
    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    for(int face_in=0; face_in<dim; face_in++){
        for(int sens_in = 0; sens_in < 2; sens_in++){
            if(!(domain_init == FULL_DOOR && m_door_collinearity[face_in][sens_in][n_vf])){ // avoid sliding mode on face in
                Face* f_in = m_pave->get_faces()[face_in][sens_in];
                Door* door_in = f_in->get_doors()[m_maze];
                const IntervalVector in(door_in->get_input_private());

                for(int face_out=0; face_out<dim; face_out++){
                    for(int sens_out = 0; sens_out < 2; sens_out++){
                        if(!(face_out == face_in && sens_out == sens_in)){
                            if(!(domain_init == FULL_DOOR && m_door_collinearity[face_out][sens_out][n_vf])){ // avoid sliding mode on face out
                                Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                Door* door_out = f_out->get_doors()[m_maze];
                                const IntervalVector out(door_out->get_output_private());

                                /// ************ IN -> OUT ************
                                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                                    IntervalVector in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        out_tmp = f_out->get_position();

                                    if(!out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields(n_vf));
                                        out_results[n_vf][face_out][sens_out] |= out_tmp;
                                    }
                                }
                                /// ************ OUT -> IN ************
                                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                                    IntervalVector in_tmp(in), out_tmp(out);
                                    if(domain_init == FULL_WALL)
                                        in_tmp = f_in->get_position();

                                    if(!out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, get_one_vector_fields(n_vf));
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

void Room::contract_consistency(){
    const int dim = m_pave->get_dim();

    /// Deal with netcdf empty data (set to empty vec_field)
    IntervalVector empty = IntervalVector(dim, Interval::EMPTY_SET);
    for(IntervalVector &vec_field:get_vector_fields()){
        if(vec_field == empty)
            return;
    }

    m_nb_contract++;

    DYNAMICS_SENS dynamics_sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();

    /// ************ Create empty results vector ************ //
    vector<vector< array<IntervalVector, 2>>> out_results, in_results; // One per vec_field, dim, sens
    const int nb_vec = m_vector_fields.size();
    for(int vf=0; vf<nb_vec; vf++){
        std::vector< std::array<IntervalVector, 2>> out_result;
        for(int i=0; i<dim; i++){
            std::array<IntervalVector, 2> output = {IntervalVector::empty(dim), IntervalVector::empty(dim)};
            out_result.push_back(output);
        }
        out_results.push_back(out_result);
        in_results.push_back(out_result);
    }

    /// ************ Compute propagation ************ //
    bool global_compute = false;
    for(int n_vf=0; n_vf<nb_vec; n_vf++){
        if(m_vector_field_zero[n_vf]){ // Case Zero in f
            if(domain_init == FULL_WALL)
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
                Face* f= m_pave->get_faces()[face][sens];
                Door* door = f->get_doors()[m_maze];

                if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
                    IntervalVector door_out_iv(door->get_face()->get_position());
                    bool one_possible = false;
                    for(int n_vf=0; n_vf<nb_vec; n_vf++){
                        if(domain_init == FULL_DOOR || (domain_init == FULL_WALL && door->is_possible_out()[n_vf])){
                            one_possible = true;
                            door_out_iv &= out_results[n_vf][face][sens];
                        }
                    }
                    if(!one_possible) // For Kernel
                        door_out_iv.set_empty();

                    if(domain_init == FULL_DOOR)
                        door->set_output_private(door_out_iv & door->get_output_private());
                    else
                        door->set_output_private(door->get_output_private() | door_out_iv);
                }
                if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
                    IntervalVector door_in_iv(door->get_face()->get_position());
                    bool one_possible = false;
                    for(int n_vf=0; n_vf<nb_vec; n_vf++){
                        if(domain_init == FULL_DOOR || (domain_init == FULL_WALL && door->is_possible_in()[n_vf])){
                            one_possible = true;
                            door_in_iv &= in_results[n_vf][face][sens];
                        }
                    }
                    if(!one_possible) // For Kernel
                        door_in_iv.set_empty();

                    if(domain_init == FULL_DOOR)
                        door->set_input_private(door_in_iv & door->get_input_private());
                    else
                        door->set_input_private(door->get_input_private() | door_in_iv);
                }
            }
        }
    }
}

void Room::contract_sliding_mode(int n_vf, int face_in, int sens_in, IntervalVector &out_return, IntervalVector &in_return){
    if(m_debug_room)
        cout << "debug" << endl;

    Face* f_in = m_pave->get_faces()[face_in][sens_in];
    Door* door_in = f_in->get_doors()[m_maze];
    int dim = m_pave->get_dim();
    in_return = IntervalVector(dim, Interval::EMPTY_SET);
    out_return = IntervalVector(dim, Interval::EMPTY_SET);

    /// Compute IN_OUT door
    IntervalVector output_global_door(door_in->get_output_private());
    IntervalVector input_global_door(door_in->get_input_private());
    for(Face *f_n:f_in->get_neighbors()){
        Door* d_n = f_n->get_doors()[m_maze];
        output_global_door |= (d_n->get_output() & f_in->get_position());
        input_global_door |= (d_n->get_input() & f_in->get_position());
    }

    /// ************* Find adjacent paves *************
    /// --> In the direction of the zeros

    vector<Pave *> adjacent_paves;
    m_maze->get_subpaving()->get_tree()->get_intersection_pave_outer(adjacent_paves, f_in->get_position());

    // Remove pave not in the zero(s) direction
    IntervalVector vec_field_global(dim, Interval::EMPTY_SET);
    IntervalVector vec_field_neighbors(dim, Interval::EMPTY_SET);

    vector<Pave *> adjacent_paves_valid;
    IntervalVector pave_extrude(f_in->get_position());
    vector<bool> where_zeros = door_in->get_where_zeros(n_vf);

    for(int id_zero=0; id_zero<dim; id_zero++){
        if(where_zeros[id_zero])
            pave_extrude[id_zero] = Interval::ALL_REALS;
    }

    for(Pave *pave_adj:adjacent_paves){
        // Find adjacent paves that extrude this pave in the directions of zeros
        IntervalVector inter_extrude = pave_adj->get_position() & pave_extrude;

        if(get_nb_dim_flat(inter_extrude)==get_nb_dim_flat(pave_extrude)){ // Key point : dim of the intersection equal to dim of the extrude pave
            adjacent_paves_valid.push_back(pave_adj);
            Room *room_n= pave_adj->get_rooms()[m_maze];
            vec_field_global |= room_n->get_one_vector_fields(n_vf);
            if(pave_adj->get_position() != get_pave()->get_position()){
                vec_field_neighbors |= room_n->get_one_vector_fields(n_vf);
            }
        }
    }

    IntervalVector zero(dim, Interval::ZERO);
    if(zero.is_subset(vec_field_global)){ // Case no contraction (if there is a possible cycle)
        in_return = door_in->get_input_private();
        out_return = door_in->get_output_private();
        return;
    }

    /// ************* Compute Consistency *************
    /// For each Pave, propagate OUT -> IN

    for(Pave *pave_adj:adjacent_paves_valid){
        bool local_pave = false;
        if(pave_adj->get_position() == m_pave->get_position())
            local_pave = true;
        IntervalVector vec_field_local(pave_adj->get_rooms()[m_maze]->get_one_vector_fields(n_vf));

        for(int face_out_adj=0; face_out_adj<dim; face_out_adj++){
            for(int sens_out_adj = 0; sens_out_adj < 2; sens_out_adj ++){
                Face *f_out_adj = pave_adj->get_faces()[face_out_adj][sens_out_adj];
                Door *d_out_adj = f_out_adj->get_doors()[m_maze];

                if(!(d_out_adj->get_output().is_empty() && d_out_adj->get_input().is_empty())){

                    /// ************* Determine if the face is part of the hull *************
                    /// Test if the Face intersect another face of the neighbours ?
                    /// By default, the "own surface" of the adjacent face is equal to its position
                    /// But in the case the face intersect an other face of the adjacents faces
                    /// it means that the intersected part is not on the hull

                    IntervalVector own_surface(f_out_adj->get_position());
                    for(Face *face_out_n:f_out_adj->get_neighbors()){
                        Pave *pave_out_n = face_out_n->get_pave();

                        /// Find if this face is on the adjacent paves valid list ?
                        bool is_in_adj_pave_list = false;
                        bool is_same_face = false;
                        for(Pave *pave_test:adjacent_paves_valid){
                            if(pave_test->get_position() == pave_out_n->get_position()){ // Pointer comparison (?working?)
                                is_in_adj_pave_list = true;
                                if(face_out_n->get_position() == f_out_adj->get_position())
                                    is_same_face = true;
                                break;
                            }
                        }

                        /// Compute the part of the face which is on the hull (reduce the size of own_surface)
                        /// There is an over approximation made because of the diff operator
                        if(is_same_face)
                            own_surface.set_empty();
                        else if(is_in_adj_pave_list){
                            IntervalVector *diff_list;
                            int nb_boxes = f_out_adj->get_position().diff(face_out_n->get_position() & f_out_adj->get_position(), diff_list);
                            IntervalVector union_of_diff(dim, Interval::EMPTY_SET);
                            for(int i=0; i<nb_boxes; i++)
                                union_of_diff |= diff_list[i];
                            own_surface &= union_of_diff;
                        }

                        /// Note : computation of the own_surface will remove the propagation of exact face_in to itself
                    }
                    if(get_nb_dim_flat(own_surface)==2)
                        own_surface.set_empty();

                    /// ************* Compute the propagation *************
                    if(!own_surface.is_empty()){
                        // OUT -> IN
                        if(!input_global_door.is_empty()){
                            /// WARNING : do no set in_tmp_IN only to private door
                            /// because only half part is taken into account => take the union with the IN of the neighbour
                            IntervalVector in_tmp_IN(input_global_door);

                            IntervalVector out_tmp_IN(own_surface);
                            if(local_pave)
                                out_tmp_IN &= d_out_adj->get_output_private();
                            else
                                out_tmp_IN &= d_out_adj->get_output(); // Do not use the private door here !

                            // Avoid degenerated case of out_tmp_IN (when own_surface reduce out_tmp_IN to dim-2 => border)
                            if(get_nb_dim_flat(out_tmp_IN)==2)
                                out_tmp_IN.set_empty();

                            if(!out_tmp_IN.is_empty()){
                                if(local_pave)
                                    contract_flow(in_tmp_IN, out_tmp_IN, vec_field_local);
                                else
                                    contract_flow(in_tmp_IN, out_tmp_IN, vec_field_neighbors);

                                in_return |= in_tmp_IN ;
                            }
                        }

                        // IN -> OUT
                        if(!output_global_door.is_empty()){
                            IntervalVector out_tmp_OUT(output_global_door);
                            IntervalVector in_tmp_OUT(own_surface);
                            if(local_pave)
                                in_tmp_OUT &= d_out_adj->get_input_private();
                            else
                                in_tmp_OUT &= d_out_adj->get_input();

                            if(get_nb_dim_flat(in_tmp_OUT)==2)
                                in_tmp_OUT.set_empty();

                            if(!in_tmp_OUT.is_empty()){
                                if(local_pave)
                                    contract_flow(in_tmp_OUT, out_tmp_OUT, vec_field_local);
                                else
                                    contract_flow(in_tmp_OUT, out_tmp_OUT, vec_field_neighbors);
                                out_return |= out_tmp_OUT;
                            }
                        }
                    }
                }
            }
        }
    }

    out_return &= in_return;
    in_return &= out_return;

    in_return &= door_in->get_input_private();
    out_return &= door_in->get_output_private();
}

void Room::set_full_possible(){
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->set_full_possible_private();
    }
}

bool Room::contract_continuity(){
    bool change = false;
    for(Face *f:m_pave->get_faces_vector()){
        if(!f->is_border()){
            Door *d = f->get_doors()[m_maze];
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

void Room::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect){
    // assert 0 not in v.
    IntervalVector c(out-in);
    IntervalVector v(vect);
    Interval alpha(Interval::POS_REALS);

    for(int i=0; i<v.size(); i++){
        if(!(c[i]==Interval::ZERO && Interval::ZERO.is_subset(v[i])))
            alpha &= ((c[i]/(v[i] & Interval::POS_REALS)) & Interval::POS_REALS) | ((c[i]/(v[i] & Interval::NEG_REALS)) & Interval::POS_REALS);
    }
    if(alpha==Interval::ZERO)
        alpha.set_empty();

    c &= alpha*v;
    out &= c+in;
    in &= out-c;
}

bool Room::contract(){
    // Do not synchronization in this function or sub-functions

    bool change = false;
    if(!is_removed()){
        DOMAIN_INITIALIZATION domain_init = m_maze->get_domain()->get_init();
        if(m_first_contract && domain_init == FULL_DOOR){
            contract_vector_field();
            change = true;
        }
        else if(m_first_contract && domain_init == FULL_WALL){
            if(m_pave->is_border()) // Case only the border is full
                change = true;
        }

        if(m_first_contract){
            eval_vector_field_possibility();
            m_first_contract = false;
        }
        //        get_private_doors_info("before");
        change |= contract_continuity();
        //        get_private_doors_info("continuity");

        if(change){
            contract_consistency();
            //            get_private_doors_info("consistency");
        }
    }
    return change;
}

bool Room::get_private_doors_info(string message, bool cout_message){
    if(m_maze->get_domain()->get_init() != FULL_DOOR)
        return false;
    IntervalVector position(2);
    //
    position[0] = Interval(-0.0625, 0.03125);
    position[1] = Interval(0.125, 0.1875);
    //    IntervalVector position2(2);
    //    position2[0] = Interval(0.75, 1.5);
    //    position2[1] = Interval(1.5750000000000002, 3.1);
    IntervalVector position2(position);

    if((m_pave->get_position() == position || m_pave->get_position() == position2)){
        if(cout_message){
            cout << message << endl;
            if(m_pave->get_position() == position)
                cout << "position 1" << endl;
            else
                cout << "position 2" << endl;

            cout << "Room = " << m_pave->get_position() << " - " << m_pave->get_faces_vector().size() << " faces" << endl;
            for(Face *f:m_pave->get_faces_vector()){
                Door *d = f->get_doors()[m_maze];
                cout << " Face : ";
                std::ostringstream input, output;
                input << d->get_input_private();
                output << d->get_output_private();
                cout << std::left << "input = " << std::setw(46) << input.str() << " output = " << std::setw(46) << output.str() << endl;
            }
            cout << "----" << endl;
        }
        return true;
    }
    return false;
}

void Room::synchronize(){
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->synchronize();
    }
}

void Room::analyze_change(std::vector<Room *>&list_rooms) const{
    bool change = false;
    for(Face* f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        change |= d->analyze_change(list_rooms);
    }
    if(m_contain_zero_coordinate && change){
        get_all_active_neighbors(list_rooms);
    }
}

void Room::get_all_active_neighbors(std::vector<Room *> &list_rooms) const{
    for(Face* f:m_pave->get_faces_vector()){
        for(Face *f_n:f->get_neighbors()){
            Room *r_n = f_n->get_pave()->get_rooms()[m_maze];
            if(!r_n->is_removed())
                list_rooms.push_back(r_n);
        }
    }
}

bool Room::is_empty(){
    if(m_empty && !m_empty_first_eval)
        return true;
    else{
        m_empty_first_eval = false;
        for(Face *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_empty()){
                return false;
            }
        }
        if(m_maze->get_domain()->get_init()!=FULL_WALL)
            m_empty = true;
        return true;
    }
}

bool Room::is_full(){
    if(!m_full && !m_full_first_eval)
        return false;
    else{
        m_full_first_eval = false;
        for(Face *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_full()){
                if(m_maze->get_domain()->get_init()!=FULL_WALL)
                    m_full = false;
                return false;
            }
        }
        return true;
    }
}

bool Room::request_bisection(){
    return !(is_empty());
}

std::ostream& operator<< (std::ostream& stream, const Room& r) {
    stream << "Room = " << r.get_pave()->get_position() << " - " << r.get_pave()->get_faces_vector().size() << " faces";
    stream << ", vector field = ";
    for(IntervalVector &v:r.get_vector_fields()){
        stream << v << " ";
    }
    stream << endl;
    stream << " nb_contractions = " << r.get_nb_contractions();
    stream << ", removed = " << (r.is_removed()?"true":"false");
    stream << endl;
    for(Face *f:r.get_pave()->get_faces_vector()){
        Door *d = f->get_doors()[r.get_maze()];
        stream << " Face : " << d->get_face()->get_orientation() << " - " << *d << endl;
    }
    stream << " contain_zero = " << r.get_contain_zero_coordinate() << endl;
    return stream;
}



//bool Room::is_degenerated(const IntervalVector& iv){
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

const bool Room::get_one_vector_fields_zero(int n_vf) const{
    return m_vector_field_zero[n_vf];
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

void Room::set_removed(){
    m_removed = true;
    // Free memory (private doors)
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->set_removed();
    }
}

const IntervalVector Room::get_hull() const{
    IntervalVector result(m_pave->get_position().size(), Interval::EMPTY_SET);
    for(Face *f:get_pave()->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        result |= d->get_hull();
    }
    return result;
}

const IntervalVector Room::get_hull_complementary(){
    if(is_empty())
        return m_pave->get_position();
    IntervalVector hull = get_hull();
    IntervalVector *result;
    int nb_box = m_pave->get_position().diff(hull,result);

    IntervalVector complementary(hull.size(), Interval::EMPTY_SET);
    for(int i=0; i<nb_box; i++){
        complementary |= result[i];
    }
    return complementary;
}

Room& operator&=(Room& r1, const Room& r2){
    for(Face *f:r1.get_pave()->get_faces_vector()){
        Door *d1 = f->get_doors()[r1.get_maze()];
        Door *d2 = f->get_doors()[r2.get_maze()];
        *d1 &= *d2;
    }
    return r1;
}

Room& operator|=(Room& r1, const Room& r2){
    for(Face *f:r1.get_pave()->get_faces_vector()){
        Door *d1 = f->get_doors()[r1.get_maze()];
        Door *d2 = f->get_doors()[r2.get_maze()];
        *d1 |= *d2;
    }
    return r1;
}

void Room::contract_box(ibex::IntervalVector& virtual_door_out, ibex::Sep* sep_output){
    IntervalVector in(m_pave->get_dim()), out(m_pave->get_dim());
    IntervalVector v_door(virtual_door_out & m_pave->get_position());
    DYNAMICS_SENS sens = m_maze->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION init = m_maze->get_domain()->get_init();

    for(Face *f:m_pave->get_faces_vector()){
        Door *d_out = f->get_doors()[m_maze];

        // IN -> OUTPUT
        if(init!=FULL_DOOR){
            d_out->set_output_private(IntervalVector::empty(m_pave->get_dim()));
            d_out->set_input_private(IntervalVector::empty(m_pave->get_dim()));
            for(IntervalVector vect:m_vector_fields){
                in = v_door;
                out = f->get_position();
                if(sens != FWD_BWD){
                    contract_flow(in, out, ((sens==FWD)?1:-1)*vect);
                    out &= f->get_position();
                    d_out->set_output_private(d_out->get_output_private() | out);
                    d_out->set_input_private(d_out->get_input_private() | out);
                }
                else{
                    IntervalVector out2(out);
                    contract_flow(in, out, vect);
                    in = v_door;
                    contract_flow(in, out2, -vect);
                    out &= f->get_position();
                    out2 &= f->get_position();
                    d_out->set_output_private(d_out->get_output_private() | out | out2);
                    d_out->set_input_private(d_out->get_input_private() | out | out2);
                }
            }
        }
        else{
            IntervalVector x_in(f->get_position()), x_out(f->get_position());
            sep_output->separate(x_in, x_out);
            d_out->set_output_private(x_out);
            d_out->set_input_private(x_out);
        }
    }
}

}
