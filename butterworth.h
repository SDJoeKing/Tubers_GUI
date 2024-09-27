#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#include<math.h>
#include <vector>
#include <complex>



extern std::vector<double> ComputeDenCoeffs(const int &FilterOrder,const  double &Lcutoff, const  double &Ucutoff);

extern std::vector<double> TrinomialMultiply(const int &FilterOrder,const  std::vector<double> &b, const std::vector<double> &c);

extern std::vector<double> ComputeNumCoeffs(const int &FilterOrder, const double &Lcutoff, const double &Ucutoff, const std::vector<double> &DenC);

extern std::vector<double> filter(const std::vector<double>& x, const std::vector<double> &coeff_b, const std::vector<double> &coeff_a);

extern std::vector<double> ComputeLP(const int &FilterOrder);

extern std::vector<double> ComputeHP(const int &FilterOrder);



#endif // BUTTERWORTH_H
