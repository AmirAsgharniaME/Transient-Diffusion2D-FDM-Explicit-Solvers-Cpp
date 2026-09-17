#pragma once

#include "Classes2D/Field2D/Field2D.hpp"
#include "Classes2D/AnalyticalSolution2D/AnalyticalDiffusion2D.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iomanip>

enum class FieldType
{
    Initial_Numerical,
    Steady_State_Numerical,
    Steady_State_Analytical,
};

class FileWriter2D
{
public:
    FileWriter2D() =delete;

   static void WriteField2D(
    Field2D& Field2D_Obj, 
    Mesh2D& Mesh2D_Obj,
    FieldType FieldType_,
    std::string Path);

   static void WriteField2D(
    AnalyticalDiffusion2D& Analytical2D_Obj,
    Mesh2D& Mesh2D_Obj ,
    FieldType FieldType_,
    std::string Path);
   

};

