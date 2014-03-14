//Point2 Class for FindPath Project
// Mac Clayton 2012

#include <iostream>
using namespace std;

class Point2
{
public:
    Point2(int x0, int y0);
    Point2(int i0);
    
    //Accessor Functions:
    int GetX() const;
    int GetY() const;
    int GetInd() const;
    
private:
    //Private member variables:
    int x;
    int y;
    int ind;
    
private:
    static int W;
    static int H;
    
public:
    static void SetW(int w0);
    static void SetH(int h0);
};