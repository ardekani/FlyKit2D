// Tracker: Given a stream of measurement (2-D points), returns the association IDs of new measurement (assuming previous were ordered as 1,2,3...).
// It should be run independent for each view, and later "matched" with MatchViews

#ifndef __FKTRACKER_H_INCLUDED__
#define __FKTRACKER_H_INCLUDED__

//#include "Definitions.h" // To include OpenCv and standard header files
//#include "Utility.h" // For euclidDistance() function
//#include "hungarian.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "fkTypes.h"
#define Point3D CvPoint3D64f /// Redefinition of CvPoint3D64f to a more human readable name
#define Point2D CvPoint2D64f /// Redefinition of CvPoint2D64f to a more human readable name

double calculateDir(Point2D p1, Point2D p2); // Returns the angle of line joining points p1-p2

using namespace std;

class FkTracker
{
private:
	int targetNum; // Number of flies 
	int minDisThreshold; // Target "easy" or "hard"
	double mixingWeight; // mixingWeight = Weight for direction of fly ..... (1-mixingWeight) == Weight for distance

public:
	bool readyToCorrespond;
	vector<vector<Point2D> > points2D;
	vector<vector<Point3D> > points3D;
	vector <int> assignmentVec;
	
	FkTracker(void);
	~FkTracker(void);

	// The functions below are intended to be called in the orde rthey are arranged, at the end of Correspond() read the assignmentVec for result
	void setMixingWeight(double mw);
	void setTargetNum(int);
	void getNewMeasurement(vector<Point2D> measurements);
	void getNewMeasurement(vector<Point3D> measurements);
	FkInt32S Track(vector<int> currOrder);
	
   void hungCorrespondOf2Sets (std::vector<Point2D> set1, std::vector<Point2D>set2, std::vector<int>&assignment1,std::vector<int>&assignment2);
   void hungCorrespondOf2Sets (std::vector<Point3D> set1, std::vector<Point3D>set2, std::vector<int>&assignment1,std::vector<int>&assignment2);

	
	double getMixingWeight(void); // 

	//Some functions for internal use right now
	void findEasyTargets(vector<Point2D> targetPos, vector<bool> &isAnEasyTarget);
	void findEasyTargets(vector<Point3D> targetPos, vector<bool> &isAnEasyTarget);


};

#endif // __TRACKER_H_INCLUDED__