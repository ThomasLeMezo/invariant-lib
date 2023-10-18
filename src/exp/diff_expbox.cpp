
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <vector>
#include <cmath>
#include <ibex.h>
#include "diff_expbox.h"
#include "expbox_util.h"

using namespace ibex;

namespace invariant{

/* UTILITIES FROM expIMat.cpp */
// returns [x]([x]+a) 
static Interval quad_I(const Interval& x, double a) {
    return sqr(x+(a/2))-a*a/4;
}
static Interval quad_II(const Interval& x, const Interval& a) {
    return quad_I(x,a.lb())|quad_I(x,a.ub());
}
static void join_mat(int dim, IntervalMatrix& Res, const IntervalMatrix& M) {
   for (int i=0;i<dim;i++)
   for (int j=0;j<dim;j++) Res[i][j] |= M[i][j];
}
// compute [M]([M]+a I)
static IntervalMatrix quad_M(const IntervalMatrix& M, double a) {
   const int n = M.nb_rows();
   IntervalMatrix Res(n,n,Interval::zero());
   for (int k=0;k<n;k++) {
      for (int i=0;i<n;i++) {
         if (i==k) continue;
         for (int j=0;j<n;j++) {
             if (j==k) continue;
             Res[i][j] += M[i][k]*M[k][j];
         }
      }
   }
   for (int i=0;i<n;i++)
   for (int j=0;j<n;j++) {
      if (i==j) {
         Res[i][i] += quad_I(M[i][i],a);
      } else {
         Res[i][j] += M[i][j] * (M[i][i]+M[j][j]+a);
      }
   }
   return Res;
}
IntervalMatrix square_IntervalMatrix (const IntervalMatrix& M) {
   return quad_M(M,0.0);
}
// compute [M](tau[M]+a I) with tau \in [-1,1]
static IntervalMatrix quad_Mtau(const IntervalMatrix& M, double a) {
   const int n = M.nb_rows();
   IntervalMatrix Res(n,n,Interval::zero());
   for (int k=0;k<n;k++) {
      for (int i=0;i<n;i++) {
         if (i==k) continue;
         for (int j=0;j<n;j++) {
             if (j==k) continue;
             Res[i][j] += M[i][k]*M[k][j];
         }
      }
   }
   for (int i=0;i<n;i++)
   for (int j=0;j<n;j++) {
      if (i==j) {
         Interval tmp = -Res[i][i]-quad_I(M[i][i],-a);
         Res[i][i] += quad_I(M[i][i],a);
         Res[i][i] |= tmp;
      } else {
         Interval tmp = -Res[i][j]-M[i][j]*(M[i][i]+M[j][j]-a);
         Res[i][j] += M[i][j] * (M[i][i]+M[j][j]+a);
         Res[i][j] |= tmp;
      }
   }
   return Res;
}

static IntervalMatrix quad_MM(const IntervalMatrix& M, const IntervalMatrix& A) {
   const int n = M.nb_rows();
   IntervalMatrix Res(n,n,Interval::zero());
   IntervalMatrix Res2(n,n,Interval::zero());
   for (int k=0;k<n;k++) {
      for (int i=0;i<n;i++) {
         if (i==k) continue;
         for (int j=0;j<n;j++) {
             if (j==k) continue;
             Res[i][j] += M[i][k]*(M[k][j]+A[k][j]);
             Res2[i][j] += (M[i][k]+A[i][k])*M[k][j];
         }
      }
   }
   for (int i=0;i<n;i++)
   for (int j=0;j<n;j++) {
      if (i==j) {
         Interval u = quad_II(M[i][i],A[i][i]);
         Res[i][i] += u;
         Res2[i][i] += u;
      } else {
         Interval c = M[j][j]+A[j][j];
         Interval clb = (M[i][i]).lb() + c;
         Interval cub = (M[i][i]).ub() + c;
         Interval a1 = M[i][j]*clb+(M[i][i]).lb()*A[i][j];
         Interval a2 = M[i][j]*cub+(M[i][i]).ub()*A[i][j];
         Res[i][j] += (a1|a2);
         Interval b1 = M[j][i]*clb+(M[i][i]).lb()*A[j][i];
         Interval b2 = M[j][i]*cub+(M[i][i]).ub()*A[j][i];
         Res2[j][i] += (b1|b2);
      }
   }
   Res&= Res2;
   return Res;
}

// compute [M](tau [M]+[A]) inter (tau [M]+[A])[M]
// tau in [0,1] or [-1,1]
static IntervalMatrix quad_MMtau(const IntervalMatrix& M, const IntervalMatrix& A, bool with_neg) {
   const int n = M.nb_rows();
   IntervalMatrix Res = quad_MM (M,A);
   IntervalMatrix Res2 = (with_neg ? quad_MM(-M,A) : M*A);
   if (!with_neg) Res2 &= A*M;
   join_mat(n,Res,Res2);
   return Res;
}

static inline void prod_com_with(IntervalMatrix &A, const IntervalMatrix &B) {
   IntervalMatrix M=(B*A);
   A *= B;
   A &= M;
}


// upper bound for the integral of the absolute value of linear intervalsi
// ( [I]+t[P] between 0 and delta )
static double integral_abs_linint(const Interval& I, const Interval& P, const Interval& V, double delta) {
   if (I.lb()>=0 && I.lb()+P.lb()>=0) { /* pure >=0 */ 
      return V.ub();
   } else if (I.ub()<=0 && I.ub()+P.ub()<=0) { /* pure <=0 */
      return -V.lb();
   }
   double mdI = I.mid();
   double mdP = P.mid();
   double modif;
   bool useub; /* true -> use ub ; false -> use lb */
   double t = (mdP==0.0 ? 1.0 : (mdI/mdP));
   if ((t>0) || (t<=-delta)) { // no change of bound
      useub =  (mdI>=0.0);
      modif=0.0;
   } else {
      useub = (mdP>0);
      modif = mdI*t; // useub <=> mdI<0 => modif>0
   }
   Interval in = delta*(I+0.5*delta*P)+modif;
   if (useub) return in.ub();
   return - in.lb();
}

// "compute" alpha (I + alpha I1) with alpha in [0,1] (withneg=false) or
// alpha in [-1,1] (withneg=true)
static Interval sumAlphaInterval(const Interval &I, const Interval &I1, bool withneg) {
    Interval resI = Interval::zero();
    resI |= I+I1; /* alpha=1 */
    if (withneg) resI |= I1-I; /* alpha =-1 */
    if (I.mag()<2*I1.mig()) /* on trouve un extrêmum avec alpha \in [-1,1] */
	 /* cet extrêmum est avec I.mag et I1.mig et vérifie |.| < I1.mig */
         /* si I.mag > 2*I1.mig I1+I ou I1-I vérifie > I1.mig et < -I1.mig */
    {
        /* notons qu'alors I1 doit être de signe constant */
        /* en effet, si 0 \in I1, si on a un extrêmum
                 a^2 i1 + a i  avec a \in ]0,1[ ou ]-1,1[,
                    : si i1=0, pas d'extrêmum possible (dérivée/a non nulle)
                    : si i1>0, c'est un min/a, alors on réduit i1
                    : si i1<0, c'est un max/a, alors on augmente i1
        */
        /* si withneg, on peut directement prendre
           sqr(I.mag())/(-2.0*I1.mig()) */
        if (withneg) {
            Interval V=sqr(I)/(-2*I1);
            resI |= V;
        } else 
        /* si I1 est toujours >0, c'est le minimum qui est modifié,
             avec I1.lb() à utiliser et I.lb() (qui doit être négatif,
              sinon min atteint avec a = 0 */
        if (I1.lb()>0 && I.lb()<0) {
             Interval V(I.lb());
             V = sqr(V)/(-2.0*I1.lb());
             resI |= V;
        } else if (I1.ub()<0 && I.ub()>0) {
             Interval V(I.ub());
             V = sqr(V)/(-2.0*I1.ub());
             resI |= V;
        }
    }
    return resI;
}
static IntervalMatrix sumAlphaMatrix(const IntervalMatrix &M, const IntervalMatrix &M1, bool withneg){
    IntervalMatrix R(M.nb_rows(),M.nb_cols());
    for (int i=0;i<M.nb_rows();i++) 
      for (int j=0;j<M.nb_cols();j++) {
         R[i][j] = sumAlphaInterval(M[i][j],M1[i][j],withneg);
    }
    return R;
}

/* add integral of absolute value of delta (A + tB), with t between 0 and 1 */
/* and delta>0 */
static void add_int_absL (int dim, Matrix& Res,
         const IntervalMatrix& A, const IntervalMatrix& B, 
         const IntervalMatrix& V,
         double delta) {
   for (int i=0;i<dim;i++)
   for (int j=0;j<dim;j++) 
        Res[i][j] += delta*integral_abs_linint(A[i][j], B[i][j], V[i][j], 1.0);
}


/* compute 
    ExpA = exp(A)-Id
    tauExpA = exp(tau A)-Id, tau \in [0,1]
    IexpA = int_0^1 exp(tau A) dtau
*/
static void global_exp_base1(const IntervalMatrix& A,
          IntervalMatrix& ExpA,
          IntervalMatrix& tauExpA,
          IntervalMatrix& IexpA, int k) {
    int dim = A.nb_rows();
    Matrix Id = Matrix::eye (dim);
    Interval norm = infinite_norm(A);
    if (k%2==0) k=k+1;
    Interval baseErrorTerm = norm*norm/(k*(k+1)*(k+2));
    IntervalMatrix sqA = square_IntervalMatrix(A);
    ExpA = (Interval::one()/(k+1))*sqA; // will be used for base1 
    tauExpA = ExpA; // Ubase1 (/Unt)
    while (k>3) {
       ExpA += A; ExpA *= Interval::one()/k; ExpA += Id;
       prod_com_with (ExpA , (Interval::one()/(k-1))*sqA);
       tauExpA = sumAlphaMatrix(A,tauExpA,false); tauExpA *= Interval::one()/k;
       tauExpA += Id; prod_com_with (tauExpA , (Interval::one()/(k-1))*sqA);
       k-=2;
       baseErrorTerm *= norm*norm/(k*(k+1));
    }
    Interval one6 = Interval::one()/6.0;
    baseErrorTerm *= 0.5*norm; // NA^k/(k+2)!
    Interval nET0 = baseErrorTerm*(k+3)/(k+3-norm);
    Interval nET1 = nET0 * norm;
    IntervalMatrix pId(Id);
    pId.inflate(2.0*nET0.ub());
    IntervalMatrix IexpA1 = one6*quad_MM(A,3.0*pId+ExpA);
    IexpA = one6*quad_MM(A,3.0*Id+ExpA);
    IexpA.inflate(nET1.ub());
    IexpA &= IexpA1;
    ExpA += A;
    IntervalMatrix ExpA1(ExpA);
    ExpA1.inflate(6.0*nET0.ub());
    prod_com_with(ExpA1, sqA); ExpA1 *= one6;
    ExpA1 = 0.5*quad_M(A,2.0)+ExpA1;
    prod_com_with(tauExpA,A); 
    tauExpA = sumAlphaMatrix(sqA,tauExpA,false);
    IntervalMatrix tauExpA1(tauExpA);
    tauExpA1.inflate(6.0*nET0.ub());
    tauExpA1 = 0.5*quad_MMtau(A,2.0*(Id+one6*tauExpA1),false);
    nET0 *= norm*norm;
    prod_com_with(ExpA, sqA); ExpA *= one6;
    ExpA = 0.5*quad_M(A,2.0)+ExpA;
    ExpA.inflate(nET0.ub());
    ExpA &= ExpA1;
    tauExpA = 0.5*quad_MMtau(A,2.0*(Id+one6*tauExpA),false);
    tauExpA.inflate(nET0.ub());
    tauExpA &= tauExpA1;
}


/* compute 
    tauExpA = exp(tau A)-Id , tau \in [-1,1]
    IexpA = int_0^1 exp(tau A) dtau
*/
static void global_exp_base2(const IntervalMatrix& A, IntervalMatrix &tauExpA,
          IntervalMatrix& IexpA, int k) {
    int dim = A.nb_rows();
    Matrix Id = Matrix::eye (dim);
    Interval Unt(0.0,1.0);
    Interval norm = infinite_norm(A);
    if (k%2==0) k=k+1;
    Interval baseErrorTerm = norm*norm/(k*(k+1)*(k+2));
    IntervalMatrix sqA = square_IntervalMatrix(A);
    IexpA = (Interval::one()/(k+1))*sqA; // will be used for base1 
    tauExpA = IexpA;
    while (k>3) {
       IexpA += A; IexpA *= Interval::one()/k; IexpA += Id;
       prod_com_with (IexpA , (Interval::one()/(k-1))*sqA);
       tauExpA = sumAlphaMatrix(A,tauExpA,true); tauExpA *= Interval::one()/k;
       tauExpA += Id; prod_com_with (tauExpA , (Interval::one()/(k-1))*sqA);
       k-=2;
       baseErrorTerm *= norm*norm/(k*(k+1));
    }
    Interval one6 = Interval::one()/6.0;
    baseErrorTerm *= 0.5*norm; // NA^k/(k+2)!
    Interval nET0 = baseErrorTerm*(k+3)/(k+3-norm);
    Interval nET1 = nET0 * norm;
    IntervalMatrix pId(Id);
    pId.inflate(2.0*nET0.ub());
    IntervalMatrix IexpA1 = one6*quad_MM(A,3.0*pId+IexpA);
    IexpA = one6*quad_MM(A,3.0*Id+IexpA);
    IexpA.inflate(nET1.ub());
    IexpA &= IexpA1;
    prod_com_with(tauExpA,A); 
    tauExpA = sumAlphaMatrix(sqA,tauExpA,true);
    IntervalMatrix tauExpA1(tauExpA);
    tauExpA1.inflate(6.0*nET0.ub());
    tauExpA1 = 0.5*quad_MMtau(A,2.0*(Id+one6*tauExpA1),true);
    nET0 *= norm*norm;
    tauExpA = 0.5*quad_MMtau(A,2.0*(Id+one6*tauExpA),true);
    tauExpA.inflate(nET0.ub());
    tauExpA &= tauExpA1;
}



static IntervalMatrix exp_mat(const IntervalMatrix &M) {
    const int dim = M.nb_rows();
    IntervalMatrix Id = Matrix::eye(dim);
    IntervalMatrix res = Id;
    Interval n = 1.0/(1.0-0.2*infinite_norm(M));
    res.inflate(n.ub());
    res = Id+M*(Id+0.5*M*(Id+(1.0/3.0)*M*(Id+0.25*M*res)));
    return res;
}
    
static IntervalMatrix exp_mat_int(const IntervalMatrix &M) {
    const int dim = M.nb_rows();
    IntervalMatrix Id = Matrix::eye(dim);
    IntervalMatrix res = Id;
    Interval n = 1.0/(1.0-0.2*infinite_norm(M));
    res.inflate(n.ub());
    res = Id+0.5*M*(Id+(1.0/3.0)*M*(Id+0.25*M*res));
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


ExpVF::ExpVF(const IntervalVector &Box, const IntervalVector &VF,
			 const std::pair<Matrix, IntervalVector>& pair) :
    dim(Box.size()), Box(Box), A(pair.first), C(Box.mid()), Z1(Box.size()),
    V(pair.second), VF(VF), constant_field(false)
    {
      Z1 = V.mid();
      V = V-Z1;
}

ExpVF::ExpVF(const IntervalVector &Box, const IntervalVector &VF) :
    dim(VF.size()), Box(VF),
    A(dim,dim,0.0), C(dim,0.0), Z1(VF.mid()), constant_field(true),
    V(VF-Z1), VF(VF)
    {
}

ExpVF::ExpVF(const IntervalVector &VF) :
    dim(VF.size()), Box(VF),
    A(dim,dim,0.0), C(dim,0.0), Z1(VF.mid()), constant_field(true),
    V(VF-Z1), VF(VF)
    {
      assert(this->Box.is_empty());
}

ExpVF::ExpVF (const ExpVF &EVF, double sens) :
    dim(EVF.dim), Box(EVF.Box), A(sens*EVF.A), C(EVF.C), Z1(sens*EVF.Z1),
    V(sens*EVF.V), VF(sens*EVF.VF), constant_field(EVF.constant_field) {

}

ExpVF  & ExpVF::operator|=(const ExpVF &vf) {
    if (this->Box.is_empty()) {
       (*this)=vf;
       return (*this);
    }
    if (vf.Box.is_empty()) return (*this);
    this->VF |= vf.VF;
    this->constant_field |= vf.constant_field;
    if (this->constant_field) return (*this);
    IntervalVector nBox = this->Box | vf.Box;
    Vector nC = nBox.mid();
    Matrix nA = 0.5*(vf.A+this->A);
    IntervalVector V1 = this->Z1+(this->A-nA)*(this->Box-this->C)+nA*(nC-this->C)+this->V;;
    V1 |= (vf.Z1+(vf.A-nA)*(vf.Box-vf.C)+nA*(nC-vf.C)+vf.V);
    this->Z1 = V1.mid();
    this->V = V1 - this->Z1;
    this->A=nA;
    this->C=nC;
    this->Box = nBox;
    return (*this);
}

ExpVF operator-(const ExpVF &vf) {
    return ExpVF(vf,-1.0);
}


std::vector<std::pair<IntervalVector, Interval>> ExpVF::build_flow(const ExpPoly &StartR, ExpPoly &End, Interval tim) const {
   /* a small "backward to refine start */
#define BACKWARD_REFINE	1
#if (BACKWARD_REFINE)
   ExpPoly Start = StartR & this->basicFlow(End,-tim);
#else
   const ExpPoly &Start = StartR;
#endif

   double t_mid = tim.mid();
   Interval t_rad = tim-t_mid;
   Matrix Id = Matrix::eye(dim);
   /* start relative au centre */
   Vector SMid =  Start.getBox().mid();
   IntervalVector stRel = Start.getBox() -  SMid;
   /* récupération de Start sous une forme "adaptée" */
   Vector Z1p = A*(SMid-C)+Z1;

   std::vector<std::pair<IntervalVector, Interval>> lst =
		 Start.build_constraints_for_propag(C, Z1p);
//   std::cout << "constraints building " << Start << " " << Z1 << "\n";
//   for (auto &q : lst) {
//       std::cout << q.first << " (" << q.first*Z1 << ") "  << q.second << "\n";
//   }
//   std::cout << "OK et tim : " << tim << "\n";
   
   IntervalMatrix rotA(dim,dim);
   IntervalMatrix intA(dim,dim);
   IntervalMatrix tauA(dim,dim);
   global_exp_base1(-t_mid*A,rotA,tauA,intA,5); 
   IntervalMatrix R1(dim,dim);
   IntervalMatrix intdA(dim,dim);
   global_exp_base2(t_rad.ub()*A,R1,intdA,5); 
   /* calcul basique de exp(-t_mid A) */
   for (int i=0;i<lst.size();i++) {
       lst[i].second += lst[i].first*rotA*SMid;
   }
//   std::cout << "rotA " << rotA << "\n";
   IntervalMatrix R2 = t_mid*(intA+R1*(intA+Id))+t_rad*(intdA*(Id+rotA)+rotA);
//   std::cout << "R2mat*Z1 " << (R2*Z1) << "\n";
   for (int i=0;i<lst.size();i++) {
//       std::cout << "dim " << i << " R1 " << ((lst[i].first*R1)*stRel) << " R2 " << ((lst[i].first*R2)*Z1) << "\n";
       lst[i].second += (lst[i].first*R1)*stRel+(lst[i].first*R2)*Z1p
			+tim*(lst[i].first*Z1p);
   }
   Matrix R3(dim,dim,0.0);
   add_int_absL(dim,R3,Id,tauA,Id+intA,t_mid);
   add_int_absL(dim,R3,Id,R1,Id+intdA,t_rad.ub());
   for (int i=0;i<lst.size();i++) {
//      std::cout << "dim " << i << " R3 " << (lst[i].first*(R3*V)) << "\n";
       lst[i].second += lst[i].first*(R3*V);
//       lst[i].second += lst[i].first*C;
       lst[i].first = lst[i].first*(rotA+Id);
   }
   return lst;
}

Interval ExpVF::compute_basic_time(const ExpPoly &Start, const ExpPoly &End) const {
   /* calcul naïf du temps */
    IntervalVector c = End.getBox()-Start.getBox();
    Interval alpha(Interval::pos_reals());
    for(int i=0; i<dim; i++){
        if(!(c[i]==Interval::zero() && this->VF[i].contains(0.0)))
            alpha &= ((c[i]/(this->VF[i] & Interval::pos_reals())) & Interval::pos_reals()) | ((c[i]/(this->VF[i] & Interval::neg_reals())) & Interval::pos_reals());
            if (alpha.is_empty()) return alpha;
    }
    if (alpha==Interval::zero()) return Interval::empty_set();
    return alpha;
}

Interval ExpVF::refine_time(const ExpPoly &Start, const ExpPoly &End, Interval &tim) const {
   IntervalVector c = End.getBox()- Start.getBox();
   for (int i=0; i<dim; i++){
       Interval den = (A*(Start.getBox()-C)+Z1+this->V)[i] & this->VF[i];
       if (den.contains(0.0)) continue;
       Interval num = c[i];
       Interval tim2 = (num/den)
			*(1-Interval(0,2)*c[i]*(A*this->VF)[i]/sqr(den)) ;
       if (!tim.is_subset(tim2)) {
//           std::cout << "refine time : progres : " << tim << " " << tim2 << "\n";
           tim &= tim2;
       }
   }
   return tim;
}


void ExpVF::contract_flow(const ExpPoly &Start, ExpPoly &End) const {
   /* calcul naïf du temps */
    Interval alpha = this->compute_basic_time(Start, End);

    Interval &tim=alpha;
//   std::cout << "c : " << c << " VF : " << VF << " time : " << tim << "\n";

    IntervalVector basicFlow = this->basicFlow(Start,tim);  /* TODO : better ! */
    End &= basicFlow;
   if (End.is_empty() || this->constant_field || (infinite_norm(A)*tim.mag()>=0.8) || (tim==Interval::zero()))
      return;
/* refine time ? */
#define REFINE_TIME	1
#if (REFINE_TIME) 
   tim = this->refine_time(Start,End,tim);
#endif

#if 1
   std::vector<std::pair<IntervalVector, Interval>> lst = this->build_flow(Start, End, tim);
   End &= lst;
#else
   IntervalMatrix Zt = matrix_orthogonal(Z1);
   IntervalVector Y0 = Start.build_constraints_for_propag(Zt);
   IntervalMatrix Z = Zt.transpose();
   Interval speed = IntervalVector(Z1).norm2();

   IntervalMatrix ZTAZ = Zt*A*Z;
   IntervalVector R1 = (exp_mat(t_rad*ZTAZ)-Id)*(Y0-Zt*C);
   IntervalVector last(dim,0.0); last[dim-1]=tim*speed;
   IntervalVector R2 = exp_mat(-t_mid*ZTAZ)*exp_mat_int(tim*ZTAZ)*last;
   IntervalMatrix rotAinv = Zt*exp_mat(-t_mid*A);
   IntervalVector R3 = exp_mat_abs(ZTAZ,tim)*(rotAinv*this->V);
   IntervalVector Y = (Y0+R1+R2+R3+(rotAinv-Zt)*C);
   End.intersect_paral(rotAinv,Y);
#endif
}


}
