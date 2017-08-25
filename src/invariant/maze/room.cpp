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
        IntervalVector zero(m_maze->get_graph()->dim(), Interval::ZERO);
        if((zero.is_subset(vector_field)))
            m_vector_field_zero.push_back(true);
        else
            m_vector_field_zero.push_back(false);
        m_vector_fields.push_back(vector_field);
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
}

Room::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
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

void Room::set_full(){
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
    MazeSens sens = m_maze->get_sens();
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
                    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD)
                        d->set_empty_private_input();
                }
                if((f->get_orientation() & v_bool_out).is_empty()){
                    if(sens == MAZE_FWD || sens == MAZE_FWD_BWD)
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

            for(const IntervalVector&v:m_vector_fields){
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

void Room:: contract_consistency(){
    const int dim = m_pave->get_dim();

    IntervalVector empty = IntervalVector(dim, Interval::EMPTY_SET);
    for(IntervalVector &vec_field:m_vector_fields){
        if(vec_field == empty)
            return;
    }

    m_nb_contract++;

//    IntervalVector position(2);
//    position[0] = Interval(-1.5, 0);
//    position[1] = Interval(1.5, 3);
//    if(m_pave->get_position().is_subset(position) && get_maze()->get_type() == MAZE_CONTRACTOR){
//        cout << endl;
//        cout << "DEBUG ROOM : " << m_pave->get_position() << " Debug ID = " << m_time_debug << endl;
//        m_debug_room = true;
//        m_time_debug++;
//    }

    MazeSens sens = m_maze->get_sens();
    MazeType type = m_maze->get_type();

    vector<vector< array<IntervalVector, 2>>> out_results; // One per vec_field, dim, sens
    const int nb_vec = m_vector_fields.size();

    for(int vf=0; vf<nb_vec; vf++){
        std::vector< std::array<IntervalVector, 2>> out_result;
        for(int i=0; i<dim; i++){
            std::array<IntervalVector, 2> output = {IntervalVector::empty(dim), IntervalVector::empty(dim)};
            out_result.push_back(output);
        }
        out_results.push_back(out_result);
    }

    bool global_compute = false;
    int n_vf = 0;

    for(IntervalVector &vec_field:m_vector_fields){
        bool compute = true;
        if(m_vector_field_zero[n_vf]){
            if(type == MAZE_PROPAGATOR && m_vector_fields.size()==1){
                this->set_full_possible();
                compute = false;
            }
            if(type == MAZE_CONTRACTOR)
                compute = false;

        }
        // Create tmp output doors
        if(compute){
            global_compute |= true;

            vector<vector<bool>> collinear;
            for(int face_in=0; face_in<dim; face_in++){
                vector<bool> collinear_tmp;
                for(int sens_in = 0; sens_in < 2; sens_in++){
                    Face* f_in = m_pave->get_faces()[face_in][sens_in];
                    Door* door_in = f_in->get_doors()[m_maze];
                    if(door_in->is_collinear()[n_vf])
                        collinear_tmp.push_back(true);
                    else
                        collinear_tmp.push_back(false);
                }
                collinear.push_back(collinear_tmp);
            }

            // ************* SLIDING MODE *************
            for(int face_in=0; face_in<dim; face_in++){
                for(int sens_in = 0; sens_in < 2; sens_in++){
                    if(type == MAZE_CONTRACTOR && collinear[face_in][sens_in]){
                        Face* f_in = m_pave->get_faces()[face_in][sens_in];
                        Door* door_in = f_in->get_doors()[m_maze];

                        if(m_debug_room)
                            cout << "before " << door_in->get_output_private() << endl;

                        /// INPUT
                        IntervalVector out_return(dim, Interval::EMPTY_SET);
                        IntervalVector in_return(dim, Interval::EMPTY_SET);
                        contract_sliding_mode(n_vf, face_in, sens_in, out_return, in_return);
                        out_results[n_vf][face_in][sens_in] = out_return;

                        if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
                            door_in->set_input_private(in_return); // Write input
                        }
                        if(sens == MAZE_FWD || sens == MAZE_FWD_BWD){
                            door_in->set_output_private(out_return); // Write output
                        }

                        if(m_debug_room)
                            cout << "after " << out_return << endl;

                        /// Impact on other faces (out results)
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){
                                if(!(face_out == face_in && sens_in == sens_out)){
                                    Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door* door_out = f_out->get_doors()[m_maze];
                                    IntervalVector out_tmp(door_out->get_output_private());
                                    IntervalVector in_tmp(door_in->get_input_private());
                                    if(!out_tmp.is_empty() && !in_tmp.is_empty()){
                                        this->contract_flow(in_tmp, out_tmp, vec_field);
                                        out_results[n_vf][face_out][sens_out] |= out_tmp;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ************* STANDARD MODE *************
            for(int face_in=0; face_in<dim; face_in++){
                for(int sens_in = 0; sens_in < 2; sens_in++){
                    if(!(type == MAZE_CONTRACTOR && collinear[face_in][sens_in])){

                        Face* f_in = m_pave->get_faces()[face_in][sens_in];
                        Door* door_in = f_in->get_doors()[m_maze];
                        const IntervalVector in(door_in->get_input_private());

                        if(!in.is_empty()){
                            IntervalVector in_result(dim, Interval::EMPTY_SET);
                            for(int face_out=0; face_out<dim; face_out++){
                                for(int sens_out = 0; sens_out < 2; sens_out++){
                                    IntervalVector in_tmp(in);
                                    Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                    Door* door_out = f_out->get_doors()[m_maze];

                                    IntervalVector out_return(dim);
                                    if(/*type == MAZE_PROPAGATOR ||*/ !(face_out == face_in && sens_out == sens_in)){
                                        if(type == MAZE_CONTRACTOR)
                                            out_return = door_out->get_output_private();
                                        else
                                            out_return = f_out->get_position();

                                        if(!out_return.is_empty() && !in_tmp.is_empty()){
                                            this->contract_flow(in_tmp, out_return, vec_field);

                                            in_result |= in_tmp;
                                            if(type == MAZE_PROPAGATOR || !collinear[face_out][sens_out])
                                                out_results[n_vf][face_out][sens_out] |= out_return;
                                        }
                                    }
                                }
                            }
                            if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
                                if(type == MAZE_CONTRACTOR)
                                    door_in->set_input_private(in & in_result);
                                else{
                                    if(door_in->is_possible_in()[n_vf])
                                        door_in->set_input_private(in | in_result);
                                }
                            }
                        }
                    }
                }

            }
            n_vf++;
        }
    }

    if(global_compute && (sens == MAZE_FWD || sens == MAZE_FWD_BWD)){
        for(int face_out = 0; face_out<dim; face_out++){
            for(int sens_out = 0; sens_out < 2; sens_out++){
                Face* f_out = m_pave->get_faces()[face_out][sens_out];
                Door* door_out = f_out->get_doors()[m_maze];

                IntervalVector door_out_iv(door_out->get_face()->get_position());
                bool one_possible = false;
                for(int n_vf=0; n_vf<nb_vec; n_vf++){
                    if((type == MAZE_PROPAGATOR && door_out->is_possible_out()[n_vf]) || type == MAZE_CONTRACTOR){
                        one_possible = true;
                        door_out_iv &= out_results[n_vf][face_out][sens_out];
                    }
                }
                if(!one_possible){ // For Kernel
                    door_out_iv.set_empty();
                }

                if(type == MAZE_CONTRACTOR)
                    door_out->set_output_private(door_out_iv & door_out->get_output_private());
                else
                    door_out->set_output_private(door_out->get_output_private() | door_out_iv);
            }
        }
    }
}

void Room::contract_sliding_mode(int n_vf, int face_in, int sens_in, IntervalVector &out_return, IntervalVector &in_return){
    if(m_debug_room)
        cout << "debug" << endl;

    Face* f_in = m_pave->get_faces()[face_in][sens_in];
    Door* door_in = f_in->get_doors()[m_maze];
    int dim = get_pave()->get_dim();
    in_return = IntervalVector(dim, Interval::EMPTY_SET);
    out_return = IntervalVector(dim, Interval::EMPTY_SET);

    vector<bool> where_zeros = door_in->get_where_zeros(n_vf);

    // Find adjacent paves
    vector<Pave *> adjacent_paves;
    m_maze->get_graph()->get_tree()->get_intersection_pave_outer(adjacent_paves, f_in->get_position());
    //    m_maze->get_graph()->get_tree()->get_intersection_pave_outer(adjacent_paves, m_pave->get_position());

    // Remove pave not in the zero(s) direction
    IntervalVector vec_field(dim, Interval::EMPTY_SET);
    vector<Pave *> adjacent_paves_zeros;
    IntervalVector pave_extrude(f_in->get_position());
    //    IntervalVector pave_extrude(m_pave->get_position());
    for(int id_zero=0; id_zero<dim; id_zero++){
        if(where_zeros[id_zero])
            pave_extrude[id_zero] = Interval::ALL_REALS;
    }

    for(Pave *pave_n:adjacent_paves){
        // Find adjacent paves that extrude this pave in the directions of zeros
        IntervalVector inter_extrude = pave_n->get_position() & pave_extrude;

        if(get_nb_dim_flat(inter_extrude)==get_nb_dim_flat(pave_extrude)){
            adjacent_paves_zeros.push_back(pave_n);
            Room *room_n= pave_n->get_rooms()[m_maze];
            vec_field |= room_n->get_one_vector_fields(n_vf);
        }
    }

    IntervalVector zero(dim, Interval::ZERO);
    if(zero.is_subset(vec_field)){ // Case no contraction (might be a cycle)
        in_return = door_in->get_input_private();
        out_return = door_in->get_output_private();
        return;
    }

    // For each Pave, propagate OUT -> IN
    for(Pave *pave_n:adjacent_paves_zeros){
        bool local_pave = false;
        if(pave_n->get_position() == m_pave->get_position())
            local_pave = true;
        Room *r_n = pave_n->get_rooms()[m_maze];
        IntervalVector vec_field_local(r_n->get_one_vector_fields(n_vf));
//        IntervalVector vec_field_local(vec_field);

        for(int face_out=0; face_out<dim; face_out++){
            for(int sens_out = 0; sens_out < 2; sens_out ++){
                Face *f_out = pave_n->get_faces()[face_out][sens_out];
                Door *d_out = f_out->get_doors()[m_maze];

                if(!(d_out->get_output().is_empty() && d_out->get_input().is_empty())){

                    // Test if the Face intersect another face of the neighbours ?
                    // ==> Find if this face is part of the hull around the door to contract
                    IntervalVector own_surface(f_out->get_position());
                    for(Face *face_out_n:f_out->get_neighbors()){
                        Pave *pave_out_n = face_out_n->get_pave();

                        // Test if the pave is a neighbour pave of the face_in
                        bool is_neighbour_pave = false;
                        for(Pave *pave_test:adjacent_paves_zeros){
                            if(pave_test->get_position() == pave_out_n->get_position()){ // Pointer comparison (?working?)
                                is_neighbour_pave = true;
                                break;
                            }
                        }
                        // Compute the shared surface with neighbours (outer approximation => union)
                        if(is_neighbour_pave){
                            IntervalVector *diff_list;
                            int nb_boxes = f_out->get_position().diff(face_out_n->get_position() & f_out->get_position(), diff_list);
                            IntervalVector union_of_diff(dim, Interval::EMPTY_SET);
                            for(int i=0; i<nb_boxes; i++)
                                union_of_diff |= diff_list[i];
                            own_surface &= union_of_diff;
                        }
                    }

                    // Compute the possible IN that can propagate to the OUT own_surface
                    // Bug here !!!
                    if(!own_surface.is_empty()){
                        // OUT -> IN
                        IntervalVector in_tmp_IN(door_in->get_face()->get_position()); // bug if set to private door
                                                                                       // (because only half part is taken into account)
                        IntervalVector out_tmp_IN(own_surface);
                        if(local_pave)
                            out_tmp_IN &= d_out->get_output_private();
                        else
                            out_tmp_IN &= d_out->get_output();

                        if(!in_tmp_IN.is_empty() && !out_tmp_IN.is_empty()){
                            contract_flow(in_tmp_IN, out_tmp_IN, vec_field_local); // vec_field_local (NEW) to verify debug
                            in_return |= in_tmp_IN ;
                        }

                        // IN -> OUT
                        IntervalVector in_tmp_OUT(own_surface);
                        if(local_pave)
                            in_tmp_OUT &= d_out->get_input_private();
                        else
                            in_tmp_OUT &= d_out->get_input();
                        IntervalVector out_tmp_OUT(door_in->get_face()->get_position());

                        if(!in_tmp_OUT.is_empty() && !out_tmp_OUT.is_empty()){
                            contract_flow(in_tmp_OUT, out_tmp_OUT, vec_field_local); // vec_field_local (NEW) to verify debug
                            out_return |= out_tmp_OUT;
                        }
                    }
                }
            }
        }
    }

    out_return &= in_return;
    in_return &= out_return;
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
        MazeType type = m_maze->get_type();
        if(m_first_contract && type == MAZE_CONTRACTOR){
            contract_vector_field();
            change = true;
        }
        else if(m_first_contract && type == MAZE_PROPAGATOR){
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

bool Room::get_private_doors_info(string message){
    if(m_maze->get_type() != MAZE_CONTRACTOR)
        return false;
    IntervalVector position(2);
//    [-1.5, 0] ; [1.5, 3]
    position[0] = Interval(-1.5, 0);
    position[1] = Interval(1.5, 3);
//    IntervalVector position2(2);
//    position2[0] = Interval(0.75, 1.5);
//    position2[1] = Interval(1.5750000000000002, 3.1);
    IntervalVector position2(position);

    if(m_pave->get_position() == position || m_pave->get_position() == position2){
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
        return true;
    }
    return false;
}

void Room::synchronize_doors(){
    for(Face* f:m_pave->get_faces_vector()){
        Door *r = f->get_doors()[m_maze];
        r->synchronize();
    }
}

void Room::analyze_change(std::vector<Room *>&list_rooms){
    for(Face* f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->analyze_change(list_rooms);
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
    for(const IntervalVector &v:r.get_vector_fields()){
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
    return stream;
}

void Room::synchronize(){
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->synchronize();
    }
}

//bool Room::is_degenerated(const IntervalVector& iv){
//    int compt = 0;
//    int dim = m_maze->get_graph()->dim();
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

const ibex::IntervalVector Room::get_one_vector_fields(int n_vf) const{
    return m_vector_fields[n_vf];
}

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

}
