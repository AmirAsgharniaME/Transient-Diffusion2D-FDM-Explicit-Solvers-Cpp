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
#include "EssentialHeaders/KeyboardHandler.hpp"

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

//Convergence
#include "Classes2D/Convergence/RelativeResidual.hpp"


//Printer
#include "Classes2D/Printer/StatusPrinter.hpp"

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
  mesh2D.PrintXPositions();
  mesh2D.PrintYPositions();
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

//=======
//StartUp
//=======
if (ActiveScheme == SolverScheme::FTCS)
{
T_n.Swap(T_0);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
//=========================================
//FTCS is a StartUp for DuFortFrankel Method
//T_0----->Explicit FTCS------> T_1
//===========================================

  if (My_DiffNumber.GetValue() < 0.25)
      {
        Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());
        //U_n = U_0  And  U_nPlus1 = U_1
        ExplicitSolvers2D::Solve_nPlus1(T_0,T_1,Stable_DiffNumber.GetValue());
        
        //Apply Boundary Conditions To New Field
        BoundaryConditions::ApplyBoundaryCondition(T_1,TopWall,BoundaryLocation::Top);
        BoundaryConditions::ApplyBoundaryCondition(T_1,BottomWall,BoundaryLocation::Bottom);
        BoundaryConditions::ApplyBoundaryCondition(T_1,RightWall,BoundaryLocation::Right);
        BoundaryConditions::ApplyBoundaryCondition(T_1,LeftWall,BoundaryLocation::Left);

      //Swap : T_n-1 <------- T_0 And T_n <-------  T_1
        T_nMinus1.Swap(T_0);
        T_n.Swap(T_1);

      }// End of if(My_diffNumber.GetValue() < 0.25)

      if (My_DiffNumber.GetValue() >= 0.25)
      {

        ExplicitSolvers2D::Parameters_Stabilization(stabilityParams,mesh2D,alpha);

        std::size_t n = static_cast<std::size_t>(std::llround(My_dt.GetValue()/ Stable_dt.GetValue()));

        T_nMinus1 = T_0; //Saving T_n-1 = T_0

        // T_0 ----> T_1 with Lower_dt and Lower_DiffNumber
        for (size_t i = 1; i <= n; i++)
        {

        ExplicitSolvers2D::Solve_nPlus1(T_0 , T_1 , Stable_DiffNumber.GetValue());

        BoundaryConditions::ApplyBoundaryCondition(T_1,TopWall,BoundaryLocation::Top);
        BoundaryConditions::ApplyBoundaryCondition(T_1,BottomWall,BoundaryLocation::Bottom);
        BoundaryConditions::ApplyBoundaryCondition(T_1,RightWall,BoundaryLocation::Right);
        BoundaryConditions::ApplyBoundaryCondition(T_1,LeftWall,BoundaryLocation::Left);

        if (i==n)
        {
        //T_nPlus1 = T_1
        T_n.Swap(T_1);
        Stable_dt.SetValue(My_dt.GetValue());
        Stable_DiffNumber.SetValue(My_DiffNumber.GetValue());
        break;
        }

        T_0.Swap(T_1);

        }// end Of Loop
      }//End Of if (DiffusionNumber >= 0.5)
}


// ======================================
// //FortFrankel Coeffs of Discrete Algebraic Equation
// ======================================
std::vector<double> Coeffs;
if (ActiveScheme == SolverScheme::FTCS)
{}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  Coeffs = ExplicitSolvers2D::ReturnDuFortFrankelCoeffs(My_DiffNumber);
}



// ============
// Solver Loop 
// ============
std::size_t TimeLevel_start;
std::size_t TimeLevel_n;

std::size_t TimeLevel_Total = NumTimeLevels.GetValue();

// Initial TimeLevel
if (ActiveScheme == SolverScheme::FTCS)
{
  TimeLevel_start =1;
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  TimeLevel_start = 2;
}

TimeLevel_n = TimeLevel_start;

for (; TimeLevel_n <= TimeLevel_Total; TimeLevel_n++)

{

//This Condition Stops The Loop Solver Iterations by Pressing ESC
if (isEscPressed())
{
    std::cout << "\nESC pressed. Exiting program now..." << std::endl;
    return 0;
}


if (ActiveScheme == SolverScheme::FTCS)
{
//ExplicitSolver For FTCS
ExplicitSolvers2D::Solve_nPlus1(T_n, T_nPlus1 , Stable_DiffNumber.GetValue());
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  //ExplicitSolver For DuFortFrankel
  ExplicitSolvers2D::Solve_nPlus1(T_n,T_nMinus1,T_nPlus1,Coeffs);
}




//Apply Boundary Conditions To New Field
BoundaryConditions::ApplyBoundaryCondition(T_nPlus1,TopWall,BoundaryLocation::Top);
BoundaryConditions::ApplyBoundaryCondition(T_nPlus1,BottomWall,BoundaryLocation::Bottom);
BoundaryConditions::ApplyBoundaryCondition(T_nPlus1,RightWall,BoundaryLocation::Right);
BoundaryConditions::ApplyBoundaryCondition(T_nPlus1,LeftWall,BoundaryLocation::Left);

//Claculate The Relative Residual
double Residual = RelativeResidual::ReturnResidual(T_n,T_nPlus1);

if (ActiveScheme == SolverScheme::FTCS)
{
  StatusPrinter::PrintStepStatus(TimeLevel_n,Residual,Stable_dt);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
  StatusPrinter::PrintStepStatus(TimeLevel_n,Residual,My_dt);
}


//Updade
//T <--- Tnew : T=Tnew Copy The Values But Swap does not Copy
if (ActiveScheme == SolverScheme::FTCS)
{
  T_n.Swap(T_nPlus1);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
T_nMinus1.Swap(T_n);
T_n.Swap(T_nPlus1);
}

// Record data for plotting
stepHistory.push_back(static_cast<double>(TimeLevel_n));
residualHistory.push_back(Residual);

// Update plot every 20 steps
if (TimeLevel_n % 20 == 0) 
{
    plotter.updatePlot(stepHistory, residualHistory);
}


//Codition For Convergence To stady state Solution
if(Residual < Tolerance.GetValue())
{

if (ActiveScheme == SolverScheme::FTCS)
{
StatusPrinter::PrintConvergenceStatus(TimeLevel_n,Residual,Stable_dt);
}
else if (ActiveScheme == SolverScheme::DUFORT_FRANKEL)
{
StatusPrinter::PrintConvergenceStatus(TimeLevel_n,Residual,My_dt);
}

FileWriter2D::WriteField2D(T_n,mesh2D,FieldType::Steady_State_Numerical,RelativePath);
break;
}

}
//End Of Loop Solver*************


// =========================================================================
// 3. Common Post-Processing
// =========================================================================
OutputPlotter2D outputPlotter(mesh2D, "Solution Comparison (" + schemeName + ")");
outputPlotter.plotComparison(
    RelativePath + "/Initial_Numerical2D.dat", "Initial Condition",
    RelativePath + "/Steady_State_Numerical2D.dat", "Steady State Numerical Solution",
    RelativePath + "/Steady_State_Analytical2D.dat", "Steady State Analytical Solution"
);

    std::cout << "Calculations Completed Successfully for " << schemeName << std::endl;
    std::cin.get();
    return 0;
}
