#include "fkTracker.h"
#include "fkerrors.h"
#include "Munkres\Munkres.h"
#include <iostream>
#define PI 3.14159265359
double euclidDistance(Point2D a,Point2D b)
{
	return (sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) ) );
}

double euclidDistance(Point3D a,Point3D b)
{
	return (sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z) ) );
}

template <typename T> int find_in_vector(std::vector<T> V,T key) // Return the index of particular key in a vector, return -1 if not found
{
	for(int i=0;i<(int)V.size();i++)
		if(V[i]==key)
			return i;
	return -1;
}




double calculateDir(Point2D p1, Point2D p2)
{
	double m;
	m = (180*atan2((p2.x - p1.x) ,(p2.y - p1.y)))/PI;
	return m;
}


FkTracker::FkTracker(void):mixingWeight(0),\
targetNum(3),\
readyToCorrespond(false),\
minDisThreshold(20)
{
	//if(nFlies<=0) // This means nFlies has not been set up yet
	//	targetNum = 3; // Default value for nFlies (should be resized later using setTargetNum later)
}

FkTracker::~FkTracker(void)
{
}

void FkTracker::setTargetNum(int a)
{
	targetNum = a;

}

double FkTracker::getMixingWeight()
{
	return mixingWeight;
}

void FkTracker::setMixingWeight(double mw)
{
	mixingWeight = mw;
}

void FkTracker::getNewMeasurement(vector<Point2D> measurements)
{
    printf("\nBegingetNewMeasurement");
    for (size_t i = 0;i<measurements.size();i++)
        printf("\nmeasurement[%d] = (%.2lf, %.2lf)",i,measurements[i].x,measurements[i].y);


	if (points2D.size()<3)
	{
		/*for(int i=0;i<targetNum;i++)
		assignmentVec[i] = -1;*/
		//	printf("\nPoint2d. size() = %d",points2D.size());
		points2D.push_back(measurements);
		readyToCorrespond = false;
	}
	else
	{
		readyToCorrespond = true;
		points2D.push_back(measurements);
		points2D.erase(points2D.begin());
	};



	if(points2D.size()>3)
		printf("\n SOME THING IS WRONG IN GETNEWMEASUREMENT FUNCTION!"); // proly an assert would work better


        printf("\nEnd getNewMeasurement");
}


void FkTracker::getNewMeasurement(vector<Point3D> measurements)
{
	printf("\n*** in get measurement 3DD****\n");
	for(int i=0;i<measurements.size();i++)
		printf("(%.2lf,%.2lf,%.2lf)\n",measurements[i].x,measurements[i].y,measurements[i].z);

	if (points3D.size()<3)
	{
		readyToCorrespond = false;
		points3D.push_back(measurements);
	}
	else
	{
		readyToCorrespond = true;
		points3D.push_back(measurements);
		points3D.erase(points3D.begin());
	};

	if(points3D.size()>3)
		printf("\n SOME THING IS WRONG IN GETNEWMEASUREMENT FUNCTION!"); // proly an assert would work better
}

void FkTracker::findEasyTargets(vector<Point3D> targetPos, vector<bool> &isAnEasyTarget)
{
	// this function checks the minimum distance between each target and others and declares it as an easy target if the distance is big enough
	for (int i = 0;i<targetPos.size();i++)
	{
		double minDis = INT_MAX;
		for (int j = 0;j<targetPos.size();j++)
		{
			if(i != j) //to not check a point against itself
				if (euclidDistance(targetPos[i],targetPos[j]) < minDis)
					minDis = euclidDistance(targetPos[i],targetPos[j]); 
		}
		isAnEasyTarget.push_back((bool)(minDis > minDisThreshold));
	}
}

void FkTracker::findEasyTargets(vector<Point2D> targetPos, vector<bool> &isAnEasyTarget)
{
	// this function checks the minimum distance between each target and others and declares it as an easy target if the distance is big enough
	for (int i = 0;i<targetPos.size();i++)
	{
		double minDis = INT_MAX;
		for (int j = 0;j<targetPos.size();j++)
		{
			if(i != j) //to not check a point against itself
				if (euclidDistance(targetPos[i],targetPos[j]) < minDis)
					minDis = euclidDistance(targetPos[i],targetPos[j]); 
		}
		isAnEasyTarget.push_back((bool)(minDis > minDisThreshold));
	}
}

FkInt32S FkTracker::Track(vector<int> currOrder)
{ 	
    printf("\n FkTracker::Track(vector<int> currOrder) begin");
	assignmentVec.clear();
	if (!readyToCorrespond)
	{
		std::cout<<"not ready to correspond yet, needs at least 3 measurements \n";
		return(FK_WRNING_TRACKER_DOES_NOT_HAVE_ENOUGH_MEASUREMENTS) ;
	}
	else
	{

		//vector <Point3D> pt_2;
		vector <Point2D> pt_1;
		vector <Point2D> pt;

		if (points2D.size()<3)
		{
			printf("\nshould not come here at all. FkTracker::Track function");
			getchar();
			return(FK_ERR_TRACKER_SOMETHING_TO_TAKE_CARE_OF);
		}

		//pt_2 = points3D[0];
		pt_1 = points2D[1];
		pt = points2D[2];



		std::vector<int> assPre, assCurr;//,assignmentVec2;

        printf("\nbefore hungCorrespondOf2Sets");
		hungCorrespondOf2Sets(pt_1,pt,assPre,assCurr);
        printf("\\nAfter hungCorrespondOf2Sets");
        printf("\nsize of pt.size() = %d",pt.size());
        printf("\nsize of pt_1.size() = %d",pt_1.size());
        printf("\nsize of assPre.size() = %d",assPre.size());
        printf("\nsize of assCurr.size() = %d",assCurr.size());

		assignmentVec.clear(); // to make sure the size of the returned assignmentVec is same as the number of points...
			
        printf("\nsize of current order is %d", currOrder.size());
        printf("\nsize of pt.size() = %d",pt.size());
            for (size_t hh = 0;hh<currOrder.size();hh++)
                std::cout<<"\nhh = " <<hh<< " currOrder = "<<currOrder[hh]<<std::endl;

		if (pt_1.size() == pt.size()) // the case of same number of measurements and flies ..
		{
			assignmentVec.clear(); // to make sure the size of the returned assignmentVec is same as the number of points...

			printf("\nsize of current order is %d", currOrder.size());
            printf("\nsize of pt.size() = %d",pt.size());
			for (int i = 0;i<pt.size();i++)
				assignmentVec.push_back(currOrder[assCurr[i]]);

            printf("\nend of track");
			return(FK_OK);
		}
	
		// here we are considering just cases that number of measurements is nFlies -1 or nFlies ...
		if(pt_1.size() - pt.size() ==1)
		{
			printf("\n%%%%%%%%%MERGEEEEE%%%%%%%%%%%%!"); 
			printf("\n assCurr.size() = %d",assCurr.size()); 
            for (size_t hh = 0;hh<assCurr.size();hh++)
                std::cout<<"\nhh = " <<hh<< "  = "<<assCurr[hh]<<std::endl;
            
            for (int i = 0;i<assCurr.size();i++) // can be wrong ...
//			for (int i = 0;i<pt_1.size();i++) // can be wrong ...
				if (assCurr[i] == -1)
					continue;
				else
					assignmentVec.push_back(currOrder[assCurr[i]]);

			return(FK_OK);

		}
	
		if(pt.size() - pt_1.size() ==1)
		{
			printf("\n^^^^^^^^^^^^^SPLITTTT^^^^^^^^^^^!"); 
			int missedTarget=-1;// = find_in_vector(currOrder,
			for (int j = 0;j<targetNum;j++)
			{
				if (find_in_vector(currOrder,j) == -1)
				{
					missedTarget = j;
					break;
				}
			}
			// Can be buggy still
			if(missedTarget == -1) // This is a fix for very beginning frames (although this line is unaware of this fact - so it can be fix any thing in the world!).
				return(FK_ERR_TRACKER_SOMETHING_TO_TAKE_CARE_OF); // Take care of the situtaion when we have 5,4,5 measurments as first 3 measurments (so it think it is a split, but currOrder doesn't have any -1).


			for (int i = 0;i<assCurr.size();i++) // can be wrong ...
			{
				if (assCurr[i] == -1)
					assignmentVec.push_back(missedTarget);
				else
					assignmentVec.push_back(currOrder[assCurr[i]]);
			}
			//addNewTarget(); 
			return(FK_OK);

		}

	}


}



// an overload of Hungarian for two sets of points for 3D case

void FkTracker::hungCorrespondOf2Sets (std::vector<Point2D> set1, std::vector<Point2D>set2, std::vector<int>&assignment1,std::vector<int>&assignment2)
{
    printf("\n hungCorrespondOf2Sets  for 2D");
		size_t nrows = set1.size();
		size_t ncols = set2.size();

		size_t maxSize = max(nrows,ncols);
		
        printf("\n nrows = %d, ncols = %d, maxSize = %d",nrows, ncols, maxSize);
		assignment1.resize(maxSize);
		assignment2.resize(maxSize);

        printf("\n here 1");
	for (size_t i = 0;i<maxSize;i++)
	{
		assignment1[i] = -1;
		assignment2[i] = -1;
	}

        printf("\n here 2");

		Matrix<double> costMatrix(maxSize, maxSize);

		for (size_t i = 0;i<nrows;i++)
			for (size_t j = 0;j<ncols;j++)
            {
				costMatrix(i,j) = euclidDistance(set1[i],set2[j]);
                 printf("\nset1[%d].x = %.2lf , set1[%d].y = %.2lf, ",i,set1[i].x,i,set1[i].y);
                 printf("\nset2[%d].x = %.2lf , set1[%d].y = %.2lf, ",j,set2[j].x,j,set2[j].y);

                 printf("\ncost[%d,%d] = %.2lf",i,j,costMatrix(i,j));
            }

		Munkres myMunkres;
        printf("\n before myMunkres.solve");

		myMunkres.solve(costMatrix);

        printf("\n after myMunkres.solve");

		// solution is back stored in costMatrix variable, 0 for matches, otherwise -1;
		// now fill in assignment
		for (size_t i = 0;i<maxSize;i++)
		{
			for (size_t j = 0;j<maxSize;j++)
			{
				if (costMatrix(i, j) ==0)
				{
					if (i<nrows && j<ncols)
					{
					assignment1[i] = j;
					assignment2[j] = i;
					}
				}
			}
		}

            printf("\n end of hungCorrespondOf2Sets  for 2D");


}


void FkTracker::hungCorrespondOf2Sets (std::vector<Point3D> set1, std::vector<Point3D>set2, std::vector<int>&assignment1,std::vector<int>&assignment2)
{
		int nrows = set1.size();
		int ncols = set2.size();

		int maxSize = max(nrows,ncols);
		
		assignment1.resize(maxSize);
		assignment2.resize(maxSize);


	for (int i = 0;i<maxSize;i++)
	{
		assignment1[i] = -1;
		assignment2[i] = -1;
	}


		Matrix<double> costMatrix(maxSize, maxSize);

		for (int i = 0;i<nrows;i++)
			for (int j = 0;j<ncols;j++)
				costMatrix(i,j) = euclidDistance(set1[i],set2[j]);

		Munkres myMunkres;
		myMunkres.solve(costMatrix);
		// solution is back stored in costMatrix variable, 0 for matches, otherwise -1;
		// now fill in assignment
		for (int i = 0;i<maxSize;i++)
		{
			for (int j = 0;j<maxSize;j++)
			{
				if (costMatrix(i, j) ==0)
				{
					if (i<nrows && j<ncols)
					{
					assignment1[i] = j;
					assignment2[j] = i;
					}
				}
			}
		}




}



