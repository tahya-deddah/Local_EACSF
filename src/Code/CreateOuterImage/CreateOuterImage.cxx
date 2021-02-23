#include "CreateOuterImageCLP.h"
#include "itkImage.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkImageFileReader.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
 
 
int main(int argc, char *argv[])
{
  PARSE_ARGS;

  typedef itk::Image<unsigned char, 3>    ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(InputImageFile);

  ImageType::SpacingType Spacing = reader->GetOutput()->GetSpacing();


  unsigned int radius = 60/Spacing[0];
  radius = closingradius/Spacing[0];
   
  typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>
    BinaryThresholdImageFilterType;
 
  BinaryThresholdImageFilterType::Pointer thresholdFilter
    = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(reader->GetOutput());
  thresholdFilter->SetLowerThreshold(1);
  thresholdFilter->SetUpperThreshold(4);
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();
  ImageType::Pointer image = thresholdFilter->GetOutput();



  ImageType::SizeType regionSize;
  regionSize[0] = int(image->GetLargestPossibleRegion().GetSize()[0]/2);
  regionSize[1] = image->GetLargestPossibleRegion().GetSize()[1];
  regionSize[2] = image->GetLargestPossibleRegion().GetSize()[2];
 
  ImageType::IndexType regionIndex;
  regionIndex[0] = 0;
  if (Reverse == 1)
  {
    regionIndex[0] = image->GetLargestPossibleRegion().GetSize()[0]/2;
  }
  regionIndex[1] = 0;
  regionIndex[2] = 0;
 
  ImageType::RegionType region;
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  itk::ImageRegionIterator<ImageType> imageIterator(image,region);
 
  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(0);
    ++imageIterator;
    }
 

  typedef itk::BinaryBallStructuringElement<
    ImageType::PixelType,3>                  StructuringElementType;
  StructuringElementType structuringElement;
  structuringElement.SetRadius(closingradius);
  structuringElement.CreateStructuringElement();

  typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(image);
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();

  typedef itk::BinaryMorphologicalClosingImageFilter <ImageType, ImageType, StructuringElementType>
          BinaryMorphologicalClosingImageFilterType;
  BinaryMorphologicalClosingImageFilterType::Pointer closingFilter
          = BinaryMorphologicalClosingImageFilterType::New();
  closingFilter->SetInput(rescaleFilter->GetOutput());
  closingFilter->SetKernel(structuringElement);
  closingFilter->Update();

  radius = 5/Spacing[0];
  radius = dilationradius/Spacing[0];
  structuringElement.SetRadius(radius);
  structuringElement.CreateStructuringElement();



typedef itk::BinaryDilateImageFilter <ImageType, ImageType, StructuringElementType>
          BinaryDilateImageFilterType;
 
  BinaryDilateImageFilterType::Pointer dilateFilter
          = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(closingFilter->GetOutput());
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->Update();

  rescaleFilter->SetInput(dilateFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();
 
  typedef itk::ImageFileWriter< ImageType >  WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( OutputImageFile );
  writer->SetInput( rescaleFilter->GetOutput() );
  writer->UseCompressionOn ();

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "Error: " << e << std::endl;
    return EXIT_FAILURE;
    }
 
  return EXIT_SUCCESS;
}
