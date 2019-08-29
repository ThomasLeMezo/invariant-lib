#include "ibex_SepFwdBwd.h"
#include "ibex_SepInter.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

#include "language.h"

using namespace std;
using namespace ibex;
using namespace invariant;

void bathy1(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-6, 6);
    space[1] = ibex::Interval(-6, 6);

    Function f_sep(x1, x2, sqr(x1-1.78129)+sqr(x2-1.78129)-0.5);
    Function f_sep2(x1, x2, sqr(x1+1.78129)+sqr(x2+1.78129)-0.5);
    Function f_sep3(x1, x2, sqr(x1)+sqr(x2)-0.5);
    SepFwdBwd sep(f_sep, GEQ);
    SepFwdBwd sep2(f_sep2, GEQ);
    SepFwdBwd sep3(f_sep3, GEQ);
    SepInter sep_i(sep, sep2, sep3);

    // ****** Dynamics ******* //
    double h0 = -9;
    ibex::Function h(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff(h, Function::DIFF);
    ibex::Function psi(x1, x2, tanh(h(x1, x2)-h0)+ibex::Interval::PI/2.0);
    ibex::Function f(x1, x2, -Return((hdiff(x1, x2)[0]*cos(psi(x1, x2))-hdiff(x1, x2)[1]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0])),
                                    (hdiff(x1, x2)[1]*cos(psi(x1, x2))+hdiff(x1, x2)[0]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0]))));


    largest_positive_invariant(space, &f, 16, "bathymetry_positive_invariant", &sep_i);
}

void bathy2(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-6, 6);
    space[1] = ibex::Interval(-6, 6);

    Function f_sep(x1, x2, sqr(x1-1.78129)+sqr(x2-1.78129)-0.5);
    Function f_sep2(x1, x2, sqr(x1+1.78129)+sqr(x2+1.78129)-0.5);
    Function f_sep3(x1, x2, sqr(x1)+sqr(x2)-0.5);
    SepFwdBwd sep(f_sep, GEQ);
    SepFwdBwd sep2(f_sep2, GEQ);
    SepFwdBwd sep3(f_sep3, GEQ);
    SepInter sep_i(sep, sep2, sep3);

    // ****** Dynamics ******* //
    double h0 = -9.0;
    ibex::Interval error(-0.2, 0.2);
    ibex::Function h_1(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff_1(h_1, Function::DIFF);
    ibex::Function psi_1(x1, x2, tanh(h_1(x1, x2)-h0+error.lb())+ibex::Interval::PI/2.0);
    ibex::Function f_1(x1, x2, Return((hdiff_1(x1, x2)[0]*cos(psi_1(x1, x2))-hdiff_1(x1, x2)[1]*sin(psi_1(x1, x2)))/sqrt(sqr(hdiff_1(x1, x2)[1])+sqr(hdiff_1(x1, x2)[0])),
                                    (hdiff_1(x1, x2)[1]*cos(psi_1(x1, x2))+hdiff_1(x1, x2)[0]*sin(psi_1(x1, x2)))/sqrt(sqr(hdiff_1(x1, x2)[1])+sqr(hdiff_1(x1, x2)[0]))));

    ibex::Function h_2(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff_2(h_2, Function::DIFF);
    ibex::Function psi_2(x1, x2, tanh(h_2(x1, x2)-h0+error.ub())+ibex::Interval::PI/2.0);
    ibex::Function f_2(x1, x2, Return((hdiff_2(x1, x2)[0]*cos(psi_2(x1, x2))-hdiff_2(x1, x2)[1]*sin(psi_2(x1, x2)))/sqrt(sqr(hdiff_2(x1, x2)[1])+sqr(hdiff_2(x1, x2)[0])),
                                    (hdiff_2(x1, x2)[1]*cos(psi_2(x1, x2))+hdiff_2(x1, x2)[0]*sin(psi_2(x1, x2)))/sqrt(sqr(hdiff_2(x1, x2)[1])+sqr(hdiff_2(x1, x2)[0]))));

    ibex::Function h(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff(h, Function::DIFF);
    ibex::Function psi(x1, x2, tanh(h(x1, x2)-h0+error)+ibex::Interval::PI/2.0);
    ibex::Function f(x1, x2, Return((hdiff(x1, x2)[0]*cos(psi(x1, x2))-hdiff(x1, x2)[1]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0])),
                                    (hdiff(x1, x2)[1]*cos(psi(x1, x2))+hdiff(x1, x2)[0]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0]))));

    std::vector<ibex::Function *> f_list_inner{&f_1, &f_2};
    largest_positive_invariant(space, &f, f_list_inner, 16, "bathymetry_positive_invariant_uncertain", &sep_i);
}


int main(int argc, char *argv[]){
    bathy1();
//    bathy2();
}
