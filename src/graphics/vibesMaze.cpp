#include "vibesMaze.h"
#include <regex>
#include <ostream>

using namespace invariant;
using namespace ibex;
using namespace std;

template <>
void VibesMaze<ibex::IntervalVector>::drawBox(const ibex::IntervalVector &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(box,color_stroke,color_fill);
}

template <>
void VibesMaze<invariant::ExpBox>::drawBox(const invariant::ExpBox &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(box.getBox(),color_stroke,color_fill);
}

template <>
void VibesMaze<ibex::IntervalVector>::drawBox_with_scale(const ibex::IntervalVector &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(hadamard_product((box+m_offset), m_scale_factor),color_stroke, color_fill);
}

template <>
void VibesMaze<invariant::ExpBox>::drawBox_with_scale(const invariant::ExpBox &box, const std::string &color_stroke, const std::string &color_fill) const{
    this->drawBox_iv(hadamard_product((box.getBox()+m_offset), m_scale_factor),color_stroke, color_fill);
}
