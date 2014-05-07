#include "fkSilhDetector.h"
#include "fkTypes.h"
#include "log.h"
#define __USE_OPENMP_FOR_SEGMENTATION 1 // It messes up the background subtraction! TODO: get back to this..

#define PIXEL(img, i, j, k)		*((img)->imageData + (img)->widthStep * (i) + (img)->nChannels * (j) + (k))
#define UPIXEL(img, i, j, k)	*( (unsigned char*) ((img)->imageData + (img)->widthStep * (i) + img->nChannels * (j) + (k)) )
//inline double double_PixelDistance(unsigned char *thisFrame, unsigned char *bg, int nChannels)
//{ 	//Can be faster if you don't use double calculations. use ints.
//    if(nChannels == 3)
//        return (double) ( 0.299 * (((int) thisFrame[2]) - bg[2]) + 0.587 * (((int) thisFrame[1]) - bg[1]) + 0.114 * ( ((int)thisFrame[0]) - bg[0]) );		
//    else 
//        return (double) (thisFrame[0] - bg[0]);
//
//}
//

inline char pixelDistance(unsigned char *thisFrame, unsigned char *bg, int nChannels)
{
    //Can be faster if you don't use double calculations. use ints.
    if(nChannels == 3)
        return (char) ( 0.299 * (((int) thisFrame[2]) - bg[2]) + 0.587 * (((int) thisFrame[1]) - bg[1]) + 0.114 * ( ((int)thisFrame[0]) - bg[0]) );		
    else 
        return (char) (thisFrame[0] - bg[0]);
}

FkSilhDetector_PaintedFlies::FkSilhDetector_PaintedFlies()
{
    m_inputFrame = NULL; 
    m_bgModel= NULL;
    m_rawChangeMask= NULL;
    m_denoisedChangeMask= NULL;
    m_isInited = 0;
    m_alpha = 0.01f;
    m_threshold = 25;

}
FkInt32S FkSilhDetector_PaintedFlies::init(IplImage* inp)
{
    if(m_bgModel!=NULL)
        cvReleaseImage(&m_bgModel);
    m_bgModel = NULL;

    m_bgModel = cvCloneImage(inp);

    m_temp_for_denoising = cvCreateImage(cvSize(inp->width,inp->height),8,1);
    m_denoisedChangeMask = cvCreateImage(cvSize(inp->width,inp->height),8,1);
    m_rawChangeMask = cvCreateImage(cvSize(inp->width,inp->height),8,1);

    m_isInited = 1;
    return (FK_OK);
}

FkInt32S FkSilhDetector_PaintedFlies::setBackgroundModel(IplImage* inp)
{
    m_bgModel = cvCloneImage(inp);
    return (FK_OK);
}



FkInt32S FkSilhDetector_PaintedFlies::segmentFrame(const IplImage *thisFrame) // I want to keep it void
{
    FILE_LOG(logDEBUG1)<< "\nbegin of FkSilhDetector_PaintedFlies::segmentFrame";

    if(m_isInited!=1)
    {
        FILE_LOG(logDEBUG1)<< "\nsilhDetector is not init";
        return(FK_ERR_SILH_DETECT_IS_NOT_INIT);
    }
    CvPoint pt1, pt2; //this is for ROI, TODO: get this exposed 
    pt1 = cvPoint(1,1);
    pt2 = cvPoint(thisFrame->width, thisFrame->height); //setting ROI the whole image

    cvZero(m_rawChangeMask);

    int i, j, minX,maxX,minY,maxY;
    minX = FK_MIN(pt1.x, pt2.x);
    maxX = FK_MAX(pt1.x, pt2.x);
    minY = FK_MIN(pt1.y, pt2.y);
    maxY = FK_MAX(pt1.y, pt2.y);

#if __USE_OPENMP_FOR_SEGMENTATION
#pragma omp parallel shared(i,j,maxX,maxY,minX,minY)//, thisFrame, m_bgModel, m_threshold, m_rawChangeMask)
    {
#pragma omp for schedule(dynamic)
#endif
    for(i= minY+1; i<maxY-1; ++i) //just look for silhouettes in ROI
    {
        for(j=minX+1; j<maxX-1; ++j)
        {
            
            if( pixelDistance( &UPIXEL(thisFrame, i, j, 0), &UPIXEL(m_bgModel, i, j, 0), 3 ) > -m_threshold )
            {
                //for(int k = 0; k < m_bgModel->nChannels; ++k)
                //{
                //    //if (parentCamView->frameNum % 1000 == 0)
                //    //    UPIXEL(m_bgModel, i, j, k) = (unsigned char) (alpha * UPIXEL(thisFrame, i, j, k) + (1-alpha) * UPIXEL(m_bgModel, i, j, k));
                //}
            }
            else
            {
                PIXEL(m_rawChangeMask,i, j, 0) |= 0xFF;
            }
        }
    }

#if __USE_OPENMP_FOR_SEGMENTATION
    }
#endif


    FILE_LOG(logDEBUG1)<< "\nEnd of FkSilhDetector_PaintedFlies::segmentFrame";
    return (FK_OK);
}

FkInt32S FkSilhDetector_PaintedFlies::denoiseChangeMask() 
{
    cvErode(m_rawChangeMask,m_denoisedChangeMask,NULL,1);
    cvDilate(m_denoisedChangeMask,m_denoisedChangeMask,NULL,1);
    CvMemStorage* storage = cvCreateMemStorage();
    CvSeq* first_contour = NULL;

//    cvMorphologyEx( m_rawChangeMask, m_denoisedChangeMask, 0, 0, CV_MOP_CLOSE, 2);//CVCLOSE_ITR );

    int Nc = cvFindContours(m_denoisedChangeMask,storage,&first_contour,sizeof(CvContour),CV_RETR_EXTERNAL);//CV_RETR_LIST);

    cvZero(m_denoisedChangeMask);

    for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) 
    {
        double len = cvContourPerimeter( c );
        if (len>10) //to make sure we get rid of noises
        {
            cvDrawContours(m_denoisedChangeMask, c, cvScalar(255), cvScalar(255), -1, CV_FILLED, 8);
        }
    }

    cvReleaseMemStorage(&storage);
    return(FK_OK);

}
