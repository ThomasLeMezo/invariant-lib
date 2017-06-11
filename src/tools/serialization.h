/* ============================================================================
 *  tubex-lib - Serialization tools
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU General Public License (GPL). See the file LICENSE.
 *
 *  Author(s) : Simon Rohou, Thomas Le Mézo
 *  Bug fixes : -
 *  Created   : 2017
 * ---------------------------------------------------------------------------- */

#ifndef SERIALIZATION_HEADER
#define SERIALIZATION_HEADER

#include <fstream>
#include "ibex_Interval.h"
#include "ibex_IntervalVector.h"

namespace ibex_tools
{
  /**
   * \brief Write an Interval object into a binary file.
   * 
   * Interval binary structure
   *   format: [char_intv_type][double_lb][double_ub]
   *   char_intv_type refers the type of Interval:
   *   either BOUNDED, EMPTY_SET, ALL_REALS, POS_REALS, NEG_REALS.
   *   In case of unbounded intervals, the two last fields disappear.
   *
   * \param binFile binary file (ofstream object)
   * \param intv Interval object to be serialized
   */
  void serializeInterval(std::ofstream& binFile, const ibex::Interval& intv);

  /**
   * \brief Write an IntervalVector object into a binary file.
   *
   * Interval binary structure
   *   format: [int_dim][interval_0]...[interval_n]
   *
   * \param binFile binary file (ofstream object)
   * \param intv IntervalVector object to be serialized
   */
  void serializeIntervalVector(std::ofstream& binFile, const ibex::IntervalVector& intv);

  /**
   * \brief Create an Interval object from a binary file.
   *
   * The binary file has to be written by the serializeInterval() function.
   *
   * \param binFile binary file (ifstream object)
   * \param intv Interval object to be deserialized
   */
  void deserializeInterval(std::ifstream& binFile, ibex::Interval& intv);

  /**
   * \brief Create an IntervalVector object from a binary file.
   *
   * The binary file has to be written by the serializeIntervalVector() function.
   *
   * \param binFile binary file (ifstream object)
   * \return intv IntervalVector object to be deserialized
   */
  const ibex::IntervalVector deserializeIntervalVector(std::ifstream& binFile);
}
#endif
