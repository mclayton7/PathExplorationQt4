//Point3 Class for FindPath Project
// Mac Clayton 2012

#include <iostream>
#include "Point3.h"

using namespace std;

int Point3::W = 0;
int Point3::H = 0;

Point3::Point3(int x0, int y0, int z0)
: x(x0), y(y0), z(z0)
{
    ind = z * W * H + y * W + x;
}

Point3::Point3(int x0, int y0)
: x(x0), y(y0), z(0)
{
    ind = z * W * H + y * W + x;
}

Point3::Point3(int i)
: ind(i)
{
    z = ind / (W * H);
    y = (ind - z * W * H) / W;
    x = ind % W;
}

int Point3::GetX() const
{
    return x;
}

int Point3::GetY() const
{
    return y;
}

int Point3::GetZ() const
{
    return z;
}

int Point3::GetInd() const
{
    return ind;
}

void Point3::SetW(int w0)
{
    W = w0;
}

void Point3::SetH(int h0)
{
    H = h0;
}