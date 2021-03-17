#include "HeatKernelSmoothingCLP.h"
#include <vtkGenericDataObjectReader.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include "vtkIdTypeArray.h"


int  main(int argc, char** argv) 
{
    PARSE_ARGS;
    std::string inputSurfaceFilename = InputSurface;
	vtkSmartPointer<vtkPolyDataReader> surfacereader = vtkSmartPointer<vtkPolyDataReader>::New();
	surfacereader->SetFileName(inputSurfaceFilename.c_str());
	surfacereader->Update();

	vtkPolyData* inputPolyData = surfacereader->GetOutput();

	for (int iter = 0; iter < Numberofiterations; iter++)
	{	
		
		vtkSmartPointer<vtkDoubleArray> CurrentCSFDensity = vtkDoubleArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("CSFDensity"));
		//vtkSmartPointer<vtkFloatArray> CurrentCSFDensity = vtkFloatArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("EACSF"));
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
      			std::cout << squaredDistance2 << std::endl;
      			double weight = exp(-squaredDistance2/(2*sigma*sigma))/sum;
				AvgCSFDensity =AvgCSFDensity + (CurrentCSFDensity->GetValue(connectedVertices->GetId(ID))) * weight;
			}
			SmoothedCSFDensity->InsertNextValue(AvgCSFDensity);
		}
		inputPolyData->GetPointData()->AddArray(SmoothedCSFDensity);
	}

	vtkSmartPointer<vtkPolyDataWriter> SurfaceWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
    SurfaceWriter->SetInputData(inputPolyData);
    SurfaceWriter->SetFileName(OutputSurface.c_str());
    SurfaceWriter->Update();
    return EXIT_SUCCESS;
}

