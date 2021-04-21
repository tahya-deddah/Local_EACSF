#include "EstimateCortexStreamlinesDensityCLP.h"
#include <vtkPolyDataReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkWarpVector.h>
#include <vtkPolyDataNormals.h>
#include <vtkDataSetAttributes.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkPolyDataWriter.h>
#include "vtkIdTypeArray.h"
#include <vtkGradientFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>
#include <vtkDataArray.h>
#include <vtkVersion.h>
#include <vtkPoints.h>


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageFileWriter.h"
#include "itkBinaryThresholdImageFilter.h"
#include <itkLinearInterpolateImageFunction.h>
#include "itkImageRegionIterator.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>        // std::abs
#include <string>
#include <limits>       // std::numeric_limits
#include <ctime>// include this header 
#include <algorithm>// include this header 


int main ( int argc, char *argv[] )
{

  PARSE_ARGS;
  int start_s=clock();

  std::string inputSurfaceFilename = InputSurfaceFileName;
  vtkSmartPointer<vtkPolyDataReader> surfacereader = vtkSmartPointer<vtkPolyDataReader>::New();
  surfacereader->SetFileName(inputSurfaceFilename.c_str());
  surfacereader->Update();

  vtkPolyData* inputPolyData = surfacereader->GetOutput();
  std::cout << "Input surface has " << inputPolyData->GetNumberOfPoints() << " points." << std::endl;

  std::string outerstreamlinesFileName = InputOuterStreamlinesFileName;
  vtkSmartPointer<vtkGenericDataObjectReader> outerstreamlinesreader = 
      vtkSmartPointer<vtkGenericDataObjectReader>::New();
  outerstreamlinesreader->SetFileName(outerstreamlinesFileName.c_str());
  outerstreamlinesreader->Update();

  vtkPolyData* outerstreamlinesPolyData = outerstreamlinesreader->GetPolyDataOutput();
  std::cout << "Outer Streamlines File has " << outerstreamlinesPolyData->GetNumberOfLines() << " lines." << std::endl;
  vtkDoubleArray* OuterLengthArray = vtkDoubleArray::SafeDownCast(outerstreamlinesPolyData->GetCellData()->GetArray("Length"));

  // Get the segmentation filename from the command line
  std::string inputSegmentationFilename = InputSegmentationFileName;
  const unsigned int Dimension = 3;
  typedef double                      PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer Imagereader = ReaderType::New();
  Imagereader->SetFileName(inputSegmentationFilename.c_str());
  Imagereader->Update();

  ImageType::Pointer inputimage = ImageType::New();
  inputimage->CopyInformation(Imagereader->GetOutput());
  inputimage->SetRegions(Imagereader->GetOutput()->GetRequestedRegion());
  inputimage->Allocate();

  typedef itk::ImageRegionIterator< ImageType>       IteratorType;
  IteratorType      inputIt1(Imagereader->GetOutput(), Imagereader->GetOutput()->GetRequestedRegion());
  IteratorType      inputIt2(inputimage, inputimage->GetRequestedRegion());
  inputIt1.GoToBegin();
  inputIt2.GoToBegin();
  while (!inputIt1.IsAtEnd())
  {
        inputIt2.Set(double(inputIt1.Get())/double(std::numeric_limits<unsigned short>::max()));
        ++inputIt1;
        ++inputIt2;
  }
  //std::cout << "Initializing Voxel Visiting Map... "  << std::endl;
  ImageType::Pointer outputimage = ImageType::New();
  outputimage->CopyInformation(inputimage);
  outputimage->SetRegions(inputimage->GetRequestedRegion());
  outputimage->Allocate();

  IteratorType      outputIt(outputimage, outputimage->GetRequestedRegion());
  outputIt.GoToBegin();
  while (!outputIt.IsAtEnd())
  {
    outputIt.Set(0.0);
    ++outputIt;
  }
  std::string inputMaskFilename = InputMaskFileName;
  typedef itk::ImageFileReader< ImageType >  MaskReaderType;
  MaskReaderType::Pointer Maskreader = MaskReaderType::New();
  Maskreader->SetFileName(inputMaskFilename.c_str());
  Maskreader->Update();
  ImageType::Pointer inputMask = Maskreader->GetOutput();

// //-----------------------------------------------Estimate CSF Density------------------------------------------------------------------

  vtkSmartPointer<vtkDoubleArray> Array = vtkSmartPointer<vtkDoubleArray>::New();
  Array->SetNumberOfComponents(1);
  Array->SetName("CSFDensity");

  vtkSmartPointer<vtkCellArray> OuterLinearray = outerstreamlinesPolyData->GetLines();
  vtkIdType Outer_Line_ID = -1;

  for(vtkIdType Vertex_ID = 0; Vertex_ID < inputPolyData->GetNumberOfPoints(); Vertex_ID++)
  {
    std::cout << "Vertex ID " << Vertex_ID << std::endl;
    double Vertex_p[3];
    inputPolyData->GetPoint(Vertex_ID,Vertex_p);
    Outer_Line_ID+= 1;
    std::cout << "Outer Line ID " << Outer_Line_ID << std::endl;

    vtkIdType Line_Outer_cellLocation = 0;
    OuterLinearray->InitTraversal();
    int num_outer = 0;
    vtkIdType *Line_idlist_outer_final; 

    for (vtkIdType a = 0; a <= Outer_Line_ID; a++)
    {
      vtkIdType Line_Outer_numIds; // to hold the size of the cell
      vtkIdType *Line_Outer_idlist; // to hold the ids in the cell
      OuterLinearray->GetCell(Line_Outer_cellLocation, Line_Outer_numIds, Line_Outer_idlist);
      Line_Outer_cellLocation += 1 + Line_Outer_numIds;
      if (a == Outer_Line_ID)
        {
                   //std::cout << "Line " << Outer_Line_ID << " has " << Line_Outer_numIds << " outer points." << std::endl;
                   num_outer = Line_Outer_numIds;
                   Line_idlist_outer_final = Line_Outer_idlist;
        }
    }
 
    typedef itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;
                  InterpolatorType::Pointer Interpolator = InterpolatorType::New();
                  Interpolator->SetInputImage(inputimage);
    int count = 0;
    int stopcount = std::numeric_limits<int>::max();
    double CSFDensity = 0.0;
    int OuterFlag = 0;
    for(int a = 0; a < num_outer - 1; a++)
    {
      int PointID = Line_idlist_outer_final[a];
      double p[3];
      outerstreamlinesPolyData->GetPoint(PointID,p);
      int PointID_next = Line_idlist_outer_final[a + 1];
      double p_next[3];
      outerstreamlinesPolyData->GetPoint(PointID_next,p_next);
      double step = vtkMath::Distance2BetweenPoints(p, p_next);
      if (a == 0)
      {
        double SquareMathDist = vtkMath::Distance2BetweenPoints(p, Vertex_p);
        if (SquareMathDist > 0.001)
        {
          OuterFlag = 1;
          break;
          std::cout << "Not the correct Outer Line ID " << std::endl;
        }
      }
      typedef itk::Point< double, ImageType::ImageDimension > PointType;
      PointType point;
      point[0] = -p[0];    // x coordinate
      point[1] = -p[1];    // y coordinate
      point[2] =  p[2];    // z coordinate
      PointType point_next;
      point_next[0] = -p[0];    // x coordinate
      point_next[1] = -p[1];    // y coordinate
      point_next[2] =  p[2];    // z coordinate

      ImageType::IndexType pixelIndex;
      const bool isInside = inputimage->TransformPhysicalPointToIndex( point, pixelIndex );
      ImageType::PixelType Visited = outputimage->GetPixel(pixelIndex);
      ImageType::IndexType pixelIndex1;
      const bool isInside1 = inputMask->TransformPhysicalPointToIndex( point, pixelIndex1 );
                  ImageType::PixelType label = inputMask->GetPixel(pixelIndex1);
      ImageType::PixelType Propability = Interpolator->Evaluate(point);
                  ImageType::PixelType Propability_next = Interpolator->Evaluate(point_next);
      if(label > 0)
      {
        CSFDensity += ((Propability + Propability_next)*step)/2;  
        outputimage->SetPixel(pixelIndex, Outer_Line_ID); // Mark this pixel visited in current vertex
      }
      else
      {
        count+=1;
      }
      if (count > stopcount)
      {
        break;
      }
    }
    if (CSFDensity < 0.001)
    {
      CSFDensity = 0.0;
    }
    //std::cout << "CSFDensity = " << CSFDensity << std::endl;
    Array->InsertNextValue(CSFDensity);
    if (OuterFlag == 1)
    {
      Outer_Line_ID-=1;
    }
  }
  inputPolyData->GetPointData()->AddArray(Array);

//-----------------------------------------------CSF Density Gradient------------------------------------------------------------------


  vtkSmartPointer<vtkAppendFilter> appendFilter = vtkSmartPointer<vtkAppendFilter>::New();
  appendFilter->AddInputData(inputPolyData);
  appendFilter->Update();

  vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  unstructuredGrid->ShallowCopy(appendFilter->GetOutput());

  vtkSmartPointer<vtkGradientFilter> gradients =
    vtkSmartPointer<vtkGradientFilter>::New();
  gradients->SetInputData(unstructuredGrid);
  gradients->SetInputScalars(0,"CSFDensity");
  gradients->SetResultArrayName("CSFDensityGradient");
  gradients->Update();  

  vtkSmartPointer<vtkDoubleArray> ArrayCSFDensity = vtkDoubleArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("CSFDensity"));

  vtkSmartPointer<vtkDoubleArray> ArrayGradient = vtkDoubleArray::SafeDownCast(gradients->GetOutput()->GetPointData()->GetArray("CSFDensityGradient"));

 
  vtkSmartPointer<vtkDoubleArray> ArrayMagGradient = vtkSmartPointer<vtkDoubleArray>::New();
  ArrayMagGradient->SetNumberOfComponents(1);
  ArrayMagGradient->SetName("CSFDensityMagGradient");

  vtkSmartPointer<vtkDoubleArray> ArrayMagGradientNormalized = vtkSmartPointer<vtkDoubleArray>::New();
  ArrayMagGradientNormalized->SetNumberOfComponents(1);
  ArrayMagGradientNormalized->SetName("CSFDensityMagGradientNormalized");

  for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
  {
     double g[3]; 
     g[0] = ArrayGradient->GetComponent(vertex,0);
     g[1] = ArrayGradient->GetComponent(vertex,1);
     g[2] = ArrayGradient->GetComponent(vertex,2);
     double MagGradient = vtkMath::Norm(g);
     double MagGradientNormalized = MagGradient/ArrayCSFDensity->GetValue(vertex);


     if (MagGradient == 0.0 || isnan(MagGradient) || isnan(MagGradientNormalized))
     {
     MagGradient = 0;
     MagGradientNormalized = 0;
     }


     ArrayMagGradient->InsertNextValue(MagGradient);
     ArrayMagGradientNormalized->InsertNextValue(MagGradientNormalized);

  }
  inputPolyData->GetPointData()->AddArray(ArrayMagGradient);
  inputPolyData->GetPointData()->AddArray(ArrayMagGradientNormalized);

//-----------------------------------------------Output Results------------------------------------------------------------------
  std::string FileName = InputSurfaceFileName;
  std::string NewFileName = FileName.substr(0, FileName.size()-3);
  std::string ResultFileName = NewFileName + "CSFDensity.txt";
  ofstream Result;
  Result.open (ResultFileName.c_str());
  Result << "NUMBER_OF_POINTS=" << inputPolyData->GetNumberOfPoints() << endl; 
  Result << "DIMENSION=1" << endl;
  Result << "TYPE=Scalar" << endl;
  for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
      {
                Result << ArrayCSFDensity->GetValue(vertex) << endl;
            }
  Result.close();

  ofstream Result2;
  std::string Result2FileName = NewFileName + "CSFDensityMagGradient.txt";
  Result2.open (Result2FileName.c_str());
  Result2 << "NUMBER_OF_POINTS=" << inputPolyData->GetNumberOfPoints() << endl; 
  Result2 << "DIMENSION=1"  << endl;
  Result2 << "TYPE=Scalar" << endl;
  for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
      {
                Result2 << ArrayMagGradient->GetValue(vertex) << endl;
            }
  Result2.close();

  ofstream Result3;
  std::string Result3FileName = NewFileName + "NormalizedCSFDensityMagGradient.txt";
  Result3.open (Result3FileName.c_str());
  Result3 << "NUMBER_OF_POINTS=" << inputPolyData->GetNumberOfPoints() << endl; 
  Result3 << "DIMENSION=1"  << endl;
  Result3 << "TYPE=Scalar" << endl;
  for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
      {
                Result3 << ArrayMagGradientNormalized->GetValue(vertex) << endl;
            }
  Result3.close();

  std::string outputSurfaceFileName = OutputSurfacename;
  vtkSmartPointer<vtkPolyDataWriter> polywriter = vtkSmartPointer<vtkPolyDataWriter>::New();
  polywriter->SetFileName(outputSurfaceFileName.c_str());
  polywriter->SetInputData(inputPolyData);
  polywriter->Write();

  typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>
  BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter
  = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(outputimage);
  thresholdFilter->SetLowerThreshold(1);
  thresholdFilter->SetUpperThreshold(std::numeric_limits<PixelType>::max());
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();

  typedef itk::ImageFileWriter< ImageType > WriterType;
  WriterType::Pointer Imagewriter = WriterType::New();
  Imagewriter->SetInput(thresholdFilter->GetOutput());
  Imagewriter->SetFileName(OutputVoxelVistitingMap);
  Imagewriter->Update();

  int stop_s=clock();
  cout << "time: " << (((float)(stop_s-start_s))/CLOCKS_PER_SEC)/60 <<" min" << endl;
  return EXIT_SUCCESS;
}
