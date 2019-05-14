#include <iostream>
#include <ibex/ibex.h>

#include <vibes/vibes.h>
#include <VibesFigure.h>

#include <omp.h>

using namespace std;
using namespace ibex;

bool sortBox(ibex::IntervalVector &iv1,ibex::IntervalVector &iv2) { return (iv1[0].mid()<iv2[0].mid()); }

int main(int argc, char *argv[]){

  ibex::Interval z_space = ibex::Interval(0,50);

  double screw_thread = 1.75e-3;
  double tick_per_turn = 48;
  double piston_diameter = 0.05;
  double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;

  // Physical parameters
  double g = 9.81;
  double rho = 1025.0;
  double m = 9.045*2.0;
  double diam = 0.24;
  double chi = tick_to_volume*30.0;

  // Regulation parameters
  double Cf = M_PI*pow(diam/2.0,2);

  // Command
  double A = g*rho/m;
  double B = 0.5*rho*Cf/m;
  ibex::Interval Vp(-2.148e-5, 1.503e-4);

  ibex::Variable x1;
  ibex::Function f_zmax(x1, sqrt(abs((A/B)*(Vp-chi*x1))));

  list<ibex::Interval> z_list{z_space};
  vector<ibex::IntervalVector> result;

  ibex::Interval u_bounds = Interval::ZERO;

  while(z_list.size()!=0){
    ibex::Interval z = z_list.front();
    z_list.pop_front();

    if(z.diam()>0.01){
      pair<ibex::Interval, ibex::Interval> b = z.bisect();
      z_list.push_back(b.first);
      z_list.push_back(b.second);
    }
    else{
      ibex::Interval dz = f_zmax.eval(ibex::IntervalVector(z));
      ibex::IntervalVector r(2);
      r[0] = z;
      r[1] = dz;
      u_bounds |= dz;
      result.push_back(r);
    }
    cout << z_list.size() << endl;
  }

  sort(result.begin(), result.end(), sortBox);

  vibes::beginDrawing();
  vibes::newFigure("Piston");
  vibes::setFigureProperties(
            vibesParams("x", 10,
                        "y", 0,
                        "width", 1024,
                        "height", 500));

  vector<double> x_ub, y_ub, x_lb, y_lb;
  for(ibex::IntervalVector &iv:result){
//    vibes::drawBox(iv);
    x_ub.push_back(iv[0].lb());
    x_lb.push_back(iv[0].lb());

    y_ub.push_back(iv[1].lb());
    y_lb.push_back(iv[1].ub());
    if(isnan((iv[1].lb())))
        cout << "ERROR" << endl;
  }

  vibes::drawLine(x_ub, y_ub);
  vibes::drawLine(x_lb, y_lb);

  ibex::IntervalVector border(2);
  border[0] = z_space+ibex::Interval(-1,1);
  border[1] = u_bounds+ibex::Interval(-0.5, 0.5);
  vibes::drawBox(border, "black[]");
//  vibes::axisAuto("Max Velocity");
  vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/max_velocity.svg", "Max Velocity");

  cout << "u_bounds = " << u_bounds << endl;

  vibes::endDrawing();
}
