#include "resultstorage.h"
#include "room.h"

namespace invariant {

template<typename _Tp>
ResultStorage<_Tp>::ResultStorage(size_t dim, size_t nb_vf){
    m_dim = dim;
    m_nb_vf = nb_vf;
    sub_tab_type tmp_out;
    for(size_t face_out=0; face_out<dim; face_out++){
        std::array<std::vector<_Tp>, 2> tmp_array;
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
void ResultStorage<_Tp>::push_back_input(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out){
    m_output2input[face_in][sens_in][face_out][sens_out].push_back(val);
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_output(const _Tp &val, const size_t &face_in, const size_t &sens_in, const size_t &face_out, const size_t &sens_out){
    m_input2output[face_in][sens_in][face_out][sens_out].push_back(val);
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_input_initial(const _Tp &val, const size_t &face, const size_t &sens){
    m_input_initial[face][sens].push_back(val);
}

template<typename _Tp>
void ResultStorage<_Tp>::push_back_output_initial(const _Tp &val, const size_t &face, const size_t &sens){
    m_output_initial[face][sens].push_back(val);
}

#if 0
template<typename _Tp>
_Tp ResultStorage<_Tp>::get_output(const size_t &face, const size_t &sens){
    _Tp result = get_empty_door_container<_Tp>(m_dim);
    for(size_t face_in = 0; face_in < m_dim; face_in++){
        for(size_t sens_in = 0; sens_in < 2; sens_in++){
            _Tp tmp(m_dim);
            size_t nb_vf = m_input2output[face_in][sens_in][face][sens].size();
//            if(nb_vf !=2)
//                std::cout << "ERROR (output) nb_vf = " << nb_vf << std::endl;
            if(nb_vf >0){
                for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
                    tmp &= m_input2output[face_in][sens_in][face][sens][n_vf];
                result |= tmp;
            }
        }
    }
    size_t nb_vf = m_output_initial[face][sens].size();
    if(nb_vf>0){
        for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
            result |= m_output_initial[face][sens][n_vf];
    }
    return result;
}
#else
template<typename _Tp>
_Tp ResultStorage<_Tp>::get_output(const size_t &face, const size_t &sens){
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

    size_t nb_vf = m_output_initial[face][sens].size();
    if(nb_vf>0){
        for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
            result |= m_output_initial[face][sens][n_vf];
    }
    return result;
}
#endif

#if 0
template<typename _Tp>
_Tp ResultStorage<_Tp>::get_input(const size_t &face, const size_t &sens){
    _Tp result = get_empty_door_container<_Tp>(m_dim);
    for(size_t face_out = 0; face_out < m_dim; face_out++){
        for(size_t sens_out = 0; sens_out < 2; sens_out++){
            _Tp tmp(m_dim);
            size_t nb_vf = m_output2input[face][sens][face_out][sens_out].size();
            if(nb_vf >0){
                for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
                    tmp &= m_output2input[face][sens][face_out][sens_out][n_vf];
                result |= tmp;
            }
        }
    }
    size_t nb_vf = m_input_initial[face][sens].size();
    if(nb_vf>0){
        for(size_t n_vf=0; n_vf<nb_vf; n_vf++)
            result |= m_input_initial[face][sens][n_vf];
    }
    return result;
}
#else
template<typename _Tp>
_Tp ResultStorage<_Tp>::get_input(const size_t &face, const size_t &sens){
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
#endif


}
