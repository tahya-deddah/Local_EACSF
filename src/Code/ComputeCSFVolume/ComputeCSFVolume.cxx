#include "ComputeCSFVolumeCLP.h"
#include "itkImage.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkImageFileReader.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkImageFileWriter.h"
#include "itkMultiplyImageFilter.h"
#include <itkStatisticsImageFilter.h>
#include "itkImageRegionIterator.h"
#include <itkLabelStatisticsImageFilter.h>
#include "itkBinaryThresholdImageFilter.h"
#include <itkLabelMapToLabelImageFilter.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkMinimumMaximumImageCalculator.h>



using namespace std;

int  main(int argc, char** argv) 
{
	PARSE_ARGS;
  	typedef itk::Image<double, 3>    input1Type;
  	typedef itk::ImageFileReader<input1Type> Reader1Type;
  	Reader1Type::Pointer reader1 = Reader1Type::New();
  	reader1->SetFileName(VisitationMap);

  	typedef itk::Image<short, 3>    input2Type;
  	typedef itk::ImageFileReader<input2Type> Reader2Type;
  	Reader2Type::Pointer reader2 = Reader2Type::New();
  	reader2->SetFileName(CSFProbabiltyMap);

  	typedef itk::BinaryBallStructuringElement<
  	input1Type::PixelType,3>                  StructuringElementType;
  	StructuringElementType structuringElement;
  	structuringElement.SetRadius(1);
  	structuringElement.CreateStructuringElement();

	typedef itk::BinaryDilateImageFilter <input1Type, input1Type, StructuringElementType>
	    BinaryDilateImageFilterType; 
	BinaryDilateImageFilterType::Pointer dilateFilter
	    = BinaryDilateImageFilterType::New();
	dilateFilter->SetInput(reader1->GetOutput());
	dilateFilter->SetKernel(structuringElement);
	dilateFilter->SetForegroundValue(1); 
	dilateFilter->Update();

	typedef itk::MultiplyImageFilter <input1Type, input2Type, input1Type> 
		MultiplyImageFilterType ;
	MultiplyImageFilterType::Pointer multiplyfilter = MultiplyImageFilterType::New();
	multiplyfilter->SetInput1(dilateFilter->GetOutput()); 	
	multiplyfilter->SetInput2(reader2->GetOutput());
	multiplyfilter->Update();

 	typedef itk::MinimumMaximumImageCalculator <input1Type> 
       MinimumMaximumImageCalculatorType ;
    MinimumMaximumImageCalculatorType::Pointer MinimumMaximumImage = MinimumMaximumImageCalculatorType::New();
    MinimumMaximumImage->SetImage(multiplyfilter->GetOutput());   
    MinimumMaximumImage->ComputeMaximum();

  
    typedef itk::ImageRegionIterator<input1Type>       IteratorType;
  	IteratorType      MyIt1(multiplyfilter->GetOutput(), multiplyfilter->GetOutput()->GetRequestedRegion());
	MyIt1.GoToBegin();

	while (!MyIt1.IsAtEnd())
	{
		MyIt1.Set(double(MyIt1.Get())/MinimumMaximumImage->GetMaximum());
		++MyIt1;
	}

	 // Compute Volume
	itk::ImageRegionIterator<input1Type> MyIt(multiplyfilter->GetOutput(),multiplyfilter->GetOutput()->GetLargestPossibleRegion());
	MyIt.GoToBegin();
	int SumOfCSFProbabilities = 0;
	while (!MyIt.IsAtEnd())
	{	
		SumOfCSFProbabilities = SumOfCSFProbabilities + MyIt.Get();
		++MyIt;		
	}
	input2Type::SpacingType spacing = multiplyfilter->GetOutput()->GetSpacing();
	double VolumeOfOneVoxel = spacing[0] * spacing[1] * spacing[2];		
	double TotalVolume = SumOfCSFProbabilities * VolumeOfOneVoxel;
	TotalVolume = int(TotalVolume);


	// Compute CSF Density Sum
	ifstream File(CSFDenistytxtfile);    
    string line;
	getline(File, line); // nombre of point
	getline(File, line); // dimension
	getline(File, line); // scalars type
	double sum = 0;
	while(getline(File, line))
	{
		sum = sum + std::stod(line);
	}
	sum = int(sum);

  	std::string ResultFileName = Label + "_CSFVolume.txt";
  	ofstream Result;
  	Result.open (ResultFileName.c_str(), std::ios_base::app); 
  	Result << Side << " EACSF Volume  = " << TotalVolume << endl; 
  	Result << Side <<" EACSF Density Sum = " << sum << endl;
  	Result.close();
  	return EXIT_SUCCESS;
}

  	
