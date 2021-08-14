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
#include <itkMinimumMaximumImageCalculator.h>

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

  /////////// new

  typedef itk::MinimumMaximumImageCalculator <ImageType> MinimumMaximumImageCalculatorType ;
  MinimumMaximumImageCalculatorType::Pointer MinimumMaximumImage = MinimumMaximumImageCalculatorType::New();
  MinimumMaximumImage->SetImage(Imagereader->GetOutput());   
  MinimumMaximumImage->ComputeMaximum();
  std::cout << MinimumMaximumImage->GetMaximum() << std::endl;
  /////// 


  typedef itk::ImageRegionIterator< ImageType>       IteratorType;
  IteratorType      inputIt1(Imagereader->GetOutput(), Imagereader->GetOutput()->GetRequestedRegion());
  IteratorType      inputIt2(inputimage, inputimage->GetRequestedRegion());
  inputIt1.GoToBegin();
  inputIt2.GoToBegin();
  while (!inputIt1.IsAtEnd())
  {
        //inputIt2.Set(double(inputIt1.Get())/double(std::numeric_limits<unsigned short>::max()));
        inputIt2.Set(double(inputIt1.Get())/MinimumMaximumImage->GetMaximum());
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


  //// Initializing Voxel visiting Length
  ImageType::Pointer visitation_length = ImageType::New();
  visitation_length->CopyInformation(inputimage);
  visitation_length->SetRegions(inputimage->GetRequestedRegion());
  visitation_length->Allocate();

  IteratorType      outputIt2(visitation_length, visitation_length->GetRequestedRegion());
  outputIt2.GoToBegin();
  while (!outputIt2.IsAtEnd())
  {
    outputIt2.Set(0.0);
    ++outputIt2;
  }

  ///
  std::string inputMaskFilename = InputMaskFileName;
  typedef itk::ImageFileReader< ImageType >  MaskReaderType;
  MaskReaderType::Pointer Maskreader = MaskReaderType::New();
  Maskreader->SetFileName(inputMaskFilename.c_str());
  Maskreader->Update();
  ImageType::Pointer inputMask = Maskreader->GetOutput();


/// ---------------------------------------------- visitation map and visitation length -----------------------------------------------------------------

  vtkSmartPointer<vtkCellArray> OuterLinearray = outerstreamlinesPolyData->GetLines();
  vtkIdType Outer_Line_ID = -1;

  for(vtkIdType Vertex_ID = 0; Vertex_ID < inputPolyData->GetNumberOfPoints(); Vertex_ID++)
  {
  
    double Vertex_p[3];
    inputPolyData->GetPoint(Vertex_ID,Vertex_p);
    Outer_Line_ID+= 1;
   
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

    int count = 0;
    int stopcount = std::numeric_limits<int>::max();
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
      point_next[0] = -p_next[0];    // x coordinate
      point_next[1] = -p_next[1];    // y coordinate
      point_next[2] =  p_next[2];    // z coordinate

      

      ImageType::IndexType pixelIndex;
      const bool isInside = inputimage->TransformPhysicalPointToIndex( point, pixelIndex );
      ImageType::IndexType pixelIndex1;
      const bool isInside1 = inputMask->TransformPhysicalPointToIndex( point, pixelIndex1 );
      ImageType::PixelType label = 0;

      if(isInside && isInside1){
        label = inputMask->GetPixel(pixelIndex1);
      }
                  
      if(label > 0)
      {        
       //new 
        ImageType::PixelType steps_length = visitation_length->GetPixel(pixelIndex);
        steps_length = steps_length + step;
        visitation_length->SetPixel(pixelIndex, steps_length );
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
    if (OuterFlag == 1)
    {
      Outer_Line_ID-=1;
    }
  }
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //-----------------------------------------------Estimate CSF Density------------------------------------------------------------------

  vtkSmartPointer<vtkDoubleArray> Array = vtkSmartPointer<vtkDoubleArray>::New();
  Array->SetNumberOfComponents(1);
  Array->SetName("CSF_Density_Final");


  Outer_Line_ID = -1;

  for(vtkIdType Vertex_ID = 0; Vertex_ID < inputPolyData->GetNumberOfPoints(); Vertex_ID++)
  {
  
    double Vertex_p[3];
    inputPolyData->GetPoint(Vertex_ID,Vertex_p);
    Outer_Line_ID+= 1;
   
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
      point_next[0] = -p_next[0];    // x coordinate
      point_next[1] = -p_next[1];    // y coordinate
      point_next[2] =  p_next[2];    // z coordinate



      ImageType::IndexType pixelIndex;
      const bool isInside = inputimage->TransformPhysicalPointToIndex( point, pixelIndex );
      ImageType::IndexType pixelIndex1;
      const bool isInside1 = inputMask->TransformPhysicalPointToIndex( point, pixelIndex1 );
      ImageType::PixelType Propability = Interpolator->Evaluate(point);
      ImageType::PixelType Propability_next = Interpolator->Evaluate(point_next);
      ImageType::PixelType label = 0;
      if(isInside && isInside1){
        label = inputMask->GetPixel(pixelIndex1);
      }
                  
      if(label > 0)
      {        
       //new 
        ImageType::PixelType lenght_of_steps = visitation_length->GetPixel(pixelIndex); 
        if(lenght_of_steps == 0) {CSFDensity += 0;} 
        else
        {
          double  ratio = step/lenght_of_steps;            
          CSFDensity += ((Propability + Propability_next)*ratio)/2.0;
          //CSFDensity+= ((Propability + Propability_next)*step)/2.0;  
        }          
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
  gradients->SetInputScalars(0,"CSF_Density_Final");
  gradients->SetResultArrayName("CSF_Density_Gradient");
  gradients->Update();  

  vtkSmartPointer<vtkDoubleArray> ArrayCSFDensity = vtkDoubleArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("CSF_Density_Final"));

  vtkSmartPointer<vtkDoubleArray> ArrayGradient = vtkDoubleArray::SafeDownCast(gradients->GetOutput()->GetPointData()->GetArray("CSF_Density_Gradient"));

 
  vtkSmartPointer<vtkDoubleArray> ArrayMagGradient = vtkSmartPointer<vtkDoubleArray>::New();
  ArrayMagGradient->SetNumberOfComponents(1);
  ArrayMagGradient->SetName("CSF_Density_Mag_Gradient");

  for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
  {
     double g[3]; 
     g[0] = ArrayGradient->GetComponent(vertex,0);
     g[1] = ArrayGradient->GetComponent(vertex,1);
     g[2] = ArrayGradient->GetComponent(vertex,2);
     double MagGradient = vtkMath::Norm(g);
     

    if (MagGradient == 0.0 || isnan(MagGradient) )
    {
     MagGradient = 0;
    }
    ArrayMagGradient->InsertNextValue(MagGradient);
  }
  inputPolyData->GetPointData()->AddArray(ArrayMagGradient);
 
//-----------------------------------------------Output Results------------------------------------------------------------------
  std::string FileName = InputSurfaceFileName;
  std::string NewFileName = FileName.substr(0, FileName.size()-4);
  std::string ResultFileName = NewFileName + "_CSF_Density.txt";
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


  typedef itk::ImageFileWriter< ImageType > WriterType;
  WriterType::Pointer Imagewriter1 = WriterType::New();
  Imagewriter1->SetInput(visitation_length);
  Imagewriter1->SetFileName(OutputVoxelVistitingLength);
  Imagewriter1->Update();

  int stop_s=clock();
  cout << "time: " << (((float)(stop_s-start_s))/CLOCKS_PER_SEC)/60 <<" min" << endl;
  return EXIT_SUCCESS;
}
