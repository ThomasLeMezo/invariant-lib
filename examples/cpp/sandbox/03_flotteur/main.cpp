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



Vector2d f(const Vector2d &x, const double &u, const bool &compressibility=false)
{
    Vector2d dx;
    dx[0] = x[1]; // Depth
    dx[1] = 0.001*u-0.1*abs(x[1])*x[1]-(compressibility)?0.01*x[1]:0.0; // Velocity
    return dx;
}

void example1()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation1/imgs/";
    const Vector2d x_init(0.5, 0.0); // Depth, Velocity
    const double t_init = 10.0;
    const double global_t_max = 130.0;
    const double dt = 0.01;
    double u = 0.0;

    const double time_video = 15.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    size_t step = 0;
    for(double max_t = t_init; max_t <global_t_max; max_t +=dt_export)
    {
        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        IntervalVector frame(2);
        frame[0] = ibex::Interval(0,global_t_max);
        frame[1] = ibex::Interval(-1, 1);

        ipegenerator::Figure fig(frame);

        fig.reset_scale(112,63);
        fig.set_thickness_pen_factor(1e-3);

        vector<double> x1, x2;

        // Euler scheme
        Vector2d x(x_init);
        x1.push_back(0.0);
        x2.push_back(x[0]);
        for(double t=t_init; t<max_t; t+=dt)
        {
            // Command
            if(t<30.0)
                u=0;
            if(t>=30.0 && t<60.0)
                u=-1;
            if(t>=60)
                u=1;

            x += f(x, u)*dt;
            x1.push_back(t); // time
            x2.push_back(x[0]); // depth
        }
        fig.draw_curve(x1, x2);
//        cout << x2.back() << endl;

        fig.set_dashed("dashed");
        if(max_t>=30.0){
            fig.draw_segment(30.0, -1.0, 30.0, 1.0);
        }
        if(max_t>=60.0){
            fig.draw_segment(60.0, -1.0, 60.0, 1.0);
        }

        fig.draw_float(x1.back(), x2.back(),u, 0.0, ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

        fig.set_graduation_parameters(0, 10,-1,0.5);
        fig.draw_axis("t", "z");
        std::stringstream time_text;
        time_text << "$u=" << u << "$";
        fig.draw_text(time_text.str(), frame[0].ub()/2., frame[1].ub()*1.2);

        fig.save_ipe(fig_name.str());
        step++;
    }

}

void example2()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation2/imgs/";
    const Vector2d x_init(5.0, 0.0); // Depth, Velocity
    const double t_init = 0.0;
    const double global_t_max = 500.0;
    const double dt = 0.01;
    double u = 0.0;

    const double time_video = 10.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    vector<double> x1, x2, u_log;

    // Euler scheme
    Vector2d x(x_init);
    x1.push_back(x[0]);
    x2.push_back(x[1]);
    for(double t=0.0; t<global_t_max; t+=dt)
    {
        // Command
        if(t<global_t_max/4.0)
            u=0;
        else
            u=1;

        x += f(x, u)*dt;
        x1.push_back(x[0]); // depth
        x2.push_back(x[1]); // velocity
        u_log.push_back(u);
    }

    size_t step = 0;
    for(double max_t = t_init; max_t <global_t_max; max_t +=dt_export)
    {
        size_t vector_position = (size_t)round(max_t/dt)+1;
        vector<double> x1_sub(x1.begin(), x1.begin()+vector_position);
        vector<double> x2_sub(x2.begin(), x2.begin()+vector_position);

        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        IntervalVector frame(2);
        frame[0] = ibex::Interval(0, 50);
        frame[1] = ibex::Interval(-0.25, 0.25);

        ipegenerator::Figure fig(frame);

        fig.reset_scale(112,63);
        fig.set_thickness_pen_factor(1e-3);

        fig.draw_curve(x1_sub, x2_sub);
        fig.draw_float(x1_sub.back(), x2_sub.back(),u_log[vector_position], 0.0, ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

        fig.set_graduation_parameters(frame[0].lb(), 10,frame[1].lb(),0.1);
        fig.draw_axis("z", "\\dot{z}");
        std::stringstream text_time;
        text_time << "$t=" << round(max_t) << "$";
        fig.draw_text(text_time.str(), frame[0].ub()/2., frame[1].ub()*1.2);
        std::stringstream text_cmd;
        text_cmd << "$u=" << u_log[vector_position] << "$";
        fig.draw_text(text_cmd.str(), frame[0].ub()/2., frame[1].ub()*1.1);

        fig.save_ipe(fig_name.str());
        step++;
    }

}

void example3()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation2/imgs/";
    const Vector2d x_init(5.0, 0.0); // Depth, Velocity
    const double t_init = 0.0;
    const double global_t_max = 500.0;
    const double dt = 0.01;
    double u = 0.0;

    const double time_video = 10.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    vector<double> x1, x2, u_log;

    // Euler scheme
    Vector2d x(x_init);
    x1.push_back(x[0]);
    x2.push_back(x[1]);
    for(double t=0.0; t<global_t_max; t+=dt)
    {
        // Command
        if(t<global_t_max/4.0)
            u=0;
        else
            u=1;

        x += f(x, u)*dt;
        x1.push_back(x[0]); // depth
        x2.push_back(x[1]); // velocity
        u_log.push_back(u);
    }

    size_t step = 0;
    for(double max_t = t_init; max_t <global_t_max; max_t +=dt_export)
    {
        size_t vector_position = (size_t)round(max_t/dt)+1;
        vector<double> x1_sub(x1.begin(), x1.begin()+vector_position);
        vector<double> x2_sub(x2.begin(), x2.begin()+vector_position);

        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        IntervalVector frame(2);
        frame[0] = ibex::Interval(0, 50);
        frame[1] = ibex::Interval(-0.25, 0.25);

        ipegenerator::Figure fig(frame);

        fig.reset_scale(112,63);
        fig.set_thickness_pen_factor(1e-3);

        fig.draw_curve(x1_sub, x2_sub);
        fig.draw_float(x1_sub.back(), x2_sub.back(),u_log[vector_position], 0.0, ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

        fig.set_graduation_parameters(frame[0].lb(), 10,frame[1].lb(),0.1);
        fig.draw_axis("z", "\\dot{z}");
        std::stringstream text_time;
        text_time << "$t=" << round(max_t) << "$";
        fig.draw_text(text_time.str(), frame[0].ub()/2., frame[1].ub()*1.2);
        std::stringstream text_cmd;
        text_cmd << "$u=" << u_log[vector_position] << "$";
        fig.draw_text(text_cmd.str(), frame[0].ub()/2., frame[1].ub()*1.1);

        fig.save_ipe(fig_name.str());
        step++;
    }

}


int main(int argc, char *argv[])
{
//    example1();
//    example2();
    example3();
}
