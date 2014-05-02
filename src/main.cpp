#include <stdio.h>
#include "fkview.h"
#include <iostream>
#include <time.h>
#include "log.h"
//int radius = 90;

int radius = 130; // for 6 well videos
int numberOfFliesInEachArena = 3;
int firstFrame = 65860;
int lastFrame = firstFrame + 600;//13900;

int main()
{
    FILELog::initLogFile("DEBUG2"); 

    //std::vector<std::pair(double,double)> centrs;

    std::vector<CvPoint> centers;
    //centers.push_back(cvPoint(101, 108));
    //centers.push_back(cvPoint(302, 101));
    //centers.push_back(cvPoint(501, 95 ));
    //centers.push_back(cvPoint(695, 90 ));
    //centers.push_back(cvPoint(105, 309));
    //centers.push_back(cvPoint(309, 301));
    //centers.push_back(cvPoint(510, 294));
    //centers.push_back(cvPoint(707, 287));
    //centers.push_back(cvPoint(111, 512));
    //centers.push_back(cvPoint(315, 504));
    //centers.push_back(cvPoint(518, 497));
    //centers.push_back(cvPoint(716, 486));
///////////new video//////////////

    //centers.push_back(cvPoint(74,102));
    //centers.push_back(cvPoint(273,100));
    //centers.push_back(cvPoint(474,95));
    //centers.push_back(cvPoint(670,93));

    //centers.push_back(cvPoint(74,303));
    //centers.push_back(cvPoint(277,299));
    //centers.push_back(cvPoint(480,294));
    //centers.push_back(cvPoint(679,290));

    //centers.push_back(cvPoint(78,  507));
    //centers.push_back(cvPoint(281,  503));
    //centers.push_back(cvPoint(486,  497));
    //centers.push_back(cvPoint(686,  490));


////////////FOR YELLOW FLY VIDEO 12 well///
//    centers.push_back(cvPoint(111, 107));
//    centers.push_back(cvPoint(306, 106));
//    centers.push_back(cvPoint(504,.105));
    //centers.push_back(cvPoint(693,.107));
    //centers.push_back(cvPoint(696, 299));
    //centers.push_back(cvPoint(504, 300));
//    centers.push_back(cvPoint(306,.300));
//    centers.push_back(cvPoint(111,.298));
    //centers.push_back(cvPoint(130, 477));
    //centers.push_back(cvPoint(313, 480));
    //centers.push_back(cvPoint(504, 487));
    //centers.push_back(cvPoint(689, 485));
  ///////////////

////////////FOR YELLOW FLY VIDEO 6 well///
    centers.push_back(cvPoint(134, 155));
    centers.push_back(cvPoint(398, 156));
    centers.push_back(cvPoint(659, 153));
    centers.push_back(cvPoint(134, 420));
    centers.push_back(cvPoint(398, 413));
    centers.push_back(cvPoint(651, 408));
///////////////////////////////////////////

    FkViewPaintedFlies* myView = new FkViewPaintedFlies; 

    FkInt32S ret;
    std::string vidName, backgroundName;
    vidName = "C:\\Users\\dehestan\\Downloads\\yellowFlies\\AviFileChunk0_View2_6well.avi";
    backgroundName = "C:\\Users\\dehestan\\Desktop\\New folder\\New folder\\paintedFlies2D\\restructuredPaintedFlies\\x64\\Release\\CalculatedBG.bmp";

    
    //running it for one area video 
    vidName = "E:\\sampleVid\\AviFileChunk0_View0.avi";
    backgroundName = "E:\\sampleVid\\CalculatedBG.bmp";
    //these values makes the roi to be the whole image.
    centers.clear();
    centers.push_back(cvPoint(0,0));
    radius = 1000;

    FILE_LOG(logINFO)<<"video file name ="<< vidName;
    FILE_LOG(logINFO)<<"background file name ="<< backgroundName ;
    FILE_LOG(logINFO)<<"first frame : "<< firstFrame;
    FILE_LOG(logINFO)<<"last frame : "<< lastFrame;

    ret = myView->init(vidName,firstFrame,lastFrame,backgroundName);//"CalculatedBG-View2_yellowFly_6well.bmp");
    FILE_LOG(logDEBUG)<<"ret from init ="<<ret;
    int offset = radius; //(int)(floor( 1.4142f*radius));

    for (int i = 0;i<centers.size();i++)
    {
        FkArea_PaintedFlies* pArena = new FkArea_PaintedFlies;
        CvRect roi;

        roi.x = max(centers[i].x - offset,0);
        roi.y = max(centers[i].y - offset,0);
        roi.width = min(2*radius,800-roi.x);
        roi.height = min(2*radius,600-roi.y);

        pArena->setID(i);
        pArena->setArenaParams(roi,numberOfFliesInEachArena);
        pArena->m_arenaTracker.setTargetNum(3);
        myView->m_viewArenas.push_back(pArena);
    }

    myView->initTextOutput();
    clock_t startTime;
    startTime = clock();
    for (int i = firstFrame;i<lastFrame;i+=1)
    {
        ret = myView->processFrame(i);
        myView->show();
        myView->appendToVideoOutput();
        myView->appendToTextOutput(i);
        FILE_LOG(logDEBUG) << "\nFrame: "<<i;
        std::cout << "\nFrame: "<<i;

    }
	myView->close();

	std::cout<< "\nTime ="<<(double)(1.0*(clock()-startTime)/CLOCKS_PER_SEC);
    FILE_LOG(logINFO)<<"Time ="<<(double)(1.0*(clock()-startTime)/CLOCKS_PER_SEC);
    printf("\n Done processing beeeach");
    getchar();
    return 0;
}


