/*===========
C++ Libraries
=============*/ 
#include <iostream>
#include <vector>
#include <cmath> 
#include <iomanip>
#include <thread> // Added for optional small delays


/*======
Headers
========*/

//Stops The Loop Solver Iterations by Pressing ESC
//#include "EssentialHeaders/KeyboardHandler.hpp"

//Inputs
#include "EssentialHeaders/SolverInputs.hpp"


/*=======
Classes
========*/

//Geometry
#include "Classes2D/Geometry/Geometry.hpp"

//Thermophysical_Properties
#include "Classes2D/Thermophysical_Properties/ThermophysicalProperties.hpp"

//SolverSettings
#include "Classes2D/SolverSettings/SolutionParameters.hpp"

//Mesh
#include "Classes2D/Mesh2D/Mesh2D.hpp"

//Field
#include "Classes2D/Field2D/Field2D.hpp"

//InitialConditions
#include "Classes2D/InitialConditions/InitialConditions.hpp"

//Boundaries
#include "Classes2D/Boundaries/Boundary.hpp"

//BoundaryConditions
#include "Classes2D/BoundaryConditions/BoundaryConditions.hpp"

//AnalyticalSolution2D
#include "Classes2D/AnalyticalSolution2D/AnalyticalDiffusion2D.hpp"

//FileWriter2D
#include "Classes2D/FileWriter2D/FileWriter2D.hpp"

//Plotter
#include "Classes2D/Plotter/ResidualPlotter.hpp"
#include "Classes2D/Plotter/OutputPlotter2D.hpp"

//Solvers
#include "Classes2D/ExplicitSolvers2D/ExplicitSolvers2D.hpp"

// //Convergence
// #include "Classes2D/Convergence/RelativeResidual.hpp"









// //Printer
// #include "Classes2D/Printer1D/Printer1D.hpp"

// =============================================================================
// Solver Selection: Choose scheme directly here in the editor
// =============================================================================
enum class SolverScheme 
{
    FTCS,
    DUFORT_FRANKEL
};

constexpr SolverScheme ActiveScheme = SolverScheme::FTCS;
//constexpr SolverScheme ActiveScheme = SolverScheme::DUFORT_FRANKEL;

int main() 
{
const std::string schemeName = (ActiveScheme == SolverScheme::FTCS) 
                                  ? "FTCS" 
                                  : "DUFORT_FRANKEL";

std::cout << "Selected Scheme: " << schemeName << std::endl;

// =========================================================================
// Setup (Geometry, Properties, Mesh, Conditions)
// =========================================================================

//===========
//Geometry 2D
//===========

Geometry Plate;
Plate.SetHeight(SolverInputs::Height_Value);
Plate.SetWidth(SolverInputs::Width_Value);


//======================
//ThermophysicalProperties
//========================
ThermophysicalProperties alpha;    //ThermalDiffusivity
alpha.SetValue(SolverInputs::alpha_Value);


//============
//Solver Settings
//===============
SolutionParameters<std::size_t> NumTimeLevels;
SolutionParameters<double> My_dt;
SolutionParameters<double> Stable_dt;
//SolutionParameters<double> UnStable_dt;

SolutionParameters<double> My_DiffNumber;
SolutionParameters<double> Stable_DiffNumber;
//SolutionParameters<double> UnStable_DiffNumber;

SolutionParameters<double> Tolerance;


NumTimeLevels.SetValue(SolverInputs::NumTimeStep_Value);
My_dt.SetValue(SolverInputs::dt_Value);
Stable_dt.SetValue(My_dt.GetValue());
//UnStable_dt.SetValue(My_dt.GetValue());


Tolerance.SetValue(SolverInputs::Tolerance_Value);

//========
//Mesh2D
//========

  Mesh2D mesh2D(Plate);
  mesh2D.SetNumOfXNodes(SolverInputs::NumOfXNodes_Value);
  mesh2D.SetNumOfYNodes(SolverInputs::NumOfYNodes_Value);
  mesh2D.Print();

//=================
//Diffusion Nymber
//=================
My_DiffNumber.SetValue((alpha.GetValue() * My_dt.GetValue())/(mesh2D.GetdeltaY()*mesh2D.GetdeltaY()));
Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());
//UnStable_DiffNumber.SetValue(My_DiffNumber.GetValue());


//========
//Field2D
//========
Field2D T_0(mesh2D);
Field2D T_1(mesh2D);
Field2D T_n(mesh2D);
Field2D T_nPlus1(mesh2D);
Field2D T_nMinus1(mesh2D); //using For DUFORT_FRANKEL




//=====================================
//Apply Initial Conditions To Field1D T_0
//======================================
InitialConditions::ApplyInitialCondition(T_0, SolverInputs::Initial_Plate_Temp);



//====================
//Boundaries For Field2D
//======================
Boundary TopWall(mesh2D,BoundaryOrientation::Horizontal);
Boundary BottomWall(mesh2D, BoundaryOrientation::Horizontal);
Boundary RightWall(mesh2D, BoundaryOrientation::Vertical);
Boundary LeftWall(mesh2D, BoundaryOrientation::Vertical);

//Boundary Setters
TopWall.SetBoundaryValues(SolverInputs::TopWall_Temp_Values);
BottomWall.SetBoundaryValues(SolverInputs::BottomWall_Temp_Values);
RightWall.SetBoundaryValues(SolverInputs::RightWall_temp_Values);
LeftWall.SetBoundaryValues(SolverInputs::LeftWall_Temp_Values);


//=======================================
//Apply Boundary Conditions To Field1D T_0
//=========================================
BoundaryConditions::ApplyBoundaryCondition(T_0,TopWall,BoundaryLocation::Top);
BoundaryConditions::ApplyBoundaryCondition(T_0,BottomWall,BoundaryLocation::Bottom);
BoundaryConditions::ApplyBoundaryCondition(T_0,RightWall,BoundaryLocation::Right);
BoundaryConditions::ApplyBoundaryCondition(T_0,LeftWall,BoundaryLocation::Left);

T_0.Print(mesh2D);


AnalyticalDiffusion2D U_Analytical_2D(mesh2D, Plate, TopWall, BottomWall, RightWall, LeftWall);


std::string RelativePath = "OutPutData/" + schemeName;

FileWriter2D::WriteField2D(T_0, mesh2D,FieldType::Initial_Numerical, RelativePath);
FileWriter2D::WriteField2D(U_Analytical_2D, mesh2D,FieldType::Steady_State_Analytical, RelativePath);


ResidualPlotter plotter("CFD Convergence: Diffusion 1D (" + schemeName + ")");
std::vector<double> stepHistory;
std::vector<double> residualHistory;


// =========
// Stability
// =========

StabilityParams stabilityParams = {
    &My_dt,
    &My_DiffNumber,
    &Stable_dt,
    &Stable_DiffNumber
};


if (ActiveScheme == SolverScheme::FTCS)
{
    if (My_DiffNumber.GetValue() < 0.25)
        {
          Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());
          std::cout<<"FTCS Scheme is stable"<<'\n';
        }
    else if (My_DiffNumber.GetValue() >= 0.25)
        {
          ExplicitSolvers2D::Parameters_Stabilization(stabilityParams,mesh2D,alpha);
      
        }
}

else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  std::cout<<" DUFORT_FRANKEL Scheme is unconditionally stable"<<'\n';
}

/*
//=======
//StartUp
//=======
if (ActiveScheme == SolverScheme::FTCS)
{
U_n.Swap(U_0);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
//=========================================
//FTCS is a StartUp for DuFortFrankel Method
//U_0----->Explicit FTCS------> U_1
//===========================================

  if (My_DiffNumber.GetValue() < 0.5)
      {
        Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());
        //U_n = U_0  And  U_nPlus1 = U_1
        ExplicitSolvers1D::Solve_Diffusion1D(U_0,U_1,Stable_DiffNumber.GetValue());
        
        //Apply Boundary Conditions To New Field
        BoundaryConditions::ApplyBoundaryCondition(U_1,mesh1D,TopWall);
        BoundaryConditions::ApplyBoundaryCondition(U_1,mesh1D,BottomWall);
      //Swap : U_n-1 <------- U_0 And U_n <-------  U_1
        U_nMinus1.Swap(U_0);
        U_n.Swap(U_1);

      }// End of if(My_diffNumber.GetValue() < 0.5)

      if (My_DiffNumber.GetValue() >= 0.5)
      {
        SolutionParameters<double> Temp_Stable_dt;
        SolutionParameters<double> Temp_Stable_DiffNumber;
        Temp_Stable_dt.SetValue(my_dt.GetValue());
        Temp_Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());

        ExplicitSolvers1D::Reset_Stability_Params(Temp_Stable_dt,Temp_Stable_DiffNumber,mesh1D,nu);
        std::size_t n = static_cast<std::size_t>(std::llround(my_dt.GetValue()/ Temp_Stable_dt.GetValue()));

        U_nMinus1 = U_0; //Saving U_n-1 = U_0

        // U_0 ----> U_1 with Lower_dt and Lower_DiffNumber
        for (size_t i = 1; i <= n; i++)
        {


        ExplicitSolvers1D::Solve_Diffusion1D(U_0 , U_1 , Temp_Stable_DiffNumber.GetValue());

        BoundaryConditions::ApplyBoundaryCondition(U_1 ,mesh1D,TopWall);
        BoundaryConditions::ApplyBoundaryCondition(U_1 ,mesh1D,BottomWall);

        if (i==n)
        {
        //U_nPlus1 = U_1
        U_n.Swap(U_1);
        break;
        }

        U_0.Swap(U_1);
        }// end Of Loop
      }//End Of if (DiffusionNumber >= 0.5)
}


// ======================================
// Coffes of Discrete Algebraic Equation
// ======================================
std::vector<double> Coeffs;
if (ActiveScheme == SolverScheme::FTCS)
{}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  Coeffs = ExplicitSolvers1D::ReturnCoeffs(My_DiffNumber);
}



// ==========================
// Solver Loop 
// ===========================
std::size_t TimeLevel;

// Initial TimeLevel
if (ActiveScheme == SolverScheme::FTCS)
{
  TimeLevel =1;
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  TimeLevel = 2;
}

for (; TimeLevel < NumTimeLevels.GetValue(); TimeLevel++)

{

//This Condition Stops The Loop Solver Iterations by Pressing ESC
if (isEscPressed())
{
    std::cout << "\nESC pressed. Exiting program now..." << std::endl;
    return 0;
}


if (ActiveScheme == SolverScheme::FTCS)
{
ExplicitSolvers1D::Solve_Diffusion1D(U_n, U_nPlus1 , Stable_DiffNumber.GetValue());
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  ExplicitSolvers1D::Solve_Diffusion1D(U_n,U_nMinus1,U_nPlus1,Coeffs);
}


//ExplicitSolver For FTCS

//Apply Boundary Conditions To New Field
BoundaryConditions::ApplyBoundaryCondition(U_nPlus1,mesh1D,TopWall);
BoundaryConditions::ApplyBoundaryCondition(U_nPlus1,mesh1D,BottomWall);

//Claculate The Relative Residual
double Residual = RelativeResidual::ReturnResidual(U_n,U_nPlus1);

if (ActiveScheme == SolverScheme::FTCS)
{
  Printer1D::PrintStepStatus(TimeLevel,Residual,Stable_dt);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  Printer1D::PrintStepStatus(TimeLevel,Residual,my_dt);
}


//Updade
//U <--- Unew : U=Unew Copy The Values But Swap does not Copy
if (ActiveScheme == SolverScheme::FTCS)
{
  U_n.Swap(U_nPlus1);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
U_nMinus1.Swap(U_n);
U_n.Swap(U_nPlus1);
}



// Record data for plotting
stepHistory.push_back(static_cast<double>(TimeLevel));
residualHistory.push_back(Residual);

// Update plot every 20 steps
if (TimeLevel % 20 == 0) 
{
    plotter.updatePlot(stepHistory, residualHistory);
}


//Codition For Convergence To stady state Solution
if(Residual < Tolerance.GetValue())
{

if (ActiveScheme == SolverScheme::FTCS)
{
Printer1D::PrintConvergenceStatus(TimeLevel,Residual,Stable_dt);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
Printer1D::PrintConvergenceStatus(TimeLevel,Residual,my_dt);
}

FileWriter1D::WriteField1D(U_n,mesh1D,"Steady_State_U_1D",FilePath);          
break;
}

}
//End Of Loop Solver*************


// =========================================================================
// 3. Common Post-Processing
// =========================================================================
OutputPlotter2D outputPlotter("Solution Comparison (" + schemeName + ")");
outputPlotter.plotComparison(
    FilePath + "/U0_1D.dat", "Initial Condition",
    FilePath + "/Steady_State_U_1D.dat", "Steady State Numerical Solution",
    FilePath + "/U_Analytical_1D.dat", "Steady State Analytical Solution"
);
*/
    std::cout << "Calculations Completed Successfully for " << schemeName << std::endl;
    std::cin.get();
    return 0;
}
