
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
    dim(Box.size()), A(pair.first), C(Box.mid()), Z1(Box.size()),
    V(pair.second), VF(VF), constant_field(false)
    {
      Z1 = V.mid();
      V = V-Z1;
}

ExpVF::ExpVF(const IntervalVector &VF) :
    dim(VF.size()),
    A(dim,dim,0.0), C(dim,0.0), Z1(VF.mid()), constant_field(true),
    V(VF-Z1), VF(VF)
    {
}

ExpVF::ExpVF (const ExpVF &EVF, double sens) :
    dim(EVF.dim), A(sens*EVF.A), C(EVF.C), Z1(sens*EVF.Z1),
    V(sens*EVF.V), VF(sens*EVF.VF), constant_field(EVF.constant_field) {

}



void ExpVF::contract_flow(const ExpPoly &Start, ExpPoly &End, const Interval &tim) const {
   //std::cout << "contract_flow " << tim << "\n";
   if (this->constant_field || (infinite_norm(A)*tim.mag()>=0.8) || (tim==0.0)) {
      IntervalVector basicFlow = Start+tim*this->VF; 
			/* TODO: we can do better */
      End &= basicFlow;
      return;
   }
   double t_mid = tim.mid();
   Interval t_rad = tim-t_mid;
   Matrix Id = Matrix::eye(dim);


#if 1
   /* start relative au centre */
   IntervalVector stRel = Start.getBox() - C;
   /* récupération de Start sous une forme "adaptée" */
   std::vector<std::pair<IntervalVector, Interval>> lst =
		 Start.build_constraints_for_propag(C,Z1);
//   std::cout << "constraints building " << Start << " " << Z1 << "\n";
//   for (auto &q : lst) {
//       std::cout << q.first << " (" << q.first*Z1 << ") "  << q.second << "\n";
//   }
//   std::cout << "OK et tim : " << tim << "\n";
   
   /* calcul basique de exp(-t_mid A) */
   IntervalMatrix rotA = exp_mat(-t_mid*A);
   for (int i=0;i<lst.size();i++) {
       lst[i].second += lst[i].first*(rotA-Id)*C;
   }
//   std::cout << "rotA " << rotA << "\n";
   IntervalMatrix R1 = exp_mat(t_rad*A)-Id;
   IntervalMatrix R2 = rotA*exp_mat_int(tim*A);
//   std::cout << "R2mat*Z1 " << (R2*Z1) << "\n";
   for (int i=0;i<lst.size();i++) {
//       std::cout << "dim " << i << " R1 " << ((lst[i].first*R1)*stRel) << " R2 " << ((lst[i].first*R2)*Z1) << "\n";
       lst[i].second += (lst[i].first*R1)*stRel+tim*(lst[i].first*(R2-Id))*Z1
			+tim*(lst[i].first*Z1);
   }
   Matrix R3 = exp_mat_abs(-A,t_mid)+exp_mat_abs(A,t_rad.ub());
   for (int i=0;i<lst.size();i++) {
//      std::cout << "dim " << i << " R3 " << (lst[i].first*(R3*V)) << "\n";
       lst[i].second += lst[i].first*(R3*V);
//       lst[i].second += lst[i].first*C;
       lst[i].first = lst[i].first*rotA;
   }
   
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
#endif

   IntervalVector basicFlow = Start+tim*this->VF;  /* TODO : better ! */
//   std::cout <<  "basicFlow " << basicFlow << " tim " << tim << " Vf " << this->VF << "\n";
   End &= basicFlow;
#if 1
   End &= lst;
#else
   End.intersect_paral(rotAinv,Y);

#endif
}


}
