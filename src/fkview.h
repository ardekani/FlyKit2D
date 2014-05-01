/*! \file fkview.h
    \breif flykit view class
    \author Reza Ardekani
    \date 09/12/2013
*/
#ifndef __FKVIEW_H_INCLUDED
#define __FKVIEW_H_INCLUDED
#include "fkTypes.h"
#include "fkErrors.h"
#include "cv.h"
#include "highgui.h"
#include "fkSilhDetector.h"
#include "cblob\blobresult.h"
#include "fkarena.h"
typedef enum {
    FK_PAINTED_FLIES   /*!< for tracking painted flies in 2D */
} FkViewType;

struct processingParameters {
    FkInt32U startFrame;
    FkInt32U stopFrame;
    FkInt32U currentFrameNumber;
    std::string inputFileName;
    FkInt32U frameHeight;
    FkInt32U frameWidth;
    FkInt32U totalNumofFrames;
};

class   FkView {
public:
    /* member variables*/
    FkInt32S    m_viewID;
    FkInt32S    m_isOpen;

    /* member functions */
    virtual     FkInt32S open()     {return (FK_OK);};
    virtual     FkInt32S close()    {return (FK_OK);};
    virtual     FkInt32S restart()  {return (FK_OK);};
    virtual     FkInt32S init()     {return (FK_OK);};
    virtual     FkInt32S show()     {return (FK_OK);};
    FkView(){};
    virtual ~FkView(){};

};

class FkViewPaintedFlies: public FkView {

private:

   // FkInt32S                        m_TotalNumOfFrames; /*!<total number of the frames in the file */
    CvCapture*                      m_videoCapture;
    CvVideoWriter*                  m_video_orig_output;
    processingParameters            m_procParam;

public:

    std::vector<FkArea_PaintedFlies*> m_viewArenas;
//    FkInt32S    initViewArenas(std::vector<std::vector>>);
    IplImage*   m_currFrameImg;
    IplImage*   m_currChangeMaskImg;
    IplImage*   m_currBackGroundModel;
    //FkSilhDetector* m_pSilhDetect;
    FkSilhDetector_PaintedFlies* m_pSilhDetect; //TODO: fix this
    FkViewPaintedFlies();
    virtual ~FkViewPaintedFlies(){};

    FkInt32S    processFrame(FkInt32U frameNumber); // this is going to call silhouette 
    FkInt32S    open();
    FkInt32S    close();
    FkInt32S    restart(){return (FK_OK);};;
    FkInt32S    init(std::string videoFileName, FkInt32U startFrame = 0, FkInt32U stopFrame = -1,std::string backGroundFileName="");
    FkInt32S    calculateBackgroundModel();
    FkInt32S    show(); //todo get display option
    FkInt32S    readFrame(FkInt32U frameNumber);
    FkInt32S    setArenasImages(); // this is going to copy the images
    FkInt32S    initArenas(std::vector< std::vector<FkInt32S> >inp); // it expects the input to be in the form of vectors of rect.x, rect.y, rect.w, rect.h ,numberOfFliesInTheArea
    FkInt32S    appendToVideoOutput();
    FkInt32S    appendToTextOutput(int frameNumber);
	FkInt32S    initTextOutput();

};
#endif

