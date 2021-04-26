#include "AddScalarstoPolyDataCLP.h"
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <string>   
#include<fstream>    


int  main(int argc, char** argv) 
{
    PARSE_ARGS;
   
	vtkSmartPointer<vtkPolyDataReader> polyIn = vtkSmartPointer<vtkPolyDataReader>::New();
    polyIn->SetFileName(InputFileName.c_str());
    polyIn->Update();
    vtkSmartPointer<vtkPolyData> polydataAtt = polyIn->GetOutput();

    char     line[70];
    std::ifstream input;
    int      NPoints, NDimension;
    char *   aux;
    input.open(ScalarsFilename, ios::in);

    input.getline(line, 500, '\n');
    aux = strtok(line, " = ");
    aux = strtok(NULL, " = ");
    NPoints = atof(aux);
   
    input.getline(line, 500, '\n');
    aux = strtok(line, " = ");
    aux = strtok(NULL, " = ");
    NDimension = atof(aux);

    input.getline(line, 500, '\n'); // read type line (Scalars)

    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetNumberOfComponents(NDimension);
    scalars->SetName(ScalarsName.c_str());
    std::cout<< ScalarsName.c_str() << std::endl;

    for( int i = 0; i < NPoints; i++ )
    {   
        
        input.getline(line, 500, '\n');
        float value=0;
        std::string proc_string = line;
        std::istringstream iss(proc_string);
        float *tuple = new float[NDimension];
        int count = 0 ;
        do
        {
            std::string sub ;
            iss >> sub ;
            if( sub !=  "" )
            {
                if( count >= NDimension )
                {
                    std::cerr << "Error in input file format: one line contains too many components" << std::endl ;
                    return 1 ;
                }
                value = atof(sub.c_str());
                tuple[ count ] = value ;
                count++ ;
            }
        }while( iss ) ;
        if( count != NDimension )
        {
            std::cerr << "Error in input file format: one line does not contain enough components" << std::endl ;
            std::cerr << "Line: " << line << std::endl;
            std::cerr << "Components found: " << count << std::endl ;
            return 1 ;
        }
        scalars->InsertNextTuple( tuple ) ;
    }

    input.close();
  
    polydataAtt->GetPointData()->AddArray(scalars);
  
    // Writing the new mesh
    vtkSmartPointer<vtkPolyDataWriter> SurfaceWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
    SurfaceWriter->SetInputData(polydataAtt);
    SurfaceWriter->SetFileName(OutputFileName.c_str());
    SurfaceWriter->Update();

    return EXIT_SUCCESS;
    
}


    
    