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

const string directory = "/home/lemezoth/Videos/thesis/vdp/imgs/";

Vector2d f(const Vector2d &x)
{
    Vector2d dx;
    dx[0] = x[1];
    dx[1] = (1-pow(x[0],2))*x[1]-x[0];
    return dx;
}

// Visu statique du champ
void example2()
{
    IntervalVector frame(2);
    frame[0] = ibex::Interval(-3,3);
    frame[1] = ibex::Interval(-3,3);
    std::stringstream fig_name;
    fig_name << directory << "vdp_vf2.ipe";
    cout << fig_name.str() << endl;

    ipegenerator::Figure fig(frame, 112, 63, true);
    fig.set_graduation_parameters(-3, 1.0, -3, 1.0);
    fig.set_thickness_pen_factor(1e-3);
    fig.set_number_digits_axis_x(1);
    fig.set_number_digits_axis_y(1);



    // ********************* Simulate trajectory *********************
    double dt_euler = 0.01;
    vector<double> x1, x2;
    Vector2d x(1,1);
    x1.push_back(x(0));
    x2.push_back(x(1));
    for(double t=0; t<100.0; t+=dt_euler)
    {
        x += f(x)*dt_euler;
        x1.push_back(x(0));
        x2.push_back(x(1));
    }

    size_t offset = (size_t)round(90./dt_euler);
    for(size_t i=x1.size()-20; i>offset; --i)
    {
        if(abs(x1[i]-x1.back())<0.01 && abs(x2[i]-x2.back())<0.01)
        {
            offset = i;
            break;
        }
    }

    vector<double> sub_x1 = vector<double>(x1.begin()+offset, x1.end());
    vector<double> sub_x2 = vector<double>(x2.begin()+offset, x2.end());
    fig.set_color_stroke("orange");
    fig.set_line_width(2.0);
    fig.draw_polygon(sub_x1, sub_x2);

    // ********************* Vector field *********************
    vector<Vector2d> x_init;
    // Initial condition & parameters
    for(double i_x=-3; i_x<=3; i_x+=0.2){
        for(double i_y=-3; i_y<=3; i_y+=0.2){
            x_init.push_back(Vector2d(i_x, i_y));
        }
    }

    fig.reset_attribute();
    fig.set_arrow_size(2.0);
    for(Vector2d &x:x_init)
    {
        Vector2d xdot = f(x);
        xdot.normalize();
        xdot /= 10.;
        fig.draw_arrow(ipe::Vector(x(0), x(1)),ipe::Vector(x(0)+xdot(0),x(1)+xdot(1)));
    }
    fig.draw_symbol(0.0, 0.0,"disk(sx)");

    // ********************* Save figure *********************
    fig.draw_axis("x_1", "x_2");
    fig.save_ipe(fig_name.str());

}

void example1()
{
    double t_max = 7.0;
    double dt_euler = 0.02;
    double dt_plot = 0.02;
    double duration_trace = 0.2;

    // Compute trajectories
    vector<vector<double>> x1, x2;
    vector<Vector2d> x_init;

    // Initial condition & parameters

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
        for(double t=0; t<t_max; t+=dt_euler)
        {
            x += f(x)*dt_euler;
            x1[i].push_back(x[0]);
            x2[i].push_back(x[1]);
        }
    }

    size_t step = 0;
    IntervalVector frame(2);
    frame[0] = ibex::Interval(-3,3);
    frame[1] = ibex::Interval(-3,3);
    for(double t = 0.0; t <t_max; t +=dt_plot)
    {
        std::stringstream fig_name;
        fig_name << directory << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        ipegenerator::Figure fig(frame, 112, 63, true);
        fig.set_graduation_parameters(-3, 1.0, -3, 1.0);
        fig.set_thickness_pen_factor(1e-3);
        fig.set_number_digits_axis_x(1);
        fig.set_number_digits_axis_y(1);


        for(size_t n=0; n<x_init.size(); n++){
            size_t start = (size_t)std::max(0.,round((t-duration_trace)/dt_euler));
            size_t end = (size_t)std::max(1., round(t/dt_euler));
            vector<double> x1_sub(x1[n].begin()+start, x1[n].begin()+end);
            vector<double> x2_sub(x2[n].begin()+start, x2[n].begin()+end);

            fig.set_color_stroke("darkblue");
            fig.draw_curve(x1_sub, x2_sub);
            fig.set_color_stroke("black");
            fig.draw_circle(x1_sub.back(), x2_sub.back(), 0.01);
        }

        fig.draw_axis("x_1", "x_2");
        std::stringstream time_text;
        time_text << "$t=" << t << "$";
        fig.draw_text(time_text.str(), 0.0, 3.2);

        fig.save_ipe(fig_name.str());
        step++;
    }

}

int main(int argc, char *argv[])
{
    //  example1();
    example2();
}
