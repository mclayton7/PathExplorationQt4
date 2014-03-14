//Point2 Class for FindPath Project
// Mac Clayton 2012

#include <iostream>
#include "Point2.h"

using namespace std;

int Point2::W = 0;
int Point2::H = 0;

Point2::Point2(int x0, int y0)
: x(x0), y(y0)
{
    ind = y * W + x;
}

Point2::Point2(int i)
: ind(i)
{
    y = ind / W;
    x = ind % W;
}

int Point2::GetX() const
{
    return x;
}

int Point2::GetY() const
{
    return y;
}

int Point2::GetInd() const
{
    return ind;
}

void Point2::SetW(int w0)
{
    W = w0;
}

void Point2::SetH(int h0)
{
    H = h0;
}