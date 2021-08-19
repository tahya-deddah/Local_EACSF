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
	const unsigned int Dimension = 3;
	typedef double                      PixelType;
	typedef itk::Image< PixelType, Dimension > ImageType;
	typedef itk::ImageFileReader< ImageType >  ReaderType;

	ReaderType::Pointer reader1 = ReaderType::New();
	reader1->SetFileName(VisitationMap);
	reader1->Update();

	ReaderType::Pointer reader2 = ReaderType::New();
	reader2->SetFileName(CSFProbabiltyMap);
	reader2->Update();

  	typedef itk::BinaryBallStructuringElement<ImageType::PixelType,Dimension>                  StructuringElementType;
  	StructuringElementType structuringElement;
  	structuringElement.SetRadius(1);
  	structuringElement.CreateStructuringElement();

	/*typedef itk::BinaryDilateImageFilter <ImageType, ImageType, StructuringElementType> BinaryDilateImageFilterType; 
	BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
	dilateFilter->SetInput(reader1->GetOutput());
	dilateFilter->SetKernel(structuringElement);
	dilateFilter->SetForegroundValue(1); 
	dilateFilter->Update();*/

	typedef itk::MultiplyImageFilter <ImageType, ImageType, ImageType> MultiplyImageFilterType ;
	MultiplyImageFilterType::Pointer multiplyfilter = MultiplyImageFilterType::New();
	multiplyfilter->SetInput1(reader1->GetOutput()); 	
	multiplyfilter->SetInput2(reader2->GetOutput());
	multiplyfilter->Update();

 	typedef itk::MinimumMaximumImageCalculator <ImageType> MinimumMaximumImageCalculatorType ;
    MinimumMaximumImageCalculatorType::Pointer MinimumMaximumImage = MinimumMaximumImageCalculatorType::New();
    MinimumMaximumImage->SetImage(multiplyfilter->GetOutput());   
    MinimumMaximumImage->ComputeMaximum();
    std::cout << MinimumMaximumImage->GetMaximum() << std::endl;


  
    typedef itk::ImageRegionIterator<ImageType>       IteratorType;
  	IteratorType      MyIt1(multiplyfilter->GetOutput(), multiplyfilter->GetOutput()->GetRequestedRegion());
	MyIt1.GoToBegin();

	while (!MyIt1.IsAtEnd())
	{
		MyIt1.Set(double(MyIt1.Get())/MinimumMaximumImage->GetMaximum());
		++MyIt1;
	}

	 // Compute Volume
	typedef itk::ImageRegionIterator<ImageType>       IteratorType;
  	IteratorType      MyIt(multiplyfilter->GetOutput(), multiplyfilter->GetOutput()->GetRequestedRegion());
	MyIt.GoToBegin();
	double SumOfCSFProbabilities = 0;
	while (!MyIt.IsAtEnd())
	{	
		SumOfCSFProbabilities = SumOfCSFProbabilities + double(MyIt.Get()) ;
		++MyIt;		
	}
	ImageType::SpacingType spacing = multiplyfilter->GetOutput()->GetSpacing();
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

  	
