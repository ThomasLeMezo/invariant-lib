/* ============================================================================
 *  tubex-lib - Serialization tools
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU General Public License (GPL). See the file LICENSE.
 *
 *  Author(s) : Simon Rohou, Thomas Le Mézo
 *  Bug fixes : -
 *  Created   : 2016
 * ---------------------------------------------------------------------------- */

#include "../serialization/ibex_serialization.h"

using namespace std;
using namespace ibex;

enum IntervalType { BOUNDED, EMPTY_SET, ALL_REALS, POS_REALS, NEG_REALS };

void serializeInterval(ofstream& binFile, const Interval& intv)
{
    char intv_type;

    if(intv == Interval::EMPTY_SET)
        intv_type = EMPTY_SET;

    else if(intv == Interval::ALL_REALS)
        intv_type = ALL_REALS;

    else if(intv == Interval::POS_REALS)
        intv_type = POS_REALS;

    else if(intv == Interval::NEG_REALS)
        intv_type = NEG_REALS;

    else
        intv_type = BOUNDED;

    binFile.write((const char*)&intv_type, sizeof(char));

    if(intv_type == BOUNDED)
    {
        double lb = intv.lb(), ub = intv.ub();
        binFile.write((const char*)&lb, sizeof(double));
        binFile.write((const char*)&ub, sizeof(double));
    }
}

void deserializeInterval(ifstream& binFile, Interval& intv)
{
    char intv_type;
    binFile.read((char*)&intv_type, sizeof(char));

    switch(intv_type)
    {
    case EMPTY_SET:
        intv = Interval::EMPTY_SET;
        break;

    case ALL_REALS:
        intv = Interval::ALL_REALS;
        break;

    case POS_REALS:
        intv = Interval::POS_REALS;
        break;

    case NEG_REALS:
        intv = Interval::NEG_REALS;
        break;

    case BOUNDED:
        double lb, ub;
        binFile.read((char*)&lb, sizeof(double));
        binFile.read((char*)&ub, sizeof(double));
        intv = Interval(lb, ub);
        break;

    default:
        cout << "Tube::deserializeInterval(...): unhandled case" << endl;
    }
}

void serializeIntervalVector(std::ofstream& binFile, const ibex::IntervalVector& intv)
{
    const int size = intv.size();
    binFile.write((const char*) &size, sizeof(int));
    for(int dim=0; dim<size; dim++){
        serializeInterval(binFile, intv[dim]);
    }
}

const IntervalVector deserializeIntervalVector(std::ifstream& binFile)
{
    int dim;
    binFile.read((char*)&dim, sizeof(int));
    IntervalVector itv(dim);
    const int dim_const = dim;
    for(int i=0; i<dim_const; i++)
        deserializeInterval(binFile, itv[i]);
    return itv;
}
