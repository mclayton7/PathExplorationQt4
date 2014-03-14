// ECE3090 Final Project - Obstacle avoidance
//
// Mac Clayton 2012
// 
// v1.0
//
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <map>
#include <vector>
#include <cstdlib>
#include "qt4display.h"
#include "PointClass/Point2.cc"
#include "PointClass/Point3.cc"

using namespace std;

//Test different data types for efficiency:
typedef unsigned int Index_t;
typedef double Data_t;

//Predefined Distance Constants
const Data_t SQRT2 = sqrt(2);
const Data_t SQRT3 = sqrt(3);


//Rotates the avatar to the given rotation
vector<Point2> RotateImage(vector<Point2>& in, int rotation)
{
	vector<Point2> out(in);
	//make rotation from 0-350
	rotation = rotation%36;
	if(rotation < 0) rotation += 36;
	rotation *= -1; // clockwise to ccw
	double angle = rotation*10*M_PI/180; //degrees to radians
	double cosr = cos(angle); //precompute
	double sinr = sin(angle); //precompute
	for(unsigned i = 0; i < in.size(); ++i)
	{
        out[i] = Point2(-(int)round( (-in[i].GetX() * cosr) + (in[i].GetY() * sinr)), 
                         (int)round((in[i].GetX() * sinr) + (in[i].GetY() * cosr)));
	}
	return out;
}

//checks to see if a given point is valid
bool CheckValid(vector<Point2>& in, int x, int y, char* map)
{	
	//Iterate through the avatar (stored inside in):
	for(Index_t i = 0; i < in.size(); ++i)
	{
		Point2 point = in[i];
		int mapx = point.GetX() + x;
		int mapy = point.GetY() + y;
        //If the pixel is black:
		if((mapx < 0) || (mapx > 255)) return false;
		if((mapy < 0) || (mapy > 255)) return false;
        if(map[ mapy * 256 + mapx ] == 0) return false;
	}
	return true;
}

//FindPath Function
bool FindPath(int start_x,              //Start X
              int start_y,              //Start Y
              int start_rot,            //Start angle
              int end_x,                //End X
              int end_y,                //End Y
              int end_rot,              //End angle
			  char* avatarData,        //Ptr to avatar data
              char* courseData,        //Ptr to course data
              vector<Point3>& path,     //vector of path costs
              Data_t *pathLength,         //pathLength
              vector< vector<Point2> >& avatars) // Add parameters as necessary
{
	//Make sure original point is on map
	if( (start_x > 255) || (start_x < 0) ) return false;
	if( (start_y > 255) || (start_y < 0) ) return false;
	if( (end_x > 255) || (end_x < 0) ) return false;
	if( (end_y > 255) || (end_y < 0) ) return false;
    
    
	//Generate global distance array and fill it with "infinite distance" (10e10):
    vector<vector<vector<Data_t> > > distance (256, vector<vector<Data_t> > (256, vector<Data_t> (36, 10e10)));
    
    //Set the distance from the end to the end (yes, it's zero)
    end_rot /= 10;
	distance[end_x][end_y][end_rot] = 0;

	//fix start_rotation to 0-350
    start_rot = (start_rot/10)%36;
	if(start_rot < 0) start_rot += 36;

	//Create and reserve a vector to hold the avatar:
	vector<Point2> points;
    points.reserve(64);
	//Fill up the vector:
	for(Index_t x = 0; x < 32; ++x)
	{
		for(Index_t y = 0; y < 32; ++y)
		{
			if(avatarData[y * 32 + x] == 0)
			{
				points.push_back(Point2((x - 16), (y - 16)));
			}
		}
	}

	//precompute all 36 avatar rotations
	for(Index_t k = 0; k < 36; ++k)
	{
		avatars.push_back(RotateImage(points, k));	
	}

	//pending container
	multimap<Data_t, Point3> pending; 
	//Insert the end point into the pending array:
	pending.insert(pair<Data_t, Point3>(0, Point3(end_x, end_y, end_rot)));
	
	//Set the found flag to false:
	bool found = false;
	//Find the end:
	while(pending.size() != 0)
	{
        //Grab and then erase the first pair of elements out of pending:
		Data_t thisDist = pending.begin()->first;
		Point3 current = pending.begin()->second;
		pending.erase(pending.begin());
        
		//If the first element is the starting point, then the shortest path has been found:
		if ((current.GetX() == start_x) && (current.GetY() == start_y) && (current.GetZ() == start_rot))
		{ 
			found = true;
			*pathLength = thisDist;
			break;
		}
        //if i've already checked this point, skip it
		if (thisDist > distance[current.GetX()][current.GetY()][current.GetZ()]) continue; 
		//Possible optimizations here because it actually checks 27 and not 26 spots (possibly fixed by line160):
		int num_moves = 0;
		Data_t dist = 0;
		for(int x = -1; x <= 1; ++x)
		{
			int curr_x = current.GetX() + x;
			if((curr_x < 0) || (curr_x > 255)) continue; //out of bounds

			for(int y = -1; y <= 1; ++y)
			{
				int curr_y = y + current.GetY();
				if((curr_y < 0) || (curr_y > 255)) continue; //out of bounds
				for(int r = -1; r <= 1; ++r)
				{
					int curr_r = current.GetZ() + r;
					num_moves = 0;
					if (x) ++num_moves;
					if (y) ++num_moves;
					if (r) ++num_moves;
                    //this is where I already am
					if(num_moves == 0) continue;
					//set rotation 0-350 (really 0-35)
					if(curr_r < 0) curr_r = 35;
					if(curr_r > 35) curr_r = 0;
                    //make sure I'm valid
					if (!CheckValid(avatars[curr_r], curr_x, curr_y, courseData)) continue; 
					if (num_moves == 1) dist = 1;			
					if (num_moves == 2) dist = SQRT2;
					if (num_moves == 3) dist = SQRT3;
					dist += thisDist;
					if (distance[curr_x][curr_y][curr_r] > dist)
					{ 
                        // If I'm closer
						distance[curr_x][curr_y][curr_r] = dist;
						pending.insert(pair<Data_t, Point3>(dist, Point3(curr_x, curr_y, curr_r)));
					}
				}
			}
		}	
	}
    
    //If we haven't found a path:
	if(!found)
	{
		return false;
	}
	//Populate the path
	Point3 spot(start_x, start_y, start_rot);
	Point3 next(spot);
	Point3 end(end_x, end_y, end_rot);
	path.push_back(spot);
	while((spot.GetX() != end.GetX()) || (spot.GetY() != end.GetY() || (spot.GetZ() != end.GetZ())))
	{
		//Find the minimum distance
		Data_t min_dist = 10e10;
		for(int x = -1; x <= 1; ++x)
		{
			int curr_x = spot.GetX() + x;
			if((curr_x < 0) || (curr_x > 255)) continue;
			for(int y = -1; y <= 1; ++y)
			{
				int curr_y = y + spot.GetY();
				if((curr_y < 0) || (curr_y > 255)) continue;
				for(int r = -1; r <= 1; ++r)
				{
					int curr_r = spot.GetZ() + r;
					if(curr_r < 0) curr_r = 35;
					if(curr_r > 35) curr_r = 0;
					if(distance[curr_x][curr_y][curr_r] <= min_dist)
					{
						min_dist = distance[curr_x][curr_y][curr_r];
                        next = Point3(curr_x, curr_y, curr_r);
					}
				}
			}
		}
		spot = next;
		path.push_back(spot);
	}
	return true; 
}


//Welcome to Main:
int main(int argc, char** argv)
{    
    if (argc < 8)
    {
        cout <<  "Usage: placeavatar obstacles.png avatar.png x y theta"
             << " target-x target-y target-theta" << endl;
        exit(1);
    }
    //Create the application:
    QApp app(argc, argv);

    //Load images here
    QDisplay course(app);
    course.Load(argv[1]);
    char* courseData = (char*) course.ImageData();
    QDisplay avatar(app);
    avatar.Load(argv[2]);
    char* avatarData = (char*) avatar.ImageData();
    //Create and reserve space for all 36 avatars:
    vector< vector<Point2> > avatars;
    avatars.reserve(36);
    //Initial and final x/y/z positions
    int ix     = atol(argv[3]);
    int iy     = atol(argv[4]);
    int itheta = atol(argv[5]);
    int tx     = atol(argv[6]);
    int ty     = atol(argv[7]);
    int ttheta = atol(argv[8]);
    
    cout << "Avatar " << argv[2] 
         << " start at x " << ix
         << " y " << iy
         << " z " << itheta
         << " target x " << tx
         << " y " << ty
         << " z " << ttheta
    << endl;

    // Get the starting real-time clock for timing computation
    struct timeval tp;
    double startSec;
    gettimeofday(&tp, 0);
    startSec = tp.tv_sec + tp.tv_usec/1000000.0;
    
    //Creat and reeserve enough space for the final path:
    vector<Point3> finalpath;
    finalpath.reserve(512);
    double pathLength;
    
    //The function call to FindPath:
    bool found = FindPath(ix,			//Initial X
                          iy,			//Initial Y
                          itheta,		//Initial Angle
                          tx,			//Final X
                          ty,			//Final Y
                          ttheta,		//FInal Angle
                          avatarData,	//Avatar png
                          courseData,	//Course png
                          finalpath,	//Final Path (vector of point 3s
                          &pathLength,	//Address of the pathLength variable
                          avatars);		//Array to hold all of the rotated avatars
    
    gettimeofday(&tp, 0);
    cout << "Calculated path in "
         << (tp.tv_sec+tp.tv_usec/1000000.0) - startSec << " seconds" << endl;
    
    if (found)
    {
        course.Show();
        cout << "distance to target is " <<  pathLength << endl;
        course.UpdateRate(24);
        for(Index_t i = 0; i < finalpath.size(); ++i)
        {
            Point3 loc = finalpath[i];
            for(Index_t k = 0; k < avatars[loc.GetZ()].size(); ++k)
            {
                courseData[(avatars[loc.GetZ()][k].GetY()+loc.GetY()) * 256 + (avatars[loc.GetZ()][k].GetX()+loc.GetX())] = 0;
            }
            course.Update( loc.GetX() - 20 , loc.GetY() - 20, 40, 40);
            // course.Update();
            for(Index_t k = 0; k<avatars[loc.GetZ()].size(); ++k)
            {
                //Erase course data
                courseData[(avatars[loc.GetZ()][k].GetY()+loc.GetY()) * 256 + (avatars[loc.GetZ()][k].GetX()+loc.GetX())] = (char)-1;
            }
        }//End iterating through the path
    }//End if(found)
    else
    {
        cout << "No existing path" << endl;
    }
    // Call Run() here
    app.Run();
}

  
  
