#include "room.h"
#include "face.h"

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

void Room::set_empty(){
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
    IntervalVector test(2);
    test[0] = Interval(-2.25, -1.5);
    test[1] = Interval(-3, -1.5);
    if(m_pave->get_position() == test)
        cout << "TEST" << endl;

    int dim = m_pave->get_dim();
    MazeSens sens = m_maze->get_sens();
    IntervalVector zero(dim, Interval::ZERO);

    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        vector<IntervalVector> vector_fields_face = m_maze->get_dynamics()->eval(f->get_position());

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

        for(const IntervalVector&v:m_vector_fields){
            IntervalVector product = hadamard_product(v, f->get_normal());

            if(zero.is_subset(product))
                d->push_back_collinear_vector_field(true);
            else
                d->push_back_collinear_vector_field(false);
        }
    }
}

void Room::eval_vector_field_possibility(){
    int dim = m_pave->get_dim();

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
        }
    }
}

void Room::contract_consistency(){
    const int dim = m_pave->get_dim();

    IntervalVector empty = IntervalVector(dim, Interval::EMPTY_SET);
    for(IntervalVector &vec_field:m_vector_fields){
        if(vec_field == empty)
            return;
    }

    m_nb_contract++;
    //    IntervalVector test(2);
    //    test[0] = Interval(-1.171875, -1.125);
    //    test[1] = Interval(3.5625, 3.65625);
    //    if(m_pave->get_position().is_subset(test) && get_maze()->get_type() == MAZE_CONTRACTOR)
    //        cout << "DEBUG" << endl;

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
            for(int face_in=0; face_in<dim; face_in++){
                for(int sens_in = 0; sens_in < 2; sens_in++){

                    IntervalVector in_result(dim, Interval::EMPTY_SET);
                    Face* f_in = m_pave->get_faces()[face_in][sens_in];
                    Door* door_in = f_in->get_doors()[m_maze];
                    const IntervalVector in(door_in->get_input_private());

                    if(!in.is_empty()){
                        for(int face_out=0; face_out<dim; face_out++){
                            for(int sens_out = 0; sens_out < 2; sens_out++){

                                IntervalVector in_tmp(in);
                                Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                Door* door_out = f_out->get_doors()[m_maze];

                                /// CASE SLIDING MODE
                                if(type == MAZE_CONTRACTOR
                                        && door_in->is_collinear()[n_vf]
                                        && door_out->is_possible_out()[n_vf]
                                        && !(f_out->get_position() & f_in->get_position()).is_empty()
                                        ){

                                    if(!(face_out == face_in && sens_in == sens_out)){
                                        /// INPUT
                                        IntervalVector in_return(dim), out_return(dim);
                                        contract_sliding_mode_in(n_vf, face_in, sens_in, face_out, sens_out, out_return, in_return);
                                        in_result |= in_return;
                                        out_results[n_vf][face_out][sens_out] |= out_return & door_out->get_output_private();
                                    }
                                    else{
                                        /// OUTPUT
                                        IntervalVector out_tmp(dim);
                                        contract_sliding_mode_out(n_vf, face_out, sens_out, out_tmp);
                                        out_results[n_vf][face_out][sens_out] |= out_tmp & door_out->get_output_private() & door_out->get_input_private();
                                    }
                                }
                                /// CASE NO SLIDING MODE
                                else{
                                    // ToDo : improve propagation (avoid sliding same as contractor)
                                    IntervalVector out_tmp(dim);
                                    if(type == MAZE_CONTRACTOR || !(face_out == face_in && sens_out == sens_in)){
                                        if(type == MAZE_CONTRACTOR)
                                            out_tmp = door_out->get_output_private();
                                        else
                                            out_tmp = f_out->get_position();

                                        if(!out_tmp.is_empty())
                                            this->contract_flow(in_tmp, out_tmp, vec_field);
                                        else
                                            in_tmp.set_empty();

                                        in_result |= in_tmp;
                                        out_results[n_vf][face_out][sens_out] |= out_tmp;
                                    }
                                }
                            }
                        }
                        if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
                            /// TEST ?
                            if(is_degenerated(in_result))
                                in_result.set_empty();
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
                if(!one_possible){
                    door_out_iv.set_empty();
                }

                /// TEST ?
                if(is_degenerated(door_out_iv))
                    door_out_iv.set_empty();

                if(type == MAZE_CONTRACTOR)
                    door_out->set_output_private(door_out_iv & door_out->get_output_private());
                else
                    door_out->set_output_private(door_out->get_output_private() | door_out_iv);
            }
        }
    }

}

inline void Room::contract_sliding_mode_out(int n_vf, int face, int sens, IntervalVector &out_return){
    Face* f_out = m_pave->get_faces()[face][sens];
    Door* door_out = f_out->get_doors()[m_maze];
    IntervalVector out(door_out->get_output_private());
    int dim = m_pave->get_dim();

    out_return = IntervalVector(m_pave->get_dim(), Interval::EMPTY_SET);
    for(Face * f_neighbour:f_out->get_neighbors()){
        Door *d_neighbour = f_neighbour->get_doors()[m_maze];
        Pave *p_neighbour = f_neighbour->get_pave();
        Room *r_neighbour = p_neighbour->get_rooms()[m_maze];

        if(r_neighbour->get_one_vector_fields_zero(n_vf)){
            // Case Zero
            out_return |= d_neighbour->get_input() & out;
        }
        else{
            // Compute contribution of each neighbours faces
            for(int face_n=0; face_n<dim; face_n++){
                for(int sens_n=0; sens<2; sens++){
                    Face *f_in = p_neighbour->get_faces()[face_n][sens_n];
                    Door *d_in = f_in->get_doors()[m_maze];
                    IntervalVector in_tmp(d_in->get_input());

                    if(!(face_n == face && sens_n == sens) && !in_tmp.is_empty()){
                        IntervalVector vect_field_n = r_neighbour->get_one_vector_fields(n_vf);
                        IntervalVector out_tmp(out);
                        contract_flow(in_tmp, out_tmp, vect_field_n);
                        out_return |= out_tmp;
                    }
                }
            }
        }

    }
}

inline void Room::contract_sliding_mode_in(int n_vf, int face_in, int sens_in, int face_out, int sens_out, IntervalVector &out_return, IntervalVector &in_return){
    Face* f_in = m_pave->get_faces()[face_in][sens_in];
    Door* door_in = f_in->get_doors()[m_maze];
    Face* f_out = m_pave->get_faces()[face_out][sens_out];
    Door* door_out = f_out->get_doors()[m_maze];
    int dim = m_pave->get_dim();
    IntervalVector in(door_in->get_input_private());
    in_return = IntervalVector(dim, Interval::EMPTY_SET);

    // Own contribution
    out_return = door_out->get_output_private();
    IntervalVector in_tmp(in);
    contract_flow(in_tmp, out_return, m_vector_fields[n_vf]);
    in_return |= in_tmp;

    // Neighbours contribution
    Interval max_impact_seg;
    for(Face * f_neighbour:f_in->get_neighbors()){
        Door *d_neighbour = f_neighbour->get_doors()[m_maze];
        Pave *p_neighbour = f_neighbour->get_pave();
        Room *r_neighbour = p_neighbour->get_rooms()[m_maze];

        max_impact_seg &= f_neighbour->get_position()[face_out];

        if(r_neighbour->get_one_vector_fields_zero(n_vf)){
            // Case Zero
            in_return |= d_neighbour->get_output() & in;
        }
        else{
            Face *f_out_neighbour = p_neighbour->get_faces()[face_out][sens_out];
            Door *d_out_neighbour = f_out_neighbour->get_doors()[m_maze];
            IntervalVector out_neighbour(d_out_neighbour->get_output());

            IntervalVector in_tmp(in & d_neighbour->get_input());
            IntervalVector vect_field_n = r_neighbour->get_one_vector_fields(n_vf);

            contract_flow(in_tmp, out_neighbour, vect_field_n);
            in_return |= in_tmp;
        }
    }

    // Compute No impact zone
    Interval no_impact_seg, ni1, ni2;
    in[face_out].diff(max_impact_seg, ni1, ni2);
    no_impact_seg = ni1 | ni2;
    IntervalVector no_impact(in);
    no_impact[face_out] = no_impact_seg;

    in_return |= no_impact & in;
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
        alpha &= ((c[i]/(v[i] & Interval::POS_REALS)) & Interval::POS_REALS) | ((c[i]/(v[i] & Interval::NEG_REALS)) & Interval::POS_REALS);
    }
    c &= alpha*v;

    //    const MazeSens sens = m_maze->get_sens();
    //    if(sens == MAZE_FWD || sens == MAZE_FWD_BWD)
    out &= c+in;
    //    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD)
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

        change |= contract_continuity();

        if(change){
            contract_consistency();
        }
    }
    return change;
}

void Room::get_private_doors_info(){
    if(m_maze->get_type() != MAZE_PROPAGATOR)
        return;
    IntervalVector position(2);
    position[0] = Interval(0, 3);
    position[1] = Interval(-6, -3);
    IntervalVector position2(2);
    position2[0] = Interval(0, 3);
    position2[1] = Interval(-3, 0);

    if(m_pave->get_position() == position || m_pave->get_position() == position2){
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

bool Room::is_degenerated(const IntervalVector& iv){
    int compt = 0;
    int dim = m_maze->get_graph()->dim();
    for(int i=0; i<dim; i++){
        if(iv[i].is_degenerated()){
            compt++;
        }
        if(compt == 2){
            return true;
        }
    }
    return false;
}

const ibex::IntervalVector Room::get_one_vector_fields(int n_vf) const{
    return m_vector_fields[n_vf];
}

const bool Room::get_one_vector_fields_zero(int n_vf) const{
    return m_vector_field_zero[n_vf];
}

}
