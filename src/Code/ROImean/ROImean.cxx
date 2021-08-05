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
#include <vector>


int  main(int argc, char** argv) 
{
    PARSE_ARGS;


    // lists to stock the CSF measurement and atlas values
    std::vector<std::string> atlas_array;
    std::vector<std::string> measurement_array;
    std::string line;
    std::ifstream atlas_file( AtlasSurfaceLabeling.c_str());
    if (atlas_file.is_open())
    {
        
        while (std::getline(atlas_file, line)) 
        {
            atlas_array.push_back(line); 
        }
    }
    std::ifstream input_measurement_file( InputMeasurement.c_str());
    if (input_measurement_file.is_open())
    {
        // read three first lines of the csf measurement txt file
        std::getline(input_measurement_file, line);
        std::getline(input_measurement_file, line);
        std::getline(input_measurement_file, line);

        std::string atlas_line;
        while (std::getline(input_measurement_file, line)) 
        {
            measurement_array.push_back( line); 
        }
    }

     /// ROI
    std::vector<std::string> roi = atlas_array;
    auto last = std::unique(roi.begin(), roi.end());
    roi.erase(last, roi.end());
    std::sort(roi.begin(), roi.end()); 
    last = std::unique(roi.begin(), roi.end());
    roi.erase(last, roi.end());

    // initialization
    int roinum = roi.size();
    //int subjnum = 1;
    

    double ROI_Mean[roinum] ;
    double ROI_Sum[roinum] ;
    int roisize[roinum];
    int indx[measurement_array.size()];

    // calculate number of times ROI labels appears in atlas to calculate size of ROI
  
    for( int i=0; i < roinum ; i++ )
    {
        int Roi_Number = 0;
        for(int j=0; j < atlas_array.size() ; j++)
        {
            if(atlas_array[j] == roi[i])
            {
                Roi_Number = Roi_Number + 1;
            }
        }
        roisize[i] = Roi_Number;
    }

    // For each value of CSF measurement array return the index of roi that correspond to that region in atlas
   
    for( int i=0; i < measurement_array.size() ; i++ )
    {
        for(int j=0; j < roinum ; j++)
        {
            if(roi[j] == atlas_array[i]) 
            {    
                indx[i] = j;
            }
        }
    }

	// compute the regional value of csf for each region 
    for (int j=0; j < measurement_array.size() ; j++)
    {
        ROI_Mean[indx[j]] = 0;
        ROI_Sum[indx[j]] = 0;
    }
    for( int j=0; j < measurement_array.size() ; j++ )
    {
        ROI_Mean[indx[j]] = ROI_Mean[indx[j]] + (std::atof(measurement_array[j].c_str())/roisize[indx[j]]); 
        ROI_Sum[indx[j]] = ROI_Sum[indx[j]] + std::atof(measurement_array[j].c_str());     
    }
    // put the regional values in the output file

    ofstream Result;
    Result.open (OutputFileName1.c_str());
    Result << "NUMBER_OF_POINTS=" << measurement_array.size() << endl; 
    Result << "DIMENSION=1" << endl;
    Result << "TYPE=Scalar" << endl;
    for (int j=0; j < measurement_array.size() ; j++)
    {
        Result << ROI_Mean[indx[j]] << endl;      
    }
    Result.close();


    ofstream Result2;
    Result2.open (OutputFileName2.c_str());
    Result2 << "NUMBER_OF_POINTS=" << measurement_array.size() << endl; 
    Result2 << "DIMENSION=1" << endl;
    Result2 << "TYPE=Scalar" << endl;
    for (int j=0; j < measurement_array.size() ; j++)
    {
        Result2 << ROI_Sum[indx[j]] << endl;      
    }
    Result2.close();

    return EXIT_SUCCESS;    
}


    
    