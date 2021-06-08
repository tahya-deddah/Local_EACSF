#include "best_plane_fit.h"

#include "iostream"

using namespace std;

BestPlaneFit::BestPlaneFit(int unknowns, int samples)
    : vnl_least_squares_function(unknowns, samples, no_gradient)
{
    m_Samples = samples;
}

BestPlaneFit::~BestPlaneFit()
{
    // m_Points.clear();
}

void BestPlaneFit::f(vnl_vector< double > const &x, vnl_vector< double > &fx){

    vnl_vector< double > n(x.data_block(), 3);
    vnl_vector< double > q(x.data_block() + 3, 3);
  
    for(int i = 0; i < m_Points.size(); i++){
        vnl_vector< double > p = m_Points[i];
        vnl_vector< double > v = p - q;
        fx[i] = dot_product(v, n)/n.magnitude();
    }      
}

