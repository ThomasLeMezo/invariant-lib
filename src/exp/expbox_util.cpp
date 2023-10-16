#include <vector>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <cmath>
#include <ibex.h>
#include "expbox_util.h"

using namespace ibex;

namespace invariant {

static int maxsize=0;
static bool debug_simplify=false;

static inline void annule_ligne(const IntervalVector &rowPiv, IntervalVector &row, int col) {
    row -= row[col]*rowPiv;
    row[col]=0.0;
}
static inline void annule_ligne_ub_lb(const IntervalVector &rowPiv,
     IntervalVector &row, int col, bool ub) {
    double val = (ub ? row[col].ub() : row[col].lb());
    row -= val*rowPiv;
    row[col] &= (ub ? Interval::neg_reals() : Interval::pos_reals());
}
static inline void unit_ligne(IntervalVector &row, const Interval &piv, int col) {
    for (int i=0;i<row.size();i++) row[i]/=piv;
    row[col]=1.0;
}

CstVect::CstVect(int bdim, double vdim, const Vector &vect)  :
     bdim(bdim), vdim(vdim), vect(vect) {
}

CstVect traduit_vect(const IntervalVector &box, const Vector &v,
		Interval &bounds) {
     CstVect ret(-1,0.0,v);
     for (int i=0;i<box.size();i++) {
         if (box[i].is_degenerated()) {
            bounds -= ret.vect[i]*box[i];
            ret.vect[i]=0.0;
         }
     }
     for (int i=0;i<ret.vect.size();i++) {
        double vl = fabs(ret.vect[i]);
        if (vl>ret.vdim) { ret.bdim=i; ret.vdim=vl; }
     }
     double sgn=1.0;
     if (ret.bdim>=0) {
        if (ret.vect[ret.bdim]<0.0) sgn=-1.0;
        double mant; int exp;
        mant= frexp(ret.vect[ret.bdim],&exp);
        for (int i=0;i<ret.vect.size();i++) 
            ret.vect[i] = sgn*ldexp(ret.vect[i],1-exp);
        bounds = Interval(ldexp(bounds.lb(),1-exp),ldexp(bounds.ub(),1-exp));
        bounds *=sgn;
     }
     return ret;
}

bool CstVectMap::and_constraint(const IntervalVector &box,
                          const Vector &v, const Interval &bds) {
     Interval bounds(bds);
     CstVect cv = traduit_vect(box,v,bounds);
     return this->and_constraint(box,cv,bounds);
} 

static constexpr double threshold=0.01;

bool CstVectMap::and_constraint(const IntervalVector &box,
                          const CstVect &cv, const Interval &bounds) {
     CstVectMap::iterator itlw = this->lower_bound(cv);
     if (itlw!=this->end()) {
        if (itlw->first==cv) {
          if (itlw->second.is_subset(bounds)) return false;
          itlw->second &= bounds;
          return true;
        } else if (itlw->first.bdim==cv.bdim) {
          const Vector &v2 = itlw->first.vect;
          double gap = 0.0;
          for (int i=0;i<box.size();i++) {
             gap += fabs(v2[i]-cv.vect[i]);
             if (gap>threshold) break; /* FIXME : give a threshold */
          }
          if (gap<=threshold) {
             Interval u = (bounds + (v2-cv.vect)*box);
             if (itlw->second.is_subset(u)) return false;
             itlw->second &= u;
             return true;
          }
        } 
    }
    if (itlw!=this->begin()) {
        itlw--;
        if (itlw->first.bdim==cv.bdim) {
          const Vector &v2 = itlw->first.vect;
          double gap = 0.0;
          for (int i=0;i<box.size();i++) {
             gap += fabs(v2[i]-cv.vect[i]);
             if (gap>threshold) break; /* FIXME : give a threshold */
          } 
          if (gap<=threshold) {
             Interval u = (bounds + (v2-cv.vect)*box);
             if (itlw->second.is_subset(u)) return false;
             itlw->second &= u;
             return true;
          }
        }
    }
    this->insert(itlw,std::make_pair(cv,bounds));
    return true;
} 


/** utility function simplex_mat :
    apply the dual simplex on a matrix :
      the matrix used for the simplex :
        line 0->dim-1 : rows (basis)
        line dim : objective 
        column 0->dim-1 : start with ivbox
        column dim->dim+sz-1 constraints 
        column dim+sz : lambdas
          [dim][dim+sz] : -objectif
     Eg the matrix
        1     2     -1      0  |  -1
        0     1     -1      1  |   1
     ---------------------------------
      [0,3] [-1,1] [1,3] [-1,0]   -5
     is interpreted with lambda>0 :
      -1 1 |-2 2 |1 -1 | 0  0 |   1
      0  0 |1 -1 |-1 1 | 1 -1 |   1
     ---------------------------------
      3  0 |1  1 |3 -1 | 0  1 |  -5
le pas de calcul donne :
      -1 1 |-1 1 | 0 0 | 1 -1 |   2
      0  0 |1 -1 |-1 1 | 1 -1 |   1
     ---------------------------------
      3  0 |2  0 | 2 0 | 1  0 |  -4
soit 
        1     1      0     -1  |  -2
        0    -1      1     -1  |  -1
     ---------------------------------
      [0,3] [0,2] [0,2]  [0,1]    -4
**/     

static bool simplex_mat(int dim, int sz, IntervalMatrix &mat, 
			std::vector<int> &basis, int maxit) {
   int nb_iter=0;
   while (nb_iter<maxit) {
       /* looking for new variable in the basis */
       /* we try to find the best coefficient ...
          if not possible, get the first... */
       int bcol=0, brow=-1;
       double bgap=0.0;
       for (int col=0;col<dim+sz;col++) {
           if (-mat[dim][col].ub()>0.0) { /* possibilité de faire rentrer
                                             la colonne en + */
              int tmpbrow=-1; 
             /* pour avoir une possibilité, il faut
                la base actuelle est positive 
                et le coef est >positif, ou l'inverse...
                donc on passe à row++ si le coef <=0 et la base est positive
                ou si le coef est >=0 et la base est négative */
             double bquot=0.0;
             int bbasis=dim;
             for (int row=0;row<dim;row++) {
               if ((basis[row]%2==1 || mat[row][col].lb()<=0) &&
                  (basis[row]%2==0 || mat[row][col].ub()>=0)) continue;
               Interval quot=mat[row][dim+sz]/mat[row][col];
               assert(quot.lb()>=0);
               if (tmpbrow==-1 ||
	  	    quot.lb()<bquot  || 
	              (quot.lb()==bquot && basis[row]<bbasis)) {
                  tmpbrow=row;
	          bbasis=basis[row];
                  bquot=quot.lb();
               } 
             }
             if (tmpbrow==-1) /* infeasible system */ {
//                    std::cout << "infeasible " << mat << "\n";
                    return false;  
             }
             double gap = -mat[dim][col].ub()*bquot;
             assert(gap>=0);
             if (bcol==0 || gap>bgap) {
                 bcol=col+1; bgap=gap; brow=tmpbrow;
             }
           } else if (mat[dim][col].lb()>0.0) {
             int tmpbrow=-1; 
             double bquot=0.0;
             int bbasis=dim;
             for (int row=0;row<dim;row++) {
                if ((basis[row]%2==0 || mat[row][col].lb()<=0) &&
                   (basis[row]%2==1 || mat[row][col].ub()>=0)) continue;
                Interval quot=mat[row][dim+sz]/mat[row][col];
                assert(quot.ub()<=0);
                if (tmpbrow==-1 ||
	   	    quot.ub()>bquot  || 
		     (quot.ub()==bquot && basis[row]<bbasis)) {
                   tmpbrow=row;
	           bbasis=basis[row];
                   bquot=quot.ub();
                } 
             }
             if (tmpbrow==-1) /* infeasible system */ {
//                    std::cout << "infeasible " << mat << "\n";
                    return false;  
             }
             double gap = -mat[dim][col].lb()*bquot;
             assert(gap>=0);
             if (bcol==0 || gap>bgap) {
                 bcol=-col-1; bgap=gap; brow=tmpbrow;
             }
           }
       }
       if (bcol==0) break;
       if (bcol<0) { /* on entre lambda en négatif */
          bcol=-bcol-1;
          /* division de la ligne */
          basis[brow]=2*bcol+1;
          Interval valpivot=mat[brow][bcol];
          /* on calcul la nouvelle ligne */
          unit_ligne(mat[brow],valpivot,bcol);
          /* puis on soustrait sur les autres lignes */
          for (int row=0;row<dim;row++) {
              if (row==brow) continue;
              annule_ligne(mat[brow],mat[row],bcol);
          }
          /* objectif : annuler la lower bound */
          annule_ligne_ub_lb(mat[brow],mat[dim],bcol,false);
       } else { /* lambda en positif */
          bcol=bcol-1;
          /* division de la ligne */
          basis[brow]=2*bcol;
          Interval valpivot=mat[brow][bcol];
          /* on calcul la nouvelle ligne */
          unit_ligne(mat[brow],valpivot,bcol);
          /* puis on annule les autres */
          for (int row=0;row<dim;row++) {
              if (row==brow) continue;
              annule_ligne(mat[brow],mat[row],bcol);
          }
          /* objectif : annuler la upper bound */
          annule_ligne_ub_lb(mat[brow],mat[dim],bcol,true);
       }
       /* correction partie droite : les lambdas doivent rester de même signe,
          on utilise lambda'= lambda+delta et on corrige obj en conséquence */
       for (int row=0;row<dim;row++) {
              if (basis[row]%2==0 && mat[row][dim+sz].lb()<0) {
                  double correc=mat[row][dim+sz].lb();
                  mat[row][dim+sz]-=correc;
                  mat[dim][dim+sz]-=
			correc*mat[dim][basis[row]/2].ub();
              } else if (basis[row]%2==1 && mat[row][dim+sz].ub()>0) {
                  double correc=mat[row][dim+sz].ub();
                  mat[row][dim+sz]-=correc;
                  mat[dim][dim+sz]-=
			correc*mat[dim][(basis[row]-1)/2].lb();
              }
       }
//       std::cout << mat << "\n\n";
       nb_iter++;
   }
//   if (nb_iter==maxit) std::cout << "nbiter=maxit " << nb_iter << "\n";
   return true;
}


/** get upper bound of a linear form on a polyhedron defined with intervals
   (simplex algorithm). The upper bound is guaranteed to be safe (>= the exact
   bound), but not optimal 
   @param dim dimension of the space
   @param ivbox IntervalVector bounding box (used as a "first" space)
   @param csts std::vector<std::pair<IntervalVector,Interval>> the constraints
   @param obj Vector vector of the linear form (objective) 
   @param maxit maximum number of iterations 
   @return result (empty si infaisable)
**/
Interval simplex_form(int dim, const IntervalVector &ivbox,
     const CstVectMap &csts,
     const Vector &obj, int maxit=10) {
   int sz = csts.size(); /* number of constraints, beside the ivbox */
   if (sz==0) return ivbox*obj;
   IntervalMatrix mat(dim+1,dim+sz+1,0.0);
     /* the matrix used for the simplex :
        line 0->dim-1 : rows (basis)
        line dim : objective 
        column 0->dim-1 : start with ivbox
        column dim->dim+sz-1 constraints 
        column dim+sz : lambdas
          [dim][dim+sz] : -objectif
     */
   std::vector<int> basis(dim); /* the basis */
   int col=dim;
   for (const std::pair<const CstVect,Interval> &cst : csts) {
//       std::cout << "contr " << cst.first << " " << cst.second << "\n";
       mat[dim][col]=cst.second;
       for (int i=0;i<dim;i++) {
           mat[i][col]=cst.first.vect[i];
       }    
       col++;
   }
   for (int row=0;row<dim;row++) {
       mat[row][row]=1.0;
       mat[dim][row]=ivbox[row];
   }
   IntervalMatrix mat2(mat);
   for (int row=0;row<dim;row++) {
       mat2[row][dim+sz]=obj[row];
       if (obj[row]>=0) { /* =0 : on choisit arbitrairement la val positive */
          mat2[dim] -= mat2[dim][row].ub()*mat2[row];
          basis[row]=2*row;
       } else {
          mat2[dim] -= mat2[dim][row].lb()*mat2[row];
          basis[row]=2*row+1;
       }
   }
   if (!simplex_mat(dim, sz, mat2, basis, maxit)) { return Interval::empty_set(); }
   /* retour de  la valeur finale */
   double upper = -mat2[dim][dim+sz].lb();
   for (int row=0;row<dim;row++) {
       mat[row][dim+sz]=-obj[row];
       if (obj[row]<0) { /* =0 : on choisit arbitrairement la val positive */
          mat[dim] -= mat[dim][row].ub()*mat[row];
          basis[row]=2*row;
       } else {
          mat[dim] -= mat[dim][row].lb()*mat[row];
          basis[row]=2*row+1;
       }
   }
   if (!simplex_mat(dim, sz, mat, basis, maxit)) { return Interval::empty_set(); }
   double lower = mat[dim][dim+sz].lb();
   return Interval(lower, upper);
}

/* prepare a matrix for simplex */
static void prepare_mat(int dim, int sz, IntervalMatrix &mat, std::vector<int> &basis,
     const Vector &obj) {
   for (int row=0;row<dim;row++) {
       mat[row][dim+sz]=obj[row];
       if (obj[row]>=0) { /* =0 : on choisit arbitrairement la val positive */
          mat[dim] -= mat[dim][row].ub()*mat[row];
          basis[row]=2*row;
       } else {
          mat[dim] -= mat[dim][row].lb()*mat[row];
          basis[row]=2*row+1;
       }
   }
}

/** simplify a "polyhedron" defined as an IVbox + vector of (possibly 
    "fuzzy" linear forms + interval... 
    each vector is centered and kept if useful 
    returns -1 if the polyhedron is empty */
int simplify_polyhedron(int dim, IntervalVector &ivbox,
     const std::vector<std::pair<IntervalVector,Interval>> &csts,
     CstVectMap &csts_rs,
     int maxit=10) {
   
   csts_rs.clear();
   std::vector<std::pair<Vector,Interval>> csts_res;
   int sz = csts.size(); /* number of constraints, beside the ivbox */
   IntervalMatrix mat(dim+1,dim+sz+1,0.0); /* the "initial" matrix,
                                               reused several times */
   std::vector<int> basis(dim); /* the basis */
   int col=dim;
   for (const std::pair<IntervalVector,Interval> &cst : csts) {
       mat[dim][col]=cst.second;
       for (int i=0;i<dim;i++) {
           mat[i][col]=cst.first[i];
       }    
       col++;
   }
   for (int row=0;row<dim;row++) {
       mat[row][row]=1.0;
       mat[dim][row]=ivbox[row];
   }
   Vector obj(dim);
   double upper,lower;
   int nb_non_flat=0;
   /* 1st, the ivbox */
   for (int i=0;i<dim;i++) {
       obj=Vector::zeros(dim);  
       obj[i]=1;
       IntervalMatrix mat2(mat);
       Interval &result = mat2[dim][dim+sz];
       prepare_mat(dim,sz,mat2,basis,obj);
//       std::cout << "base " << i << " mat2: " << mat2 << "\n";
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
             { ivbox.set_empty(); return -1; }
       upper=-result.lb();
       mat2=mat;
       obj[i]=-1;
//       std::cout << "base " << -i << " mat2: " << mat2 << "\n";
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
             { ivbox.set_empty(); return -1; }
       lower=result.lb();
//       std::cout << "lower : " << lower << "upper : " << upper << "\n";
       ivbox[i] &= Interval(lower,upper);
//       std::cout << "nouveau ivbox[i] " << ivbox[i] << "\n";
       if (!ivbox[i].is_degenerated()) nb_non_flat++;
       mat[dim][i]=ivbox[i];
   }
   /* we use the ivbox to "center" the other constraints */
   col=dim;
   if (nb_non_flat<=1) { /* mono-dimensionnel : pas de contrainte à garder */
     return nb_non_flat; 
   }
   for (const std::pair<IntervalVector,Interval> &cst : csts) {
       IntervalVector rad = cst.first - cst.first.mid();
       Vector v = cst.first.mid();
       mat[dim][col]=cst.second+rad*ivbox;
       /* variables relative to a flat dimension are removed */
       for (int i=0;i<dim;i++) {
           if (ivbox[i].is_degenerated()) {
              mat[dim][col]-=ivbox[i]*v[i];
              v[i]=0.0;
           }
           mat[i][col]=v[i];
       }    
       csts_res.push_back(std::pair<Vector,Interval>(v,mat[dim][col]));
       col++;
   }
   /* now we keep (or not) the different constraints */
   std::vector<std::pair<Vector,Interval>>::iterator csts_it = 
           csts_res.begin();
   for (int i=dim;i<dim+sz;i++) {
       obj = (*csts_it).first;
       Interval act=(*csts_it).second;
       IntervalMatrix mat2(mat);
       Interval &result = mat2[dim][dim+sz];
       /* on supprime la colonne i */
       for (int j=0;j<=dim;j++) mat2[j][i]=0.0;
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
            { ivbox.set_empty(); csts_res.clear(); return -1; }
       upper = -result.lb();
       mat2=mat;
       for (int j=0;j<=dim;j++) mat2[j][i]=0.0;
       obj=-obj;
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
            { ivbox.set_empty(); csts_res.clear(); return -1; }
       lower = result.ub();
       Interval res(lower, upper);
       if (res.is_subset(act)) { /* the constraint is useless */
          (*csts_it).second.set_empty();
          for (int j=0;j<=dim;j++) mat[j][i]=0.0;
          csts_it++;
          continue;
       }
       (*csts_it).second &= res;
       mat[dim][i]&= res;
       csts_it++;  
     }
     for (int i=0;i<csts_res.size();i++) {
        if (!csts_res[i].second.is_empty()) {
          csts_rs.and_constraint(ivbox,csts_res[i].first,csts_res[i].second);
        }
     }
     return nb_non_flat;
}

/* simplify the polyhedron, returns the number of non_flat variables, or -1
   for empty box */
int simplify_polyhedron(int dim, IntervalVector &ivbox,
     CstVectMap &csts,
     int maxit=10) {
   int sz = csts.size(); /* number of constraints, beside the ivbox */
   IntervalMatrix mat(dim+1,dim+sz+1,0.0); /* the "initial" matrix,
                                               reused several times */
   std::vector<int> basis(dim); /* the basis */
   int col=dim;
   for (const std::pair<CstVect,Interval> &cst : csts) {
       mat[dim][col]=cst.second;
       for (int i=0;i<dim;i++) {
           mat[i][col]=cst.first.vect[i];
       }    
       col++;
   }
   for (int row=0;row<dim;row++) {
       mat[row][row]=1.0;
       mat[dim][row]=ivbox[row];
   }
   Vector obj(dim);
   double upper,lower;
   /* 1st, the ivbox */
   for (int i=0;i<dim;i++) {
       obj=Vector::zeros(dim);  
       obj[i]=1;
       IntervalMatrix mat2(mat);
       Interval &result = mat2[dim][dim+sz];
       prepare_mat(dim,sz,mat2,basis,obj);
//       std::cout << "base " << i << " mat2: " << mat2 << "\n";
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
             { ivbox.set_empty(); csts.clear(); return (-1); }
       upper=-result.lb();
       mat2=mat;
       obj[i]=-1;
//       std::cout << "base " << -i << " mat2: " << mat2 << "\n";
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
             { ivbox.set_empty(); csts.clear(); return (-1); }
       lower=result.lb();
//       std::cout << "lower : " << lower << "upper : " << upper << "\n";
       ivbox[i] &= Interval(lower,upper);
//       std::cout << "nouveau ivbox[i] " << ivbox[i] << "\n";
       mat[dim][i]=ivbox[i];
   }
   /* we cannot remove "new" flat variables from the constraints,
      but it may not be needed */
   int nb_non_flat=0;
   for (int i=0;i<dim;i++) {
      if (!ivbox[i].is_degenerated()) nb_non_flat++;
   }
   if (nb_non_flat<=1) { csts.clear(); return nb_non_flat; }
   /* now we minimize and keep (or not) the different constraints */
   CstVectMap::iterator csts_it = csts.begin();
   for (int i=dim;i<dim+sz;i++) {
       obj = (*csts_it).first.vect;
#if 0
       if (debug_simplify) {
          std::cout << std::setprecision(std::numeric_limits<double>::digits10+2);
          std::cout << "constraint " << (*csts_it).first << ":" << (*csts_it).second << "\n";
       }
#endif
       Interval &act=(*csts_it).second;
       IntervalMatrix mat2(mat);
       Interval &result = mat2[dim][dim+sz];
       /* on supprime la colonne i */
       for (int j=0;j<=dim;j++) mat2[j][i]=0.0;
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
            { ivbox.set_empty(); csts.clear(); return (-1); }
#if 0
       if (debug_simplify) {
          std::cout << "simplex sup " << mat2 << "\n";
       }
#endif
       upper = -result.lb();
       mat2=mat;
       for (int j=0;j<=dim;j++) mat2[j][i]=0.0;
       obj=-obj;
       prepare_mat(dim,sz,mat2,basis,obj);
       if (!simplex_mat(dim,sz,mat2,basis,maxit))
            { ivbox.set_empty(); csts.clear(); return (-1); }
#if 0
       if (debug_simplify) {
           std::cout << "simplex inf " << mat2 << "\n";
       }
#endif
       lower = result.ub();
       Interval res(lower, upper);
#if 0
       if (debug_simplify) {
          std::cout << std::setprecision(std::numeric_limits<double>::digits10+1);
          std::cout << "bilan " << act << " " << (*csts_it).second << " " << res << " " << res.is_subset(act) << "\n";
       }
#endif
       if (res.is_subset(act)) { /* the constraint is useless */
          for (int j=0;j<=dim;j++) mat[j][i]=0.0;
          csts_it = csts.erase(csts_it);
          continue;
       }
       act &= res;
       mat[dim][i]&= res;
       csts_it++;  
     }
#if 0
       if (debug_simplify) {
            std::cout << "size result : " << csts.size() << "\n";
       }
#endif
     return nb_non_flat;
}

ExpPoly::ExpPoly(int dim, bool empty) :
  dim(dim), dim_not_flat(empty ? -1 : dim), 
  Box(dim, (empty ? Interval::empty_set() : Interval::all_reals())),
  minimized(true), csts()
{  }

ExpPoly::ExpPoly(const IntervalVector &Box) : 
  dim(Box.size()), dim_not_flat(Box.is_empty() ? -1 : 0),
  minimized(true), Box(Box), csts() {
  if (dim_not_flat==0) 
    for (int i=0;i<dim;i++) 
         if (!this->Box[i].is_degenerated()) dim_not_flat++;
}

ExpPoly::ExpPoly(const IntervalVector &Box, const std::vector<std::pair<IntervalVector,Interval>> &Csts, bool minimized) :
     ExpPoly(Box) 
{
   if (!minimized) {
      dim_not_flat = simplify_polyhedron(dim,this->Box,Csts,this->csts);
   } else {
      for (const std::pair<IntervalVector,Interval> &ct : Csts) {
         this->csts.and_constraint(Box,ct.first.mid(),ct.second);
      }
   }
}

ExpPoly::ExpPoly (const ExpPoly &P) :
     dim(P.dim), dim_not_flat(P.dim_not_flat),
     Box(P.Box), csts(P.csts), minimized(P.minimized) {
}


bool ExpPoly::is_subset (const ExpPoly &Q) const {
    /* A inclus dans Q si toutes les contraintes de Q sont inutiles */
    /* peut-être pas correct si (*this) n'est pas minimisé */
    if (!this->Box.is_subset(Q.Box)) return false;
    for (const std::pair<CstVect,Interval> &ct : Q.csts) {
        const CstVect &C = ct.first;
        const Interval &Ret= ct.second; 
        /* check if already there */
        const CstVectMap::const_iterator el = this->csts.find(C);
        if (el!=this->csts.end()) {
            Interval res = (*el).second;
            if (el->second.is_subset(Ret)) continue;
        }
        Interval result = simplex_form(dim,this->Box,this->csts,C.vect);
        if (!result.is_subset(Ret)) return false;
    }
    return true;
}

bool ExpPoly::is_subset (const IntervalVector &IV) const {
    return (this->Box.is_subset(IV));
}


bool operator==(const ExpPoly &C1, const ExpPoly &C2) {
    if (C1.Box != C2.Box) return false;
    return (C1.is_subset(C2) && C2.is_subset(C1));
}

bool ExpPoly::is_superset (const IntervalVector &IV) const {
    /* A inclus dans Q si toutes les contraintes de Q sont inutiles */
    /* peut-être pas correct si (*this) n'est pas minimisé */
    if (!IV.is_subset(this->Box)) return false;
    for (const std::pair<CstVect,Interval> &ct : this->csts) {
        const CstVect &C = ct.first;
        const Interval &Ret= ct.second; 
        if (!(C.vect*IV).is_subset(Ret)) return false;
    }
    return true;
}


void ExpPoly::compute_dim_not_flat() {
    if (!this->Box.is_empty()) {
      this->dim_not_flat=0;
      for (int i=0;i<dim;i++) {
         if (!this->Box[i].is_degenerated()) this->dim_not_flat++;
      }
    } else this->dim_not_flat=-1;
}

void ExpPoly::minimize() {
    if (minimized) return;
    if (this->Box.is_empty()) { this->set_empty(); return; }
    if (this->csts.size()==0) { minimized=true; return; }
    dim_not_flat = simplify_polyhedron(dim,this->Box,this->csts);
}

ExpPoly &ExpPoly::operator&=(const IntervalVector &iv) {
    if (this->Box.is_subset(iv)) return (*this);
    this->Box &= iv;
    if (this->Box.is_empty()) { this->dim_not_flat=-1; this->csts.clear();
				 return (*this); }
    /* recompute dim_not_flat */
    this->compute_dim_not_flat();
    if (dim_not_flat<=1 && this->csts.size()>0) { minimized=false; this->minimize(); return (*this); }
    if (this->csts.size()>0) minimized=false;
    return (*this);
}


ExpPoly &ExpPoly::operator&=(const ExpPoly &Q) {
    if (this->Box.is_unbounded()) {
       (*this)=Q;
       return (*this);
    } 
    this->Box &= Q.Box;
    if (this->Box.is_empty()) { this->dim_not_flat=0; this->csts.clear();
				 return (*this); }
    /* recompute dim_not_flat */
    this->compute_dim_not_flat();
    if (this->csts.size()+Q.csts.size()==0) return (*this);
    for (auto &q : Q.csts) { this->csts.and_constraint(this->Box,q.first, q.second); }
    if (dim_not_flat<=1 && this->csts.size()>0) { 
              this->minimized=false; this->minimize(); return (*this); 
    }
    if (this->csts.size()>0) minimized=false;
    return (*this);
}
ExpPoly &ExpPoly::operator&=(ExpPoly &&Q) {
    if (this->Box.is_unbounded()) {
       (*this)=Q;
       return (*this);
    } 
    this->Box &= Q.Box;
    if (this->Box.is_empty()) { this->dim_not_flat=0; this->csts.clear();
				 return (*this); }
    /* recompute dim_not_flat */
    this->compute_dim_not_flat();
    if (this->csts.size()+Q.csts.size()==0) return (*this);
    for (auto &q : Q.csts) { this->csts.and_constraint(this->Box,q.first,q.second); }
    if (dim_not_flat<=1 && this->csts.size()>0) { 
              this->minimized=false; this->minimize(); return (*this); 
    }
    if (this->csts.size()>0) minimized=false;
    return (*this);
}


ExpPoly & ExpPoly::operator&=
	(const std::vector<std::pair<IntervalVector, Interval>> &Res) {
//    this->csts.reserve(this->csts.size()+Res.size());
    bool modified=false;
    for (auto &q : Res) { 
             Vector V(q.first.mid());
             if (this->csts.and_constraint(this->Box, V,
				 q.second+(q.first-V)*Box)) modified=true;
    }
    if (!modified) return (*this);
    this->minimized=false;
//    std::cout << "operator& avant minimize " << (*this) << "\n";
    this->minimize();
//    std::cout << "operator& " << (*this) << "\n";
    return (*this);
}

void ExpPoly::intersect_paral(const IntervalMatrix &M, const IntervalVector &V) {
    bool modified=false;
    for (int i=0;i<dim;i++) {
           Vector Z(M[i].mid());
           if (this->csts.and_constraint(this->Box, Z,V[i]+(M[i]-Z)*Box)) modified=true;
    }
    if (!modified) return;
    this->minimized=false;
//    std::cout << "operator& avant minimize " << (*this) << "\n";
    this->minimize();
//    std::cout << "operator& " << (*this) << "\n";
}
 
ExpPoly &ExpPoly::operator|=(const IntervalVector &iv) {
    if (iv.is_empty()) return (*this);
    if (this->Box.is_subset(iv)) {
        this->csts.clear(); this->Box=iv;
        this->minimized=true;
        this->compute_dim_not_flat();
        return (*this);
    }
    this->Box |= iv;
    this->compute_dim_not_flat();
    bool modifie=false;
//    if (this->csts.size()==0) return (*this);
    CstVectMap::iterator ct_it = this->csts.begin();
    while (ct_it != this->csts.end()) {
      const CstVect& C = ct_it->first;
      Interval& Ret= ct_it->second;
      Interval resultB = C.vect*this->Box;
      if (Ret.is_subset(resultB)) {
         ct_it = this->csts.erase(ct_it);
         modifie=true;
         continue;
      } else if (!resultB.is_subset(Ret)) { 
         Ret |= resultB;
         modifie=true;
      }
      ct_it++;
    }
    if (modifie) {
       minimized=(this->csts.size()==0);
       if (!minimized) this->minimize();
    }   
    return (*this);
}

ExpPoly &ExpPoly::operator|=(ExpPoly &&Q) {
   if (Q.Box.is_empty()) return (*this);
    if (this->Box.is_empty()) {
       (*this) = Q;
       return (*this);
    }
    (*this)|=static_cast<const ExpPoly &>(Q);
    return (*this);
}

ExpPoly &ExpPoly::operator|=(const ExpPoly &Q) {
    if (Q.Box.is_empty()) return (*this);
    if (this->Box.is_empty()) {
       this->Box = Q.Box;
       this->csts = Q.csts;
       this->minimized=Q.minimized;
       this->dim_not_flat=Q.dim_not_flat;
       return (*this);
    }
    this->Box |= Q.Box;
    this->compute_dim_not_flat();
    if (this->csts.size()+Q.csts.size()==0) return (*this);
#if 0
    if (this->csts.size()+Q.csts.size()>maxsize) {
       maxsize = this->csts.size()+Q.csts.size();
       std::cout << "new size : " << maxsize << " : " << (*this) << "\n" << Q << "\n";
    debug_simplify=true;
    }
#endif
//    bool debug=false;
//    if (this->csts.size()+Q.csts.size()>10) {
//    std::cout << "operator| " << this->dim << " " << this->dim_not_flat << " " << this->Box << " " << this->csts.size() << " " << Q.csts.size() << "\n";
//    debug=true;
//    }
//    bool filtre=false;
//    if (this->csts.size()>4) filtre=true;
    CstVectMap nw;
    for (const std::pair<CstVect,Interval> &ct : this->csts) {
      const CstVect& C = ct.first;
      const Interval& Ret= ct.second;
      const CstVectMap::const_iterator el = Q.csts.find(C);
      if (el!=Q.csts.end()) {
         nw.and_constraint(this->Box, C,Ret | el->second); continue;
      }
      Interval result = simplex_form(dim,this->Box,Q.csts,C.vect);
      nw.and_constraint(this->Box,C,Ret | result);
    }
//    if (filtre) {
    for (const std::pair<CstVect,Interval> &ct : Q.csts) {
      const CstVect& C = ct.first;
      const Interval& Ret= ct.second;
      const CstVectMap::const_iterator el = this->csts.find(C);
      if (el!=this->csts.end()) continue;
//      std::cout << "cstQ " << ct.first << " " << ct.second << "\n";
      Interval result = simplex_form(dim,this->Box,this->csts,C.vect);
//     if (!Ret.is_subset(result)) {
       nw.and_constraint(this->Box, C,Ret | result);
//      } 
    }
//    }
    this->csts=nw;
#if 0
    if (debug_simplify) std::cout << "before min " << (*this) << "\n";
#endif
    if (this->csts.size()>0) minimized=false; /* pas minimisé mais presque ? */
    this->minimize();
#if 0
    debug_simplify=false;
#endif
//    if (this->csts.size()>200) {
//      std::cout << "trop gros, on arrête\n";
//      std::cout << (*this) << "\n";
//      assert(false);
//    }
//    if (debug) std::cout << "après minimize : " << this->csts.size() << "\n";
    return (*this);
}

/* constraint-based => hard to produce a good widening... */
ExpPoly &ExpPoly::widen(const ExpPoly &Q) {
//    std::cout << "widen " << (*this).csts.size() << " " << Q.csts.size() << "\n";
    if (Q.Box.is_empty()) return (*this);
    if (this->Box.is_empty()) {
       this->Box = Q.Box;
       this->csts = Q.csts;
       this->minimized=Q.minimized;
       this->dim_not_flat=Q.dim_not_flat;
       return (*this);
    }
    IntervalVector join = this->Box | Q.Box;
    for (int i=0;i<dim;i++) {
       double a = join[i].rad();
       if (join[i].lb()!=this->Box[i].lb()) join[i] |= (join[i].lb()-a);
       if (join[i].ub()!=this->Box[i].ub()) join[i] |= (join[i].ub()+a);
    }
    this->Box = join;
    this->compute_dim_not_flat();
    if (this->dim_not_flat<=1) {
       this->csts.clear();
       this->minimized=true;
       return (*this);
    }
    CstVectMap::iterator ct_it = this->csts.begin();
    while (ct_it != this->csts.end()) {
      const CstVect& C = ct_it->first;
      Interval& Ret= ct_it->second;
      Interval join = simplex_form(dim,this->Box,Q.csts,C.vect);
      join |= Ret;
      double a = join.rad();
      if (join.lb()!=Ret.lb()) join|=(join.lb()-a);
      if (join.ub()!=Ret.ub()) join|=(join.ub()+a);
      ct_it->second = join;
    }
    this->minimized=false;
    this->minimize();
    return (*this);
}

ExpPoly operator&(const ExpPoly &C1, const ExpPoly &C2) {
   ExpPoly A(C1);
   A &= C2;
   return A;
}
ExpPoly operator|(const ExpPoly &C1, const ExpPoly &C2) {
   ExpPoly A(C1);
   A |= C2;
   return A;
}

std::vector<std::pair<IntervalVector,Interval>> 
		ExpPoly::build_constraints_for_propag
			(const Vector &C, const Vector &Z1) const {
   std::vector<std::pair<IntervalVector,Interval>> result;
   int flat_dim=-1;
   for (int i=0;i<dim;i++) {
       if (Box[i].is_degenerated() && Z1[i]!=0.0) { flat_dim=i; break; }
   }
   if (flat_dim==-1)  {
        for (const std::pair<CstVect,Interval>&q: this->csts) {
           result.push_back(std::pair<IntervalVector,Interval>
		(IntervalVector(q.first.vect),q.second));
        }
        for (int i=0;i<dim;i++) {
           IntervalVector V(dim,0.0);
           V[i]=1.0;
           result.push_back(std::pair<IntervalVector,Interval>(V,Box[i]));
        }
	return result;
   }
   result.reserve(dim+this->csts.size()+1);
   Interval resform = simplex_form(dim,this->Box,this->csts,Z1);
   IntervalVector Dir(Z1);
   Interval speed = Dir.norm2();
   resform /= speed;
   Dir *= (1.0/speed);
   result.push_back(std::pair<IntervalVector,Interval>(Dir,resform));
   for (int i=0;i<dim;i++) {
           if (i==flat_dim) continue;
           IntervalVector V(dim,0.0);
           V[i]=Dir[flat_dim];
           V[flat_dim]=-Dir[i];
           resform = Box[i]*V[i]+V[flat_dim]*Box[flat_dim];
           result.push_back(std::pair<IntervalVector,Interval>(V,resform));
   }
   for (const std::pair<CstVect,Interval>&q: this->csts) {
       IntervalVector V(q.first.vect);
       Interval Coef = V*Dir;
       Coef /= Dir[flat_dim];
       V[flat_dim] -= Coef;
       resform = q.second-Coef*Box[flat_dim];
       result.push_back(std::pair<IntervalVector,Interval>(V,resform));
   }
   return result;
}

IntervalVector ExpPoly::build_constraints_for_propag
			(const IntervalMatrix &Z) const {
   IntervalVector result(dim,0.0);
   for (int i=0;i<dim;i++) {
         Interval resform = simplex_form(dim,this->Box,this->csts,Z[i].mid());
         resform += (Z[i] - Z[i].mid())*Box;
         result[i] = resform;
   }
   return result;
}


/* returns [C1 - (C1 \cap IV)] (plus ou moins). */
void diff_hull_box(ExpPoly &C1, const IntervalVector &IV) {
   IntervalVector IV2 = IV & C1.Box;
   if (IV2.is_empty()) return;
   ExpPoly res(C1.get_dim(), true);
   for (int i=0;i<C1.dim;i++) {
       if (IV[i].lb()>C1.Box[i].lb()) {
          ExpPoly A(C1);
          A.Box[i]=Interval(C1.Box[i].lb(),IV[i].lb());
          A.minimize();
          res|=A;
       }
       if (IV[i].ub()<C1.Box[i].ub()) {
          ExpPoly A(C1);
          A.Box[i]=Interval(IV[i].ub(),C1.Box[i].ub());
          A.minimize();
          res|=A;
       }
   }
   C1 &= res;
}

IntervalVector& operator&=(IntervalVector &V, const ExpPoly &C) {
   ExpPoly A(C);
   A &= V;
   V &= A.Box;
   return V;
}
IntervalVector& operator|=(IntervalVector &V, const ExpPoly &C) {
   V |= C.Box;
   return V;
}
IntervalVector operator+(const ExpPoly &C, const IntervalVector &V) {
   return C.Box + V;
}

std::ostream& operator<< (std::ostream &str, const ExpPoly& C) {
   str << "Poly(" << C.Box ;
   for (const std::pair<CstVect,Interval>&c : C.csts) {
       str << "; " << c.first.vect << ":" << c.second;
   }
   str << "). ";
   return str;
}


}

#if 0
using namespace invariant;

/* test de simplex_form */
int main() {
   IntervalVector ivbox(3);
   ivbox[0]=Interval(-4,-1); ivbox[1]=Interval(1,4); ivbox[2]=Interval(-6,-1);
   std::vector<std::pair<IntervalVector,Interval>> csts;
 
   IntervalVector cst(3); cst[0]=1; cst[1]=-1; cst[2]=-1.5;
   csts.push_back(std::pair<IntervalVector,Interval>(cst,Interval(-5.25,4.3)));
   cst[0]=-2; cst[1]=-1; cst[2]=0.5;
   csts.push_back(std::pair<IntervalVector,Interval>(cst,Interval(-5,2)));
   cst[0]=-1.5; cst[1]=Interval(1.4,1.6); cst[2]=Interval(0.8,0.8);
   csts.push_back(std::pair<IntervalVector,Interval>(cst,Interval(-1.5,4)));

   Vector obj(3);
   obj[0]=2; obj[1]=1; obj[2]=-0.5;
   
   double result;
   std::vector<std::pair<Vector,Interval>> cst_res;
   bool r = simplify_polyhedron(3,ivbox,csts,cst_res, 1000);
   std::cout << r << "\n"; 
   std::cout << ivbox << "\n"; 
   for (auto a : cst_res) {
     std::cout << a.first << " in " << a.second << "\n";
   }
   return 0;

}

#endif
