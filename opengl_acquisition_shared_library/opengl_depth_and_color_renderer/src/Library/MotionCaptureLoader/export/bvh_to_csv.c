#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bvh_to_csv.h"

#include "../bvh_loader.h"

#include "../bvh_project.h"

#define CONVERT_EULER_TO_RADIANS M_PI/180.0
#define DUMP_SEPERATED_POS_ROT 0
#define DUMP_3D_POSITIONS 0

unsigned int invisibleJoints=0;
unsigned int   visibleJoints=0;
unsigned int filteredOutCSVBehindPoses=0;
unsigned int filteredOutCSVOutPoses=0;
unsigned int filteredOutCSVPoses=0;

int fileExists(const char * filename)
{
 FILE *fp = fopen(filename,"r");
 if( fp ) { /* exists */ fclose(fp); return 1; }
 return 0;
}


float eulerAngleToRadiansIfNeeded( float eulerAngle , unsigned int isItNeeded)
{
  if (isItNeeded)
  {
    return eulerAngle*CONVERT_EULER_TO_RADIANS;
  }
 return eulerAngle;
}



int csvSkeletonFilter(
                       struct BVH_MotionCapture * mc,
                       struct BVH_Transform * bvhTransform,
                       struct simpleRenderer * renderer,
                       unsigned int filterOutSkeletonsWithAnyLimbsBehindTheCamera,
                       unsigned int filterOutSkeletonsWithAnyLimbsOutOfImage,
                       unsigned int filterWeirdSkeletons
                     )
{
   unsigned int jID=0;

   //-------------------------------------------------
   if (filterOutSkeletonsWithAnyLimbsBehindTheCamera)
   {
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (bvhTransform->joint[jID].isBehindCamera)
         {
           ++filteredOutCSVPoses;
           ++filteredOutCSVBehindPoses;
           //Just counting to reduce spam..
           return 0;
         }
       }
   }//-----------------------------------------------


   //-------------------------------------------------
   if (filterOutSkeletonsWithAnyLimbsOutOfImage)
   {
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
        float x = bvhTransform->joint[jID].pos2D[0];
        float y = bvhTransform->joint[jID].pos2D[1];

        if (
             (x<0.0) || (y<0.0) || (renderer->width<x) || (renderer->height<y)
           )
        {
           ++filteredOutCSVPoses;
           ++filteredOutCSVOutPoses;
           //Just counting to reduce spam..
           return 0;
        }
       }
   }//-----------------------------------------------



   //-------------------------------------------------
   if (filterWeirdSkeletons)
   { //If all x,y 0 filter out
       unsigned int jointCount=0;
       unsigned int jointsInWeirdPositionCount=0;
       for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
        float x = bvhTransform->joint[jID].pos2D[0];
        float y = bvhTransform->joint[jID].pos2D[1];
        ++jointCount;
        if ( (x<0.5) && (y<0.5) )
        {
           ++jointsInWeirdPositionCount;
        }
       }

        if (jointCount==jointsInWeirdPositionCount)
        {
           ++filteredOutCSVPoses;
           return 0;
        }
   }//-----------------------------------------------

 return 1;
}



int dumpBVHToCSVHeader(
                       struct BVH_MotionCapture * mc,
                       const char * filename2D,
                       const char * filename3D,
                       const char * filenameBVH
                      )
{
   unsigned int jID=0;

   if ( (filename2D!=0) && (filename2D[0]!=0) && (!fileExists(filename2D)) )
   {
    FILE * fp2D = fopen(filename2D,"a");

    if (fp2D!=0)
    {
     char comma=',';
     //2D Positions -------------------------------------------------------------------------------------------------------------
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (jID==mc->jointHierarchySize-1) { comma=' '; }

         if (!mc->jointHierarchy[jID].isEndSite)
         {
          fprintf(
                  fp2D,"2DX_%s,2DY_%s,visible_%s",
                  mc->jointHierarchy[jID].jointName,
                  mc->jointHierarchy[jID].jointName,
                  mc->jointHierarchy[jID].jointName
                 );
          if (comma==',')
                  { fprintf(fp2D,",");  }
         }
         else
         {
          unsigned int parentID=mc->jointHierarchy[jID].parentJoint;
          fprintf(
                  fp2D,"2DX_EndSite_%s,2DY_EndSite_%s,visible_EndSite_%s",
                  mc->jointHierarchy[parentID].jointName,
                  mc->jointHierarchy[parentID].jointName,
                  mc->jointHierarchy[parentID].jointName
                 );
           if (comma==',')
                  { fprintf(fp2D,",");  }
         }
       }
     //--------------------------------------------------------------------------------------------------------------------------
     fprintf(fp2D,"\n");
     fclose(fp2D);
   }
  }else
    {
     fprintf(stderr,"We don't need to regenerate the CSV 2D header, it already exists\n");
    }



   //3D Positions -------------------------------------------------------------------------------------------------------------
   if ( (filename3D!=0) && (filename3D[0]!=0) && (!fileExists(filename3D)) )
   {
     FILE * fp3D = fopen(filename3D,"a");
     if (fp3D!=0)
     {
      char comma=',';

      for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (jID==mc->jointHierarchySize-1) { comma=' '; }

         if (!mc->jointHierarchy[jID].isEndSite)
         {
           fprintf(fp3D,"3DX_%s,3DY_%s,3DZ_%s",mc->jointHierarchy[jID].jointName,mc->jointHierarchy[jID].jointName,mc->jointHierarchy[jID].jointName);

           if (comma==',') { fprintf(fp3D,","); }
         }
       }
      fprintf(fp3D,"\n");
      fclose(fp3D);
     }
   } else
    {
     fprintf(stderr,"We don't need to regenerate the CSV 3D header, it already exists\n");
    }
   //--------------------------------------------------------------------------------------------------------------------------




   if ( (filenameBVH!=0) && (filenameBVH[0]!=0) && (!fileExists(filenameBVH)) )
   {
     FILE * fpBVH = fopen(filenameBVH,"a");
     if (fpBVH!=0)
     {

      //----------------------------------------------
      unsigned int lastElement=0;
      for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (!mc->jointHierarchy[jID].isEndSite)
         {
           lastElement=jID;
         }
       }
      //----------------------------------------------



      char comma=',';
      //Model Configuration
      for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (!mc->jointHierarchy[jID].isEndSite)
         {
           if (jID==lastElement) { comma=' '; }

           unsigned int channelID=0;
           for (channelID=0; channelID<mc->jointHierarchy[jID].loadedChannels; channelID++)
           {
            fprintf(
                    fpBVH,"%s_%s",
                    mc->jointHierarchy[jID].jointName,
                    channelNames[(unsigned int) mc->jointHierarchy[jID].channelType[channelID]]
                   );

             if ((comma!=',')&&(channelID==mc->jointHierarchy[jID].loadedChannels-1)) {  } else  { fprintf(fpBVH,",");  }
           }
         }
       }
      //Append Frame ID
      fprintf(fpBVH,"\n");
      fclose(fpBVH);
     }
    } else
    {
     fprintf(stderr,"We don't need to regenerate the CSV header, it already exists\n");
    }
   //--------------------------------------------------------------------------------------------------------------------------



 return 1;
}



int dumpBVHToCSVBody(
                       struct BVH_MotionCapture * mc,
                       struct BVH_Transform * bvhTransform,
                       struct simpleRenderer * renderer,
                       unsigned int fID,
                       const char * filename2D,
                       const char * filename3D,
                       const char * filenameBVH,
                       unsigned int csvOrientation,
                       unsigned int filterOutSkeletonsWithAnyLimbsBehindTheCamera,
                       unsigned int filterOutSkeletonsWithAnyLimbsOutOfImage,
                       unsigned int filterWeirdSkeletons,
                       unsigned int encodeRotationsAsRadians
                      )
{
   unsigned int jID=0;

   if (
       !csvSkeletonFilter(
                           mc,
                           bvhTransform,
                           renderer,
                           filterOutSkeletonsWithAnyLimbsBehindTheCamera,
                           filterOutSkeletonsWithAnyLimbsOutOfImage,
                           filterWeirdSkeletons
                         )
       )
   {
     return 0;
   }

   //-------------------------------------------------
   if (encodeRotationsAsRadians)
   {
    fprintf(stderr,"encodeRotationsAsRadians not implemented, please switch it off\n");
    exit(0);
   }//-----------------------------------------------


   unsigned int dumped=0;
   unsigned int requestedToDump=0;
   FILE * fp2D = 0;
   FILE * fp3D = 0;
   FILE * fpBVH = 0;

   if ( (filename2D!=0) && (filename2D[0]!=0) )   { fp2D = fopen(filename2D,"a");   ++requestedToDump; }
   if ( (filename3D!=0) && (filename3D[0]!=0) )   { fp3D = fopen(filename3D,"a");   ++requestedToDump; }
   if ( (filenameBVH!=0) && (filenameBVH[0]!=0) ) { fpBVH = fopen(filenameBVH,"a"); ++requestedToDump; }


     //2D Positions -------------------------------------------------------------------------------------------------------------
     if (fp2D!=0)
     {
      char comma=',';
      for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (bvhTransform->joint[jID].isOccluded) { ++invisibleJoints; } else { ++visibleJoints; }

         if (jID==mc->jointHierarchySize-1) { comma=' '; }

         fprintf(
                 fp2D,"%0.6f,%0.6f,%u",
                 (float) bvhTransform->joint[jID].pos2D[0]/renderer->width,
                 (float) bvhTransform->joint[jID].pos2D[1]/renderer->height,
                 (bvhTransform->joint[jID].isOccluded==0)
                );

        if (comma==',')
                  { fprintf(fp2D,",");  }
       }
     fprintf(fp2D,"\n");
     fclose(fp2D);
     ++dumped;
     }
     //-----------------------------------------------------------------------------------------------------------------------------


   //3D Positions -------------------------------------------
   if (fp3D!=0)
   {
     char comma=',';
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (jID==mc->jointHierarchySize-1) { comma=' '; }

         if (!mc->jointHierarchy[jID].isEndSite)
         {
          fprintf(
                  fp3D,"%0.4f,%0.4f,%0.4f",
                  bvhTransform->joint[jID].pos3D[0],
                  bvhTransform->joint[jID].pos3D[1],
                  bvhTransform->joint[jID].pos3D[2]
                 );
          if (comma==',')
                  { fprintf(fp3D,",");  }
         }
       }
     fprintf(fp3D,"\n");
     fclose(fp3D);
     ++dumped;
   }
   //-------------------------------------------------------------------


   //Joint Configuration
   if (fpBVH!=0)
   {

     //----------------------------------------------
     //This is inneficient
     unsigned int lastElement=0;
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (!mc->jointHierarchy[jID].isEndSite)
         {
           lastElement=jID;
         }
       }
     //----------------------------------------------


     char comma=',';
     for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         if (!mc->jointHierarchy[jID].isEndSite)
         {
           if (jID==lastElement) { comma=' '; }
           
           
           unsigned int channelID=0;
           for (channelID=0; channelID<mc->jointHierarchy[jID].loadedChannels; channelID++)
           {
             unsigned int channelType =  mc->jointHierarchy[jID].channelType[channelID];
             
             float value = bvh_getJointChannelAtFrame(mc,jID,fID,channelType);
             
             //Due to the particular requirements of MocapNET we need to be able to split orientations in CSV files..
             if (csvOrientation!=BVH_ENFORCE_NO_ORIENTATION)
             {
              //TODO: add here a check for hip Y rotation and perform orientation change..
              if ( (jID==0) && (channelID==BVH_POSITION_X) ) //BVH_ROTATION_X
              {   
                  //Test using : 
                  //./BVHTester --from Motions/MotionCapture/01/01_02.bvh  --repeat 0 --csvOrientation right --randomize2D 1000 5000 -35 45 -35 35 135 35 --occlusions --csv tmp test.csv 2d+bvh 
                  //value=666; <- highlight the correct 
                  //value=(float) bvh_constrainAngleCentered0((double) value,0);
                  value=(float) bvh_RemapAngleCentered0((double) value,csvOrientation);
              }
             }
             
             fprintf(
                     fpBVH,"%0.6f",
                     value
                    );

             if ( (comma!=',') && (channelID==mc->jointHierarchy[jID].loadedChannels-1) ) {  } else  { fprintf(fpBVH,",");  }
           }
         }
       }
     fprintf(fpBVH,"\n");
     //-------------------------------------------------------------------
     fclose(fpBVH);
     ++dumped;
  }
   //-------------------------------------------------------------------

 return (dumped==requestedToDump);
}

