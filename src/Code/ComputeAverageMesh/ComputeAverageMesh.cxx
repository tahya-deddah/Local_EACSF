#include "ComputeAverageMeshCLP.h"
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>


int  main(int argc, char** argv) 
{
    PARSE_ARGS;

    vtkSmartPointer<vtkPolyDataReader> meshin1 = vtkSmartPointer<vtkPolyDataReader>::New();
	meshin1->SetFileName(inputFilename1.c_str());
	meshin1->Update();
	vtkSmartPointer<vtkPoints> meshPoints1 = vtkSmartPointer<vtkPoints>::New();
	meshPoints1=meshin1->GetOutput()->GetPoints();

	vtkSmartPointer<vtkPolyDataReader> meshin2 = vtkSmartPointer<vtkPolyDataReader>::New();
	meshin2->SetFileName(inputFilename2.c_str());
	meshin2->Update();
	vtkSmartPointer<vtkPoints> meshPoints2 = vtkSmartPointer<vtkPoints>::New();
	meshPoints2=meshin2->GetOutput()->GetPoints();


	vtkSmartPointer<vtkPolyData> polydata = meshin1->GetOutput();
    vtkSmartPointer<vtkPoints> avgPoints = vtkSmartPointer<vtkPoints>::New();
	for( unsigned int pointID = 0; pointID < meshPoints2->GetNumberOfPoints(); pointID++ )
	{
		double avgPoint[3];
		double Point1[3];
		double Point2[3];
        meshPoints1->GetPoint(pointID, Point1);
	    meshPoints2->GetPoint(pointID, Point2);
        for( unsigned int dim = 0; dim < 3; dim++ )
        {
		    avgPoint[dim] = ( Point1[dim] + Point2[dim] ) / 2.0 ;
	    }
	    avgPoints->InsertPoint(pointID, avgPoint);
	}

    polydata->SetPoints(avgPoints);
    vtkSmartPointer<vtkPolyDataWriter> SurfaceWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
    SurfaceWriter->SetInputData(polydata);
    SurfaceWriter->SetFileName(outputFilename.c_str());
    SurfaceWriter->Update();

return EXIT_SUCCESS;
}

    
