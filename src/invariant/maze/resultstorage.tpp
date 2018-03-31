#include "resultstorage.h"
#include "room.h"

namespace invariant {

template<typename _Tp>
ResultStorage<_Tp>::ResultStorage(const size_t &dim, const size_t &nb_vf){
    m_dim = dim;
    m_nb_vf = nb_vf;
    std::vector<_Tp> tmp_vec;
    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++){
        tmp_vec.push_back(get_empty_door_container<_Tp>(m_dim));
    }

    sub_tab_type tmp_out;
    for(size_t face_out=0; face_out<dim; face_out++){
        std::array<std::vector<_Tp>, 2> tmp_array;
        tmp_array[0] = tmp_vec;
        tmp_array[1] = tmp_vec;
        tmp_out.push_back(tmp_array);
    }

    m_input_initial = tmp_out;
    m_output_initial = tmp_out;

    std::array<sub_tab_type, 2> tmp_array;
    tmp_array[0] = tmp_out;
    tmp_array[1] = tmp_out;
    for(size_t face_in=0; face_in<dim; face_in++){
        m_input2output.push_back(tmp_array);
        m_output2input.push_back(tmp_array);
    }
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_input(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out, const size_t &n_vf){
    m_output2input[face_in][sens_in][face_out][sens_out][n_vf] |= val;
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_output(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out, const size_t &n_vf){
    m_input2output[face_in][sens_in][face_out][sens_out][n_vf] |= val;
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_input_initial(const _Tp &val, const size_t &face, const size_t &sens, const size_t &n_vf){
    m_input_initial[face][sens][n_vf] |= val;
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_output_initial(const _Tp &val, const size_t &face, const size_t &sens, const size_t &n_vf){
    m_output_initial[face][sens][n_vf] |= val;
}

template<typename _Tp>
_Tp ResultStorage<_Tp>::get_output(const size_t &face, const size_t &sens){
    if(m_nb_vf>1)
        return get_output2(face, sens);
    _Tp result = get_empty_door_container<_Tp>(m_dim);
    bool first = true;

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++){
        _Tp tmp = get_empty_door_container<_Tp>(m_dim);
        for(size_t face_in = 0; face_in < m_dim; face_in++){
            for(size_t sens_in = 0; sens_in < 2; sens_in++){
                if(m_input2output[face_in][sens_in][face][sens].size()>n_vf)
                    tmp |= m_input2output[face_in][sens_in][face][sens][n_vf];
            }
        }
        if(first){
            result = tmp;
            first = false;
        }
        else
            result &= tmp;
    }

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++)
        result |= m_output_initial[face][sens][n_vf];
    return result;
}

template<typename _Tp>
_Tp ResultStorage<_Tp>::get_output2(const size_t &face, const size_t &sens){
    _Tp result = get_empty_door_container<_Tp>(m_dim);

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++){
        for(size_t face_in = 0; face_in < m_dim; face_in++){
            for(size_t sens_in = 0; sens_in < 2; sens_in++){

                for(size_t n_vf2=0; n_vf2<m_nb_vf; n_vf2++){
                    if(n_vf2 != n_vf){
                        for(size_t face_in2 = 0; face_in2 < m_dim; face_in2++){
                            for(size_t sens_in2 = 0; sens_in2 < 2; sens_in2++){

                                if(!(n_vf == n_vf2 && face_in == face_in2 && sens_in == sens_in2))
                                    result |= m_input2output[face_in][sens_in][face][sens][n_vf] & m_input2output[face_in2][sens_in2][face][sens][n_vf2];

                            }
                        }
                    }
                }

            }
        }
    }

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++)
        result |= m_output_initial[face][sens][n_vf];
    return result;
}

template<typename _Tp>
_Tp ResultStorage<_Tp>::get_input(const size_t &face, const size_t &sens){
    if(m_nb_vf>1)
        return get_input2(face, sens);
    _Tp result = get_empty_door_container<_Tp>(m_dim);
    bool first = true;

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++){
        _Tp tmp = get_empty_door_container<_Tp>(m_dim);
        for(size_t face_out = 0; face_out < m_dim; face_out++){
            for(size_t sens_out = 0; sens_out < 2; sens_out++){
                if(m_output2input[face][sens][face_out][sens_out].size()>n_vf)
                    tmp |= m_output2input[face][sens][face_out][sens_out][n_vf];
            }
        }
        if(first){
            result = tmp;
            first = false;
        }
        else
            result &= tmp;
    }

    size_t nb_vf = m_input_initial[face][sens].size();
    if(nb_vf>0){
        for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
            result |= m_input_initial[face][sens][n_vf];
    }
    return result;
}

template<typename _Tp>
_Tp ResultStorage<_Tp>::get_input2(const size_t &face, const size_t &sens){
    _Tp result = get_empty_door_container<_Tp>(m_dim);

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++){
        for(size_t face_out = 0; face_out < m_dim; face_out++){
            for(size_t sens_out = 0; sens_out < 2; sens_out++){

                for(size_t n_vf2=0; n_vf2<m_nb_vf; n_vf2++){
                    if(n_vf2 != n_vf){
                        for(size_t face_out2 = 0; face_out2 < m_dim; face_out2++){
                            for(size_t sens_out2 = 0; sens_out2 < 2; sens_out2++){

                                if(!(n_vf == n_vf2 && face_out == face_out2 && sens_out == sens_out2))
                                    result |= m_output2input[face][sens][face_out][sens_out][n_vf] & m_output2input[face][sens][face_out2][sens_out2][n_vf2];
                            }
                        }
                    }
                }

            }
        }
    }

    for(size_t n_vf=0; n_vf<m_nb_vf; n_vf++)
        result |= m_input_initial[face][sens][n_vf];

    return result;
}


}
