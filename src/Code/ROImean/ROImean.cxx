#include "ROImeanCLP.h"
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <string>   
#include<fstream>    


int  main(int argc, char** argv) 
{
    PARSE_ARGS;

    char     line[70];
    std::vector<std::string> atlas_array;
    std::vector<std::string> measurement_array;
    std::ifstream inputmeasurement, inputatlas ;
    inputmeasurement.open(InputMeasurement, ios::in);
    inputatlas.open(AtlasSurfaceLabeling, ios::in);
    // read 3 first lines
    inputmeasurement.getline(line, 500, '\n');
    inputmeasurement.getline(line, 500, '\n');
    inputmeasurement.getline(line, 500, '\n');

    while (inputatlas.getline(line, 500, '\n'))                          
    {   atlas_array.push_back(line);  }

    while (inputmeasurement.getline(line, 500, '\n'))                          
    {   measurement_array.push_back(line);  }

    sort( atlas_array.begin(), atlas_array.end() );
    atlas_array.erase( std::unique( atlas_array.begin(), atlas_array.end() ), atlas_array.end() );

    int roinum = atlas_array.size();
    int subjnum = measurement_array.size();

    double ROImean[subjnum][roinum] ;
    double roisize[roinum][1];
    double indx[1][1];


   /* for (int i=0 ; i<roinum ; i++)
    {
       roisize[i][0] = 
    }

    for i=1:1:roinum;
        roisize(i)=size(find(atlas==rois(i)),1);
    end;*/
	return EXIT_SUCCESS;    
}


    
    