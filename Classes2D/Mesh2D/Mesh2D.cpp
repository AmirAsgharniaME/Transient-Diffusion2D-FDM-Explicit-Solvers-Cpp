#include "Classes2D/Mesh2D/Mesh2D.hpp"

#include <cstddef>
#include <iostream>
#include <stdexcept>



Mesh2D::Mesh2D(
    const Geometry& Geometry_Obj,
    std::size_t Default_NumOfXNodes_,
    std::size_t Default_NumOfYNodes_
    ):NumOfXNodes(Default_NumOfXNodes_),
     NumOfYNodes(Default_NumOfYNodes_),        
     NumOfXIntervals(NumOfXNodes - 1),
     NumOfYIntervals(NumOfYNodes - 1),
     nrows(NumOfYNodes),
     ncols(NumOfXNodes),
     Height(Geometry_Obj.GetHeight()),
     Width(Geometry_Obj.GetWidth()),
     deltaX(Width / static_cast<double>(NumOfXIntervals)),
     deltaY(Height / static_cast<double>(NumOfYIntervals)),
     XPositions(NumOfXNodes,0.0),
     YPositions(NumOfYNodes,0.0)
{
    DiscretizeXDirection();
    DiscretizeYDirection();
}



/***************Public Getters********************/
std::size_t Mesh2D::GetNumOfXNodes() const
{
    return NumOfXNodes;
}
std::size_t Mesh2D::GetNumOfYNodes() const
{
    return NumOfYNodes;
}

std::size_t Mesh2D::GetNumOfXIntervals() const
{
    return NumOfXIntervals;
}

std::size_t Mesh2D::GetNumOfYIntervals() const
{
    return NumOfYIntervals;
}

std::size_t Mesh2D::GetNrows() const
{return NumOfYNodes;}

std::size_t Mesh2D::GetNcols() const
{return NumOfXNodes;}


double Mesh2D::GetdeltaX() const
{
    return deltaX;
}

double Mesh2D::GetdeltaY() const
{
    return deltaY;
}

double Mesh2D::GetXPositions(std::size_t i) const
{
    return XPositions.at(i);
}
double Mesh2D::GetYPositions(std::size_t j) const
{
    return YPositions.at(j);
}

const std::vector<double>& Mesh2D::GetXPositionVector() const
{
return XPositions;
}
const std::vector<double>& Mesh2D::GetYPositionVector() const
{
return YPositions;
}
/***************Public Setters********************/

void Mesh2D::SetNumOfXNodes(std::size_t NumOfXNodes_)
{
    if (NumOfXNodes_ < 2)
    {
        throw std::invalid_argument("The number of mesh nodes must be at least 2.");
    }

    NumOfXNodes = NumOfXNodes_;
    NumOfXIntervals = NumOfXNodes - 1;
    ncols = NumOfXNodes;
    deltaX = Width / static_cast<double>(NumOfXIntervals);
    XPositions.resize(NumOfXNodes);
    DiscretizeXDirection();
}
void Mesh2D::SetNumOfYNodes(std::size_t NumOfYNodes_)
{
    if (NumOfYNodes_ < 2)
    {
        throw std::invalid_argument("The number of mesh nodes must be at least 2.");
    }

    NumOfYNodes = NumOfYNodes_;
    NumOfYIntervals = NumOfYNodes - 1;
    nrows = NumOfYNodes;
    deltaY = Height / static_cast<double>(NumOfYIntervals);
    YPositions.resize(NumOfYNodes);
    DiscretizeYDirection();
}




//Private Methods
void Mesh2D::DiscretizeXDirection()
{
    for (std::size_t Index = 0; Index < XPositions.size(); ++Index)
    {
        XPositions[Index] = static_cast<double>(Index) * deltaX;
    }
}

void Mesh2D::DiscretizeYDirection()
{
    for (std::size_t Index = 0; Index < YPositions.size(); ++Index)
    {
        YPositions[Index] = static_cast<double>(Index) * deltaY;
    }
}


//Public Print Values
void Mesh2D::PrintXPositions() const
{
    for (std::size_t Index = 0; Index < XPositions.size(); ++Index)
    {
        std::cout << XPositions[Index] << " ";
    }

    std::cout << '\n';
}

void Mesh2D::PrintYPositions() const
{
    for (std::size_t Index = 0; Index < YPositions.size(); ++Index)
    {
        std::cout << YPositions[Index] << " ";
    }

    std::cout << '\n';
}


void Mesh2D::Print() const
{
    const std::ios::fmtflags oldFlags = std::cout.flags();
    const std::streamsize oldPrecision = std::cout.precision();

    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.precision(3);

    std::cout << "Mesh2D (" << nrows << " rows x " << ncols << " columns)\n"
              << "Spacing: deltaX = " << deltaX
              << ", deltaY = " << deltaY << "\n\n";

    std::cout << "       x ->";
    for (double x : XPositions)
    {
        std::cout.width(12);
        std::cout << x;
    }
    std::cout << '\n';

    for (std::size_t row = YPositions.size(); row-- > 0;)
    {
        std::cout << "y = ";
        std::cout.width(7);
        std::cout << YPositions[row] << " |";

        for (std::size_t column = 0; column < XPositions.size(); ++column)
        {
            std::cout.width(12);
            std::cout << "o";
        }
        std::cout << '\n';

        if (row != 0)
        {
            std::cout << "              ";
            for (std::size_t column = 0; column < XPositions.size(); ++column)
            {
                std::cout.width(12);
                std::cout << "|";
            }
            std::cout << '\n';
        }
    }

    std::cout.flags(oldFlags);
    std::cout.precision(oldPrecision);
}






