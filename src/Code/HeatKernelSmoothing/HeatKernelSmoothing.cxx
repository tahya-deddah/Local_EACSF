#include "HeatKernelSmoothingCLP.h"
#include <vtkGenericDataObjectReader.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkGradientFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>
#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include "vtkIdTypeArray.h"


int  main(int argc, char** argv) 
{
    PARSE_ARGS;
    //std::string inputSurfaceFilename = InputSurface;
	vtkSmartPointer<vtkPolyDataReader> surfacereader = vtkSmartPointer<vtkPolyDataReader>::New();
	//surfacereader->SetFileName(inputSurfaceFilename.c_str());
	surfacereader->SetFileName(InputSurface.c_str());
	surfacereader->Update();

	vtkPolyData* inputPolyData = surfacereader->GetOutput();

	//- - -------------------------------------------  Smoothed CSF Density ----------------------------------------------------------- //

	for (int iter = 0; iter < Numberofiterations; iter++)
	{	
		
		vtkSmartPointer<vtkDoubleArray> CurrentCSFDensity = vtkDoubleArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("CSFDensity"));
		vtkSmartPointer<vtkDoubleArray> SmoothedCSFDensity = vtkSmartPointer<vtkDoubleArray>::New();
		SmoothedCSFDensity->SetNumberOfComponents(1);
		SmoothedCSFDensity->SetName("CSFDensity");
		for(vtkIdType seed = 0; seed < inputPolyData->GetNumberOfPoints(); seed++)
		{
			
			vtkSmartPointer<vtkIdList> connectedVertices = vtkSmartPointer<vtkIdList>::New(); 
		    vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
		    connectedVertices->InsertNextId(seed);
		    inputPolyData->GetPointCells(seed, cellIdList);

		    for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
		    {
		    	vtkCell* cell = inputPolyData->GetCell(cellIdList->GetId(i));
		      	if(cell->GetNumberOfEdges() <= 0)
		      	{
		        	continue;
		      	}
		      	for(vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++)
		      	{
		      		vtkCell* edge = cell->GetEdge(e);
					vtkIdList* pointIdList = edge->GetPointIds();

		        	if(pointIdList->GetId(0) == seed || pointIdList->GetId(1) == seed)
		        	{
		        		if(pointIdList->GetId(0) == seed)
		          		{
		          			connectedVertices->InsertNextId(pointIdList->GetId(1));
		          		}
		          		else
		          		{
		           			connectedVertices->InsertNextId(pointIdList->GetId(0));
		          		}
		        	}
		      	}
		    }
		    double sum = 0;
    		double seed_p1[3];
    		inputPolyData->GetPoint(seed,seed_p1);
    		for(vtkIdType ID = 0; ID < connectedVertices->GetNumberOfIds(); ID++)
    		{
    			double seed_np1[3];
      			inputPolyData->GetPoint(connectedVertices->GetId(ID),seed_np1);
      			double squaredDistance1 = vtkMath::Distance2BetweenPoints(seed_p1, seed_np1);
      			sum = sum + exp(-squaredDistance1/(2*sigma*sigma));
			}
			double AvgCSFDensity = 0;
    		double seed_p2[3];
    		inputPolyData->GetPoint(seed,seed_p2);
    		for(vtkIdType ID = 0; ID < connectedVertices->GetNumberOfIds(); ID++)
    		{
    			double seed_np2[3];
      			inputPolyData->GetPoint(connectedVertices->GetId(ID),seed_np2);
      			double squaredDistance2 = vtkMath::Distance2BetweenPoints(seed_p2, seed_np2);
      			double weight = exp(-squaredDistance2/(2*sigma*sigma))/sum;
				AvgCSFDensity =AvgCSFDensity + (CurrentCSFDensity->GetValue(connectedVertices->GetId(ID))) * weight;
			}
			SmoothedCSFDensity->InsertNextValue(AvgCSFDensity);
		}
		inputPolyData->GetPointData()->AddArray(SmoothedCSFDensity);
	}

	//- - -------------------------------------------  Smoothed CSF Density Gradient ----------------------------------------------------------- //
	
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
	   

	    if (MagGradient == 0.0 || isnan(MagGradient))
	    {
	    	MagGradient = 0;
	    }

	    ArrayMagGradient->InsertNextValue(MagGradient);
	}
	inputPolyData->GetPointData()->AddArray(ArrayMagGradient);
	
	//- - -------------------------------------------update txt files ----------------------------------------------------------- //

	vtkSmartPointer<vtkDoubleArray> array = vtkDoubleArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("CSFDensity"));
	std::string FileName = InputSurface;
	
	std::string NewFileName = FileName.substr(0, 3);
	std::string ResultFileName = NewFileName + "MID.CSFDensity.txt";
	ofstream Result;
	Result.open(ResultFileName.c_str(), ofstream::trunc);
	Result << "NUMBER_OF_POINTS=" << inputPolyData->GetNumberOfPoints() << endl; 
	Result << "DIMENSION=1" << endl;
	Result << "TYPE=Scalar" << endl;
	for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
	{
		Result << array->GetValue(vertex) << endl;
	}
	Result.close();

	ofstream Result2;
	std::string Result2FileName = NewFileName + "MID.CSFDensityMagGradient.txt";
	Result2.open (Result2FileName.c_str(), ofstream::trunc);
	Result2 << "NUMBER_OF_POINTS=" << inputPolyData->GetNumberOfPoints() << endl; 
	Result2 << "DIMENSION=1"  << endl;
	Result2 << "TYPE=Scalar" << endl;
	for(vtkIdType vertex = 0; vertex < inputPolyData->GetNumberOfPoints(); vertex++)
	{
	    Result2 << ArrayMagGradient->GetValue(vertex) << endl;
	}
	Result2.close();

	vtkSmartPointer<vtkPolyDataWriter> SurfaceWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
    SurfaceWriter->SetInputData(inputPolyData);
    SurfaceWriter->SetFileName(OutputSurface.c_str());
    SurfaceWriter->Update();
    return EXIT_SUCCESS;
}

