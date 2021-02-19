#ifndef BURGALGORITHM_H
#define BURGALGORITHM_H

#include <math.h>
#include <vector>
using namespace std;
#include "AudioInterface.h"

class BurgAlgorithm
{
public:
    BurgAlgorithm();
    void compute( vector<long double> &coeffs, const vector<double> &x );
    void train( vector<long double> &coeffs, const vector<sample_t> &x );
    void predict( vector<long double> &coeffs, vector<sample_t> &predicted );
};

#endif // BURGALGORITHM_H
