#include <stdio.h>
#include <math.h>

#include "../bvh_loader.h"
#include "bvh_project.h"

void bvh_cleanTransform(
                        struct BVH_MotionCapture * mc,
                        struct BVH_Transform     * bvhTransform
                       )
{
 bvhTransform->jointsOccludedIn2DProjection=0;
 bvhTransform->torso.exists=0;

 unsigned int jID=0;
 for (jID=0; jID<mc->jointHierarchySize; jID++)
         {
          bvhTransform->joint[jID].pos3D[0]=0.0;
          bvhTransform->joint[jID].pos3D[1]=0.0;
          bvhTransform->joint[jID].pos3D[2]=0.0;

          bvhTransform->joint[jID].pos2DCalculated=0;
          bvhTransform->joint[jID].isBehindCamera=0;

          bvhTransform->joint[jID].pos2D[0]=0.0;
          bvhTransform->joint[jID].pos2D[1]=0.0;
         }
}



int bvh_projectTo2DHandleOcclusions(
                                    struct BVH_MotionCapture * mc,
                                    struct BVH_Transform     * bvhTransform
                                   )
{
  unsigned int jID;
       //First set all points as unoccluded..!
       for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
         bvhTransform->joint[jID].isOccluded=0;
       }

       #define USE_TORSO_OCCLUSIONS 1
       #define DEBUG_TORSO_OCCLUSIONS 0
       #define OCCLUSION_THRESHOLD 6 // pixels
       //bvhTransform->joint[jID].isOccluded=0;

       #if DEBUG_TORSO_OCCLUSIONS
        fprintf(stderr,"Occlusion checking..\n");
       #endif // DEBUG_TORSO_OCCLUSIONS

       unsigned int jID2=0;
       for (jID=0; jID<mc->jointHierarchySize; jID++)
       {
        if (bvhTransform->joint[jID].pos2DCalculated)
        {

         #if USE_TORSO_OCCLUSIONS
         //If we have a torso ----------------------------------
         if ((bvhTransform->torso.exists) && (!mc->jointHierarchy[jID].isImmuneToTorsoOcclusions))
         {
           //If the point is behind torso (which is a fast check) ----------------------------------
           if (bvhTransform->joint[jID].pos3D[2]<bvhTransform->torso.averageDepth)
           {
            //If the point is inside the 2D rectangle of the torso ----------------------------------
            if (
                 (bvhTransform->torso.rectangle2D.x < bvhTransform->joint[jID].pos2D[0]) &&
                 (bvhTransform->joint[jID].pos2D[0] < bvhTransform->torso.rectangle2D.x + bvhTransform->torso.rectangle2D.width ) &&
                 (bvhTransform->torso.rectangle2D.y < bvhTransform->joint[jID].pos2D[1]) &&
                 (bvhTransform->joint[jID].pos2D[1] < bvhTransform->torso.rectangle2D.y + bvhTransform->torso.rectangle2D.height )
              )
            {
             bvhTransform->joint[jID].isOccluded=1;
             ++bvhTransform->jointsOccludedIn2DProjection;

             #if DEBUG_TORSO_OCCLUSIONS
             fprintf(stderr,"TORSO OCCLUSION\n");
             fprintf(
                     stderr,"Point @ %0.2f,%0.2f -> box %0.2f,%0.2f (%0.2fx%0.2f)\n",
                     bvhTransform->joint[jID].pos2D[0],
                     bvhTransform->joint[jID].pos2D[1],
                     bvhTransform->torso.rectangle2D.x,
                     bvhTransform->torso.rectangle2D.y,
                     bvhTransform->torso.rectangle2D.width,
                     bvhTransform->torso.rectangle2D.height
                    );
              #endif // DEBUG_TORSO_OCCLUSIONS

            }
            //------

           }
           #if DEBUG_TORSO_OCCLUSIONS
            else
           {
               fprintf(
                       stderr,"Point %s is behind torso (%0.2f < %0.2f )\n",
                        mc->jointHierarchy[jID].jointName,
                        bvhTransform->joint[jID].pos3D[2],
                        bvhTransform->torso.averageDepth
                       );
           }
           #endif // DEBUG_TORSO_OCCLUSIONS

          //------
         }
         #endif // USE_TORSO_OCCLUSIONS


         //If the joint is still not occluded then do extra checks, otherwise conserve our CPU
         if (!bvhTransform->joint[jID].isOccluded)
         {
         //Check Joint for occlusion with all other joints that may be in front of it
         //------------------------------------------------------------------------------------------
         for (jID2=0; jID2<mc->jointHierarchySize; jID2++)
          {
           if ( (mc->selectedJoints==0) || (mc->selectedJoints[jID2]) )
           {
            if ( (jID2!=jID)  )
            {
             if (bvhTransform->joint[jID2].pos2DCalculated)
             {
               //We check if these two joints are very close together
               float diffA=bvhTransform->joint[jID].pos2D[0]-bvhTransform->joint[jID2].pos2D[0];
               float diffB=bvhTransform->joint[jID].pos2D[1]-bvhTransform->joint[jID2].pos2D[1];
               float distance=sqrt((diffA*diffA)+(diffB*diffB));
               if (distance<OCCLUSION_THRESHOLD)
                {
                  //If they are close together and joint jID2 is in front of jID
                  if (bvhTransform->joint[jID].pos3D[2]>bvhTransform->joint[jID2].pos3D[2])
                  {
                    //fprintf(stderr," %0.2f",distance);
                    //then jID is occluded..!
                    bvhTransform->joint[jID].isOccluded=1;
                    ++bvhTransform->jointsOccludedIn2DProjection;
                  } //If they have  the correct order
                } //If it is closer than the minimum distance required by OCCLUSION_THRESHOLD
             } //Only if we have 2D position calculated
           } //Excluding occlusions with itself
          } //Only occlude with selected joints, ignore all else
          } // Compare to all other joints
          //------------------------------------------------------------------------------------------
         }//If joint not already occluded

        }
       }
     //------------------------------------------------------------------------------------------

  return 1;
}













int bvh_projectTo2D(
                     struct BVH_MotionCapture * mc,
                     struct BVH_Transform     * bvhTransform,
                     struct simpleRenderer    * renderer,
                     unsigned int               occlusions,
                     unsigned int               directRendering
                   )
{
      bvhTransform->jointsOccludedIn2DProjection=0;
      unsigned int pointsDumped=0;
      float position2DX=0.0;
      float position2DY=0.0;
      float position2DW=0.0;


      //Then project 3D positions on 2D frame and save results..
      unsigned int jID=0;
      for (jID=0; jID<mc->jointHierarchySize; jID++)
      {
        if ( (!bvhTransform->useOptimizations) || (!bvhTransform->joint[jID].skipCalculations) )
        {
          if (bvhTransform->joint[jID].pos3D[3]!=1.0)
             { fprintf(stderr,"bvh_projectTo2D: W coord (%0.2f) of joint %u not normalized..\n",bvhTransform->joint[jID].pos3D[3],jID); }

           float pos3DFloat[4];
           pos3DFloat[0]= (float) bvhTransform->joint[jID].pos3D[0];
           pos3DFloat[1]= (float) bvhTransform->joint[jID].pos3D[1];
           pos3DFloat[2]= (float) bvhTransform->joint[jID].pos3D[2];
           pos3DFloat[3]=1.0;

    //Two cases here , we either want to directly render using the matrices in our simplerenderer
    //-------------------------------------------------------------------------------------------
    if (directRendering)
          {
           fprintf(stderr,"DIRECT RENDER ");
           simpleRendererRenderUsingPrecalculatedMatrices(
                                                          renderer,
                                                          pos3DFloat,
                                                          &position2DX,
                                                          &position2DY,
                                                          &position2DW
                                                         );
          }
    //-------------------------------------------------------------------------------------------
    //Or we want to render using our camera position where we have to do some extra matrix operations
    //to recenter our mesh and transform it accordingly
    //-------------------------------------------------------------------------------------------
        else
          {
           float pos3DCenterFloat[4];
           pos3DCenterFloat[0]= (float) bvhTransform->centerPosition[0];
           pos3DCenterFloat[1]= (float) bvhTransform->centerPosition[1];
           pos3DCenterFloat[2]= (float) bvhTransform->centerPosition[2];
           pos3DCenterFloat[3]=1.0;

           simpleRendererRender(
                                 renderer ,
                                 pos3DFloat,
                                 pos3DCenterFloat,
                                 0,
                                 0,
                                 &position2DX,
                                 &position2DY,
                                 &position2DW
                               );

            //Should this only happen when position2DW>=0.0
            //bvhTransform->joint[jID].pos3D[0] = (double) pos3DCenterFloat[0];
            //bvhTransform->joint[jID].pos3D[1] = (double) pos3DCenterFloat[1];
            //bvhTransform->joint[jID].pos3D[2] = (double) pos3DCenterFloat[2];               );
           }
    //-------------------------------------------------------------------------------------------

           if (position2DW<0.0)
           {
              bvhTransform->joint[jID].pos2D[0] = 0.0;
              bvhTransform->joint[jID].pos2D[1] = 0.0;
              bvhTransform->joint[jID].isBehindCamera=1;
           } else
           {
              bvhTransform->joint[jID].pos2D[0] = (double) position2DX;
              bvhTransform->joint[jID].pos2D[1] = (double) position2DY;
              bvhTransform->joint[jID].pos2DCalculated=1;
           }

        }
        ++pointsDumped;
      } //Joint Loop , render all joint points..

    //------------------------------------------------------------------------------------------
    if (occlusions)
     {
      //Also project our Torso coordinates..
      bvh_populateRectangle2DFromProjections(
                                             mc,
                                             bvhTransform,
                                             &bvhTransform->torso
                                            );



       bvh_projectTo2DHandleOcclusions(
                                       mc,
                                       bvhTransform
                                      );
     //------------------------------------------------------------------------------------------
     }//End of occlusion code

 return (pointsDumped==mc->jointHierarchySize);
}
