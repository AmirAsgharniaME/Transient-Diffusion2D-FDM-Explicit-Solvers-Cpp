#include "Classes2D/Field2D/Field2D.hpp"

#include <algorithm>

Field2D::Field2D(const Mesh2D& Mesh2D_Obj,double Default_Initial_Value_)
    :nrows(Mesh2D_Obj.GetNrows()),
     ncols(Mesh2D_Obj.GetNcols()),
     FieldValues(nrows,std::vector<double>(ncols,Default_Initial_Value_))
    
{}

 /*****Public Setters*****/ 
void Field2D::SetValue(std::size_t jIndex_, std::size_t iIndex_, double Value_)
{
   FieldValues[jIndex_][iIndex_] = Value_;
}

void Field2D::SetAllSingleValue(double Value_)
{
    for (std::vector<double>& Row : FieldValues)
    {
        std::fill(Row.begin(), Row.end(), Value_);
    }
}

void Field2D::SetAll2DValue(const std::vector<std::vector<double> >& Value2D_)
{
    for (std::size_t j = 0; j < nrows ; j++)
    {  
        for (std::size_t i = 0; i < ncols; i++)
        {
            FieldValues[j][i] = Value2D_[j][i];
        }  
    }
}

void Field2D::SetSideValues(const BoundaryLocation Location, const std::vector<double>& BoundaryValues_)
{
    if (Location == BoundaryLocation::Top)
    {
        std::size_t j = 0;
        for (std::size_t i = 0; i < ncols; i++)
        {
            FieldValues[j][i] = BoundaryValues_[i];
        }
    }

    else if (Location == BoundaryLocation::Bottom)
    {
        std::size_t j = nrows-1;
        for (std::size_t i = 0; i < ncols; i++)
        {
            FieldValues[j][i] = BoundaryValues_[i];
        }
    }

    else if (Location == BoundaryLocation::Right)
    {
        std::size_t i = ncols-1;
        for (std::size_t j = 0; j < nrows; j++)
        {
            FieldValues[j][i] = BoundaryValues_[j];
        }
    }

    else if (Location == BoundaryLocation::Left)
    {
        std::size_t i = 0;
        for (std::size_t j = 0; j < nrows; j++)
        {
            FieldValues[j][i] = BoundaryValues_[j];
        }
    }

}



    /*****Public Getters*****/ 
std::size_t Field2D::GetNrows() const
{
 return nrows;
}
std::size_t Field2D::GetNcols() const
{
    return ncols;
}

double Field2D::GetValue(std::size_t jIndex_, std::size_t iIndex_) const
{

    return FieldValues[jIndex_][iIndex_];
}


//Print Method
void Field2D::Print(const Mesh2D& Mesh2D_Obj)
{
    const std::ios::fmtflags oldFlags = std::cout.flags();
    const std::streamsize oldPrecision = std::cout.precision();

    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.precision(3);

    std::cout << "Field2D (" << nrows << " rows x "
              << ncols << " columns)\n\n";

    std::cout << "       x ->";
    for (std::size_t i = 0; i < ncols; ++i)
    {
        std::cout.width(12);
        std::cout << Mesh2D_Obj.GetXPositions(i);
    }
    std::cout << '\n';

    for (std::size_t j = 0; j < nrows; ++j)
    {
        const std::size_t yIndex = nrows - 1 - j;

        std::cout << "y = ";
        std::cout.width(7);
        std::cout << Mesh2D_Obj.GetYPositions(yIndex) << " |";

        for (std::size_t i = 0; i < ncols; ++i)
        {
            std::cout.width(12);
            std::cout << FieldValues[j][i];
        }
        std::cout << '\n';
    }

    std::cout.flags(oldFlags);
    std::cout.precision(oldPrecision);
}





//Swap Method

void Field2D::Swap(Field2D& Other) noexcept
{
    FieldValues.swap(Other.FieldValues);
}

void swap(Field2D& Left, Field2D& Right) noexcept
{
    Left.Swap(Right);
}

