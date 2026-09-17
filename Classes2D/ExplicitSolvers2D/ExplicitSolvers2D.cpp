#include "Classes2D/ExplicitSolvers2D/ExplicitSolvers2D.hpp"

   void ExplicitSolvers2D::Parameters_Stabilization
   (
    StabilityParams& Params,
    const Mesh2D& Mesh2D_obj,
    const ThermophysicalProperties& alpha_Obj)
{


// Stable_dt_Obj and Stable_DiffNumber are initialized with my inputs
    double Lower_dt = Params.My_dt_Obj->GetValue();
    double Lower_DiffNumber = Params.My_DiffNumber_Obj->GetValue();
    double deltaY = Mesh2D_obj.GetdeltaY();
    double alpha = alpha_Obj.GetValue();

  std::cout << "Diffusion Number limit is 0.5 but Current Diffusion Number = "<< Params.My_DiffNumber_Obj->GetValue() << '\n';
  std::cout << "FTCS is Unstable with dt = " <<Params.My_dt_Obj->GetValue()<< '\n';

        double Reduction_Factor = 0.95;

        while (Lower_DiffNumber >= 0.25)
        {
        Lower_dt= Reduction_Factor * Lower_dt;
        Lower_DiffNumber = alpha * Lower_dt /(deltaY * deltaY);
        }

        std::cout << "dt is has been reduced for the stability of FTCS method." << '\n';

        Params.Stable_dt_Obj->SetValue(Lower_dt);
        Params.Stable_DiffNumber_Obj->SetValue(Lower_DiffNumber);

        std::cout <<" New dt = "<<Lower_dt<<'\n';
        std::cout <<" New Diffusin Number = "<<Lower_DiffNumber<<'\n';

  }

void ExplicitSolvers2D::Solve_nPlus1(const Field2D& Field2D_n_Obj,Field2D& Field2D_nPlus1_Obj , double Stable_Diffusion_Number_)
{
    std::size_t nrows = Field2D_n_Obj.GetNrows();
    std::size_t ncols = Field2D_n_Obj.GetNcols();

    //FTCS 2D explicit Solve for n+1 for Internal nodes
    for (std::size_t j = 1; j < nrows - 1; ++j)
    {
        for (std::size_t i = 1; i < ncols - 1; ++i)
        {
            double center = Field2D_n_Obj.GetValue(j, i);
            double dxx = Field2D_n_Obj.GetValue(j, i + 1) - 2.0 * center + Field2D_n_Obj.GetValue(j, i - 1);
            double dyy = Field2D_n_Obj.GetValue(j + 1, i) - 2.0 * center + Field2D_n_Obj.GetValue(j - 1, i);
            double RHS = center + Stable_Diffusion_Number_ * (dxx + dyy);
            Field2D_nPlus1_Obj.SetValue(j, i, RHS);
        }
    }
}




void ExplicitSolvers2D::Solve_nPlus1(const Field2D& Field2D_n_Obj,const Field2D& Field2D_nminus1_Obj,Field2D& Field2D_nplus1_Obj,std::vector<double> CoeffsVector_)
  {

std::size_t nrows = Field2D_n_Obj.GetNrows();
std::size_t ncols = Field2D_n_Obj.GetNcols();

for (std::size_t j = 1; j < nrows-1; j++)
{
   for (std::size_t i = 1; i < ncols-1; i++)
   {
         // DuFort-Frankel 2D explicit Solve for n+1 for Internal nodes
         double RHS = CoeffsVector_[0] * Field2D_nminus1_Obj.GetValue(j,i)
         + CoeffsVector_[1] * (Field2D_n_Obj.GetValue(j + 1,i)+ Field2D_n_Obj.GetValue(j - 1,i)) 
         + CoeffsVector_[2] * (Field2D_n_Obj.GetValue(j,i+1) + Field2D_n_Obj.GetValue(j,i-1));
         Field2D_nplus1_Obj.SetValue(j,i,RHS);
   }
}

  }


  //FortFrankel Coeffs
  std::vector<double> ExplicitSolvers2D::ReturnDuFortFrankelCoeffs(const SolutionParameters<double>& My_Diff_Number_Obj)
   {
       

        double r = My_Diff_Number_Obj.GetValue();
        double C_Old = (1.0 - 2.0 * r)/(1.0 + 2.0 * r);
        double C = (2.0 * r)/(1.0 + 2.0 * r);
        


     std::vector<double> CoeffsVector = {C_Old, C};


     return CoeffsVector;

   }