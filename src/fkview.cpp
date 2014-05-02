#include "fkview.h"
#include <iostream>
#include "log.h"
//FkViewPaintedFlies Class
#define __USE_OPENMP 0
#define _YELLOW_FLY 1

#if __USE_OPENMP
#include <omp.h>
#endif
FkViewPaintedFlies::FkViewPaintedFlies()
//:m_isOpen(0),m_currFrameImg(NULL)
{
    m_isOpen       = 0;
    m_currFrameImg = NULL;
    m_currBackGroundModel = NULL;
    m_currChangeMaskImg = NULL;
    m_video_orig_output = NULL;
}

//
//FkViewPaintedFlies::FkViewPaintedFlies()
//: m_initialized(0),m_dataFromCamera(NULL), m_state(UNINITIALIZED)
//{
//    m_outputFilePtr= NULL;
//}

FkInt32S FkViewPaintedFlies::readFrame(FkInt32U frameIndex)
{
    if(m_isOpen == false) // Cannot grab if view is not open
        return (FK_ERR_VIEW_IS_NOT_OPEN);

    if(m_currFrameImg!=NULL) // Relese currFrame, if some frame has been previously grabbed.
        cvReleaseImage(&m_currFrameImg);
        
    m_currFrameImg = NULL;

    if(frameIndex>=0L && frameIndex<m_procParam.totalNumofFrames) // <==> if(Valid Frame number)
        cvSetCaptureProperty(m_videoCapture,CV_CAP_PROP_POS_FRAMES,double(frameIndex));
    else
        return (FK_ERR_CANNOT_READ_FRAME); // Cannot grab .. illegal frame number

    IplImage *temp = cvQueryFrame(m_videoCapture); // Read the next frame ("temp" points to internal OpenCV memory and should not be freed : See OpenCV doc on cvQueryFrame() )
    if(temp==NULL) // Some error occured in grabbing frame from AVI
        return (FK_ERR_CANNOT_READ_FRAME);

    m_currFrameImg = cvCloneImage(temp); // Copy the temp image to currFrame
    m_procParam.currentFrameNumber = frameIndex;
    return (FK_OK);
}

FkInt32S FkViewPaintedFlies::processFrame(FkInt32U frameNumber)
{
    FILE_LOG(logDEBUG1)<<"\nStart FkViewPaintedFlies::processFrame";
    readFrame(frameNumber);
    m_pSilhDetect->segmentFrame(m_currFrameImg);
    m_pSilhDetect->denoiseChangeMask();

    if(m_currChangeMaskImg!=NULL)
        cvReleaseImage(&m_currChangeMaskImg);

    //TODO m_currChangeMaskImg is almost redundant
    m_currChangeMaskImg = cvCloneImage(m_pSilhDetect->m_denoisedChangeMask);

    CBlobResult blobs;
    CBlob *currentBlob;


    blobs = CBlobResult( m_currChangeMaskImg, NULL, 0 );
    cv::Mat changeMaskMat(m_pSilhDetect->m_denoisedChangeMask);
    int i;
    int numOfRegions = (int)m_viewArenas.size();
    if (numOfRegions < 1) //TODO add this to the log file error
    {
        std::cout<<"\n in fkview - number of regions are less than 1, help me!"<<std::endl;
            getchar();
    }

#if __USE_OPENMP
#pragma omp parallel shared(i)
{
    omp_set_num_threads(numOfRegions);
#pragma omp for
#endif
    for( i = 0;i<numOfRegions;i++)
    {
        IplImage roiImgIPL;
        cv::Rect roi(m_viewArenas[i]->m_roi);
//        printf("\ni = %d, %d, %d, %d, %d", i, roi.x,roi.y,roi.width,roi.height);
        roiImgIPL = changeMaskMat(roi);
        m_viewArenas[i]->setChangeMask(&roiImgIPL);
    }
#if __USE_OPENMP
}
#endif


    //for(int i = 0;i<m_viewArenas.size();i++)
    //{
    //    char name[200];
    //    sprintf(name,"W%d",i);
    //    cvShowImage(name, m_viewArenas[i]->m_changeMaskIpl);

    //}

    for(int i = 0;i<m_viewArenas.size();i++)
    {
        FILE_LOG(logDEBUG3)<<"****************************************arena ="<< i<< "*****************************************";

        FILE_LOG(logDEBUG3)<<"m_viewArenas[i]->findBlobs()";
        m_viewArenas[i]->findBlobs();
        FILE_LOG(logDEBUG3)<<"m_viewArenas[i]->fixBlobs()";

        m_viewArenas[i]->fixBlobsAndReturnGoodBlobs();
        FILE_LOG(logDEBUG3)<<"m_viewArenas[i]->track;";
        FkInt32S ret =  m_viewArenas[i]->track();

#if _YELLOW_FLY
        //find the yellow fly

        int maxGrayLevel = -1;
        int yellowFlyIndex = -1;
        for (int yc = 0;yc<m_viewArenas[i]->m_currFixedCentroids.size();yc++)
        {
            CBlob* currBlob2 = m_viewArenas[i]->m_fixedListOfBlobs.GetBlob(yc);
            int meanGray = (int)currBlob2->Mean(m_currFrameImg,currBlob2->getIDNumber(),m_viewArenas[i]->m_roi.x, m_viewArenas[i]->m_roi.y);
//            grayLevels.push_back(meanGray);
            if(meanGray>maxGrayLevel)
            {
                maxGrayLevel = meanGray;
                yellowFlyIndex = yc;
            }
        }

#endif

        for (int j = 0;j<m_viewArenas[i]->m_currFixedCentroids.size();j++)
        {
            CBlob* currBlob = m_viewArenas[i]->m_fixedListOfBlobs.GetBlob(j);

            char tmp[10];

#if _YELLOW_FLY 
            if ((maxGrayLevel != -1) && (yellowFlyIndex != -1) && (j==yellowFlyIndex))
                sprintf(tmp,"%c",'f');
            else
                sprintf(tmp,"%c",'m');

#else
            if (ret == FK_OK)
                sprintf(tmp,"%d , %d",currBlob->getIDNumber(),100);//meanGray);
            else
                sprintf(tmp,"%d",-1);
#endif
            CvFont font;
            cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,0.5,0.5,0,2,8);
            cvPutText(m_currFrameImg,tmp,cvPoint((int)(floor(m_viewArenas[i]->m_currFixedCentroids[j].x + m_viewArenas[i]->m_roi.x)) , 
                                                 (int)(floor(m_viewArenas[i]->m_currFixedCentroids[j].y)+ m_viewArenas[i]->m_roi.y) ) ,&font,cvScalar(255,0,0) );
        }

    }

    FILE_LOG(logDEBUG1)<<"\nEnd of FkViewPaintedFlies::processFrame";
    return(FK_OK);
}

FkInt32S FkViewPaintedFlies::init(std::string videoFileName, FkInt32U startFrame, FkInt32U stopFrame, std::string backgroundModelFileName)
{
    m_procParam.inputFileName = videoFileName;
    FkInt32S ret = this->open(); // try to open the video file first
    if( ret!=FK_OK)
    {
        return(ret);
    }

    m_procParam.frameHeight         =       (FkInt32U)cvGetCaptureProperty(m_videoCapture, CV_CAP_PROP_FRAME_HEIGHT);
    m_procParam.frameWidth          =        (FkInt32U)cvGetCaptureProperty(m_videoCapture, CV_CAP_PROP_FRAME_WIDTH);
    m_procParam.totalNumofFrames    =  (FkInt32U)cvGetCaptureProperty(m_videoCapture, CV_CAP_PROP_FRAME_COUNT);

    if(stopFrame == -1)
        stopFrame = m_procParam.totalNumofFrames;

    if(  (startFrame>=0) && (startFrame<m_procParam.totalNumofFrames)     )
        m_procParam.startFrame = startFrame;
    else
        return(FK_ERR_BAD_PROCESSING_PARAM);

    if((stopFrame>startFrame) && (stopFrame<=m_procParam.totalNumofFrames))
        m_procParam.stopFrame = stopFrame;
    else
        return(FK_ERR_BAD_PROCESSING_PARAM);

    //IplImage* calculatedBGModel

    m_currBackGroundModel = NULL;
    if(backgroundModelFileName == "")
        calculateBackgroundModel();
    else
    {
        m_currBackGroundModel = cvLoadImage(backgroundModelFileName.c_str());
        if (m_currBackGroundModel==NULL)
            return(FK_ERR_BACKGROUND_FILE_CAN_NOT_BE_LOADED);
    }
    //TODO: come back to this and fix this
    m_pSilhDetect = /*(FkSilhDetector*)*/   new FkSilhDetector_PaintedFlies;
    m_pSilhDetect->init(m_currBackGroundModel);


    std::string videoOutName = m_procParam.inputFileName.substr(0,m_procParam.inputFileName.length()-4);
    videoOutName+="_out.avi";

    m_video_orig_output = cvCreateVideoWriter(videoOutName.c_str(),-1,/*CV_FOURCC('F', 'F', 'D', 'S'),*/60.0,cvSize(m_procParam.frameWidth,m_procParam.frameHeight));


    return(FK_OK);
}


FkInt32S FkViewPaintedFlies::open()
{
    m_videoCapture = NULL;
    m_videoCapture=cvCreateFileCapture(m_procParam.inputFileName.c_str());
    if(m_videoCapture == NULL)
        return(FK_ERR_CAN_NOT_OPEN_VIDEO_FILE);
    else
    {
        m_isOpen = 1;
        return(FK_OK);
    }
}

FkInt32S FkViewPaintedFlies::close()
{
    if(m_videoCapture != NULL)
    {
        cvReleaseCapture(&m_videoCapture);
        m_videoCapture = NULL;
    }

    for (FkInt32U i = 0;i<m_viewArenas.size();i++)
    {
        m_viewArenas[i]->finish();
        delete m_viewArenas[i];
    }

    if( m_video_orig_output != NULL)
        cvReleaseVideoWriter(&m_video_orig_output);

	
    return(FK_OK);
}


FkInt32S FkViewPaintedFlies::calculateBackgroundModel()
{
    if (!m_isOpen)
        return(FK_ERR_CALCULATE_BACKGROUND);

    IplImage *x = cvQueryFrame(m_videoCapture);
    m_currBackGroundModel = cvCloneImage(x);

    cvZero(m_currBackGroundModel);
    IplImage* sumMatrix = cvCreateImage(cvSize(m_currBackGroundModel->width,m_currBackGroundModel->height), IPL_DEPTH_64F, m_currBackGroundModel->nChannels);
    cvZero(sumMatrix);

    FkInt32U nFramesBG = 100; //hard coded
    FkInt32U step = (FkInt32U)((m_procParam.totalNumofFrames-m_procParam.startFrame)/nFramesBG);

    int countRun=0;
    assert(m_procParam.totalNumofFrames > 1); // No video file should be only 1 frame long!!
    if(step<1) // This is to handle case when number of frames to process are less than number of frames required to avergae background from, i.e., m_procParam.totalNumofFrames < nFramesBG
        step=1;

    for(FkInt32U i=m_procParam.startFrame;i<m_procParam.totalNumofFrames-step;i+=step,countRun++) // I changed the maxFramesInFile to maxFramesInFile -step due to the error that I was getting for a video .TODO: FIX IT!
    {
    printf("\n for calculating bg frame number = %d", i);
    cvSetCaptureProperty(m_videoCapture,CV_CAP_PROP_POS_FRAMES,double(i));
    IplImage *x = cvQueryFrame(m_videoCapture);
    cvShowImage("orig", x);
    cvWaitKey(1);
    for(int i=0;i<m_currBackGroundModel->height;i++)
        for(int j=0;j<m_currBackGroundModel->width;j++)
        {
            CvScalar col1 = cvGet2D(x,i,j);
            CvScalar origColor = cvGet2D(sumMatrix,i,j);
            origColor.val[0] += col1.val[0];
            origColor.val[1] += col1.val[1];
            origColor.val[2] += col1.val[2];
            cvSet2D(sumMatrix,i,j,origColor);
        }
    }
    for(int i=0;i<sumMatrix->height;i++)
        for(int j=0;j<sumMatrix->width;j++)
        {
            CvScalar origColor = cvGet2D(sumMatrix,i,j);
            origColor.val[0] /= double(countRun);
            origColor.val[1] /= double(countRun);
            origColor.val[2] /= double(countRun);
            cvSet2D(m_currBackGroundModel,i,j,origColor);
        }
        cvReleaseImage(&sumMatrix);
        char fnameCurr[400];
        sprintf(fnameCurr,"CalculatedBG-View-%d.bmp",m_viewID);
        cvSaveImage(fnameCurr,m_currBackGroundModel);
        cvSetCaptureProperty(m_videoCapture,CV_CAP_PROP_POS_FRAMES,double(0)); // go to the first frame again
        cvDestroyAllWindows();
        return(FK_OK);
}

FkInt32S FkViewPaintedFlies::show()
{
    if(m_currFrameImg!=NULL)
        cvShowImage("current Frame",m_currFrameImg);

    if(m_currChangeMaskImg!=NULL)
        cvShowImage("m_currChangeMaskImg",m_currChangeMaskImg);

    //if(m_pSilhDetect->m_denoisedChangeMask!=NULL)
    //    cvShowImage("m_pSilhDetect->m_denoisedChangeMask",m_pSilhDetect->m_denoisedChangeMask); 

    //if(m_currBackGroundModel!=NULL)
    //    cvShowImage("m_currBackGroundModel",m_currBackGroundModel);

    cvWaitKey(1);

    return(FK_OK);
}

FkInt32S FkViewPaintedFlies::appendToVideoOutput()
{
    if (m_video_orig_output != NULL)
    {
        cvWriteFrame(m_video_orig_output,m_currFrameImg);
        return(FK_OK);
    }
    else
    {
       return(FK_ERR_CAN_NOT_WRITE_VIDEO_OUTPUT);

    }

}


FkInt32S FkViewPaintedFlies::appendToTextOutput(int frameNum)
{
	for (FkInt32U i = 0;i<m_viewArenas.size();i++)
	{
		m_viewArenas[i]->appendToTextOutput(frameNum);

	}
    return(FK_OK);
}



FkInt32S    FkViewPaintedFlies::initTextOutput()
{
    FILE_LOG(logDEBUG1)<<"\nStart initTextOutput";
    for (FkInt32U i = 0;i<m_viewArenas.size();i++)
    {
        std::string fn = m_procParam.inputFileName;
        fn = fn.substr(0,fn.length()-4);
        char tmp[100];
        sprintf(tmp,"_arena%.2d.csv",i);
        fn+=tmp;
        m_viewArenas[i]->initTextOutput(fn);
    }
    FILE_LOG(logDEBUG1)<<"\nEnd of initTextOutput";
    return(FK_OK);
}
