/************************************************************************
  			Blob.cpp

- FUNCIONALITAT: Implementació de la classe CBlob
- AUTOR: Inspecta S.L.
MODIFICACIONS (Modificació, Autor, Data):


FUNCTIONALITY: Implementation of the CBlob class and some helper classes to perform
			   some calculations on it
AUTHOR: Inspecta S.L.
MODIFICATIONS (Modification, Author, Date):

**************************************************************************/
#include <algorithm>
#include <iostream>
#ifdef __COLOR_DETECTION_ACTIVATED
//---------------------begin of added for color -----------------------
int max4(int a,int b,int c,int d) // return Maximum of 4integer  values
{
	int m = a;
	m = std::max(m,b);
	m = std::max(m,c);
	m = std::max(m,d);
	return m;
}

int max3(int a,int b,int c) // Return maximum of 3 integer values
{
	int m=a;
	m = std::max(m,b);
	m = std::max(m,c);
	return m;
}
//---------------------------end of added for color-----------------------------
#endif

#include "blob.h"



CBlob::CBlob()
{
	m_area = m_perimeter = -1;
	m_externPerimeter = m_meanGray = m_stdDevGray = -1;
	m_boundingBox.width = -1;
	m_ellipse.size.width = -1;
	m_storage = NULL;
	m_id = -1;
	blobOrigImage = NULL;
    m_IDnumber = -1;
    m_sex = 'U';
}
CBlob::CBlob( t_labelType id, CvPoint startPoint, CvSize originalImageSize )
{
    m_IDnumber = -1;
    m_sex = 'U';
	m_id = id;
	m_area = m_perimeter = -1;
	m_externPerimeter = m_meanGray = m_stdDevGray = -1;
	m_boundingBox.width = -1;
	m_ellipse.size.width = -1;
	m_storage = cvCreateMemStorage();
	m_externalContour = CBlobContour(startPoint, m_storage);
	m_originalImageSize = originalImageSize;
	blobOrigImage = NULL;
}

//CBlob::CBlob(CvMemStorage* inputMemStorage, CvSeq* contour )
//{
//	CvPoint* tmp = (CvPoint*)cvGetSeqElem(contour,0);
//	CvPoint startPoint;
//	startPoint.x = tmp->x;
//	startPoint.y = tmp->y;
//	//m_id = id;
//	m_area = m_perimeter = -1;
//	m_externPerimeter = m_meanGray = m_stdDevGray = -1;
//	m_boundingBox.width = -1;
//	m_ellipse.size.width = -1;
//	m_storage = inputMemStorage;
//	m_externalContour = CBlobContour(startPoint, m_storage);
//	//m_originalImageSize = originalImageSize;
//}

void CBlob::setBlobContour(CvSeq *inContour, int ID)
{
	CvPoint strtPt;
	CvPoint *tmp;
	tmp = (CvPoint*)cvGetSeqElem(inContour,0);
	strtPt.x = tmp->x;strtPt.y = tmp->y;
	ClearContours();
	if( m_storage )
		cvReleaseMemStorage( &m_storage );
	m_storage = cvCreateMemStorage();
	m_externalContour = CBlobContour(strtPt, m_storage );
	//if( src.m_externalContour.m_contour )
	m_externalContour.m_contour = cvCloneSeq( inContour, m_storage);
	//m_externalContour.m
	m_id = ID;

    m_IDnumber = -1;
    m_sex = 'U';


		//if( source.m_contourPoints )
		//{
		//	if( m_contourPoints )
		//		cvClearSeq( m_contourPoints );
		//	m_contourPoints = cvCloneSeq( source.m_contourPoints, m_parentStorage);
		//}

}





//! Copy constructor
CBlob::CBlob( const CBlob &src )
{
	m_storage = NULL;
	blobOrigImage = NULL;
	*this = src;
}

CBlob::CBlob( const CBlob *src )
{
	if (src != NULL )
	{
		m_storage = NULL;
		blobOrigImage = NULL;
		*this = *src;
	}
}

CBlob& CBlob::operator=(const CBlob &src )
{
	if( this != &src )
	{
		m_id = src.m_id;
		m_area = src.m_area;
		m_perimeter = src.m_perimeter;
		m_externPerimeter = src.m_externPerimeter;
		m_meanGray = src.m_meanGray;
		m_stdDevGray = src.m_stdDevGray;
		m_boundingBox = src.m_boundingBox;
		m_ellipse = src.m_ellipse;
		m_originalImageSize = src.m_originalImageSize;

        m_IDnumber = src.m_IDnumber;
		// clear all current blob contours
		ClearContours();

		if( m_storage )
			cvReleaseMemStorage( &m_storage );

		m_storage = cvCreateMemStorage();

		m_externalContour = CBlobContour(src.m_externalContour.GetStartPoint(), m_storage );
		if( src.m_externalContour.m_contour )
			m_externalContour.m_contour = cvCloneSeq( src.m_externalContour.m_contour, m_storage);
		m_internalContours.clear();

		// copy all internal contours
		if( src.m_internalContours.size() )
		{
			m_internalContours = t_contourList( src.m_internalContours.size() );
			t_contourList::const_iterator itSrc;
			t_contourList::iterator it;

			itSrc = src.m_internalContours.begin();
			it = m_internalContours.begin();

			while (itSrc != src.m_internalContours.end())
			{
				*it = CBlobContour((*itSrc).GetStartPoint(), m_storage);
				if( (*itSrc).m_contour )
					(*it).m_contour = cvCloneSeq( (*itSrc).m_contour, m_storage);

				it++;
				itSrc++;
			}
		}
	}

	return *this;
}

CBlob::~CBlob()
{
	ClearContours();

	if( m_storage )
		cvReleaseMemStorage( &m_storage );
	if(blobOrigImage)
		cvReleaseImage( &blobOrigImage);
}

void CBlob::ClearContours()
{
	t_contourList::iterator it;

	it = m_internalContours.begin();

	while (it != m_internalContours.end())
	{
		(*it).ResetChainCode();
		it++;
	}
	m_internalContours.clear();

	m_externalContour.ResetChainCode();

}
void CBlob::AddInternalContour( const CBlobContour &newContour )
{
	m_internalContours.push_back(newContour);
}

//! Indica si el blob està buit ( no té cap info associada )
//! Shows if the blob has associated information
bool CBlob::IsEmpty()
{
	return GetExternalContour()->m_contour == NULL;
}

/**
- FUNCIÓ: Area
- FUNCIONALITAT: Get blob area, ie. external contour area minus internal contours area
- PARÀMETRES:
	-
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/04/30
- MODIFICACIÓ: Data. Autor. Descripció.
*/
double CBlob::Area()
{
	double area;
	t_contourList::iterator itContour;

	area = m_externalContour.GetArea();

	itContour = m_internalContours.begin();

	while (itContour != m_internalContours.end() )
	{
		area -= (*itContour).GetArea();
		itContour++;
	}
	return area;
}

/**
- FUNCIÓ: Perimeter
- FUNCIONALITAT: Get blob perimeter, ie. sum of the lenght of all the contours
- PARÀMETRES:
	-
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/04/30
- MODIFICACIÓ: Data. Autor. Descripció.
*/
double CBlob::Perimeter()
{
	double perimeter;
	t_contourList::iterator itContour;

	perimeter = m_externalContour.GetPerimeter();

	itContour = m_internalContours.begin();

	while (itContour != m_internalContours.end() )
	{
		perimeter += (*itContour).GetPerimeter();
		itContour++;
	}
	return perimeter;

}

/**
- FUNCIÓ: Exterior
- FUNCIONALITAT: Return true for extern blobs
- PARÀMETRES:
	- xBorder: true to consider blobs touching horizontal borders as extern
	- yBorder: true to consider blobs touching vertical borders as extern
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/05/06
- MODIFICACIÓ: Data. Autor. Descripció.
*/
int	CBlob::Exterior(IplImage *mask, bool xBorder /* = true */, bool yBorder /* = true */)
{
	if (ExternPerimeter(mask, xBorder, yBorder ) > 0 )
	{
		return 1;
	}

	return 0;
}
/**
- FUNCIÓ: ExternPerimeter
- FUNCIONALITAT: Get extern perimeter (perimeter touching image borders)
- PARÀMETRES:
	- maskImage: if != NULL, counts maskImage black pixels as external pixels and contour points touching
				 them are counted as external contour points.
	- xBorder: true to consider blobs touching horizontal borders as extern
	- yBorder: true to consider blobs touching vertical borders as extern
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/05/05
- MODIFICACIÓ: Data. Autor. Descripció.
- NOTA: If CBlobContour::GetContourPoints aproximates contours with a method different that NONE,
		this function will not give correct results
*/
double CBlob::ExternPerimeter( IplImage *maskImage, bool xBorder /* = true */, bool yBorder /* = true */)
{
	t_PointList externContour, externalPoints;
	CvSeqReader reader;
	CvSeqWriter writer;
	CvPoint actualPoint, previousPoint;
	bool find = false;
	int i,j;
	int delta = 0;

	// it is calculated?
	if( m_externPerimeter != -1 )
	{
		return m_externPerimeter;
	}

	// get contour pixels
	externContour = m_externalContour.GetContourPoints();

	m_externPerimeter = 0;

	// there are contour pixels?
	if( externContour == NULL )
	{
		return m_externPerimeter;
	}

	cvStartReadSeq( externContour, &reader);

	// create a sequence with the external points of the blob
	externalPoints = cvCreateSeq( externContour->flags, externContour->header_size, externContour->elem_size,
								  m_storage );
	cvStartAppendToSeq( externalPoints, &writer );
	previousPoint.x = -1;

	// which contour pixels touch border?
	for( j=0; j< externContour->total; j++)
	{
		CV_READ_SEQ_ELEM( actualPoint, reader);

		find = false;

		// pixel is touching border?
		if ( xBorder & ((actualPoint.x == 0) || (actualPoint.x == m_originalImageSize.width - 1 )) ||
			 yBorder & ((actualPoint.y == 0) || (actualPoint.y == m_originalImageSize.height - 1 )))
		{
			find = true;
		}
		else
		{
			if( maskImage != NULL )
			{
				// verify if some of 8-connected neighbours is black in mask
				char *pMask;

				pMask = (maskImage->imageData + actualPoint.x - 1 + (actualPoint.y - 1) * maskImage->widthStep);

				for ( i = 0; i < 3; i++, pMask++ )
				{
					if(*pMask == 0 && !find )
					{
						find = true;
						break;
					}
				}

				if(!find)
				{
					pMask = (maskImage->imageData + actualPoint.x - 1 + (actualPoint.y ) * maskImage->widthStep);

					for ( i = 0; i < 3; i++, pMask++ )
					{
						if(*pMask == 0 && !find )
						{
							find = true;
							break;
						}
					}
				}

				if(!find)
				{
					pMask = (maskImage->imageData + actualPoint.x - 1 + (actualPoint.y + 1) * maskImage->widthStep);

					for ( i = 0; i < 3; i++, pMask++ )
					{
						if(*pMask == 0 && !find )
						{
							find = true;
							break;
						}
					}
				}
			}
		}

		if( find )
		{
			if( previousPoint.x > 0 )
				delta = abs(previousPoint.x - actualPoint.x) + abs(previousPoint.y - actualPoint.y);

			// calculate separately each external contour segment
			if( delta > 2 )
			{
				cvEndWriteSeq( &writer );
				m_externPerimeter += cvArcLength( externalPoints, CV_WHOLE_SEQ, 0 );

				cvClearSeq( externalPoints );
				cvStartAppendToSeq( externalPoints, &writer );
				delta = 0;
				previousPoint.x = -1;
			}

			CV_WRITE_SEQ_ELEM( actualPoint, writer );
			previousPoint = actualPoint;
		}

	}

	cvEndWriteSeq( &writer );

	m_externPerimeter += cvArcLength( externalPoints, CV_WHOLE_SEQ, 0 );

	cvClearSeq( externalPoints );

	// divide by two because external points have one side inside the blob and the other outside
	// Perimeter of external points counts both sides, so it must be divided
	m_externPerimeter /= 2.0;

	return m_externPerimeter;
}

//! Compute blob's moment (p,q up to MAX_CALCULATED_MOMENTS)
double CBlob::Moment(int p, int q)
{
	double moment;
	t_contourList::iterator itContour;

	moment = m_externalContour.GetMoment(p,q);

	itContour = m_internalContours.begin();

	while (itContour != m_internalContours.end() )
	{
		moment -= (*itContour).GetMoment(p,q);
		itContour++;
	}
	return moment;
}

/**
- FUNCIÓ: Mean
- FUNCIONALITAT: Get blob mean color in input image
- PARÀMETRES:
	- image: image from gray color are extracted
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/05/06
- MODIFICACIÓ: Data. Autor. Descripció.
*/
double CBlob::Mean( IplImage *image, int ID, int roiX, int roiY )
{
	// it is calculated?
/*	if( m_meanGray != -1 )
	{
		return m_meanGray;
	}
*/
	// Create a mask with same size as blob bounding box
	IplImage *mask;
	CvScalar mean, std;
	CvPoint offset;

	GetBoundingBox();

	if (m_boundingBox.height == 0 ||m_boundingBox.width == 0 || !CV_IS_IMAGE( image ))
	{
		m_meanGray = 0;
		return m_meanGray;
	}

	// apply ROI and mask to input image to compute mean gray and standard deviation
	mask = cvCreateImage( cvSize(m_boundingBox.width, m_boundingBox.height), IPL_DEPTH_8U, 1);
	cvSetZero(mask);

	offset.x = -m_boundingBox.x;
	offset.y = -m_boundingBox.y;

    
	// draw contours on mask
	cvDrawContours( mask, m_externalContour.GetContourPoints(), CV_RGB(255,255,255), CV_RGB(255,255,255),0, CV_FILLED, 8,
					offset );

	// draw internal contours
	t_contourList::iterator it = m_internalContours.begin();
	while(it != m_internalContours.end() )
	{
		cvDrawContours( mask, (*it).GetContourPoints(), CV_RGB(0,0,0), CV_RGB(0,0,0),0, CV_FILLED, 8,
					offset );
		it++;
	}


    m_boundingBox.x += roiX;
    m_boundingBox.y += roiY;



	cvSetImageROI( image, m_boundingBox );

    //char wName[2000];
    //sprintf(wName,"afterROI---- %d",ID);
    //cvShowImage(wName,image);
    //sprintf(wName,"%d_%d_%d.jpg",ID,roiX,roiY);
    //cvSaveImage(wName,image);
    //sprintf(wName,"Mask--- %d",ID);
    //cvShowImage(wName,mask);

    //sprintf(wName,"%d_%d_%d_mask.jpg",ID,roiX,roiY);
    //cvSaveImage(wName,mask);
    
    cvWaitKey(1);
	cvAvgSdv( image, &mean, &std, mask );

	m_meanGray = mean.val[0];
	m_stdDevGray = std.val[0];
//BGR 2 gray
//intensity = 0.2989*red + 0.5870*green + 0.1140*blue
    m_meanGray = (int) (0.2989*mean.val[2] + 0.5870*mean.val[1] + 0.1140*mean.val[0]);


	cvReleaseImage( &mask );
	cvResetImageROI( image );

	return m_meanGray;
}

double CBlob::StdDev( IplImage *image )
{
	// it is calculated?
/*	if( m_stdDevGray != -1 )
	{
		return m_stdDevGray;
	}
*/
	// call mean calculation (where also standard deviation is calculated)
	Mean( image ,0,0,0 );

	return m_stdDevGray;
}
/**
- FUNCIÓ: GetBoundingBox
- FUNCIONALITAT: Get bounding box (without rotation) of a blob
- PARÀMETRES:
	-
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/05/06
- MODIFICACIÓ: Data. Autor. Descripció.
*/
CvRect CBlob::GetBoundingBox()
{
	// it is calculated?
	if( m_boundingBox.width != -1 )
	{
		return m_boundingBox;
	}

	t_PointList externContour;
	CvSeqReader reader;
	CvPoint actualPoint;

	// get contour pixels
	externContour = m_externalContour.GetContourPoints();

	// it is an empty blob?
	if( !externContour )
	{
		m_boundingBox.x = 0;
		m_boundingBox.y = 0;
		m_boundingBox.width = 0;
		m_boundingBox.height = 0;

		return m_boundingBox;
	}

	cvStartReadSeq( externContour, &reader);

	m_boundingBox.x = m_originalImageSize.width;
	m_boundingBox.y = m_originalImageSize.height;
	m_boundingBox.width = 0;
	m_boundingBox.height = 0;

	for( int i=0; i< externContour->total; i++)
	{
		CV_READ_SEQ_ELEM( actualPoint, reader);

		m_boundingBox.x = MIN( actualPoint.x, m_boundingBox.x );
		m_boundingBox.y = MIN( actualPoint.y, m_boundingBox.y );

		m_boundingBox.width = MAX( actualPoint.x, m_boundingBox.width );
		m_boundingBox.height = MAX( actualPoint.y, m_boundingBox.height );
	}

	//m_boundingBox.x = max( m_boundingBox.x , 0 );
	//m_boundingBox.y = max( m_boundingBox.y , 0 );

	m_boundingBox.width -= m_boundingBox.x;
	m_boundingBox.height -= m_boundingBox.y;

	return m_boundingBox;
}

/**
- FUNCIÓ: GetEllipse
- FUNCIONALITAT: Calculates bounding ellipse of external contour points
- PARÀMETRES:
	-
- RESULTAT:
	-
- RESTRICCIONS:
	-
- AUTOR: rborras
- DATA DE CREACIÓ: 2008/05/06
- MODIFICACIÓ: Data. Autor. Descripció.
- NOTA: Calculation is made using second order moment aproximation
*/
CvBox2D CBlob::GetEllipse()
{
	// it is calculated?
	if( m_ellipse.size.width != -1 )
		return m_ellipse;

	double u00,u11,u01,u10,u20,u02, delta, num, den, temp;

	// central moments calculation
	u00 = Moment(0,0);

	// empty blob?
	if ( u00 <= 0 )
	{
		m_ellipse.size.width = 0;
		m_ellipse.size.height = 0;
		m_ellipse.center.x = 0;
		m_ellipse.center.y = 0;
		m_ellipse.angle = 0;
		return m_ellipse;
	}
	u10 = Moment(1,0) / u00;
	u01 = Moment(0,1) / u00;

	u11 = -(Moment(1,1) - Moment(1,0) * Moment(0,1) / u00 ) / u00;
	u20 = (Moment(2,0) - Moment(1,0) * Moment(1,0) / u00 ) / u00;
	u02 = (Moment(0,2) - Moment(0,1) * Moment(0,1) / u00 ) / u00;


	// elipse calculation
	delta = sqrt( 4*u11*u11 + (u20-u02)*(u20-u02) );
	m_ellipse.center.x = u10;
	m_ellipse.center.y = u01;

	temp = u20 + u02 + delta;
	if( temp > 0 )
	{
		m_ellipse.size.width = sqrt( 2*(u20 + u02 + delta ));
	}
	else
	{
		m_ellipse.size.width = 0;
		return m_ellipse;
	}

	temp = u20 + u02 - delta;
	if( temp > 0 )
	{
		m_ellipse.size.height = sqrt( 2*(u20 + u02 - delta ) );
	}
	else
	{
		m_ellipse.size.height = 0;
		return m_ellipse;
	}

	// elipse orientation
	if (u20 > u02)
	{
		num = u02 - u20 + sqrt((u02 - u20)*(u02 - u20) + 4*u11*u11);
		den = 2*u11;
	}
    else
    {
		num = 2*u11;
		den = u20 - u02 + sqrt((u20 - u02)*(u20 - u02) + 4*u11*u11);
    }
	if( num != 0 && den  != 00 )
	{
		m_ellipse.angle = 180.0 + (180.0 / CV_PI) * atan( num / den );
	}
	else
	{
		m_ellipse.angle = 0;
	}

	return m_ellipse;

}

/**
- FUNCTION: FillBlob
- FUNCTIONALITY:
	- Fills the blob with a specified colour
- PARAMETERS:
	- imatge: where to paint
	- color: colour to paint the blob
- RESULT:
	- modifies input image and returns the seed point used to fill the blob
- RESTRICTIONS:
- AUTHOR: Ricard Borràs
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlob::FillBlob( IplImage *imatge, CvScalar color, int offsetX /*=0*/, int offsetY /*=0*/)
{
	cvDrawContours( imatge, m_externalContour.GetContourPoints(), color, color,0, CV_FILLED, 8 );
}


/**
- FUNCTION: GetConvexHull
- FUNCTIONALITY: Calculates the convex hull polygon of the blob
- PARAMETERS:
	- dst: where to store the result
- RESULT:
	- true if no error ocurred
- RESTRICTIONS:
- AUTHOR: Ricard Borràs
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
t_PointList CBlob::GetConvexHull()
{
	CvSeq *convexHull = NULL;

	if( m_externalContour.GetContourPoints() )
		convexHull = cvConvexHull2( m_externalContour.GetContourPoints(), m_storage,
					   CV_COUNTER_CLOCKWISE, 1 );

	return convexHull;
}

/**
- FUNCTION: JoinBlob
- FUNCTIONALITY: Add's external contour to current external contour
- PARAMETERS:
	- blob: blob from which extract the added external contour
- RESULT:
	- true if no error ocurred
- RESTRICTIONS: Only external contours are added
- AUTHOR: Ricard Borràs
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlob::JoinBlob( CBlob *blob )
{
	CvSeqWriter writer;
	CvSeqReader reader;
	t_chainCode chainCode;

	cvStartAppendToSeq( m_externalContour.GetChainCode(), &writer );
	cvStartReadSeq( blob->GetExternalContour()->GetChainCode(), &reader );

	for (int i = 0; i < blob->GetExternalContour()->GetChainCode()->total; i++ )
	{
		CV_READ_SEQ_ELEM( chainCode, reader );
		CV_WRITE_SEQ_ELEM( chainCode, writer );
	}
	cvEndWriteSeq( &writer );

}




bool CBlob::saveBlobImg(std::string fileName)
{
	if (blobOrigImage)
	{
		cvSaveImage(fileName.c_str(),blobOrigImage);
		return true;
	}

	else
	{
		printf("\nCan not save current blob");
		return false;
	}

	//CvRect blbrct = this->GetBoundingBox();//GetBoundsRect();
	//IplImage* tempImg = cvCloneImage(img);

	//cvSetImageROI(tempImg,blbrct);
	//IplImage *img2 = cvCreateImage(cvGetSize(tempImg),tempImg->depth,tempImg->nChannels);
	//cvZero(img2);
	//cvCopy(tempImg, img2, NULL);

	//
	//
//	cvSaveImage(fileName.c_str(),img2);
	//cvResetImageROI(tempImg);	
	//cvReleaseImage(&tempImg);

	//cvReleaseImage(&img2);
//	return true;


}

/////////////////////////////////////////

//#define __JUSTSAVEBLOBSFORTRAINING 1
#ifdef __COLOR_DETECTION_ACTIVATED

char CBlob::getBlobColor(IplImage *img,double &ConfidenceValue,int SIZE_OF_BOX,bool DisplayImage,CCamView* view, int WaitKey)//, std::string fileName = NULL)
{
	CvRect blbrct = this->GetBoundingBox();//GetBoundsRect();

//return UNIDENTIFIED; //TODO: this is a quick HACK TO TEST SOMETHING ELSE...FIX IT!


	//IplImage *DispImg; // Stores the image (displaying "thresholded colors") to be displayed, if DisplayImage == true
	//if(DisplayImage)
	//{
	//	DispImg = cvCreateImage( cvGetSize(img), img->depth, img->nChannels);
	//	if(DispImg==NULL) // Could not allocate memory for image then do not display image
	//		DisplayImage = false;
	//}


	//printf("\n(x= %d,w = %d,y = %d,h = %d)\n",blbrct.x,blbrct.width,blbrct.y,blbrct.height);
	//getchar();

	int nr=0,ny=0,nb=0,ng=0,nu=0; // Number of red, yellow, blue, green, unindentified pixels in image respectively

	
	// Go thru all pixels of the image

	for(int row=blbrct.y;row<blbrct.y + blbrct.height ;row++) for(int col=blbrct.y;col<blbrct.y + blbrct.height;col++)
	{
		for(int col=blbrct.x;col<blbrct.x + blbrct.width;col++) //an impartant bug has removed here,row an column were mixed!
		{
			int bavg=0,gavg=0,ravg=0;
			int total_pixel=0;
			CvScalar sC;
			if(SIZE_OF_BOX == 0) // Special case when no averaging is done
			{
				sC = cvGet2D(img,row,col);
				bavg = (int)sC.val[0];
				gavg = (int)sC.val[1];
				ravg = (int)sC.val[2];
				total_pixel=1;
			}
			else
			{
				// Compute the total sum of all blue, green and red pixels in the "BOX"
				for(int r=max(row-SIZE_OF_BOX,0);r<min(img->height,row+SIZE_OF_BOX);r++)
				{
					for(int c=max(col-SIZE_OF_BOX,0);c<min(img->width,col+SIZE_OF_BOX);c++)
					{
						total_pixel++;
						sC = cvGet2D(img,r,c);
						bavg += (int)sC.val[0];
						gavg += (int)sC.val[1];
						ravg += (int)sC.val[2];
					}
				}
			}
		
			assert(total_pixel!=0); // No box could be drawn, Should NEVER be the case .... try tweaking SIZE_OF_BOX
			
			// Calculate the real average now (till now "bavg", "ravg", "gavg" actually contained the total summation)
			bavg /= total_pixel;
			gavg /= total_pixel;
			ravg /= total_pixel;
			
			if(bavg<5 && ravg<5 && gavg<5) // Approximatly black pixel, Do nothing (just mark the Pixel as black if DisplayImage == true);
			{
				//if(DisplayImage)
				//	cvSet2D(DispImg,row,col,cvScalar(0));
				continue;
			}
//			int ret_color = whichColor(bavg,gavg,ravg); // This is the function which really classify individual pixel, need to be changed according to experimental setup
			char ret_color = view->viewColorClassifier.whichColor(bavg,gavg,ravg);
			ret_color = toupper(ret_color);
//			std::cout<<"here: -> "<<ret_color;
			CvScalar ToWrite(cvScalar(0,0,0)); // This pixel is going to be written on DispImg, has no sense if DisplayImage==false
			
			switch(ret_color)
			{
			case 'Y':
				ny++;
				ToWrite.val[1] = 255;
				ToWrite.val[2] = 255;
				break;
			case 'R':
				nr++;
				ToWrite.val[2] = 255;
				break;
			case 'B':
				nb++;
				ToWrite.val[0] = 255;
				break;
			case 'G':
				ng++;
				ToWrite.val[1] = 255;
				break;
			case 'U':
				nu++;
				break;
			}
			//if(DisplayImage)
			//{
			//	cvSet2D(DispImg,row,col,ToWrite);
			//}

		}
	}
	//if(DisplayImage)
	//{
	//	cvNamedWindow("Colors Identified in Image");
	//	cvShowImage("Colors Identified in Image",DispImg);
	//	cvWaitKey(WaitKey);

	//	//Uncomment the cvDestroyWindow() below (if you are not destroying all the window yourself) -- 
	//	//Commented just for testing code, should be uncommented in final version

	//	//cvDestroyWindow("Colors Identified in Image");
	//	cvReleaseImage(&DispImg);
	//}
	int TotalColoredPixels = nr+ny+ng+nb; // Total Colored pixels (of any color - RED,GREEN, BLUE or YELLOW)
	int MaxColor = max4(nr,ny,ng,nb); //The maximum number of colored pixel of a single color (not counting "unidentified" pixel)

//	printf("\n(r= %d,y = %d,g = %d,b = %d)\n",nr,ny,ng,nb);

	if(TotalColoredPixels == 0) // => None of the colors (R,G,B or Y) was present, then we we have 100% confidence of image to be classified as: UNIDENTIFIED
	{
		ConfidenceValue = 1.0;
		return 'U';
	}
	else
		ConfidenceValue = (double)MaxColor/(double)TotalColoredPixels;

	// Note with the method below, if two colors have same number of pixel (and are higher than other two), then priority order would be decided by 
	// the order in which the "if" are written below (g > b > r > y), Note: In this case, ConfValue < 50% 
	if(MaxColor == ng)
		return 'G';
	if(MaxColor == nb)
		return 'B';
	if(MaxColor == nr)
		return 'R';
	if(MaxColor == ny)
		return 'Y';

	return 'U';
}
////////////////////////////////////////////////
#endif
#define MAXITERATIONS 50 //for k-means
#define MAXEPS 1.0	
#define MAXTRIES 5


bool CBlob::returnBlobsCentroids(CvPoint2D64f &center)
{

#if 0//def  __NOT_USING_MOMENTS
//	CvPoint2D32f center;
	center.x = 0;center.y = 0;
	double totalX=0.0,totalY=0.0;
	double total=0.0;

	CvSeq* contour = NULL;
	contour = m_externalContour.m_contourPoints;
	if (contour == NULL)
	{
		printf("\n contour is NULL!");
		return false;
	}

	CvRect boundbox ;
	boundbox = cvBoundingRect(contour);
// NOTE IMPORTANT! MAKE SURE THE WAY THAT THIS THING RETURNS RECT IS CORRENT...BOUNDBOX.WIDTH-BOUDBOX.X MIGHT BE NEGATIVE (MAKE SURE IT RETURNS TOP LEFT AND WIDTH AND HEIGHT
	//IT SEEMS IT DOESNT!
	for (int ii=0;ii<boundbox.width-boundbox.x;ii++) 
		for (int jj=0;jj<boundbox.height-boundbox.y;jj++)
		{
			CvPoint2D32f tmpPt = cvPoint2D32f((float)( boundbox.x+ ii),(float)(boundbox.y + jj));

			if (cvPointPolygonTest(contour, tmpPt,false)>0)
			{
				totalX += tmpPt.x;
				totalY += tmpPt.y;
				total++;
			}
		}

		center.x = totalX/total;
		center.y = totalY/total;

#else
    //double m00 = this->Moment(0,0);
    //double m10 = this->Moment(1,0);
    //double m01 = this->Moment(0,1);
    //printf("\n m00 = %d m10 = %d m01 = %d",m00,m10,m01);
    //center.x = floor(m10/m00);
    //center.y = floor(m01/m00);

    //std::cout<<"\ncenter.x" << center.x <<std::endl;
    //std::cout<<"\ncenter.y" << center.y <<std::endl;

    CvBox2D elps = this->GetEllipse();
    if( (elps.center.x == 0) || (elps.center.y == 0) )
    {
        //getchar();
    }else
    {
        center.x = (double)elps.center.x;
        center.y = (double)elps.center.y;
    }
    
    //std::cout<<"\nelps.center.x = " << elps.center.x<<std::endl;
    //std::cout<<"\nelps.center.y = " << elps.center.y<<std::endl;

#endif




	return true;

}

bool CBlob::splitBlob(int numCluster, std::vector<std::vector<CvPoint2D32f>> &clusteredPoint)// std::vector<CvSeq*>& clusteredContours, std::vector<std::vector<CvPoint2D32f>> &clusteredPoint)
{
    FILE_LOG(logDEBUG1)<<"begin of CBlob::splitBlob";
    //populate the m_externalContour list..
    this->m_externalContour.GetContourPoints();
    FILE_LOG(logDEBUG3)<<"area = "<<this->Area();

	CvSeq* contour;
    if (m_externalContour.IsEmpty())
    {
        FILE_LOG(logDEBUG3)<<"m_externalContour IsEmpty() - nothing to split ";
        return false; //TODO -- change the return values, make sure the return value for split fuction is getting checked somewhere!
    }

	contour = m_externalContour.m_contourPoints;
    FILE_LOG(logDEBUG3)<<"contour->total = "<<contour->total;
	std::vector <CvPoint2D32f> contourkmResults;

	clusteredPoint.clear(); // just to make sure there is nothing in this vector
	for (int nc = 0;nc<numCluster;nc++)
	{
		std::vector<CvPoint2D32f> tmp;
		clusteredPoint.push_back(tmp); // this is gonna keep the clut
	}

	contourkmResults.clear();
	std::vector <CvPoint2D32f> pointsInsideContour; // this finds all points inside the contour

	pointsInsideContour.clear();
	CvRect boundbox ;
	boundbox = cvBoundingRect(contour);

	for (int ii=0;ii<boundbox.width-boundbox.x;ii++)
		for (int jj=0;jj<boundbox.height-boundbox.y;jj++)
		{
			CvPoint2D32f tmpPt = cvPoint2D32f((float)( boundbox.x+ ii),(float)(boundbox.y + jj));
			if (cvPointPolygonTest(contour, tmpPt,false)>0)
				pointsInsideContour.push_back(tmpPt);
		
		}

	
	CvMat* allThePoints = cvCreateMat((int)pointsInsideContour.size(), 1, CV_32FC2);
	CvMat* clusters = cvCreateMat((int)pointsInsideContour.size(), 1, CV_32SC1); // to keep the K-Means results
	allThePoints->rows = 0;	

	for (int jj=0;jj<pointsInsideContour.size();jj++) // putting all inside points to the allThePoints Matrix 
	{
		CvPoint2D32f tmpPt = cvPoint2D32f((float)pointsInsideContour[jj].x,(float)pointsInsideContour[jj].y);
		allThePoints->data.fl[(allThePoints->rows)*2]   = (float)tmpPt.x;
		allThePoints->data.fl[(allThePoints->rows)*2+1] = (float)tmpPt.y;
		(allThePoints->rows)++;	
			
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double *centroids=new double[2*numCluster]; //2 is because of x and y
	int *numPointsInCentroid=new int[numCluster];
	bool allCentersAreInside = false;
	int tr = 0;
	while (!allCentersAreInside && tr<MAXTRIES) //we repleat kmeans and MAXTRIES times, and hope we find a solution that all the centers are inside of the contour 
	{
		tr++;
		clusters->rows = allThePoints->rows;
		cvKMeans2( allThePoints, numCluster, clusters, cvTermCriteria( CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, MAXITERATIONS, 0.1 ));

		for (int ii = 0;ii<clusters->rows;ii++) // goes over cluster results and pushes every point to its own cluster
		{
			int clstr = clusters->data.i[ii];

			CvPoint2D32f tmp = pointsInsideContour[ii];

			for(int jj = 0;jj<numCluster;jj++)
				if (jj == clstr)
					clusteredPoint[jj].push_back(tmp);

		}

		//Calculate the centroids

		for(int i=0; i<numCluster; i++)
		{
			centroids[i*2+0]=centroids[i*2+1]=0;
			numPointsInCentroid[i] = 0;
		}

		for(int i=0; i<clusters->rows; i++)
		{
			centroids[ (clusters->data.i[i])*2 + 0] += allThePoints->data.fl[i*2];
			centroids[ (clusters->data.i[i])*2 + 1] += allThePoints->data.fl[i*2 + 1];
			numPointsInCentroid [ clusters->data.i[i] ] ++;
		}

		for(int i=0; i<numCluster; i++)
		{
			centroids[i*2 + 0] /= numPointsInCentroid[i];
			centroids[i*2 + 1] /= numPointsInCentroid[i];
		}


		CvPoint2D32f tmp; bool tmpbool = true; // I don't need this tmp bool, just to make code more clear ...

		for (int  i=0; i<numCluster; i++)
		{
			tmp.x = (float)centroids[2*i];
			tmp.y = (float)centroids[2*i + 1];
            char msg[100];
			sprintf(msg,"\ntemp.x = %.2lf, tmp.y = %.2lf",tmp.x, tmp.y);
            FILE_LOG(logDEBUG3)<<msg;
			tmpbool=tmpbool&&(cvPointPolygonTest(contour, tmp,false)>0); //if both points are inside of the contour then stop trying kmeans again
		}
		if(tmpbool)
			allCentersAreInside = true;

	}

	delete [] centroids;
	delete [] numPointsInCentroid;
	cvReleaseMat(&allThePoints);
	cvReleaseMat(&clusters);

    FILE_LOG(logDEBUG1)<<"end of CBlob::splitBlob";;
	//printf("\nEnd of separatedBlob");

return true;
}








int CBlob::numberOfMergedBlobs(double thresh1, double thresh3)
{
	int numCluster = 2;
	if(this->Area() < thresh1)
		numCluster = 1;
	if(this->Area() > thresh3)
		numCluster = 3;

return numCluster;
}

bool CBlob::isMerged(double thresh1, double thresh3)
{
	if(this->Area() > thresh1) 
		return true;
	else return false;
}

