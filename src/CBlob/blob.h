/************************************************************************
  			Blob.h

FUNCIONALITAT: Definició de la classe CBlob
AUTOR: Inspecta S.L.
MODIFICACIONS (Modificació, Autor, Data):

FUNCTIONALITY: Definition of the CBlob class and some helper classes to perform
			   some calculations on it
AUTHOR: Inspecta S.L.
MODIFICATIONS (Modification, Author, Date):

**************************************************************************/

//! Disable warnings referred to 255 character truncation for the std:map
//#pragma warning( disable : 4786 )

#ifndef CBLOB_INSPECTA_INCLUDED
#define CBLOB_INSPECTA_INCLUDED

#include "cxcore.h"
#include "highgui.h"
#include "..\log.h"


//#include "BlobLibraryConfiguration.h"
#include "BlobContour.h"

#ifdef __COLOR_DETECTION_ACTIVATED
//#include "..\CamView.h"
#endif


//#ifdef BLOB_OBJECT_FACTORY
//	//! Object factory pattern implementation
//	#include "..\inspecta\DesignPatterns\ObjectFactory.h"
//#endif


//! Type of labelled images
typedef unsigned int t_labelType;


//////////////////////////////////definitions for getBlobColor////////////////////////


#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define UNIDENTIFIED -1

///////////////////////////////////////////////////////////////////////////////////////////

	typedef std::list<CBlobContour> t_contourList; 


//! Blob class
class CBlob
{


public:
	CBlob();
	CBlob( t_labelType id, CvPoint startPoint, CvSize originalImageSize );
	~CBlob();
	//CBlob(CvMemStorage* inputMemStorage, CvSeq* contour );

	void setBlobContour(CvSeq *in, int id);


	//! Copy constructor
	CBlob( const CBlob &src );
	CBlob( const CBlob *src );

	//! Operador d'assignació
	//! Assigment operator
	CBlob& operator=(const CBlob &src );

	//! Adds a new internal contour to the blob
	void AddInternalContour( const CBlobContour &newContour );

	//! Retrieves contour in Freeman's chain code
	CBlobContour *GetExternalContour()
	{
		return &m_externalContour;
	}

	//! Retrieves blob storage
	CvMemStorage *GetStorage()
	{
		return m_storage;
	}

	//! Get label ID
	t_labelType GetID()
	{
		return m_id;
	}
	//! > 0 for extern blobs, 0 if not
	int	  Exterior( IplImage *mask, bool xBorder = true, bool yBorder = true );
	//! Compute blob's area
	double Area();
	//! Compute blob's perimeter
	double Perimeter();
	//! Compute blob's moment (p,q up to MAX_CALCULATED_MOMENTS)
	double Moment(int p, int q);

	//! Compute extern perimeter
	double ExternPerimeter( IplImage *mask, bool xBorder  = true, bool yBorder = true );

	//! Get mean grey color
	double Mean( IplImage *image, int ID, int roiX, int roiY );

	//! Get standard deviation grey color
	double StdDev( IplImage *image );

	//! Indica si el blob està buit ( no té cap info associada )
	//! Shows if the blob has associated information
	bool IsEmpty();

	//! Retorna el poligon convex del blob
	//! Calculates the convex hull of the blob
	t_PointList GetConvexHull();

	//! Pinta l'interior d'un blob d'un color determinat
	//! Paints the blob in an image
	void FillBlob( IplImage *imatge, CvScalar color, int offsetX = 0, int offsetY = 0 );

	//! Join a blob to current one (add's contour
	void JoinBlob( CBlob *blob );

	//! Get bounding box
	CvRect GetBoundingBox();
	//! Get bounding ellipse
	CvBox2D GetEllipse();

	//! Minimun X
	double MinX()
	{
		return GetBoundingBox().x;
	}
	//! Minimun Y
	double MinY()
	{
		return GetBoundingBox().y;
	}
	//! Maximun X
	double MaxX()
	{
		return GetBoundingBox().x + GetBoundingBox().width;
	}
	//! Maximun Y
	double MaxY()
	{
		return GetBoundingBox().y + GetBoundingBox().height;
	}


#ifdef __COLOR_DETECTION_ACTIVATED
   char getBlobColor(IplImage *img,double &ConfidenceValue,int SIZE_OF_BOX,bool DisplayImage,CCamView* view,int WaitKey);//,std::string fileName);
#endif

	bool saveBlobImg(std::string fileName);

    bool returnBlobsCentroids(CvPoint2D64f &returnedCenter);

//	int NumberOfCentroids;

	int numberOfMergedBlobs(double thresh1, double thresh3);

	bool isMerged(double thresh1, double thresh3);
	//std::vector<CBlob*> splitBlob(int numCluster);

//	bool splitBlob(int numCluster, std::vector<CBlob*>& resultBlobs);

	//bool CBlob::splitBlob(int numCluster, std::vector<CvSeq*>& clusteredContours);
//bool CBlob::splitBlob(int numCluster, std::vector<CvSeq*>& clusteredContours, std::vector<std::vector<CvPoint2D32f>> &clusteredPoint);
bool CBlob::splitBlob(int numCluster, std::vector<std::vector<CvPoint2D32f>> &clusteredPoint);

IplImage* blobOrigImage;

    void setIDNumber(int inp)
    {
        m_IDnumber = inp;
    }
    int getIDNumber()
    {
        return(m_IDnumber);
    }


     void setSex(char inp)
    {
        m_sex = inp;
    }
    char getSex()
    {
        return(m_sex);
    }

private:

	//! Deallocates all contours
	void ClearContours();
	//////////////////////////////////////////////////////////////////////////
	// Blob contours
	//////////////////////////////////////////////////////////////////////////


	//! Contour storage memory
	CvMemStorage *m_storage;
	//! External contour of the blob (crack codes)
	CBlobContour m_externalContour;
	//! Internal contours (crack codes)
	t_contourList m_internalContours;

	//////////////////////////////////////////////////////////////////////////
	// Blob features
	//////////////////////////////////////////////////////////////////////////

	//! Label number
	t_labelType m_id;
	//! Area
	double m_area;
	//! Perimeter
	double m_perimeter;
	//! Extern perimeter from blob
	double m_externPerimeter;
	//! Mean gray color
	double m_meanGray;
	//! Standard deviation from gray color blob distribution
	double m_stdDevGray;
	//! Bounding box
	CvRect m_boundingBox;
	//! Bounding ellipse
	CvBox2D m_ellipse;
	//! Sizes from image where blob is extracted
	CvSize m_originalImageSize;
    //Ardekani added
    int m_IDnumber; // for tracking;
    char m_sex; // for fly tracking 'M'ale, 'F'emale, 'U'nknown



};

#endif //CBLOB_INSPECTA_INCLUDED
