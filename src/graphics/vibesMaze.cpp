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
