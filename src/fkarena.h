/*! \file fkarea.h
\breif each view has single or multiple arena (containters), tracking for each arena can be independent
\author Reza Ardekani
\date 09/15/2013
*/
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "cblob\blobresult.h"
#include "fkTracker.h"
#include <iostream>
#include <fstream>
#include "log.h"
class FkArena_base{
public:
    FkArena_base()
    {
        m_changeMaskIpl = NULL;
    };
    CvRect m_roi;
    CBlobResult m_origListOfBlobs;
    CBlobResult m_fixedListOfBlobs;
    FkInt32S m_ID;          // ID of the arena
    FkInt32S m_numOfTargets; // number of flies in arena
    std::ofstream m_OutputText;
    //cv::Mat m_changeMaskMat; 
    FkTracker m_arenaTracker;
    std::vector<int> m_lastTrackIndex2D; 
    std::vector<CvPoint2D64f> m_currFixedCentroids; 
    IplImage* m_changeMaskIpl;
    virtual FkInt32S setChangeMask(IplImage* inp){
        if(m_changeMaskIpl!=NULL)
            cvReleaseImage(&m_changeMaskIpl);

        m_changeMaskIpl  = cvCloneImage(inp); //TODO: see if you need to to cvCloneImage
        //cvCopyImage(inp,m_changeMaskIpl); //TODO: see if you need to to cvCloneImage
        return(FK_OK);
    };
    virtual FkInt32S track(){return(FK_OK);};
    virtual FkInt32S writeOutput(){return(FK_OK);};
    //virtual FkInt32S setBlobs(){return(FK_OK);}; //set the 
    virtual FkInt32S findBlobs(){return(FK_OK);}; 
    virtual FkInt32S finish(){return(FK_OK);}; 

    virtual FkInt32S setArenaParams(CvRect roi,FkInt32S numOfFlies)
    {
        m_roi.x = roi.x;
        m_roi.y = roi.y;
        m_roi.width = roi.width;
        m_roi.height = roi.height;
        m_numOfTargets = numOfFlies;
        m_changeMaskIpl = cvCreateImage(cvSize(roi.width,roi.height),8,1);
        for (FkInt32U i = 0;i<m_numOfTargets;i++)
            m_lastTrackIndex2D.push_back(i);

        return(FK_OK);
    }; 
    virtual FkInt32S setID(FkInt32S id){ m_ID = id; return(FK_OK);}; //set the 
};

class FkArea_PaintedFlies: public FkArena_base{
private:

    std::vector<CBlobResult> m_prevDetectedBlobs;
public:
    //CvRect m_roi;
    //CBlobResult m_allBlobs;
    //FkInt32S m_ID;

    FkArea_PaintedFlies()
    {
        m_changeMaskIpl = NULL;
    };
    ~FkArea_PaintedFlies(){};
    FkInt32S track()
    {
        FILE_LOG(logDEBUG1)<<"begin of arena track";
        //std::cout<<"\nIn arean track-- begin ";
        m_arenaTracker.getNewMeasurement(m_currFixedCentroids);

        if(m_lastTrackIndex2D.size()==0)
        {
            //std::cout<<"\n        if(m_lastTrackIndex2D.size()==0)";
            //std::cout<< "\nm_currFixedCentroids.size() = "<<m_currFixedCentroids.size();
            for(int i=0;i<m_currFixedCentroids.size();i++) 
                m_lastTrackIndex2D.push_back(i);
        }
        FILE_LOG(logDEBUG3)<<"\n m_arenaTracker.Track";
        m_arenaTracker.Track(m_lastTrackIndex2D);
        FILE_LOG(logDEBUG3)<<"\n m_arenaTracker.Track";
        m_lastTrackIndex2D = m_arenaTracker.assignmentVec; // read back tracks

        // assign ID's to blobs after tracking

        if(m_lastTrackIndex2D.size() == m_fixedListOfBlobs.GetNumBlobs())
        {
            FILE_LOG(logDEBUG3)<<"in : if(m_lastTrackIndex2D.size() == m_fixedListOfBlobs.GetNumBlobs())";
            CBlob *currBlob;

            FILE_LOG(logDEBUG3)<<"\n m_fixedListOfBlobs.GetNumBlobs() = %d"<<m_fixedListOfBlobs.GetNumBlobs();
            //m_fixedListOfBlobs.GetNumBlobs()

            for (int i = 0;i<m_fixedListOfBlobs.GetNumBlobs();i++)
            {
                currBlob = m_fixedListOfBlobs.GetBlob(i);
                currBlob->setIDNumber(m_lastTrackIndex2D[i]);
                //printf("\nIn the loopp i = %d",i);

            }

        }

        FILE_LOG(logDEBUG1)<<"end of arena track";
        return(FK_OK);
    };




    FkInt32S finish()
    {
        if(m_changeMaskIpl!=NULL)
            cvReleaseImage(&m_changeMaskIpl);

		m_OutputText.close();
        return(FK_OK);
    }

    FkInt32S findBlobs(){
        FILE_LOG(logDEBUG1)<<"Start of findBlobs()";
        m_origListOfBlobs = CBlobResult(m_changeMaskIpl,NULL, 0);
        FILE_LOG(logDEBUG1)<<"End of findBlobs()";
        return(FK_OK);
    }; 

    FkInt32S fixBlobsAndReturnGoodBlobs()
    {
        FILE_LOG(logDEBUG1)<<"Start of fixBlobsAndReturnGoodBlobs";
        //std::cout<<"beginning"<<std::endl;
        //equal measurement
        if ( (FkInt32S) m_origListOfBlobs.GetNumBlobs() == m_numOfTargets) //if the number of detected blobs is same as the number of flies.. don't do any thing
        {
            m_fixedListOfBlobs = m_origListOfBlobs;
        }

        //std::cout<<"middle1"<<std::endl;


        //more measurement, some blobs should be eliminated 
        if ( (FkInt32S) m_origListOfBlobs.GetNumBlobs()> m_numOfTargets) //if the number of detected blobs is same as the number of flies.. don't do any thing
        {
#if __USE_PREVIOUS_MEASUREMENT_TO_FIX
            if (m_prevDetectedBlobs.size()< 1) //no previous info -- just return the originals
            {
                m_fixedListOfBlobs = m_origListOfBlobs;
            }
            else // we have more blobs more than the number of flies, and we have info about previous blobs.. first see if the previous info is useful
            {
                FkInt32U indx = returnLastValidMeasurementIndx();
                if (indx == -1) //couldn't find any valid measurement in previous 
                {
                    m_fixedListOfBlobs = m_origListOfBlobs;
                }
                else  // now fix this...you have more measurement than number of flies, one blob should be eliminated
                {
                }
            }
#else
            // remove all but the most m_numOfTargets biggest blobs

            m_fixedListOfBlobs = m_origListOfBlobs;

            while ( (FkInt32S) m_fixedListOfBlobs.GetNumBlobs()> m_numOfTargets )
            {
                    FkReal32F minArea =99999.0;
                    FkInt32S smallestBlobID = -1;

//                    std::cout<<"\n------------before deleting ------------ = " << m_fixedListOfBlobs.GetNumBlobs() <<std::endl;
                    for (int i = 0;i<m_fixedListOfBlobs.GetNumBlobs();i++)
                    {
                        CBlob* currBlob = m_origListOfBlobs.GetBlob(i);
                        if (currBlob->Area()<minArea)
                        {
                            minArea = currBlob->Area();
                            smallestBlobID = i;
                        }
                    }
                    m_fixedListOfBlobs.removeNthBlob(smallestBlobID);
//                    std::cout<<"\n------------after deleting ------------ = " << m_fixedListOfBlobs.GetNumBlobs() <<std::endl;
            }
            assert((FkInt32S) m_fixedListOfBlobs.GetNumBlobs() == m_numOfTargets);
#endif
            }

    //std::cout<<"middle2"<<std::endl;

    // less measurements
    if ( (FkInt32S) m_origListOfBlobs.GetNumBlobs()< m_numOfTargets) //if the number of detected blobs is same as the number of flies.. don't do any thing
    {
 //       std::cout<<"middle2.1"<<std::endl;
#if __USE_PREVIOUS_MEASUREMENT_TO_FIX
        if (m_prevDetectedBlobs.size()< 1) //no previous info -- just return the originals
        {
            m_fixedListOfBlobs = m_origListOfBlobs;
            std::cout<<"middle2.2"<<std::endl;

        }
        else // we have less measurements than the number of flies first see if the previous info is useful
        {

            FkInt32U indx = returnLastValidMeasurementIndx();
            std::cout<<"\nindex = "<<indx;
            if (indx == -1) //couldn't find any valid measurement in previous 
            {
                m_fixedListOfBlobs = m_origListOfBlobs;
            }
            else  // now fix this...you have less measurement than number of flies, some blobs are going to be splited!
            {
            }
        }
#else // just use the size and split the larger blob
            FkReal32F maxArea = -1;
            FkInt32S biggerBlobID = -1;
            FkInt32S numOfMissing = m_numOfTargets - m_origListOfBlobs.GetNumBlobs();
            FILE_LOG(logDEBUG3)<<"numOfMissing = "<<numOfMissing;
            //std::cout<<"\nnumOfMissing = " << numOfMissing <<std::endl;
            m_fixedListOfBlobs = m_origListOfBlobs;
            FILE_LOG(logDEBUG3)<<"before Splitting num of blobs are"<<m_fixedListOfBlobs.GetNumBlobs();
            //std::cout<<"\n------------before spliting ------------ = " << m_fixedListOfBlobs.GetNumBlobs() <<std::endl;
            for (int i = 0;i<m_fixedListOfBlobs.GetNumBlobs();i++)
            {
                CBlob* currBlob = m_origListOfBlobs.GetBlob(i);
                if (currBlob->Area()>maxArea)
                {
                    maxArea = currBlob->Area();
                    biggerBlobID = i;
                }
            }
            //a simple logic, just split the bigger blob 
            m_fixedListOfBlobs.splitBlobNthToMSubBlob(m_fixedListOfBlobs,biggerBlobID,numOfMissing+1);
            //std::cout<<"\n------------after spliting ------------ = " << m_fixedListOfBlobs.GetNumBlobs() <<std::endl;
            FILE_LOG(logDEBUG3)<<"after Splitting num of blobs are"<<m_fixedListOfBlobs.GetNumBlobs();
            
            //for (size_t ss = 0;ss<m_fixedListOfBlobs.GetNumBlobs();ss++)
            //{
            //    CBlob* currBlob = m_fixedListOfBlobs.GetBlob(ss);
            //    currBlob->returnBlobsCentroids(
            //}




#endif
    }


    //std::cout<<"middle3"<<std::endl;

    m_prevDetectedBlobs.push_back(m_fixedListOfBlobs);
    if(m_prevDetectedBlobs.size()>3) //TODO: write this more efficient
        m_prevDetectedBlobs.erase(m_prevDetectedBlobs.begin(),m_prevDetectedBlobs.begin()+1);

    //std::cout<< "\n---after deleting it is : " << m_prevDetectedBlobs.size();
    CBlob *currentBlob;
    m_currFixedCentroids.clear();

	for (size_t i = 0;i<m_fixedListOfBlobs.GetNumBlobs();i++)
    {
        currentBlob = m_fixedListOfBlobs.GetBlob(i);
        CvPoint2D64f cntr;
        currentBlob->returnBlobsCentroids(cntr);
        m_currFixedCentroids.push_back(cntr);
        //std::cout<<"\n   i == "<< i<<"("<<cntr.x<<","<<cntr.y<<")"<<std::endl;

    }

    FILE_LOG(logDEBUG1)<<"End of fixBlobsAndReturnGoodBlobs";
    //std::cout<<"end"<<std::endl;
    return(FK_OK);



}


FkInt32U returnLastValidMeasurementIndx()
{
    FILE_LOG(logDEBUG1)<<"begin of returnLastValidMeasurementIndx";
    FkInt32U indxOfMostRecentValidMeasurement = -1;

    //std::cout<<"\nm_prevDetectedBlobs.size()  =  "<<m_prevDetectedBlobs.size()<<std::endl;

    int sizeOfPrevBlobs = (int)m_prevDetectedBlobs.size();

    for (int i = sizeOfPrevBlobs - 1;i>=0;i--)
    {
        if (m_prevDetectedBlobs[i].GetNumBlobs() == m_numOfTargets)
        {
            indxOfMostRecentValidMeasurement = i;
            break;
        }
    }
        FILE_LOG(logDEBUG1)<<"end of returnLastValidMeasurementIndx";
    return (indxOfMostRecentValidMeasurement);

}




FkInt32S initTextOutput(std::string fileName)
{
		
	m_OutputText.open(fileName.c_str());
m_OutputText<<"FrameNumber,";
	for (int i = 0;i<m_numOfTargets-1;i++) //writing the header
		m_OutputText<<"ID"<<i<<"sex"<<i<<",x"<<i<<",y"<<i<<",";

	m_OutputText<<"ID"<<m_numOfTargets-1<<"sex"<<m_numOfTargets-1<<",x"<<m_numOfTargets-1<<",y"<<m_numOfTargets-1<<std::endl;
	return(FK_OK);

	
	};



FkInt32S appendToTextOutput(FkInt32S frameNumber)
{
    m_OutputText<<frameNumber<<",";
    if(!m_OutputText.is_open())
        return(FK_ERR_ARENA_CAN_NOT_APPEND_TO_TEXT_OUTPUT);

        FkInt32U j;
        CBlob* currBlob;
        for (j= 0;j<(this->m_currFixedCentroids.size())-1;j++)
        {
            currBlob = this->m_fixedListOfBlobs.GetBlob(j);
            m_OutputText<<currBlob->getIDNumber()<<","<<currBlob->getSex()<<","<<(int)m_currFixedCentroids[j].x<<","<<(int)m_currFixedCentroids[j].y<<",";
        }
        currBlob = this->m_fixedListOfBlobs.GetBlob((this->m_currFixedCentroids.size())-1);
        m_OutputText<<currBlob->getIDNumber()<<","<<currBlob->getSex()<<","<<(int)m_currFixedCentroids[j].x<<","<<(int)m_currFixedCentroids[j].y<<std::endl;
    return (FK_OK);

}

};


