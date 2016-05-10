#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/nonfree/nonfree.hpp>
//#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const double THRESHOLD = 350;


void clear_line()
{
  fputs("\033[A\033[2K\033[A\033[2K",stdout);
  rewind(stdout);
  int i=ftruncate(1,0);
  if (i!=0) { /*fprintf(stderr,"Error with ftruncate\n");*/ }
}


double sumDistanceOfAllDescriptors(cv::Mat& vec1, cv::Mat& vec2)
{
  double sum = 0.0 , diff = 0.0;
  for (int i = 0; i < vec1.cols; i++)
  {
    diff = (vec1.at<uchar>(0,i) - vec2.at<uchar>(0,i)) ;
    sum +=  diff * diff;
  }
  return sqrt(sum);
}

int nearestNeighbor(cv::Mat& vec, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors) {
  int neighbor = -1;
  double minDist = THRESHOLD+1;

  for (int i = 0; i < descriptors.rows; i++)
  {
    cv::KeyPoint pt = keypoints[i];
    cv::Mat v = descriptors.row(i);
    double d = sumDistanceOfAllDescriptors(vec, v);
    //printf("%d %f\n", v.cols, d);
    if (d < minDist) { minDist = d; neighbor = i; }
  }

  if (minDist < THRESHOLD) { return neighbor; } else
                           { /*Failed */}

  return -1;
}

void findPairs(std::vector<cv::KeyPoint>& keypoints1, cv::Mat& descriptors1,
               std::vector<cv::KeyPoint>& keypoints2, cv::Mat& descriptors2,
               std::vector<cv::Point2f>& srcPoints, std::vector<cv::Point2f>& dstPoints) {
  for (int i = 0; i < descriptors1.rows; i++)
  {
    clear_line();
    fprintf(stderr,"Checking SIFT pairs , threshold %0.2f : \n",THRESHOLD);
    fprintf(stderr,"%u / %u checks  - %u matches \n",i, descriptors1.rows , srcPoints.size());
    cv::KeyPoint pt1 = keypoints1[i];
    cv::Mat desc1 = descriptors1.row(i);
    int nn = nearestNeighbor(desc1, keypoints2, descriptors2);
    if (nn >= 0) {
      cv::KeyPoint pt2 = keypoints2[nn];
      srcPoints.push_back(pt1.pt);
      dstPoints.push_back(pt2.pt);
    }
  }
}



int visualizeMatches(
                      const char * filenameOutput ,
                      cv::Mat & left ,
                      std::vector<cv::KeyPoint>&keypointsLeft,
                      cv::Mat &descriptorsLeft,
                      cv::Mat & right ,
                      std::vector<cv::KeyPoint> &keypointsRight,
                      cv::Mat &descriptorsRight ,
                      std::vector<cv::Point2f> &srcPoints ,
                      std::vector<cv::Point2f> &dstPoints ,
                      std::vector<cv::Point2f> &srcRANSACPoints ,
                      std::vector<cv::Point2f> &dstRANSACPoints
                    )
{

 // Create a image for displaying mathing keypoints

    cv::Size size = left.size();
    cv::Size sz = cv::Size(size.width + right.size().width, size.height + right.size().height);
    cv::Mat matchingImage = cv::Mat::zeros(sz, CV_8UC3);

    // Draw camera frame
    cv::Mat roi1 = cv::Mat(matchingImage, cv::Rect(0, 0, size.width, size.height));
    left.copyTo(roi1);
    // Draw original image
    cv::Mat roi2 = cv::Mat(matchingImage, cv::Rect(size.width, size.height, right.size().width, right.size().height));
    right.copyTo(roi2);

    //char text[256];
    //sprintf(text, "%zd/%zd keypoints matched.", srcPoints.size(), keypointsLeft.size());
    //putText(matchingImage, text, cv::Point(0, cvRound(size.height + 30)), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 1, cv::Scalar(0,0,255));


    for (unsigned int i=0; i<keypointsLeft.size(); i++)
    {
      cv::KeyPoint kp = keypointsLeft[i];
      cv::circle(matchingImage, kp.pt, cvRound(kp.size*0.25), cv::Scalar(255,255,0), 1, 8, 0);
    }


    for (int i=0; i<keypointsRight.size(); i++)
    {
      cv::KeyPoint kp = keypointsRight[i];

      cv::Point2f rightPT = kp.pt;
      rightPT.x += size.width;
      rightPT.y += size.height;

      cv::circle(matchingImage, rightPT , cvRound(kp.size*0.25), cv::Scalar(255,255,0), 1, 8, 0);
    }


if (srcRANSACPoints.size()>0)
{    // Draw line between ransac neighbor pairs
    for (int i = 0; i < (int)srcRANSACPoints.size(); ++i)
    {
      cv::Point2f pt1 = srcRANSACPoints[i];
      cv::Point2f pt2 = dstRANSACPoints[i];
      cv::Point2f from = pt1;
      cv::Point2f to   = cv::Point(size.width + pt2.x, size.height + pt2.y);
      cv::line(matchingImage, from, to, cv::Scalar(0, 255, 255));
    }
} else
{
    // Draw line between nearest neighbor pairs
    for (unsigned int i = 0; i < (int)srcPoints.size(); ++i)
    {
      cv::Point2f pt1 = srcPoints[i];
      cv::Point2f pt2 = dstPoints[i];
      cv::Point2f from = pt1;
      cv::Point2f to   = cv::Point(size.width + pt2.x, size.height + pt2.y);
      cv::line(matchingImage, from, to, cv::Scalar(0, 0 , 255));
    }
}

    // Display mathing image
    cv::imwrite(filenameOutput , matchingImage);
    imshow("mywindow", matchingImage);
    //int c = cv::waitKey(0);
}


void checkAffineSolution ( double * outX ,double * outY , double * dstX,double * dstY , double * M , double * srcX, double * srcY)
{
  double a=M[0] , b=M[1] , c=M[2];
  double d=M[3] , e=M[4] , f=M[5];

  *outX = c + ( (double) a * *srcX ) + ((double) b * *srcY );
  *outY = f + ( (double) d * *srcX ) + ((double) e * *srcY );


   if ( *outX > *dstX ) { *outX = *outX - *dstX; } else { *outX = *dstX - *outX; }
   if ( *outY > *dstY ) { *outY = *outY - *dstY; } else { *outY = *dstY - *outY; }

}

int checkAffineFitness(
                         double * M ,
                         std::vector<cv::Point2f> &srcPoints ,
                         std::vector<cv::Point2f> &dstPoints ,
                         double * avgX ,
                         double * avgY ,
                         std::vector<cv::Point2f> &srcRANSACPoints ,
                         std::vector<cv::Point2f> &dstRANSACPoints
                      )
{
  srcRANSACPoints.clear();
  dstRANSACPoints.clear();
  *avgX = 0; *avgY = 0;

  // { { c a * sx + b sy - dx } ,  { f d sx + e sy - dy } } = 0
  unsigned int inlierCount=0;
  unsigned int i=0;

  double a=M[0] , b=M[1] , c=M[2];
  double d=M[3] , e=M[4] , f=M[5];

  double sumX=0,sumY=0;
  double sx=0.0 , sy=0.0 ,  dx=0.0 , dy=0.0 , rx=0.0 , ry=0.0;




  for (i=0; i<srcPoints.size(); i++)
  {
    sx = (double) srcPoints[i].x;  sy = (double) srcPoints[i].y;
    dx = (double) dstPoints[i].x;  dy = (double) dstPoints[i].y;

    rx = (double) c + ( a * sx ) + ( b * sy )  ;
    ry = (double) f + ( d * sx ) + ( e * sy )  ;

    if ( rx > dx ) { rx = rx - dx; } else { rx = dx - rx; }
    if ( ry > dy ) { ry = ry - dy; } else { ry = dy - ry; }
    //fprintf(stderr," %0.2f %0.2f \n",rx,ry);


    if ( (rx<3)&&(ry<3) )
    {
       sumX+=rx; sumY+=ry;
       ++inlierCount;

      cv::Point2f srcPT; srcPT.x = sx; srcPT.y = sy;
      cv::Point2f dstPT; dstPT.x = dx; dstPT.y = dy;
      srcRANSACPoints.push_back(srcPT);
      dstRANSACPoints.push_back(dstPT);
    }
  }

 if (inlierCount>0)
 {
  *avgX = (double) sumX/inlierCount;
  *avgY = (double) sumY/inlierCount;
  }

  //fprintf(stderr,"{ { a=%0.2f , b=%0.2f , c=%0.2f } , { d=%0.2f , e=%0.2f , f=%0.2f } }  ",a,b,c,d,e,f);
  //fprintf(stderr,"%u inliers / %u points  , average ( %0.2f,%0.2f ) \n",inlierCount , srcPoints.size() , *avgX , *avgY );
  return inlierCount;
}


int fitAffineTransformationMatchesRANSAC(
                                          unsigned int loops ,
                                          double * M ,
                                          cv::Mat & warp_mat ,
                                          std::vector<cv::Point2f> &srcPoints ,
                                          std::vector<cv::Point2f> &dstPoints ,
                                          std::vector<cv::Point2f> &srcRANSACPoints ,
                                          std::vector<cv::Point2f> &dstRANSACPoints
                                        )
{
  fprintf(stderr,"fitAffineTransformationMatchesRANSAC start %0.2f \n",(float) srcPoints.size()/loops );
  if (srcPoints.size()<=3) { fprintf(stderr,"Cannot calculate an affine transformation without 3 or more point correspondances\n"); return 0; }
  if ((float)srcPoints.size()/loops <= 1.0 ) { fprintf(stderr,"Too many loops of ransac for our problem \n");   }

  //
  // DST | x |  = M | a b c | * SRC | x |
  //     | y |      | d e f |       | y |
  //                                | 1 |
  //{ {dx} , {dy } } = { { a ,b, c} , { d , e ,f  } } . { { sx } , { sy } , { 1 } }
  //{ {dx} , {dy } } = { { c a * sx + b sy } ,  { f d sx + e sy } }
  // { { c a * sx + b sy - dx } ,  { f d sx + e sy - dy } } = 0
  //
  //

  unsigned int ptA=0,ptB=0,ptC=0;


  std::vector<cv::Point2f> bestSrcRANSACPoints;
  std::vector<cv::Point2f> bestDstRANSACPoints;

  cv::Point2f srcTri[3];
  cv::Point2f dstTri[3];


  cv::Mat bestWarp_mat( 2, 3,  CV_64FC1  );
  double bestM[6]={0};
  unsigned int bestInliers=0;
  double avgX,avgY,bestAvgX=66660.0,bestAvgY=66660.0;

  unsigned int i=0,z=0;
  for (i=0; i<loops; i++)
  {
    //RANSAC pick 3 points
    ptA=rand()%srcPoints.size();
    ptB=rand()%srcPoints.size();
    ptC=rand()%srcPoints.size();
    do { ptB=rand()%srcPoints.size(); } while (ptB==ptA);
    do { ptC=rand()%srcPoints.size(); } while ( (ptC==ptA)||(ptB==ptC) );

    dstTri[0].x = dstPoints[ptA].x; dstTri[0].y = dstPoints[ptA].y;
    dstTri[1].x = dstPoints[ptB].x; dstTri[1].y = dstPoints[ptB].y;
    dstTri[2].x = dstPoints[ptC].x; dstTri[2].y = dstPoints[ptC].y;
    srcTri[0].x = srcPoints[ptA].x; srcTri[0].y = srcPoints[ptA].y;
    srcTri[1].x = srcPoints[ptB].x; srcTri[1].y = srcPoints[ptB].y;
    srcTri[2].x = srcPoints[ptC].x; srcTri[2].y = srcPoints[ptC].y;

   /// Get the Affine Transform
   //derive resultMatrix with Gauss Jordan
   warp_mat = cv::getAffineTransform( srcTri, dstTri );
   //fprintf(stderr," ____________________________________________________________ \n");
   //std::cout << warp_mat<<"\n";

   M[0] = warp_mat.at<double>(0,0); M[1] = warp_mat.at<double>(0,1); M[2] = warp_mat.at<double>(0,2);
   M[3] = warp_mat.at<double>(1,0); M[4] = warp_mat.at<double>(1,1); M[5] = warp_mat.at<double>(1,2);

  /*
   for (z=0; z<3; z++)
   {
    fprintf(stderr,"{ { %0.2f } , { %0.2f } }  = ",dstTri[z].x,dstTri[z].y);
    fprintf(stderr,"{ { %0.2f , %0.2f , %0.2f } , { %0.2f , %0.2f , %0.2f } }  ",M[0],M[1],M[2],M[3],M[4],M[5]);
    fprintf(stderr," . { { %0.2f } , { %0.2f } , { 1 } }  \n\n",srcTri[z].x,srcTri[z].y);


     double outX , outY , dstX = dstTri[z].x , dstY = dstTri[z].y , srcX = srcTri[z].x , srcY= srcTri[z].y;
     checkAffineSolution ( &outX , &outY , &dstX, &dstY , M , &srcX, &srcY);

     fprintf(stderr,"diff is %0.2f , %0.2f\n",outX , outY);
   }
  */


   unsigned int inliers = checkAffineFitness( M , srcPoints , dstPoints , &avgX, &avgY , srcRANSACPoints , dstRANSACPoints );

   if (inliers > bestInliers )
    {
      bestInliers = inliers;
      bestAvgX = avgX;
      bestAvgY = avgY;
      for (z=0; z<6; z++) { bestM[z]=M[z]; }
      bestSrcRANSACPoints=srcRANSACPoints;
      bestDstRANSACPoints=dstRANSACPoints;
      bestWarp_mat = warp_mat;
    }


    clear_line();
    fprintf(stderr,"RANSACing affine transform , %u / %u loops : \n",i,loops);
    fprintf(stderr,"Best : %u inliers  , avg ( %0.2f , %0.2f )  \n",bestInliers,bestAvgX,bestAvgY);


    for (z=0; z<6; z++) { M[z]=bestM[z]; }
    srcRANSACPoints=bestSrcRANSACPoints;
    dstRANSACPoints= bestDstRANSACPoints;
    warp_mat = bestWarp_mat;


  }

  //fprintf(stderr,"fitAffineTransformationMatchesRANSAC done got %u inliers with avg ( %0.2f , %0.2f ) \n",bestInliers,bestAvgX,bestAvgY);
  return 1;
}


int blendImages( cv::Mat & out,
                 cv::Mat & left ,
                 cv::Mat & right )
{
   unsigned char lR=0,lG=0,lB=0,rR=0,rG=0,rB=0;
   unsigned int leftBlank,rightBlank;
   unsigned int x=0,y=0,offset=0,width=left.size().width;

   for ( x=0; x<width; x++ )
   {
    for ( y=0; y<left.size().height; y++ )
    {
      offset=y*width*3+x*3;
      lR=left.data[offset+0];  lG=left.data[offset+1];  lB=left.data[offset+2];
      rR=right.data[offset+0]; rG=right.data[offset+1]; rB=right.data[offset+2];

       if ( (  lR==0 ) && ( lG==0 ) && ( lB == 0 )  )  { leftBlank=1; }  else { leftBlank=0; }
       if ( (  rR==0 ) && ( rG==0 ) && ( rB == 0 )  )  { rightBlank=1; } else { rightBlank=0; }

       if ( (  leftBlank ) && ( rightBlank ) )  { } else
       if ( (  !leftBlank ) && ( rightBlank ) )
       {
         out.data[offset+0]=lR;
         out.data[offset+1]=lG;
         out.data[offset+2]=lB;
       } else
       if ( (  leftBlank ) && ( !rightBlank ) )
       {
         out.data[offset+0]=rR;
         out.data[offset+1]=rG;
         out.data[offset+2]=rB;
       } else
       {
         out.data[offset+0]=(lR+rR)/2;
         out.data[offset+1]=(lG+rG)/2;
         out.data[offset+2]=(lB+rB)/2;
       }
    }
   }
 return 1;
}


int stitchMatch(
                const char * filenameOutput ,
                cv::Mat & left ,
                cv::Mat & right ,
                cv::Mat & warp_mat
               )
{
    unsigned int borderX = left.size().width/2;
    unsigned int borderY = 100;

    warp_mat.at<double>(0,2) += borderX;
    warp_mat.at<double>(1,2) += borderY;

    cv::Size sz = cv::Size(left.size().width + right.size().width  + borderX , left.size().height + right.size().height + borderY );
    cv::Mat matchingImageLeft = cv::Mat::zeros(sz, CV_8UC3);
    cv::Mat matchingImageRight = cv::Mat::zeros(sz, CV_8UC3);
    cv::Mat matchingImageBlended = cv::Mat::zeros(sz, CV_8UC3);


    // Draw camera frame
    cv::Mat roi1 = cv::Mat(matchingImageRight, cv::Rect(borderX, borderY, right.size().width, right.size().height));
    right.copyTo(roi1);


   /// Apply the Affine Transform just found to the src image
   cv::Mat warp_dst = cv::Mat::zeros( left.rows, left.cols, left.type() );
   warpAffine( left, matchingImageLeft /* warp_dst */ , warp_mat, /*warp_dst*/ matchingImageLeft.size() );

   //double alpha = 0.5 , beta = ( 1.0 - alpha );
   //addWeighted(   matchingImageLeft /*warp_dst*/  , alpha, matchingImageRight, beta, 0.0, matchingImageBlended);


   blendImages(matchingImageBlended , matchingImageLeft , matchingImageRight);

   cv::imwrite( filenameOutput  ,  matchingImageBlended /* warp_dst */);

}

int main(int argc, const char* argv[])
{

    cv::Mat left = cv::imread("uttower_left.JPG"  , CV_LOAD_IMAGE_COLOR);
    if(! left.data ) { fprintf(stderr,"Left Image missing \n"); return 1; }

    cv::Mat right = cv::imread("uttower_right.JPG", CV_LOAD_IMAGE_COLOR);
    if(! right.data ) { fprintf(stderr,"Right Image missing \n"); return 1; }


    cv::DescriptorExtractor* extractor = new cv::SiftDescriptorExtractor();
    cv::SiftFeatureDetector detector;
    std::vector<cv::KeyPoint> keypointsLeft;
    cv::Mat descriptorsLeft;
    detector.detect(left, keypointsLeft);
    extractor->compute(left, keypointsLeft, descriptorsLeft);

    // Add results to image and save.
    cv::Mat output;
    cv::drawKeypoints(left, keypointsLeft, output);
    cv::imwrite("sift_result_left.jpg", output);


    std::vector<cv::KeyPoint> keypointsRight;
    cv::Mat descriptorsRight;
    detector.detect(right, keypointsRight);
    extractor->compute(right, keypointsRight, descriptorsRight);
    cv::drawKeypoints(right, keypointsRight, output);
    cv::imwrite("sift_result_right.jpg", output);

    //fprintf(stderr,"SIFT features ready \n");


    std::vector<cv::Point2f> srcRANSACPoints;
    std::vector<cv::Point2f> dstRANSACPoints;


    std::vector<cv::Point2f> srcPoints;
    std::vector<cv::Point2f> dstPoints;
    findPairs(keypointsLeft, descriptorsLeft, keypointsRight, descriptorsRight, srcPoints, dstPoints);
    //printf("%zd keypoints are matched.\n", srcPoints.size());



   visualizeMatches(
                      "sift_initial_match.jpg" ,
                      left ,
                      keypointsLeft,
                      descriptorsLeft,
                      right ,
                      keypointsRight,
                      descriptorsRight,
                      srcPoints,
                      dstPoints,
                      srcRANSACPoints,
                      dstRANSACPoints
                    );



   cv::Mat warp_mat( 2, 3,  CV_64FC1  );
   double M[6]={0};
   fitAffineTransformationMatchesRANSAC( 1000, M , warp_mat, srcPoints , dstPoints ,  srcRANSACPoints, dstRANSACPoints);


   stitchMatch(
                "wrapped.jpg"  ,
                 left ,
                 right ,
                 warp_mat
               );


   visualizeMatches(
                      "sift_affine_match.jpg" ,
                      left ,
                      keypointsLeft,
                      descriptorsLeft,
                      right ,
                      keypointsRight,
                      descriptorsRight,
                      srcPoints,
                      dstPoints ,
                      srcRANSACPoints,
                      dstRANSACPoints
                    );



    return 0;
}