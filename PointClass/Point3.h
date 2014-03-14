//Point2 Class for FindPath Project
// Mac Clayton 2012

#include <iostream>
using namespace std;

class Point3
{
public:
    Point3(int x0, int y0, int z0);
    Point3(int x0, int y0); // z = 0
    Point3(int i0);
    
    //Accessor Functions:
    int GetX() const;
    int GetY() const;
    int GetZ() const;
    int GetInd() const;
    
private:
    //Private member variables:
    int x;
    int y;
    int z;
    int ind;
    
private:
    static int W;
    static int H;
    
public:
    static void SetW(int w0);
    static void SetH(int h0);
};