#include "taylor.h"

using namespace ibex;

namespace invariant {

ibex::Interval taylor(const ibex::Interval &t, const ibex::Interval &x_dot2, const ibex::Interval &x0_dot, const ibex::Interval &x0){
    return 0.5*x_dot2*t*t+x0_dot*t+x0;
}

ibex::IntervalVector taylor_compute_point(const ibex::Interval &t, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c){
    ibex::IntervalVector result(2, ibex::Interval::EMPTY_SET);
    result[0] = a[0]*pow(t,2)+b[0]*t+c[0];
    result[1] = a[1]*pow(t,2)+b[1]*t+c[1];
    return result;
}

ibex::Interval taylor_contract_t(const ibex::Interval &t, const ibex::Interval &a, const ibex::Interval &b, const ibex::Interval &c){
    if(a != ibex::Interval::ZERO){
        ibex::Interval t1 = (-b+sqrt(pow(b, 2)-4*a*c))/(2*a) & Interval::POS_REALS & t;
        ibex::Interval t2 = (-b-sqrt(pow(b,2)-4*a*c))/(2*a) & Interval::POS_REALS & t;

        if(!(t2 & ibex::Interval::POS_REALS).is_empty()){
            if(t1.lb() <= t2.ub())
                return (t2 | t1);
            else
                return t2;
        }
        else
            return t1;
    }
    else{
        if(!(b == ibex::Interval::ZERO))
            return -c/b & ibex::Interval::POS_REALS & t;
        else
            return ibex::Interval::POS_REALS & t;
    }
}

ibex::IntervalVector taylor_contract_trajectory(const ibex::IntervalVector &box, const ibex::Interval &t, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c, const size_t &sens){
    ibex::IntervalVector box_tmp(2, ibex::Interval::EMPTY_SET);
    ibex::Interval c1, t_y, t_x;
    if(sens == 1){ // Contract y
        c1 = c[1].lb()-box[1];
        t_y = taylor_contract_t(t, a[1], b[1], c1.lb());
        box_tmp |= taylor_compute_point(t_y, a, b, c);
        t_y = taylor_contract_t(t, a[1], b[1], c1.ub());
        box_tmp |= taylor_compute_point(t_y, a, b, c);

        c1 = c[1].ub()-box[1];
        t_y = taylor_contract_t(t, a[1], b[1], c1.lb());
        box_tmp |= taylor_compute_point(t_y, a, b, c);
        t_y = taylor_contract_t(t, a[1], b[1], c1.ub());
        box_tmp |= taylor_compute_point(t_y, a, b, c);
    }
    else{
        c1 = c[0].lb()-box[0];
        t_x = taylor_contract_t(t, a[0], b[0], c1.lb());
        box_tmp |= taylor_compute_point(t_x, a, b, c);
        t_x = taylor_contract_t(t, a[0], b[0], c1.ub());
        box_tmp |= taylor_compute_point(t_x, a, b, c);

        c1 = c[0].ub()-box[0];
        t_x = taylor_contract_t(t, a[0], b[0], c1.lb());
        box_tmp |= taylor_compute_point(t_x, a, b, c);
        t_x = taylor_contract_t(t, a[0], b[0], c1.ub());
        box_tmp |= taylor_compute_point(t_x, a, b, c);
    }
    return box_tmp;
}

ibex::IntervalVector taylor_contrat_box(const ibex::IntervalVector &box, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c){
    size_t sens = 0;
    if(c[0].is_degenerated())
        sens = 1;

    ibex::IntervalVector box_contract(2, ibex::Interval::EMPTY_SET);
    ibex::IntervalVector a_tmp(2, ibex::Interval::EMPTY_SET);
    ibex::IntervalVector b_tmp(2, ibex::Interval::EMPTY_SET);

    for(size_t i1=0; i1<2; i1++){
        for(size_t i2=0; i2<2; i2++){
            for(size_t i3=0; i3<2; i3++){
                for(size_t i4=0; i4<2; i4++){
                    a_tmp[0] = (i1==0)?a[0].lb():a[0].ub();
                    a_tmp[1] = (i2==0)?a[1].lb():a[1].ub();
                    b_tmp[0] = (i3==0)?b[0].lb():b[0].ub();
                    b_tmp[1] = (i4==0)?b[1].lb():b[1].ub();
                    box_contract |= taylor_contract_trajectory(box, ibex::Interval::POS_REALS, a_tmp, b_tmp, c, sens);
                }
            }
        }
    }
    return box_contract & box;
}
}


