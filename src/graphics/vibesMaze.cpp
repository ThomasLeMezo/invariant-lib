#include "vibesMaze.h"
#include <regex>
#include <ostream>

using namespace invariant;
using namespace ibex;
using namespace std;

template <>
void VibesMazeIBEX::drawBox(const ibex::IntervalVector &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(box,color_stroke,color_fill);
}

template <>
void VibesMazeEXP::drawBox(const invariant::ExpPoly &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(box.getBox(),color_stroke,color_fill);
}

template <>
void VibesMazeIBEX::drawBox_with_scale(const ibex::IntervalVector &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(hadamard_product((box+m_offset), m_scale_factor),color_stroke, color_fill);
}

template <>
void VibesMazeEXP::drawBox_with_scale(const invariant::ExpPoly &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(hadamard_product((box.getBox()+m_offset), m_scale_factor),color_stroke, color_fill);
}

template <>
void VibesMazeIBEX::show_flows(invariant::PaveIBEX *p, invariant::MazeIBEX* maze) const{
    return;
}


template <>
void VibesMazeEXP::show_flows(invariant::PaveEXP *p, invariant::MazeEXP* maze) const{
    if(!m_enable_flows)
        return;
    invariant::RoomEXP *r = p->get_rooms()[maze];
//    invariant::DYNAMICS_SENS sens = r->get_maze()->get_dynamics()->get_sens();
    const std::vector<FaceEXP *> &vctFaces = p->get_faces_vector();
    const std::vector<ExpVF> vector_fields = r->get_vector_fields_typed_fwd();
    if (vector_fields.size()==0) return;
    const ExpVF &VF = vector_fields[0];
    for (FaceEXP *face : vctFaces) {
       const invariant::DoorEXP *d = face->get_doors()[maze];
       const ExpPoly in = d->get_input();
       if (in.is_empty()) continue;
       for (FaceEXP *face2 : vctFaces) {
          if (face2==face) continue;
          const invariant::DoorEXP *d2 = face2->get_doors()[maze];
          ExpPoly out = d2->get_output();
          if (out.is_empty()) continue;
          ibex::Interval t = VF.compute_basic_time(in,out);
          t = VF.refine_time(in,out,t);
          if (t.is_empty()) continue;
          std::vector<std::pair<ibex::IntervalVector, ibex::Interval>>  lst = 
		VF.build_flow(in,out,t);
          ExpPoly flow(VF.basicFlow(in,t),lst,false);
          if (flow.is_empty()) continue;
          std::vector<double> X,Y;
          flow.vertices2D(X,Y);
          for (int i=0;i<X.size();i++) {
               X[i] = (X[i]+m_offset[0].mid())*m_scale_factor[0].mid();
               Y[i] = (Y[i]+m_offset[1].mid())*m_scale_factor[1].mid();
          }
          this->drawPolygon(X,Y,"white","#8080807F");
       }
    }
}

