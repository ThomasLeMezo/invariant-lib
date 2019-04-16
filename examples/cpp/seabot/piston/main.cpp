#include <iostream>
#include <ibex/ibex.h>

#include <vibes/vibes.h>
#include <VibesFigure.h>

#include <omp.h>

using namespace std;
using namespace ibex;

bool sortBox(ibex::IntervalVector &iv1,ibex::IntervalVector &iv2) { return (iv1[0].mid()<iv2[0].mid()); }

int main(int argc, char *argv[]){

  double screw_thread = 1.75e-3;
  double tick_per_turn = 48;
  double piston_diameter = 0.05;
  double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;

  // Physical parameters
  double g = 9.81;
  double rho = 1025.0;
  double m = 9.045*2.0;
  double diam = 0.24;
  double alpha = tick_to_volume*0.0;

  // Regulation parameters
  double x2_target = 0.0; // Desired depth
  double beta = 0.05/M_PI_2;

  double r = -1.0;
  double l1 = -2.0*r;
  double l2 = pow(r, 2);
//    cout << "l1 = " << l1 << endl;
//    cout << "l2 = " << l2 << endl;

  double Cf = M_PI*pow(diam/2.0,2);

  // Command
  double A = g*rho/m;
  double B = 0.5*rho*Cf/m;

  ibex::Variable x1, x2, x3;
  ibex::Function e(x2, (x2_target-x2));
  ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
  ibex::Function dx1(x1, x2, x3, (-A*(x3-alpha*x2)-B*abs(x1)*x1));
  ibex::Function D(x2, (1+pow(e(x2), 2)));
  ibex::Function dy(x1, x2, x3, (dx1(x1, x2, x3)+beta*x1/D(x2)));

  ibex::Function u(x1, x2, x3, ((l1*dy(x1, x2, x3)+l2*y(x1, x2)
                                 +beta*(dx1(x1, x2, x3)*D(x2)+2.0*e(x2)*pow(x1,2))/(pow(D(x2),2))-2.0*B*abs(x1)*dx1(x1, x2, x3))/A+alpha*x1));

  list<ibex::Interval> z_list{ibex::Interval(-10,10)};
  vector<ibex::IntervalVector> result;
  ibex::LargestFirst bisector();

  while(z_list.size()!=0){
    ibex::Interval z = z_list.front();
    z_list.pop_front();

    if(z.diam()>0.001 /*&& dV.diam()>tick_to_volume*/){
      pair<ibex::Interval, ibex::Interval> b = z.bisect();
      z_list.push_back(b.first);
      z_list.push_back(b.second);
    }
    else{
      ibex::Interval dz = beta*atan(-z);
      ibex::Interval V = (1./A)*(beta*dz/D.eval(ibex::IntervalVector(1,z))-B*ibex::abs(dz)*dz) + alpha*z;
      ibex::IntervalVector tmp(3);
      tmp[0] = dz ;//+ ibex::Interval(-0.002, 0.002);
      tmp[1] = z ;
      tmp[2] = V ;
      ibex::Interval dV = u.eval(tmp);

      ibex::IntervalVector r(2);
      r[0] = z;
      r[1] = dV;
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
                        "height", 1024));
  vibes::axisLimits(-10, 10, -40.*tick_to_volume, 40.*tick_to_volume, "Piston");

  vector<double> x_ub, y_ub, x_lb, y_lb;
  for(ibex::IntervalVector &iv:result){
//    vibes::drawBox(iv);
    x_ub.push_back(iv[0].lb());
    x_lb.push_back(iv[0].lb());

    y_ub.push_back(iv[1].lb());
    y_lb.push_back(iv[1].ub());
  }

  vibes::drawLine(x_ub, y_ub);
  vibes::drawLine(x_lb, y_lb);

  ibex::IntervalVector u_max(2);
  u_max[0] = ibex::Interval(-10,10);
  u_max[1] = ibex::Interval(-20.*tick_to_volume, 20.*tick_to_volume);
  vibes::drawBox(u_max, "r");

  ibex::IntervalVector u_max2(2);
  u_max2[0] = ibex::Interval(-10,10);
  u_max2[1] = ibex::Interval(-100.*tick_to_volume, 100.*tick_to_volume);
  vibes::drawBox(u_max2, "b");


  vibes::endDrawing();
}
