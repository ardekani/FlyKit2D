/*! \file fkSilhDetector.h
    \breif flykit silhouette detector - related classes
    \author Reza Ardekani
    \date 09/13/2013
*/
#ifndef __FKSILHDETECT_H_INCLUDED
#define __FKSILHDETECT_H_INCLUDED
#include "fkErrors.h"
#include "fkTypes.h"
#include "cv.h"
#include "highgui.h"
typedef enum {
    FK_SILH_DETECT1  /*!< todo, a better name! */
} FkSilhDetectType;

class FkSilhDetector
{
public:
    IplImage* m_inputFrame; //might be redundant
    IplImage* m_bgModel;
    IplImage* m_rawChangeMask;
    IplImage* m_denoisedChangeMask;
    FkInt32S  m_isInited;
    FkReal32F m_alpha;
    FkInt8U   m_threshold;

    FkSilhDetector(){};
    virtual ~FkSilhDetector(){};

//    inline char pixelDistance(unsigned char *thisFrame, unsigned char *bg, int nChannels);
    virtual FkInt32S init(IplImage* inp){return(FK_OK);};
    virtual FkInt32S finish(){return(FK_OK);};

    virtual FkInt32S setThreshold(FkInt8U thresh)
    { m_threshold = thresh; return (FK_OK);}
    
    virtual FkInt32S setAlpha(float inp)
    { m_alpha = inp; return (FK_OK); }

    virtual FkInt32S segmentFrame(IplImage* inp){return(FK_OK);};
    virtual FkInt32S denoiseChangeMask(){return(FK_OK);};
    virtual FkInt32S setBackgroundModel(IplImage* inp){return(FK_OK);};
    //IplImage* segmentFrame(IplImage *thisFrame);
    //IplImage* segmentFrame2(IplImage *thisFrame) ;
};

class FkSilhDetector_PaintedFlies : public FkSilhDetector {
private:
    IplImage* m_temp_for_denoising;
public:

    FkSilhDetector_PaintedFlies();
    ~FkSilhDetector_PaintedFlies(){};
    FkInt32S    setBackgroundModel(IplImage* inp);
    FkInt32S    init(IplImage* inp);

    virtual FkInt32S segmentFrame(const IplImage* inp);
    FkInt32S denoiseChangeMask();

};


#endif 
