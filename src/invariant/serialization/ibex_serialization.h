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

#ifndef IBEX_SERIALIZATION_HEADER
#define IBEX_SERIALIZATION_HEADER

#include <fstream>

#include <ibex_Interval.h>
#include <ibex_IntervalVector.h>

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

  /**
   * @brief serialize Vector
   * @param v
   * @param binFile
   */
  template<typename _Tp=short int>
  void serializeVector(const std::vector<_Tp> &v, std::ofstream &binFile);

  /**
   * @brief deserialize Vector
   * @param binFile
   * \return vector
   */
  template<typename _Tp=short int>
  std::vector<_Tp> deserializeVector(std::ifstream &binFile);

// Inline functions

  template<typename _Tp=short int>
  inline void serializeVector(const std::vector<_Tp> &v, std::ofstream &binFile){
      size_t size = v.size();
      binFile.write((const char*)&size, sizeof(size_t));
      for(size_t i=0; i<size; i++){
          binFile.write((const char*)&v[i], sizeof(_Tp));
      }
  }

  template<typename _Tp=short int>
  inline std::vector<_Tp> deserializeVector(std::ifstream &binFile){
      std::vector<_Tp> result;
      size_t size;
      binFile.read((char*)&size, sizeof(size_t));
      for(size_t i=0; i<size; i++){
          _Tp tmp;
          binFile.read((char*)&tmp, sizeof(_Tp));
          result.push_back(tmp);
      }
      return result;
  }

#endif
