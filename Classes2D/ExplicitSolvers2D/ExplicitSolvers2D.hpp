#pragma once

#include <cstddef>
#include <iostream>
#include "Classes2D/Field2D/Field2D.hpp"
#include "Classes2D/Thermophysical_Properties/ThermophysicalProperties.hpp"
#include "Classes2D/SolverSettings/SolutionParameters.hpp"
#include "Classes2D/Mesh2D/Mesh2D.hpp"

struct StabilityParams 
{
    const SolutionParameters<double>* My_dt_Obj;
    const SolutionParameters<double>* My_DiffNumber_Obj;
    SolutionParameters<double>* Stable_dt_Obj;
    SolutionParameters<double>* Stable_DiffNumber_Obj;
};


class ExplicitSolvers2D
{
public:
    ExplicitSolvers2D() =delete;


//FTCS
static void Parameters_Stabilization
   (StabilityParams& Params,
    const Mesh2D& Mesh2D_obj,
    const ThermophysicalProperties& alpha_Obj);
static void Solve_nPlus1(const Field2D& Field2D_n_Obj,Field2D& Field2D_nPlus1_Obj , double Stable_Diffusion_Number_);

//DuFortFrankel
static void Solve_nPlus1(const Field2D& Field2D_n_Obj,const Field2D& Field2D_nminus1_Obj,Field2D& Field1D_nplus1_Obj,std::vector<double> CoeffsVector_);
static std::vector<double> ReturnDuFortFrankelCoeffs(const SolutionParameters<double>& My_Diff_Number);

};
