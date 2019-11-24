#include "ibex_IntervalVector.h"
#include "ipegenerator/figure.h"

#include <iostream>
#include <omp.h>

#include <sstream>
#include <string>
#include <iomanip>

#include <eigen3/Eigen/Dense>

using namespace std;

using namespace Eigen;
double g, rho, m, diam, chi, Cf, A, B;

Vector2d f(const Vector2d &x, const double &u)
{
    Vector2d dx;
    dx[0] = x[1]; // Depth
    dx[1] = -A*(u-chi*x[0])-B*abs(x[1])*x[1];
    return dx;
}

void example1()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation6/imgs/";

    g = 9.81;
    rho = 1025.0;
    m = 9.045*2.;
    diam = 0.24;
    chi = 2.15e-6;
    Cf = M_PI*pow(diam/2.0,2);
    A = g*rho/m;
    B = 0.5*rho*Cf/m;
    // Command
    ibex::Interval Vp(-2.148e-5, 1.503e-4);

    const Vector2d x_init(0.0, 0.0); // Depth, Velocity
    const double t_init = 0.0;
    const double global_t_max = 600.0;
    const double global_t_max2 = 300.0;
    const double dt = 0.1;
    double u = 0.0;

    const double time_video = 10.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);
    const double dt_export2 = (global_t_max/global_t_max2)*(global_t_max2-t_init)/(time_video*frame_rate);

    vector<double> x1, x2, u_log;
    vector<double> y1, y2, u2_log;

    // Euler scheme
    Vector2d x(x_init);
    u = Vp.lb();

    x1.push_back(x(0));
    x2.push_back(x(1));
    u2_log.push_back(u);

    size_t k = 0;
    bool reached_limit = false;
    for(double t=0.0; t<global_t_max; t+=dt)
    {
        if(x(0)>69)
            reached_limit = true;
        if(reached_limit)
            u=Vp.ub();

        x += f(x, u)*dt;
        cout << k << " " << x(0) << " " << x(1) << " " << u << endl;
        x1.push_back(x(0)); // depth
        x2.push_back(x(1)); // velocity
        u_log.push_back(u);
        ++k;
    }

    x = Vector2d(x1[3720], x2[3720]);
    y1.push_back(x(0));
    y2.push_back(x(1));
    u2_log.push_back(u);
    for(double t=0.0; t<global_t_max2; t+=dt)
    {
        x += f(x, u)*dt;
        cout << x(0) << " " << x(1) << endl;
        y1.push_back(x(0)); // depth
        y2.push_back(x(1)); // velocity
        u2_log.push_back(u);
    }

    ibex::IntervalVector frame(2);
    frame[0] = ibex::Interval(-20, 80.0);
    frame[1] = ibex::Interval(-0.4,0.4);
    ipegenerator::Figure fig("/home/lemezoth/Documents/ensta/paper/thesis/presentation/imgs/ipe/piston_kernel_positive.ipe",frame,112,63);
    fig.set_line_width(0.5);
    double text_x_pos = 30.0;
    double text_y_pos = 0.45;

    size_t step = 0;
    size_t id_circle, id_curve, id_text;
    for(double max_t = t_init; max_t <global_t_max; max_t +=dt_export)
    {
        size_t vector_position = (size_t)round((max_t/(global_t_max+1))*x1.size())+1;
        vector<double> x1_sub(x1.begin(), x1.begin()+vector_position);
        vector<double> x2_sub(x2.begin(), x2.begin()+vector_position);
        vector<double> u_log_sub(u_log.begin(), u_log.begin()+vector_position);

        std::stringstream fig_name;
        fig_name << directory << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        fig.set_color_fill("gray");
        fig.set_color_type(ipegenerator::STROKE_AND_FILL);
        fig.set_opacity(75);
        id_circle = fig.draw_circle_radius_final(x1_sub.back(), x2_sub.back(), 5.0);

        fig.set_color_type(ipegenerator::STROKE_ONLY);
        fig.set_opacity(100);
        id_curve = fig.draw_curve(x1_sub, x2_sub);

        std::stringstream time_text;
        time_text << "$u=" << ((u_log_sub.back()>0.)?"1":"-1") << "$";
        id_text = fig.draw_text(time_text.str(), text_x_pos, text_y_pos);

        fig.save_ipe(fig_name.str());

        if(max_t+dt_export<global_t_max)
        {
            fig.remove_object(id_curve);
            fig.remove_object(id_circle);
        }
        fig.remove_object(id_text);

        step++;
    }

    for(double max_t = t_init; max_t <global_t_max2; max_t +=dt_export2)
    {
        size_t vector_position = (size_t)round((max_t/(global_t_max2+1))*y1.size())+1;
        vector<double> x1_sub(y1.begin(), y1.begin()+vector_position);
        vector<double> x2_sub(y2.begin(), y2.begin()+vector_position);
        vector<double> u_log_sub(u2_log.begin(), u2_log.begin()+vector_position);

        std::stringstream fig_name;
        fig_name << directory << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        fig.set_color_fill("gray");
        fig.set_color_type(ipegenerator::STROKE_AND_FILL);
        fig.set_opacity(75);
        id_circle = fig.draw_circle_radius_final(x1_sub.back(), x2_sub.back(), 5.0);

        fig.set_color_type(ipegenerator::STROKE_ONLY);
        fig.set_opacity(100);
        id_curve = fig.draw_curve(x1_sub, x2_sub);
//        fig.draw_float(x1_sub.back(), x2_sub.back(),u_log[vector_position], 0.0, ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

        std::stringstream time_text;
        time_text << "$u=" << ((u_log_sub.back()>0)?"1":"-1") << "$";
        id_text = fig.draw_text(time_text.str(), text_x_pos, text_y_pos);

        fig.save_ipe(fig_name.str());
        fig.remove_object(id_curve);
        fig.remove_object(id_circle);
        fig.remove_object(id_text);
        step++;
    }

}

int main(int argc, char *argv[])
{
    example1();
}
