#include "FitPlaneCLP.h"
#include "best_plane_fit.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMultiplyImageFilter.h"
#include "itkBinaryThinningImageFilter.h"
#include "itkImageRegionIterator.h"
#include <itkImageMomentsCalculator.h>
#include <vnl/vnl_cross.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vtkPolyDataReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCenterOfMass.h>

int  main(int argc, char** argv) 
{
	PARSE_ARGS;

	// reading data
	typedef itk::Image<double, 3>    input1Type;
	typedef itk::ImageFileReader<input1Type> Reader1Type;
	Reader1Type::Pointer reader1 = Reader1Type::New();
	reader1->SetFileName(input1);

	typedef itk::Image<double, 3>    input2Type;
	typedef itk::ImageFileReader<input2Type> Reader2Type;
	Reader2Type::Pointer reader2 = Reader2Type::New();
	reader2->SetFileName(input2);

	typedef itk::MultiplyImageFilter <input1Type, input2Type, input1Type> 
		MultiplyImageFilterType ;
	MultiplyImageFilterType::Pointer multiplyfilter = MultiplyImageFilterType::New();
	multiplyfilter->SetInput1(reader1->GetOutput()); 	
	multiplyfilter->SetInput2(reader2->GetOutput());
	multiplyfilter->Update();

    // left center
    typedef itk::ImageMomentsCalculator <input1Type> ImageMomentsCalculatorType ;
	ImageMomentsCalculatorType::Pointer imagemomentcalculator = ImageMomentsCalculatorType::New();
	imagemomentcalculator->SetImage(reader1->GetOutput());
	imagemomentcalculator->Compute();
	input1Type::PointType leftCenterpoint = imagemomentcalculator->GetCenterOfGravity();
	input1Type::IndexType leftCenterpixel; 
	reader1->GetOutput()->TransformPhysicalPointToIndex( leftCenterpoint, leftCenterpixel );
	double leftcenter[3];
	leftcenter[0]= leftCenterpixel[0] ;
	leftcenter[1]= leftCenterpixel[1] ;
	leftcenter[2]= leftCenterpixel[2] ;


	// right center
    typedef itk::ImageMomentsCalculator <input1Type> ImageMomentsCalculatorType ;
	ImageMomentsCalculatorType::Pointer imagemomentcalculator2 = ImageMomentsCalculatorType::New();
	imagemomentcalculator2->SetImage(reader2->GetOutput());
	imagemomentcalculator2->Compute();
	input1Type::PointType rightCenterpoint = imagemomentcalculator2->GetCenterOfGravity();
	input1Type::IndexType rightCenterpixel; 
	reader2->GetOutput()->TransformPhysicalPointToIndex( rightCenterpoint, rightCenterpixel );
	double rightcenter[3];
	rightcenter[0]= rightCenterpixel[0] ;
	rightcenter[1]= rightCenterpixel[1] ;
	rightcenter[2]= rightCenterpixel[2] ;


   // points set 
	itk::ImageRegionConstIterator<input1Type> MyIt(multiplyfilter->GetOutput(),multiplyfilter->GetOutput()->GetLargestPossibleRegion());
	MyIt.GoToBegin();
	vnl_vector< double > p(3);
	vector< vnl_vector<double> > points_set;
	vnl_vector< double > normal(3) ;
		
	while (!MyIt.IsAtEnd())
	{
		if (MyIt.Get() == 1)
		{ 		
			input1Type::IndexType idx = MyIt.GetIndex();
			vnl_vector<double> point(3);
			
            point[0] = idx[0]; 
            point[1] = idx[1];
            point[2] = idx[2]; 
          
			points_set.push_back(point);
		}
		++MyIt;
	}


  //Initialize the normal and a point of the plane
  p[0]= (rightcenter[0] + leftcenter[0])/2.0;
  p[1]= (rightcenter[1] + leftcenter[1])/2.0;
  p[2]= (rightcenter[2] + leftcenter[2])/2.0;
	normal[0]= rightcenter[0] - leftcenter[0];
	normal[1]= rightcenter[1] - leftcenter[1];
	normal[2]= rightcenter[2] - leftcenter[2];
		
  vnl_vector<double> x_planeparam(6);
  x_planeparam[0] = normal[0];
  x_planeparam[1] = normal[1];
  x_planeparam[2] = normal[2];
  x_planeparam[3] = p[0];
  x_planeparam[4] = p[1];
  x_planeparam[5] = p[2];
   
  std::cout<< points_set.size() << std::endl;
  BestPlaneFit bestfit = BestPlaneFit(6, points_set.size());
  bestfit.SetPoints(points_set);
  
  vnl_levenberg_marquardt levenberg(bestfit);
  levenberg.set_trace(true);

  cout<<"Initial guess: "<<x_planeparam<<endl;
  levenberg.minimize(x_planeparam);
  cout<<"Best fit: "<<x_planeparam<<endl;

 
  vnl_vector< double > n(x_planeparam.data_block(), 3); 
  n /= n.magnitude();
  std::cout << " the normal normalized: " << n << std::endl;
  vnl_vector< double > q(x_planeparam.data_block() + 3 , 3);
  itk::ImageRegionConstIterator<input1Type> It1(multiplyfilter->GetOutput(),multiplyfilter->GetOutput()->GetLargestPossibleRegion());
	It1.GoToBegin();
  while(!It1.IsAtEnd()){
    if(It1.Get() == 1){
      input1Type::IndexType xy_index = It1.GetIndex();
      vnl_vector<double> p0(3, 0);
      p0[0] = xy_index[0];
      p0[1] = xy_index[1];
      p0[2] = xy_index[2];

      vnl_vector<double> v = p0 - q;
      double DotProduct = dot_product(v, n);
      if(DotProduct >0 || DotProduct==0)
      {
      	reader1->GetOutput()->SetPixel(xy_index, 1 );
      	reader2->GetOutput()->SetPixel(xy_index, 0 );
      }
      else if(DotProduct < 0){
      	reader1->GetOutput()->SetPixel(xy_index, 0 );
        reader2->GetOutput()->SetPixel(xy_index, 1 );
      }    		
    }        
  ++It1;
  }
   
	typedef itk::ImageFileWriter< input1Type > WriterType;
	WriterType::Pointer Imagewriter1 = WriterType::New();
	Imagewriter1->SetInput(reader1->GetOutput());
	Imagewriter1->SetFileName(output1);
	Imagewriter1->Update();

	typedef itk::ImageFileWriter< input1Type > WriterType;
	WriterType::Pointer Imagewriter2 = WriterType::New();
	Imagewriter2->SetInput(reader2->GetOutput());
	Imagewriter2->SetFileName(output2);
	Imagewriter2->Update();
	return EXIT_SUCCESS;
}

