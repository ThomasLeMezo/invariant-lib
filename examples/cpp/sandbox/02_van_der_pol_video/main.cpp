#include "ibex_IntervalVector.h"
#include "ipegenerator/figure.h"

#include <iostream>
#include <omp.h>

#include <sstream>
#include <string>
#include <iomanip>

#include <eigen3/Eigen/Dense>

using namespace std;
using namespace ibex;

using namespace Eigen;

const string directory = "/home/lemezoth/Videos/thesis/vdp/";

Vector2d f(const Vector2d &x)
{
  Vector2d dx;
  dx[0] = x[1];
  dx[1] = (1-pow(x[0],2))*x[1]-x[0];
  return dx;
}

void example1()
{
  size_t step = 0;
  for(double max_t = 0.001; max_t <3.0; max_t +=0.1)
  {
    std::stringstream fig_name;
    fig_name << directory << "van_der_pol_simu" << std::setfill('0') << std::setw(5) << step << ".ipe";
    cout << fig_name.str() << endl;

    IntervalVector frame(2);
    frame[0] = ibex::Interval(-3,3);
    frame[1] = ibex::Interval(-3,3);

    ipegenerator::Figure fig(frame);

    fig.reset_scale(112,63,true);
    fig.set_graduation_parameters(-3, 1.0, -3, 1.0);
    fig.set_thickness_pen_factor(1e-3);

    vector<vector<double>> x1, x2;
    vector<Vector2d> x_init;

    // Initial condition & parameters
    double dt = 0.01;
    for(double i_x=-2; i_x<=2; i_x+=0.5){
      for(double i_y=-2; i_y<=2; i_y+=0.5){
        x_init.push_back(Vector2d(i_x, i_y));
        x_init.push_back(Vector2d(i_x, i_y));
      }
    }

    // Euler scheme
    for(size_t i=0; i<x_init.size(); ++i)
    {
      Vector2d x(x_init[i]);
      x1.push_back(vector<double>{x[0]});
      x2.push_back(vector<double>{x[1]});
      for(double t=0; t<max_t; t+=dt)
      {
        x += f(x)*dt;
        x1[i].push_back(x[0]);
        x2[i].push_back(x[1]);
      }
      fig.draw_curve(x1[i], x2[i]);
      fig.draw_circle(x1[i].back(), x2[i].back(),0.01);
    }

    fig.draw_axis("x_1", "x_2");
    std::stringstream time_text;
    time_text << "$t=" << max_t << "$";
    fig.draw_text(time_text.str(), 0.0, 3.2);

    fig.save_ipe(fig_name.str());
    step++;
  }

}

int main(int argc, char *argv[])
{
  example1();
}
