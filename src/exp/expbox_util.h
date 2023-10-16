#ifndef __EXPBOX_UTIL_H__
#define __EXPBOX_UTIL_H__

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <vector>
#include <map>
#include <list>
#include <cmath>
#include <ibex.h>

using namespace ibex;

namespace invariant {

class ExpPoly;
/* gestion des contraintes du polyèdre */
struct CstVect {
   int bdim; /* dim of greatest value (always between 0.5 and 1) */
   double vdim; /* its value */
   Vector vect;  /* the constraint */

   CstVect(int bdim, double vdim, const Vector &vect);
   friend bool operator==(const CstVect &lhs, const CstVect& rhs);
};

CstVect traduit_vect(const IntervalVector &box,
			const Vector &v, Interval &bounds);

inline bool operator==(const CstVect &lhs, const CstVect& rhs) {
   return (lhs.bdim==rhs.bdim && lhs.vdim==rhs.vdim &&
          lhs.vect==rhs.vect);
}
      

struct CstVectComp {
inline bool operator()(const CstVect& lhs, const CstVect& rhs) const
    {
        if (lhs.bdim<rhs.bdim) return true;
        else if (lhs.bdim>rhs.bdim) return false;
        if (lhs.vdim<rhs.vdim) return true;
        else if (lhs.vdim>rhs.vdim) return false;
        for (int i=0;i<lhs.vect.size();i++) 
           if (lhs.vect[i]<rhs.vect[i]) return true;
           else if (lhs.vect[i]>rhs.vect[i]) return false;
        return false;
    }
};

struct CstVectMap : std::map<CstVect,Interval,CstVectComp> {
//      CstVectMap();
//      CstVectMap(const CstVectMap &cvm);
      
      bool and_constraint(const IntervalVector &box,
			  const Vector &v, const Interval &bds);
#if 0
      bool and_constraint(const IntervalVector &box,
			  const Vector &v, Interval &&i);
#endif
      bool and_constraint(const IntervalVector &box,
			  const CstVect &v, const Interval &bounds);
#if 0
      bool and_constraint(const IntervalVector &box,
			  const CstVect &&v, Interval &&i);
      bool and_constraint(const IntervalVector &box,
			  CstVect &&v, const Interval &i);
      bool and_constraint(const IntervalVector &box,
			  CstVect &&v, Interval &&i);
#endif
      friend class ExpPoly;
};


/** representation of polyhedron with ivbox and interval constraints */
class ExpPoly
{
   public :
      /** build an empty or full polyhedron **/
      ExpPoly (int dim, bool empty);
      /* with a box */
      ExpPoly (const IntervalVector &Box);
      /* with a box and a set of (interval vector) constraints */
      ExpPoly (const IntervalVector &Box, const std::vector<std::pair<IntervalVector,Interval>> &Csts, bool minimized=false);
      /* clone */
      ExpPoly (const ExpPoly &P);
      
      int get_dim() const;
      int get_not_flat_dim() const; 
      const IntervalVector &getBox() const;

      /* comparaison */
      bool is_empty() const;
      bool is_flat() const;
      bool is_bounded() const;
      bool is_box() const;
      bool is_subset (const ExpPoly &Q) const;  /* true is guaranteed */
      bool is_subset (const IntervalVector &IV) const; /* true is guaranteed */
      bool is_superset (const IntervalVector &IV) const; /* true is guaranteed */

      /* internal modification */
      void minimize();     

      /* modification */
      void set_empty();
      ExpPoly &operator&=(const IntervalVector &iv);
      ExpPoly &operator|=(const IntervalVector &iv); /* non optimal union */
      ExpPoly &operator&=(const ExpPoly &Q);
      ExpPoly &operator|=(const ExpPoly &Q); /* non optimal union */
      ExpPoly &operator&=(ExpPoly &&Q);
      ExpPoly &operator|=(ExpPoly &&Q); /* non optimal union */
      ExpPoly &operator&=(const std::vector<std::pair<IntervalVector, Interval>> &Res);
      void intersect_paral(const IntervalMatrix &M, const IntervalVector &V);
      ExpPoly &widen(const ExpPoly &Q);      /* union + widening */
      friend ExpPoly operator&(const ExpPoly &C1, const ExpPoly &C2);
      friend ExpPoly operator|(const ExpPoly &C1, const ExpPoly &C2);
      friend void diff_hull_box(ExpPoly &B1, const IntervalVector &IV);
      friend bool operator==(const ExpPoly &C1, const ExpPoly &C2); 

      /* crée une description des contraintes centrée sur C,
           avec si possible la première colinéaire à Z1 et les autres
           orthogonales */
      std::vector<std::pair<IntervalVector,Interval>> 
	build_constraints_for_propag(const Vector &C, const Vector &Z1) const;
      /* compute the intervals of the domain */
      IntervalVector 
	build_constraints_for_propag(const IntervalMatrix &Z) const;


						/* true is guaranteed */
      friend IntervalVector& operator&=(IntervalVector &V, const ExpPoly &C);
      friend IntervalVector& operator|=(IntervalVector &V, const ExpPoly &C);
      friend IntervalVector operator+(const ExpPoly &C, const IntervalVector &V);
        
      friend std::ostream& operator<<(std::ostream& str, const ExpPoly& C);

  private :
      int dim; /* dimension of the space */
      int dim_not_flat;  /* number of non-flat dimension, empty=0 */

      IntervalVector Box; /* bounding box */
      bool minimized;    /* are csts minimized */
      
      CstVectMap csts;
      void compute_dim_not_flat();
      
      void add_cst(const Vector& V, const Interval& I);
      void add_cst(Vector&& V, const Interval& I);
      CstVect compute_key(const Vector& V);
      CstVect compute_key(Vector&& V);
};

inline int ExpPoly::get_dim() const { return this->dim; }
inline int ExpPoly::get_not_flat_dim() const { return this->dim_not_flat; }
inline const IntervalVector &ExpPoly::getBox() const { return this->Box; }
inline bool ExpPoly::is_empty() const { return this->Box.is_empty(); }
inline bool ExpPoly::is_flat() const { std::cout << "isflat " << this->dim_not_flat << " " << this->dim << "\n"; return this->dim_not_flat<this->dim; }
inline bool ExpPoly::is_bounded() const {return !this->Box.is_unbounded(); }
inline bool ExpPoly::is_box() const { return this->csts.size()==0; }
inline void ExpPoly::set_empty() { this->dim_not_flat=-1; 
	this->Box.set_empty(); this->csts.clear(); this->minimized=true; }
}

#endif
      
     
