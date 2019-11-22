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


double depth_equilibrium = 5.0;

Vector2d f(const Vector2d &x, const double &u, const bool &compressibility=false, const double &chi=1e-4)
{
    Vector2d dx;
    dx[0] = x[1]; // Depth
    dx[1] = -0.001*u-0.1*abs(x[1])*x[1]+(compressibility?(chi*(x[0]-depth_equilibrium)):0.0); // Velocity
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

        ibex::IntervalVector frame(2);
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
                u=1;
            if(t>=60)
                u=-1;

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
            u=-1;

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

        ibex::IntervalVector frame(2);
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
    const string directory = "/home/lemezoth/Videos/thesis/animation3/imgs/";
    const Vector2d x_init(5.0, 0.0); // Depth, Velocity
    const double t_init = 0.0;
    const double global_t_max = 240.0;
    const double global_t_max_simulation = 400;
    const double dt = 0.01;
    double u = -1.0;

    const double time_video = 10.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    vector<double> x1, x2, u_log;
    vector<double> x1_compressibility, x2_compressibility;

    ibex::IntervalVector frame(2);
    frame[0] = ibex::Interval(0, 50);
    frame[1] = ibex::Interval(-0.05, 0.25);

    // Euler scheme
    Vector2d x(x_init);
    x1.push_back(x[0]);
    x2.push_back(x[1]);
    Vector2d x_compressibility(x_init);
    x1_compressibility.push_back(x[0]);
    x2_compressibility.push_back(x[1]);
    for(double t=0.0; t<global_t_max_simulation; t+=dt)
    {
        // Command
//        if(t<global_t_max/4.0)
//            u=0.0;
//        else
//            u=-1;

        x += f(x, u)*dt;
        x_compressibility += f(x_compressibility, u, true)*dt;

        x1.push_back(x[0]); // depth
        x2.push_back(x[1]); // velocity
        u_log.push_back(u);
        x1_compressibility.push_back(x_compressibility[0]); // depth
        x2_compressibility.push_back(x_compressibility[1]); // velocity
    }

    size_t step = 0;
    for(double max_t = t_init; max_t <global_t_max-dt_export; max_t +=dt_export)
    {
        size_t vector_position = (size_t)round(max_t/dt)+1;
        vector<double> x1_sub(x1_compressibility.begin(), x1_compressibility.begin()+vector_position);
        vector<double> x2_sub(x2_compressibility.begin(), x2_compressibility.begin()+vector_position);

        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        cout << fig_name.str() << endl;

        ipegenerator::Figure fig(frame);

        fig.reset_scale(112,63);
        fig.set_thickness_pen_factor(1e-3);

        fig.draw_curve(x1_sub, x2_sub);

        fig.set_dashed("dashed");
        fig.draw_curve(x1, x2);

        fig.draw_float(x1_sub.back(), x2_sub.back(),u_log[vector_position], x1_sub.back()/frame[0].ub(), ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

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

void example4()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation4/imgs/";
    const double t_init = 0.0;
    const double global_t_max = 240.0;

    const double time_video = 10.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    ibex::IntervalVector frame(2);
    frame[0] = ibex::Interval(-3, 3);
    frame[1] = ibex::Interval(-3, 3);


    size_t step = 0;
    for(double max_t = t_init; max_t <global_t_max; max_t +=dt_export)
    {
        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
//        cout << fig_name.str() << endl;
        ipegenerator::Figure fig(frame);
        fig.reset_scale(112,63);
        fig.set_thickness_pen_factor(1e-3);

        double compressibility = min(1.0, 2*max_t/global_t_max);

        ipe::Vector pt1(0.8, 0.0);
        ipe::Vector pt2(0.5-compressibility*0.3, 0.0);
        ipe::Matrix m(ipe::Linear(-1.0, 0.0, 0.0, 0.0), ipe::Vector(0.0, 0.0));

        fig.draw_arrow(pt1, pt2);
        fig.draw_arrow(m*pt1, m*pt2);
        fig.draw_float(0, 0, 0, compressibility, ipegenerator::FLOAT_PISTON_EQUAL, 0.25);

        std::stringstream text_time;
        text_time << "$z=" << round(compressibility*50.0) << "$";
        fig.draw_text(text_time.str(), 0.0, 2.0);

        fig.save_ipe(fig_name.str());
        step++;
    }
}

void simu1(const Vector2d &x_init, vector<double> &x1, vector<double> &x2, const double &t_end, const double &dt){
  // Simu
  Vector2d x(x_init);
  x1.push_back(x[0]);
  x2.push_back(x[1]);
  for(double t=0.0; t<t_end; t+=dt)
  {
      x += f(x, 0.0, true)*dt;
//      cout << x[0] << endl;

      x1.push_back(x[0]); // depth
      x2.push_back(x[1]); // velocity
  }
}

void example5_2(){
    const string directory = "/home/lemezoth/Videos/thesis/animation5/imgs2/";
    const double t_init = 0.0;

    const double time_video = 15.0; // in sec
    const double dt_export = 1/30.; // framerate = 30

    ibex::IntervalVector frame(2);
    frame[0] = ibex::Interval(-0.5, 2.0);
    frame[1] = ibex::Interval(-0.5, 4);
    size_t step = 0;

    // First Figure
    const double width_max = 0.5;
    double x = 0.75;
    double max_depth = 3.0;
    double compressibility_factor_water = 0.5;
    double compressibility_factor_float = 0.1;
    double depth_eq = 1.5;
    depth_equilibrium = 1.5;

    vector<double> x0, x1;
    simu1(Vector2d(2.0, 0.0),x0,x1,210.0, 1.0);

    for(double max_t = t_init; max_t<time_video; max_t +=dt_export)
    {
        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        ipegenerator::Figure fig(frame);
        fig.set_scale_offset(false);
        fig.reset_scale(112/4.,63, true);
        fig.set_thickness_pen_factor(1e-3);
        double depth, compressibility;

        const int t1 = 4;
        const int t2 = 8;

        if(max_t<t1)
        {
            depth = 1.5;
            compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth));
            fig.set_color_fill("gray");
            fig.set_color_type(ipegenerator::STROKE_AND_FILL);
            fig.draw_box(ipe::Vector(x, depth), compressibility, true);
        }
        else if(max_t>=t1 && max_t <t2){
            depth = 1.5;
            compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth));
            fig.set_color_fill("lightgray");
            fig.set_color_type(ipegenerator::STROKE_AND_FILL);
            fig.draw_box(ipe::Vector(x, depth), compressibility, true);

            depth = 2.0;
            compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth)-compressibility_factor_float*(depth-depth_eq));
            fig.set_color_fill("gray");
            fig.set_color_type(ipegenerator::STROKE_AND_FILL);
            fig.draw_box(ipe::Vector(x, depth), compressibility, true);

            compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth));
            fig.set_color_type(ipegenerator::STROKE_ONLY);
            fig.set_dashed("dashed");
            fig.draw_box(ipe::Vector(x, depth), compressibility, true);

            fig.set_dashed("");
            fig.draw_arrow(ipe::Vector(x, 1.5),ipe::Vector(x, depth));
        }
        else{
          depth = x0[round(((max_t-t2)/(time_video-t2))*x0.size())];
//          cout << depth << endl;
          compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth)-compressibility_factor_float*(depth-depth_eq));
          fig.set_color_fill("gray");
          fig.set_color_type(ipegenerator::STROKE_AND_FILL);
          fig.draw_box(ipe::Vector(x, depth), compressibility, true);

          compressibility = width_max*(1-compressibility_factor_water*(depth/max_depth));
          fig.set_color_type(ipegenerator::STROKE_ONLY);
          fig.set_dashed("dashed");
          fig.draw_box(ipe::Vector(x, depth), compressibility, true);
        }

        fig.draw_text("More compressible", x, -0.8);

        fig.save_ipe(fig_name.str());
        step++;
    }
    cout << step << endl;

}

void example5_1()
{
    const string directory = "/home/lemezoth/Videos/thesis/animation5/imgs1/";
    const double t_init = 0.0;
    const double global_t_max = 240.0;

    const double time_video = 3.0; // in sec
    const double frame_rate = 30; // frame/sec
    const double dt_export = (global_t_max-t_init)/(time_video*frame_rate);

    ibex::IntervalVector frame(2);
    frame[0] = ibex::Interval(-0.5, 2.0);
    frame[1] = ibex::Interval(-0.5, 4);
    size_t step = 0;

    // First Figure
    const double width_max = 0.5;
    double x = 0.75;

    for(double max_t = t_init; max_t<global_t_max; max_t +=dt_export)
    {
        std::stringstream fig_name;
        fig_name << directory << "flotteur" << std::setfill('0') << std::setw(5) << step << ".ipe";
        ipegenerator::Figure fig(frame);
        fig.set_scale_offset(false);
        fig.reset_scale(112/4.,63, true);
        fig.set_thickness_pen_factor(1e-3);

        double compressibility = width_max*(1-0.5*(max_t/global_t_max));
        double depth = (max_t/global_t_max)*3.0;
        ipe::Vector pt(x, depth);

        // Draw memory of boxes
        fig.set_color_fill("lightblue");
        fig.set_color_type(ipegenerator::STROKE_AND_FILL);

        fig.draw_box(ipe::Vector(x, 0.0), width_max, true);
        if(depth>=1.5)
            fig.draw_box(ipe::Vector(x, 1.5), width_max*0.75, true);

        // Draw box
        fig.draw_box(pt, compressibility, true);

        fig.draw_arrow(ipe::Vector(0.0, 0.0), ipe::Vector(0.0, 3.2));
        fig.draw_text("z",0.0, 3.3, true);
        fig.draw_text("Water", x, -0.8);

        fig.save_ipe(fig_name.str());
        step++;
    }
    cout << step << endl;

}


int main(int argc, char *argv[])
{
//    example1();
//    example2();
//    example3();
//    example4();
//    example5_1();
    example5_2();
}
