#ifndef BestPlaneFit_H
#define BestPlaneFit_H

#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vnl/vnl_least_squares_function.h>
#include <vector>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageRegionIterator.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

using namespace std;

class BestPlaneFit : public vnl_least_squares_function
{
public:    

    BestPlaneFit(int unknowns = 6, int nsamples = 4);
    ~BestPlaneFit();

    virtual void f(vnl_vector< double > const &x, vnl_vector< double > &fx);

    void SetPoints(vector< vnl_vector<double> > points){
        m_Points = points;
    }
    vector< vnl_vector<double> > GetPoints(){
        return m_Points;
    }
private:

    vector< vnl_vector<double> > m_Points;
    int m_Samples;

};

#endif 