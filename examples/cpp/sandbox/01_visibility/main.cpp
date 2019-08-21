//#include "smartSubPaving.h"
//#include "domain.h"
//#include "dynamicsFunction.h"
//#include "maze.h"
//#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"
#include "ibex_LargestFirst.h"
#include "ibex_SepInter.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include <queue>

using namespace std;
using namespace ibex;

void f_visibility(ibex::IntervalVector &a, ibex::IntervalVector &b, ibex::IntervalVector &c, ibex::Interval &t, ibex::IntervalVector &v){
  a &= (b-c);
  b &= (c+a);
  c &= (b-a) & (t*v);
  // t &= ((c[0]/(v[0]&Interval.POS_REALS))&Interval.POS_REALS) | ((c[0]/(v[0]&Interval.NEG_REALS))&Interval.NEG_REALS);
  // t &= ((c[1]/(v[1]&Interval.POS_REALS))&Interval.POS_REALS) | ((c[1]/(v[1]&Interval.NEG_REALS))&Interval.NEG_REALS);
  t &= c[0]/v[0];
  t &= c[1]/v[1];
  v &= ((1/t)*c);
}

void v2(){
  ibex::IntervalVector x_init(9);
  // a
  x_init[0] = ibex::Interval(-2,2);
  x_init[1] = ibex::Interval(-2,2);
  // b
  x_init[2] = ibex::Interval(2.5,3.5);
  x_init[3] = ibex::Interval(1,2);
  // c
  x_init[4] = ibex::Interval();
  x_init[5] = ibex::Interval();
  // t
  x_init[6] = ibex::Interval::POS_REALS;
  // v
  x_init[7] = ibex::Interval(1);
  x_init[8] = ibex::Interval(0.4, 0.5);

  ibex::Variable x(9); // a(2), b(2), c(2), t(1), v(2)
  ibex::Function f_circleA(x, pow(x[0],2)+pow(x[1],2)-pow(2,2));
  ibex::SepFwdBwd sep_A(f_circleA, LEQ);

  ibex::Function f_circleB(x, pow(x[2]-3,2)+pow(x[3]-1.5,2)-pow(0.5,2));
  ibex::SepFwdBwd sep_B(f_circleB, LEQ);

  ibex::Variable v_a(2), v_b(2), v_c(2), v_t(1), v_v(2);
  ibex::Function f_visi(v_a, v_b, v_c, v_t, v_v, (v_a-v_b+v_t*v_v));
  ibex::SepFwdBwd sep_visi(f_visi,EQ);

  ibex::Function f_visi1(v_a, v_b, v_c, v_t, v_v, (v_b-v_a-v_c));
  ibex::SepFwdBwd sep_visi1(f_visi1,EQ);
  ibex::Function f_visi2(v_a, v_b, v_c, v_t, v_v, (v_t-v_c[0]/v_v[0]));
  ibex::SepFwdBwd sep_visi2(f_visi2,EQ);
  ibex::Function f_visi3(v_a, v_b, v_c, v_t, v_v, (v_t-v_c[1]/v_v[1]));
  ibex::SepFwdBwd sep_visi3(f_visi3,EQ);

  ibex::SepInter sep(Array<Sep>(sep_A, sep_B, sep_visi, sep_visi1, sep_visi2, sep_visi3));

  double epsilon = 0.1;
  ibex::LargestFirst bisector(0., 0.5);

  queue<ibex::IntervalVector> list;
  vector<ibex::IntervalVector> result_inner;
  vector<ibex::IntervalVector> result_outer;
  vector<ibex::IntervalVector> result_unknown;

  ibex::IntervalVector a(2),b(2);
  a[0] = x_init[0];
  a[1] = x_init[1];
  b[0] = x_init[2];
  b[1] = x_init[3];
  list.push(a);

  while(list.size()!=0){
    ibex::IntervalVector a_tmp(list.front());
    list.pop();

    // Contract box visibility
    ibex::IntervalVector x_tmp_out(x_init);
    x_tmp_out[0] = a_tmp[0];
    x_tmp_out[1] = a_tmp[1];
    ibex::IntervalVector x_tmp_in(x_tmp_out);
    sep.separate(x_tmp_out, x_tmp_in);

    a_tmp[0] = x_tmp_out[0];
    a_tmp[1] = x_tmp_out[1];

    // case inside S
    if(x_tmp_in.is_empty()){
      result_outer.push_back(a_tmp);
    }
    else if(x_tmp_out.is_empty()){
      result_inner.push_back(a_tmp);
    }
    else{
      if(a_tmp.max_diam()<epsilon)
        result_unknown.push_back(a_tmp);
      else{
        std::pair<ibex::IntervalVector,ibex::IntervalVector> bisection_result = bisector.bisect(a_tmp);
        list.push(bisection_result.first);
        list.push(bisection_result.second);
      }
    }
  }

  vibes::beginDrawing();
  vibes::newFigure("visibility_sivia");
  vibes::setFigureProperties(
            vibesParams("x", 10,
                        "y", 0,
                        "width", 1024,
                        "height", 500));

  for(ibex::IntervalVector &box:result_outer)
    vibes::drawBox(box, "black[blue]");

  for(ibex::IntervalVector &box:result_inner)
    vibes::drawBox(box, "black[#FF00FF]");

  for(ibex::IntervalVector &box:result_unknown)
    vibes::drawBox(box, "black[yellow]");

  vibes::drawCircle(0., 0., 2, "black[]");
  vibes::drawBox(a, "black[]");
  vibes::drawBox(b, "black[]");

  ibex::IntervalVector space(2);
  space[0] = ibex::Interval(-2.5, 4);
  space[1] = ibex::Interval(-2.5, 2.5);
  vibes::drawBox(space, "white[]");
  vibes::axisLimits(space[0].lb(),space[0].ub(),space[1].lb(),space[1].ub());

  vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/visibility_sivia.svg", "visibility_sivia");
  vibes::endDrawing();
}

void v1(){
  ibex::IntervalVector a(2), b(2),v(2);
  a[0] = ibex::Interval(-2,2);
  a[1] = ibex::Interval(-2,2);

  b[0] = ibex::Interval(2.5,3.5);
  b[1] = ibex::Interval(1,2);

  v[0] = ibex::Interval(1);
  v[1] = ibex::Interval(0.4, 0.5);

  ibex::Variable x(2);
  ibex::Function f_circleA(x, pow(x[0],2)+pow(x[1],2)-pow(2,2));
  ibex::CtcFwdBwd ctc_A(f_circleA, LEQ);

  ibex::Function f_circleB(x, pow(x[0]-3,2)+pow(x[1]-1.5,2)-pow(0.5,2));
  ibex::CtcFwdBwd ctc_B(f_circleB, LEQ);

  double epsilon = 0.1;
  ibex::LargestFirst bisector(0., 0.5);

  queue<ibex::IntervalVector> list;
  vector<ibex::IntervalVector> results;
  list.push(a);

  while(list.size()!=0){
    ibex::IntervalVector a_tmp(list.front());
    list.pop();

    // Contract box visibility
    ibex::IntervalVector c(2);
    ibex::Interval t(ibex::Interval::POS_REALS);
    ibex::IntervalVector b_tmp(b);
    ibex::IntervalVector v_tmp(v);
    for(size_t i=0; i<5; i++){
      ctc_A.contract(a_tmp);
      ctc_B.contract(b_tmp);
      f_visibility(a_tmp, b_tmp, c, t, v_tmp);
    }

    if(!a_tmp.is_empty()){
      if(a_tmp.max_diam()<epsilon)
        results.push_back(a_tmp);
      else{
        std::pair<ibex::IntervalVector,ibex::IntervalVector> bisection_result = bisector.bisect(a_tmp);
        list.push(bisection_result.first);
        list.push(bisection_result.second);
      }
    }
  }

  vibes::beginDrawing();
  vibes::newFigure("Visibility");
  vibes::setFigureProperties(
            vibesParams("x", 10,
                        "y", 0,
                        "width", 1024,
                        "height", 500));

  for(ibex::IntervalVector &box:results)
    vibes::drawBox(box, "black[yellow]");

  vibes::drawCircle(0., 0., 2, "black[]");
  vibes::drawBox(a, "black[]");
  vibes::drawBox(b, "black[]");

//    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/visibility.svg", "Visibility");
  vibes::endDrawing();
}

int main(int argc, char *argv[]){
  v2();
}
