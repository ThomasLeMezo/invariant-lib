
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <vector>
#include <cmath>
#include <ibex.h>
#include "diff_expbox.h"

using namespace ibex;

namespace invariant{

static IntervalMatrix exp_mat(const IntervalMatrix &M) {
    const int dim = M.nb_rows();
    IntervalMatrix Id = Matrix::eye(dim);
    IntervalMatrix res = Id;
    Interval n = 1.0/(1.0-0.25*infinite_norm(M));
    res.inflate(n.ub());
    res = Id+M*(Id+0.5*M*(Id+(1.0/3.0)*M*res));
    return res;
}
    
static IntervalMatrix exp_mat_int(const IntervalMatrix &M) {
    const int dim = M.nb_rows();
    IntervalMatrix Id = Matrix::eye(dim);
    IntervalMatrix res = Id;
    Interval n = 1.0/(1.0-0.25*infinite_norm(M));
    res.inflate(n.ub());
    res = Id+0.5*M*(Id+(1.0/3.0)*M*res);
    return res;
}
    
static Matrix exp_mat_abs(const IntervalMatrix &M, Interval tm) {
    const int dim = M.nb_rows();
    double v = infinite_norm(M);
    double tmax = tm.mag();
    Matrix res(M.nb_rows(),M.nb_rows(),1.0);
    double r= tmax*(1.0+0.5*tmax*v*(1.0+(1.0/3.0)*tmax*v*(1.0+1.0/(1.0-0.25*tmax*v))));
    return r*res;
}

/* generate a orthogonal basis from a vector V */
static IntervalMatrix matrix_orthogonal(const Vector &V) {
    int dim = V.size();
    IntervalMatrix ret(dim,dim,0.0);
    ret[dim-1] = V;
    Interval nrm=ret[dim-1].norm2();
    if (nrm.contains(0.0)) { /* V est nul */
       ret = Matrix::eye(dim);
       return ret;
    }
    ret[dim-1] *= (1.0/nrm);
    
    for (int i=dim-2;i>=0;i--) {
       if (ret[dim-1][i].mag()>=1.0) { /* V[i] = +ou- 1 */
          ret[i][dim-1]=1.0; /* this can only happen once */
          continue;
       }
       ret[i][i]=1.0;
       for (int j=dim-1;j>i;j--) {
           Interval prod = ret[i]*ret[j];
           ret[i] -= prod*ret[j];
       }
       ret[i] *= (1.0/ret[i].norm2());
    }
    return ret;
}



ExpBox::ExpBox(const IntervalVector &Box):
    dim(Box.size()), flat(Box.is_flat()),
    tpe(EXPBOX_BOX),
    Box(Box), 
    paral(false), A(dim,dim), Z(dim,dim),
    Zinv(dim,dim), V(dim), VF(dim), speed(0.0)
    {
}
ExpBox::ExpBox(int dim, bool empty):
    dim(dim), flat(empty),
    tpe(EXPBOX_BOX),
    Box(dim,Interval::empty_set()), 
    paral(false), A(dim,dim), Z(dim,dim),
    Zinv(dim,dim), V(dim), VF(dim), speed(0.0)
    {
      if (!empty) { Box.init(Interval::all_reals()); }
}
ExpBox::ExpBox(const IntervalVector &Box, EXPBOX_T tBox):
    dim(Box.size()), flat(Box.is_flat()),
    tpe(tBox),
    Box(Box), 
    paral(false), A(dim,dim), Z(dim,dim),
    Zinv(dim,dim), V(dim), VF(dim), speed(0.0)
    {
}
ExpBox::ExpBox(const IntervalVector &Box, const IntervalVector &VF, const std::pair<Matrix, IntervalVector>& pair) :
    dim(Box.size()), flat(Box.is_flat()),
    tpe(EXPBOX_ROOM),
    Box(Box), 
    paral(true), A(pair.first), Z(dim,dim),
    Zinv(dim,dim), V(pair.second), VF(VF), speed(0.0)
    {
      Vector direction = V.mid();
      this->speed = norm(direction);
      if (this->speed==0.0) { paral=false; return; }
      V = V-V.mid();
      Zinv = matrix_orthogonal(direction);
      Z = Zinv.transpose();
}
ExpBox::ExpBox(const IntervalVector &Box, const IntervalVector &VF) :
    dim(Box.size()), flat(Box.is_flat()),
    tpe(EXPBOX_ROOM),
    Box(Box), 
    paral(false), A(dim,dim), Z(dim,dim),
    Zinv(dim,dim), V(dim), VF(VF), speed(0.0)
    {
}
ExpBox::ExpBox(const ExpBox &EBox, double sens):
    dim(EBox.dim), flat(EBox.flat),
    tpe(EBox.tpe),
    Box(EBox.Box), 
    paral(EBox.paral), A(sens*EBox.A), Z(sens*EBox.Z),
    Zinv(sens*EBox.Zinv), V(sens*EBox.V), VF(sens*EBox.VF), speed(EBox.speed)
    {
}
ExpBox::ExpBox(const ExpBox &EBox):
    dim(EBox.dim), flat(EBox.flat),
    tpe(EBox.tpe),
    Box(EBox.Box), 
    paral(EBox.paral), A(EBox.A), Z(EBox.Z),
    Zinv(EBox.Zinv), V(EBox.V), VF(EBox.VF), speed(EBox.speed)
    {
}

void ExpBox::set_empty() {
    this->Box.set_empty();
    this->flat= true;
    this->paral=false;
}

bool ExpBox::is_subset(const ExpBox &Q) const {
     if (!this->Box.is_subset(Q.Box)) return false;
     if (Q.is_box() || (dim==2 && flat)) return true;
     IntervalMatrix ZinvQ = Q.getZinv();
     const IntervalVector &VQ = Q.getV();
     IntervalVector BoxTmp = this->Box;
     bwd_mul(VQ,ZinvQ,BoxTmp,0.01);
     if (!Box.is_subset(BoxTmp)) /* BoxTmp smaller => false */
          return false;
     if (!paral) return true;
     ZinvQ = ZinvQ*this->Z;
     BoxTmp = this->V;
     bwd_mul(VQ,ZinvQ,BoxTmp,0.01);
     if (!V.is_subset(BoxTmp)) /* BoxTmp smaller => false */
          return false;
     return true;
}

ExpBox& ExpBox::operator&=(const ExpBox &Q) {
    this->Box &= Q.Box;
    if (Q.is_flat()) this->flat=true;
    if (this->Box.is_empty()) { paral=false; flat=true; return (*this); }
    if (paral) {
       bool n_emp = bwd_mul(this->Box,this->Z,this->V,0.01);
       if (!n_emp) { paral=false; flat=true;
                     this->Box.set_empty(); return (*this); }
    }
    if (dim==2 && Q.is_flat()) return (*this);
    if (!Q.is_box()) {
      IntervalMatrix ZinvQ = Q.getZinv();
      const IntervalVector &VQ = Q.getV();
      bool n_emp = bwd_mul(VQ,ZinvQ,this->Box,0.01);
      if (!n_emp) { paral=false; flat=true; return (*this); }
      if (!paral) {
         paral = true;
         this->Zinv = Q.getZinv();
         this->Z = Q.getZ();
         this->V = VQ & (this->Zinv * this->Box);
         return (*this);
      }
      ZinvQ = ZinvQ*this->Z;
      n_emp = bwd_mul(VQ,ZinvQ,this->V,0.01);
      if (!n_emp) { paral=false; flat=true;
                    this->Box.set_empty(); return (*this); }
    }
    if (paral) {
      bool n_emp = bwd_mul(this->V,this->Zinv,this->Box,0.01);
      if (!n_emp) { paral=false; flat=true; return (*this); }
    }
    return (*this);
}

ExpBox& ExpBox::operator&=(const IntervalVector &iv) {
    this->Box &= iv;
    if (iv.is_flat()) this->flat=true;
    if (this->Box.is_empty()) { paral=false; flat=true; return (*this); }
    if (paral) {
       bool n_emp = bwd_mul(this->Box,this->Z,this->V,0.01);
       if (!n_emp) { paral=false; flat=true;
                     this->Box.set_empty(); return (*this); }
    }
    if (dim==2 && iv.is_flat()) return (*this);
    bool n_emp = bwd_mul(this->V,this->Zinv,this->Box,0.01);
    if (!n_emp) { paral=false; flat=true; return (*this); }
    return (*this);
}

ExpBox& ExpBox::operator|=(const ExpBox &Q) {
    if (Q.is_empty()) return (*this);
    if (this->is_empty()) {
       this->Box = Q.Box;
       this->flat = Q.flat;
       if (!Q.paral) { paral=false; return (*this); }
       paral=true;
       this->Z = Q.Z; this->Zinv = Q.Zinv; this->V = Q.V;
       return (*this);
    }
    if (!paral && !Q.is_box()) {
       paral=true;
       this->Z = Q.Z; this->Zinv = Q.Zinv; this->V= Q.V;
       this->V |= this->Zinv * this->Box;
       this->Box |= Q.Box;
       if (!this->Box.is_flat()) flat=false;
       return (*this);
    }
    this->Box |= Q.Box;
    if (!this->Box.is_flat()) flat=false;
    if (Q.is_box()) {
       if (paral) {
          this->V |= this->Zinv * Q.Box;
       }
       return (*this);
    }
    IntervalMatrix ZinvZQ = this->Zinv * Q.getZ();
    const IntervalVector &VQ = Q.getV();
    IntervalVector tmpV = ZinvZQ * VQ;
    bwd_mul(Q.Box,this->Z, tmpV,0.01);
    this->V |= tmpV;
    return (*this);
}
ExpBox& ExpBox::operator|=(const IntervalVector &iv) {
    if (iv.is_empty()) return (*this);
    if (this->is_empty()) {
       this->Box = iv;
       this->flat = iv.is_flat();
       this->paral=false;
       return (*this);
    }
    if (!iv.is_flat()) flat=false;
    this->Box |= iv;
    if (paral) {
       this->V |= this->Zinv * iv;
    }
    return (*this);
}


ExpBox operator&(const ExpBox &C1, const ExpBox &C2) {
    ExpBox Q(C1);
    Q &= C2;
    return Q;
}
ExpBox operator|(const ExpBox &C1, const ExpBox &C2) {
    ExpBox Q(C1);
    Q |= C2;
    return Q;
}
ExpBox diff_hull(const ExpBox &B1, const ExpBox &B2) {
    ExpBox Q(B1);
    IntervalVector *diff_list;
    int nb_boxes = Q.Box.diff(B2.Box, diff_list);
    IntervalVector union_of_diff(Q.dim,Interval::empty_set());
//    std::cout << "diff_hull " << B1.Box << " " << B2.Box << " ";
    for (int i=0;i<nb_boxes;i++) {
        if (Q.paral) diff_list[i] &= Q.Z*Q.V;
        union_of_diff |= diff_list[i];
    }
    delete[] diff_list;
    Q.Box = union_of_diff;
//    std::cout << Q.Box << "\n";
    if (Q.Box.is_empty()) {
       Q.flat=true;
       return Q;
    }
    if (Q.flat && (Q.dim==2)) {
       return Q;
    }
    if (Q.paral) {
       bool n_emp = bwd_mul(Q.Box,Q.Zinv,Q.V,0.01);
       if (!n_emp) {
           Q.set_empty(); return Q;
       }
    }
    if (B2.paral) {
       IntervalVector comp = B2.Zinv * B1.Box;
       IntervalMatrix transZ(Q.dim,Q.dim);
       IntervalMatrix transZinv(Q.dim,Q.dim);
       if (B1.paral) {
          transZ = B2.Zinv * B1.Z;
          transZinv = B1.Zinv * B2.Z;
          comp &= transZ * B1.V;
       }
       nb_boxes = comp.diff(B2.V,diff_list);
       IntervalVector union2(Q.dim,Interval::empty_set());
       IntervalVector unionZ(Q.dim,Interval::empty_set());
       for (int i=0;i<nb_boxes;i++) {
           if (B1.paral) diff_list[i] &= transZ*B1.V;
           union2 |= (B1.Box & (B2.Z * diff_list[i]));
           if (B1.paral) { 
              unionZ |= (B1.V & (transZinv * diff_list[i])); 
           }
       }
       delete[] diff_list;
       bool n_emp = bwd_mul(union2,Q.Zinv,unionZ,0.01);
       if (n_emp) {
         Q.V |= unionZ;
         Q.Box |= union2;
       }
    }
    return Q;
}



bool operator==(const ExpBox &C1, const ExpBox &C2) {
    if (C1.Box != C2.Box) return false;
    if (C1.paral != C2.paral) return false;
    if (!C1.paral) return true;
    if (C1.Z != C2.Z) return false;
    return (C1.V != C2.V);
}

IntervalVector &operator&=(IntervalVector &V, const ExpBox &C) {
   V &= C.Box;
   return V;
}
IntervalVector &operator|=(IntervalVector &V, const ExpBox &C) {
   V |= C.Box;
   return V;
}

IntervalVector operator+(const ExpBox &C, const IntervalVector &V) {
   return (C.Box + V);
}


void ExpBox::contract_res(const IntervalVector &iv, const IntervalMatrix &Z1, const IntervalMatrix &Z1inv, const IntervalVector &V1) {
    this->Box &= iv;
    if (iv.is_flat()) this->flat=true;
    if (this->Box.is_empty()) { paral=false; flat=true; return; }
    if (paral) {
       bool n_emp = bwd_mul(this->Box,this->Z,this->V,0.01);
       if (!n_emp) { paral=false; flat=true;
                     this->Box.set_empty(); return; }
    }
    IntervalMatrix ZinvQ = Z1inv;
    bool n_emp = bwd_mul(V1,ZinvQ,this->Box,0.01);
    if (!n_emp) { paral=false; flat=true; return; }
    if (!paral) {
       paral = true;
       this->Zinv = ZinvQ;
       this->Z = Z1;
       this->V = V1 & (this->Zinv * this->Box);
       return;
    }
    ZinvQ = ZinvQ*this->Z;
    n_emp = bwd_mul(V1,ZinvQ,this->V,0.01);
    if (!n_emp) { paral=false; flat=true;
                  this->Box.set_empty(); return; }
    n_emp = bwd_mul(this->V,this->Zinv,this->Box,0.01);
    if (!n_emp) { paral=false; flat=true; }
}

void ExpBox::contract_flow(const ExpBox &Start, ExpBox &End, const Interval &tim) const {
   if (!this->paral || (infinite_norm(A)*tim.mag()>=1.0)) {
      IntervalVector basicFlow = Start.getBox()+tim*this->VF;
      End &= basicFlow;
      return;
   }
   double t_mid = tim.mid();
   Interval t_rad = tim-t_mid;
   Vector center = Box.mid();
   Matrix Id = Matrix::eye(dim);
   /* calcul basique de exp(-t_mid A) */
   IntervalMatrix ZTAZ = Zinv*A*Z;
   IntervalVector Y0(dim);
   Y0 = Zinv*Start.getBox();
   if (!Start.is_box()) {
      Y0 &= (Zinv*Start.getZ())*Start.getV();
   }
   IntervalVector R1 = (exp_mat(t_rad*ZTAZ)-Id)*(Y0-Zinv*center);
   IntervalVector last(dim,0.0); last[dim-1]=tim*this->speed;
   IntervalVector R2 = exp_mat(-t_mid*ZTAZ)*exp_mat_int(tim*ZTAZ)*last;
   IntervalMatrix rotAinv = Zinv*exp_mat(-t_mid*A);
   IntervalMatrix rotA = exp_mat(t_mid*A)*Z;
   IntervalVector R3 = exp_mat_abs(ZTAZ,tim)*(rotAinv*this->V);
   IntervalVector Y = (Y0+R1+R2+R3+(rotAinv-Zinv)*center);
   IntervalVector basicFlow = Start.getBox()+tim*this->VF;
   End.contract_res(basicFlow, rotA, rotAinv, Y);
}

std::ostream& operator<<(std::ostream& str, const ExpBox& C) {
    if (!C.paral) { return (str << C.Box); }
    str << "box:" << C.Box << " Z:" << C.Z << " V:" << C.V << " ";
    return str;
}


}
