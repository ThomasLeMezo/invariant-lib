#ifndef __DIFF_EXPBOX_H__
#define __DIFF_EXPBOX_H__

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <vector>
#include <list>
#include <cmath>
#include <ibex.h>

using namespace ibex;

namespace invariant {

enum EXPBOX_T{EXPBOX_BOX, EXPBOX_ROOM, EXPBOX_DOOR};

/** representation of bax with several quadratic forms 
    from different vector fields, hence complex forms */
class ExpBox
{
   public :
     /* initialize with a box */
     ExpBox (const IntervalVector &Box);
     /* empty/full frame ; must be "set" later */
     ExpBox (int dim, bool empty);
     /* special initialize : DOOR ? */
     ExpBox (const IntervalVector &Box, EXPBOX_T tBox);
     /* special initialize : vector field */
     ExpBox (const IntervalVector &Box, const IntervalVector &VF, const std::pair<Matrix, IntervalVector>& pair);
     ExpBox (const IntervalVector &Box, const IntervalVector &VF);
     /* special initialize : negate vector */
     ExpBox (const ExpBox &EBox, double sens);
     /* clone */
     ExpBox (const ExpBox &EBox);
     
     const IntervalVector& getBox() const;
     bool is_empty() const;
     bool is_flat() const;
     bool is_subset(const ExpBox &Q) const; /* note: for doors */
     bool is_box() const; /* is there parallepiped constraints */
     
     int get_dim() const; 
     const IntervalMatrix &getZ()    const; /* constraint matrix */
     const IntervalMatrix &getZinv() const; /* constraint matrix */
     const IntervalVector &getV()    const; /* constraint values */
     const IntervalVector &getVF()   const; /* vector field */

     void set_empty();
     ExpBox &operator&=(const ExpBox &Q);
     ExpBox &operator|=(const ExpBox &Q);
     ExpBox &operator&=(const IntervalVector &iv);
     ExpBox &operator|=(const IntervalVector &iv);
     friend ExpBox operator&(const ExpBox &C1, const ExpBox &C2);
     friend ExpBox operator|(const ExpBox &C1, const ExpBox &C2);
     friend ExpBox diff_hull(const ExpBox &B1, const ExpBox &B2);
     friend bool operator==(const ExpBox &C1, const ExpBox &C2);
     friend IntervalVector& operator&=(IntervalVector &V, const ExpBox &C);
     friend IntervalVector& operator|=(IntervalVector &V, const ExpBox &C);
     friend IntervalVector operator+(const ExpBox &C, const IntervalVector &V);

     friend std::ostream& operator<<(std::ostream& str, const ExpBox& C);

     void contract_flow(const ExpBox &Start, ExpBox &End, const Interval& tim) const;
     void contract_res(const IntervalVector &iv, const IntervalMatrix &Z1, const IntervalMatrix &Z1inv, const IntervalVector &V1);

   private :
     unsigned int dim; /* dimension of the space */
     bool flat;        /* is the (initial) box flat (door/room) */
     EXPBOX_T tpe;     /* type of the expbox */

     IntervalVector Box; /* bounding box */

     bool paral;         /* added parallepiped constraints */
     Matrix A;           /* room : dynamics */
     IntervalMatrix Z;    /* vectors of the constraints */
                           /* room : orthogonal base on AC+B */
                           /* door : parallepiped base */
     IntervalMatrix Zinv; /* inverse of Z */

     IntervalVector V;     /* values */ 
                           /* door : x in ZV   (or Zinv x in V) */
                           /* room : dynamics : Xdot = A(X-C)+speed*Z_1+V */
     
     IntervalVector VF;    /* vector field (room only) */
     double speed;     /* norme  */


     
};

inline int ExpBox::get_dim() const { return this->dim; }
inline const IntervalVector &ExpBox::getBox() const { return this->Box; }
inline bool ExpBox::is_empty() const { return this->Box.is_empty(); }
inline bool ExpBox::is_flat() const { return this->Box.is_flat(); }
inline bool ExpBox::is_box() const { return !(this->paral); }
inline const IntervalMatrix &ExpBox::getZ()    const { return this->Z; }
inline const IntervalMatrix &ExpBox::getZinv() const { return this->Zinv; }
inline const IntervalVector &ExpBox::getV()    const { return this->V; }
inline const IntervalVector &ExpBox::getVF()    const { return this->VF; }


}

#endif

