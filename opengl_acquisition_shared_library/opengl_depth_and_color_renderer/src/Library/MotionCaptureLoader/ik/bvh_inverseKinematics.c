#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <time.h>

#include "bvh_inverseKinematics.h"
#include "levmar.h"

#include "../export/bvh_to_svg.h"
#include "../edit/bvh_cut_paste.h"



#define NORMAL   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */


//This is a good idea because it reduces jitter..
#define TRY_TO_STAY_AT_GLOBAL_OPTIMUM 1

#define DISCARD_POSITIONAL_COMPONENT 0
const float distance=-150;

unsigned long tickBaseIK = 0;


void clear_line()
{
  fputs("\033[A\033[2K\033[A\033[2K",stdout);
  rewind(stdout);
  int i=ftruncate(1,0);
  if (i!=0) { /*fprintf(stderr,"Error with ftruncate\n");*/ }
}

unsigned long GetTickCountMicrosecondsIK()
{
    struct timespec ts;
    if ( clock_gettime(CLOCK_MONOTONIC,&ts) != 0)
        {
            return 0;
        }

    if (tickBaseIK==0)
        {
            tickBaseIK = ts.tv_sec*1000000 + ts.tv_nsec/1000;
            return 0;
        }

    return ( ts.tv_sec*1000000 + ts.tv_nsec/1000 ) - tickBaseIK;
}


unsigned long GetTickCountMillisecondsIK()
{
    return (unsigned long) GetTickCountMicrosecondsIK()/1000;
}


float getSquared3DPointDistance(float aX,float aY,float aZ,float bX,float bY,float bZ)
{
  float diffX = (float) aX-bX;
  float diffY = (float) aY-bY;
  float diffZ = (float) aZ-bZ;
    //We calculate the distance here..!
  return (diffX*diffX) + (diffY*diffY) + (diffZ*diffZ);
}


float get3DPointDistance(float aX,float aY,float aZ,float bX,float bY,float bZ)
{
  return sqrt(getSquared3DPointDistance(aX,aY,aZ,bX,bY,bZ));
}


float getSquared2DPointDistance(float aX,float aY,float bX,float bY)
{
  float diffX = (float) aX-bX;
  float diffY = (float) aY-bY;
    //We calculate the distance here..!
  return (diffX*diffX) + (diffY*diffY);
}


float get2DPointDistance(float aX,float aY,float bX,float bY)
{
  return sqrt(getSquared2DPointDistance(aX,aY,bX,bY));
}



int writeHTML(
               unsigned int fIDSource,
               unsigned int fIDTarget,
               float initialMAEInPixels,
               float finalMAEInPixels,
               float initialMAEInMM,
               float finalMAEInMM,
               int dumpScreenshots
             )
{
  if (dumpScreenshots)
  {
  int i=system("convert initial.svg initial.png&");
  if (i!=0) { fprintf(stderr,"Error converting image..\n"); }
  i=system("convert target.svg target.png&");
  if (i!=0) { fprintf(stderr,"Error converting image..\n"); }
  i=system("convert solution.svg solution.png&");
  if (i!=0) { fprintf(stderr,"Error converting image..\n"); }

  FILE * html=fopen("report.html","w");
  if (html!=0)
  {
    fprintf(html,"<html><body><br>\n");
    //------------------------------------------------------------
    fprintf(html,"<table><tr>\n");
    fprintf(html,"<td><img src=\"initial.png\" width=400></td>\n");
    fprintf(html,"<td><img src=\"target.png\" width=400></td>\n");
    fprintf(html,"<td><img src=\"solution.png\" width=400></td>\n");
    fprintf(html,"</tr>\n");
    //------------------------------------------------------------
    fprintf(html,"<tr>\n");
    fprintf(html,"<td align=\"center\">Initial Pose (frame %u)</td>\n",fIDSource);
    fprintf(html,"<td align=\"center\">Target Pose (frame %u)</td>\n",fIDTarget);
    fprintf(html,"<td align=\"center\">Solution</td>\n");
    fprintf(html,"</tr>\n");
    fprintf(html,"</table>\n");
    //------------------------------------------------------------

    fprintf(html,"<br><br><br><br>");
    fprintf(html,"MAE in 2D Pixels went from %0.2f to %0.2f <br>",initialMAEInPixels,finalMAEInPixels);
    fprintf(html,"MAE in 3D mm went from %0.2f to %0.2f <br>",initialMAEInMM*10,finalMAEInMM*10);

    fprintf(html,"</body></html>");
    fclose(html);
    return 1;
  }

 }
  return 0;
}

float meanBVH2DDistace(
                       struct BVH_MotionCapture * mc,
                       struct simpleRenderer *renderer,
                       int useAllJoints,
                       BVHMotionChannelID onlyConsiderChildrenOfThisJoint,
                       struct BVH_Transform * bvhSourceTransform,
                       struct BVH_Transform * bvhTargetTransform,
                       unsigned int verbose
                      )
{
   if (verbose) { fprintf(stderr,"\nmeanBVH2DDistace\n"); }
   if (bvh_projectTo2D(mc,bvhSourceTransform,renderer,0,0))
      {
       //-----------------
       float sumOf2DDistances=0.0;
       unsigned int numberOfSamples=0;
       for (unsigned int jID=0; jID<mc->jointHierarchySize; jID++)
            {
              int isSelected = 1;

              if (mc->selectedJoints!=0)
              {
                if (!mc->selectedJoints[jID])
                {
                  isSelected=0;
                }
              }

               if ( (isSelected) && ( (useAllJoints) || (mc->jointHierarchy[jID].parentJoint == onlyConsiderChildrenOfThisJoint) ) )
               {
                 float tX=bvhTargetTransform->joint[jID].pos2D[0];
                 float tY=bvhTargetTransform->joint[jID].pos2D[1];

                 if ( (tX!=0.0) || (tY!=0.0) )
                 {
                  float this2DDistance=get2DPointDistance(
                                                        (float) bvhSourceTransform->joint[jID].pos2D[0],
                                                        (float) bvhSourceTransform->joint[jID].pos2D[1],
                                                        (float) bvhTargetTransform->joint[jID].pos2D[0],
                                                        (float) bvhTargetTransform->joint[jID].pos2D[1]
                                                       );
                  if (verbose)
                  {
                   fprintf(stderr,"src(%0.1f,%0.1f)->tar(%0.1f,%0.1f) : ",bvhSourceTransform->joint[jID].pos2D[0],bvhSourceTransform->joint[jID].pos2D[1],bvhTargetTransform->joint[jID].pos2D[0],bvhTargetTransform->joint[jID].pos2D[1]);
                   fprintf(stderr,"2D %s distance = %0.1f\n",mc->jointHierarchy[jID].jointName,this2DDistance);
                  }

                  numberOfSamples+=1;
                  sumOf2DDistances+=this2DDistance;
                 }
               }
            }
    if (verbose)
            { fprintf(stderr,"\n"); }

       if (numberOfSamples>0)
       {
         return (float)  sumOf2DDistances/numberOfSamples;
       }
     } //-----------------

 return 0.0;
}



float meanBVH3DDistace(
                        struct BVH_MotionCapture * mc,
                        struct simpleRenderer *renderer,
                        int useAllJoints,
                        BVHMotionChannelID onlyConsiderChildrenOfThisJoint,
                        float * sourceMotionBuffer,
                        struct BVH_Transform * bvhSourceTransform,
                        float * targetMotionBuffer,
                        struct BVH_Transform * bvhTargetTransform
                      )
{

  if (targetMotionBuffer==0) { return NAN;}

   if (
        (
          performPointProjectionsForMotionBuffer(
                                                 mc,
                                                 bvhSourceTransform,
                                                 sourceMotionBuffer,
                                                 renderer,
                                                 0,
                                                 0
                                                )
        ) &&
        (
          performPointProjectionsForMotionBuffer(
                                                 mc,
                                                 bvhTargetTransform,
                                                 targetMotionBuffer,
                                                 renderer,
                                                 0,
                                                 0
                                                )
        )
      )
      {
       //-----------------
       float sumOf3DDistances=0.0;
       unsigned int numberOfSamples=0;
       for (unsigned int jID=0; jID<mc->jointHierarchySize; jID++)
            {
              int isSelected = 1;

              if (mc->selectedJoints!=0)
              {
                if (!mc->selectedJoints[jID])
                {
                  isSelected=0;
                }
              }

               if ( (isSelected) && ( (useAllJoints) || (mc->jointHierarchy[jID].parentJoint == onlyConsiderChildrenOfThisJoint) ) )
               {
                 float tX=bvhTargetTransform->joint[jID].pos3D[0];
                 float tY=bvhTargetTransform->joint[jID].pos3D[1];
                 float tZ=bvhTargetTransform->joint[jID].pos3D[2];

                 if ( (tX!=0.0) || (tY!=0.0) || (tZ!=0.0) )
                 {
                  float this3DDistance=get3DPointDistance(
                                                         (float) bvhSourceTransform->joint[jID].pos3D[0],
                                                         (float) bvhSourceTransform->joint[jID].pos3D[1],
                                                         (float) bvhSourceTransform->joint[jID].pos3D[2],
                                                         (float) tX,
                                                         (float) tY,
                                                         (float) tZ
                                                        );

                 fprintf(stderr,"src(%0.1f,%0.1f,%0.1f)->tar(%0.1f,%0.1f,%0.1f) : ",(float) bvhSourceTransform->joint[jID].pos3D[0],
                                                               (float) bvhSourceTransform->joint[jID].pos3D[1],
                                                               (float) bvhSourceTransform->joint[jID].pos3D[2],
                                                               (float) tX,
                                                               (float) tY,
                                                               (float) tZ);
                 fprintf(stderr," %s distance^2 = %0.1f\n",mc->jointHierarchy[jID].jointName,this3DDistance);

                 numberOfSamples+=1;
                 sumOf3DDistances+=this3DDistance;
                }
              }
            }

       if (numberOfSamples>0)
       {
         return (float)  sumOf3DDistances/numberOfSamples;
       }
     } //-----------------

 return 0.0;
}


int updateProblemSolutionToAllChains(struct ikProblem * problem,struct MotionBuffer * updatedSolution)
{
  if (updatedSolution==0)          { return 0; }
  if (problem->currentSolution==0) { return 0; }
  if (problem->initialSolution==0) { return 0; }


  if (!copyMotionBuffer(problem->currentSolution,updatedSolution) ) { return 0; }
  //if (!copyMotionBuffer(problem->initialSolution,updatedSolution) ) { return 0; }

  for (unsigned int chainID=0; chainID<problem->numberOfChains; chainID++)
  {
     if (!copyMotionBuffer(problem->chain[chainID].currentSolution,updatedSolution))
     { return 0; }
  }
  return 1;
}

int cleanProblem(struct ikProblem * problem)
{
  freeMotionBuffer(problem->previousSolution);
  freeMotionBuffer(problem->initialSolution);
  freeMotionBuffer(problem->currentSolution);

  for (unsigned int chainID=0; chainID<problem->numberOfChains; chainID++)
  {
    freeMotionBuffer(problem->chain[chainID].currentSolution);
  }

 return 1;
}

int prepareProblem(
                   struct ikProblem * problem,
                   struct BVH_MotionCapture * mc,
                   struct simpleRenderer *renderer,
                   struct MotionBuffer * previousSolution,
                   struct MotionBuffer * solution,
                   struct BVH_Transform * bvhTargetTransform
                  )
{
  problem->mc = mc;
  problem->renderer = renderer;

  problem->previousSolution = mallocNewMotionBufferAndCopy(mc,previousSolution);
  problem->initialSolution  = mallocNewMotionBufferAndCopy(mc,solution);
  problem->currentSolution  = mallocNewMotionBufferAndCopy(mc,solution);

  //2D Projections Targeted
  //----------------------------------------------------------
  problem->bvhTarget2DProjectionTransform = bvhTargetTransform;



  //Chain #0 is Joint Hip-> to all its children
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  unsigned int groupID=0;
  unsigned int jobID=0;
  unsigned int chainID=0;
  unsigned int partID=0;
  BVHJointID thisJID=0;
  //----------------------------------------------------------





  //Chain 0 is the Hip and all of the rigid torso
  //----------------------------------------------------------
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

  #if DISCARD_POSITIONAL_COMPONENT
   fprintf(stderr,"Ignoring positional component..\n");
  #else
   if (bvh_getJointIDFromJointName(mc,"hip",&thisJID) )
   {
     bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
     problem->chain[chainID].part[partID].partParent=0; //This is the parent
     problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
     problem->chain[chainID].part[partID].jID=thisJID;
     problem->chain[chainID].part[partID].mIDStart=0; //First Position
     problem->chain[chainID].part[partID].mIDEnd=2; //First Position
     problem->chain[chainID].part[partID].bigChanges=1;
     problem->chain[chainID].part[partID].jointImportance=2.0;
     ++partID;
   }
  #endif // DISCARD_POSITIONAL_COMPONENT


  if (bvh_getJointIDFromJointName(mc,"hip",&thisJID) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=3; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=5; //First Rotation
   problem->chain[chainID].part[partID].jointImportance=2.0;
   ++partID;
  } else
  { fprintf(stderr,"No hip in armature..\n"); return 0; }

  if (bvh_getJointIDFromJointName(mc,"neck",&thisJID) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No neck in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"rshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rShldr",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No rshoulder in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lShldr",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No lshoulder in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"rhip",&thisJID) )  || (bvh_getJointIDFromJointName(mc,"rThigh",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.5; //Hips are more important to hips upper body can rotate via chest
   ++partID;
  } else
  { fprintf(stderr,"No rhip in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lhip",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lThigh",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.5; //Hips are more important to hips upper body can rotate via chest

   ++partID;
  } else
  { fprintf(stderr,"No lhip in armature..\n"); return 0; }

 problem->chain[chainID].numberOfParts=partID;

 ++chainID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------




  //Chain 1 is the Right Arm
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  partID=0;
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

 if (bvh_getJointIDFromJointName(mc,"chest",&thisJID) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=0.5;
   ++partID;
  } else
  { fprintf(stderr,"No rshoulder in armature..\n"); return 0; }

 if (bvh_getJointIDFromJointName(mc,"neck",&thisJID) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].jointImportance=0.5; //Less important because it can be addressed by chest
   ++partID;
  } else
  { fprintf(stderr,"No rshoulder in armature..\n"); return 0; }


  if ( (bvh_getJointIDFromJointName(mc,"rshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rShldr",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].jointImportance=1.0; //Less important because it can be addressed by chest
   ++partID;
  } else
  { fprintf(stderr,"No rshoulder in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lForeArm",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].jointImportance=1.0; //Less important because it can be addressed by chest
   ++partID;
  } else
  { fprintf(stderr,"No relbow in armature..\n"); return 0; }


 problem->chain[chainID].numberOfParts=partID;
 ++chainID;
 ++jobID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------


 //These are first group..
 ++groupID;




  //Chain 1 is the Right Arm
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  partID=0;
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

  if ( (bvh_getJointIDFromJointName(mc,"rshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rShldr",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=0.5;
   ++partID;
  } else
  { fprintf(stderr,"No rshoulder in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"relbow",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rForeArm",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0; //This is the parent
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No relbow in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"rhand",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rHand",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=1;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.5;
   ++partID;
  } else
  { fprintf(stderr,"No rhand in armature..\n"); return 0; }

 problem->chain[chainID].numberOfParts=partID;
 ++chainID;
 ++jobID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------





  //Chain 2 is the Left Arm
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  partID=0;
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

  if ( (bvh_getJointIDFromJointName(mc,"lshoulder",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lShldr",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=0.5;
   ++partID;
  } else
  { fprintf(stderr,"No lshoulder in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lelbow",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lForeArm",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No lelbow in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lhand",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lHand",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=1;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.5;
   ++partID;
  } else
  { fprintf(stderr,"No lhand in armature..\n"); return 0; }

 problem->chain[chainID].numberOfParts=partID;
 ++chainID;
 ++jobID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------




  //Chain 3 is the Right Leg
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  partID=0;
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

  if ( (bvh_getJointIDFromJointName(mc,"rhip",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rThigh",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=0.5;
   ++partID;
  } else
  { fprintf(stderr,"No rhip in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"rknee",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"rShin",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No rknee in armature..\n"); return 0; }


  if ( (bvh_getJointIDFromJointName(mc,"rfoot",&thisJID) )  || (bvh_getJointIDFromJointName(mc,"rFoot",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=1;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No rfoot in armature..\n"); return 0; }


  if ( (bvh_getJointIDFromJointName(mc,"EndSite_toe1-2.R",&thisJID) )  || (bvh_getJointIDFromJointName(mc,"endsite_toe1-2.r",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=2;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No R toe in armature..\n"); return 0; }



 problem->chain[chainID].numberOfParts=partID;
 ++chainID;
 ++jobID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------



  //Chain 4 is the Left Leg
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  partID=0;
  problem->chain[chainID].groupID=groupID;
  problem->chain[chainID].jobID=jobID;
  problem->chain[chainID].currentSolution=mallocNewMotionBufferAndCopy(mc,problem->initialSolution);

  bvh_markAllJointsAsUselessInTransform(mc,&problem->chain[chainID].current2DProjectionTransform);

  if ( (bvh_getJointIDFromJointName(mc,"lhip",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lThigh",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=0.5;
   ++partID;
  } else
  { fprintf(stderr,"No hip in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lknee",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lShin",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=0;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No lknee in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"lfoot",&thisJID) ) || (bvh_getJointIDFromJointName(mc,"lFoot",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=1;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=0;
   problem->chain[chainID].part[partID].mIDStart=mc->jointToMotionLookup[thisJID].jointMotionOffset; //First Rotation
   problem->chain[chainID].part[partID].mIDEnd=problem->chain[chainID].part[partID].mIDStart + mc->jointHierarchy[thisJID].loadedChannels-1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No lfoot in armature..\n"); return 0; }

  if ( (bvh_getJointIDFromJointName(mc,"EndSite_toe1-2.L",&thisJID) )  || (bvh_getJointIDFromJointName(mc,"endsite_toe1-2.l",&thisJID)) )
  {
   bvh_markJointAndParentsAsUsefulInTransform(mc,&problem->chain[chainID].current2DProjectionTransform,thisJID);
   problem->chain[chainID].part[partID].partParent=2;
   problem->chain[chainID].part[partID].evaluated=0; //Not evaluated yet
   problem->chain[chainID].part[partID].jID=thisJID;
   problem->chain[chainID].part[partID].endEffector=1;
   problem->chain[chainID].part[partID].jointImportance=1.0;
   ++partID;
  } else
  { fprintf(stderr,"No L toe in armature..\n"); return 0; }



 problem->chain[chainID].numberOfParts=partID;
 ++chainID;
 ++jobID;
  //----------------------------------------------------------
  //----------------------------------------------------------
  //----------------------------------------------------------
  ++groupID;

  problem->numberOfChains = chainID;
  problem->numberOfGroups = groupID;

 return 1;
}


int viewProblem(
                struct ikProblem * problem
               )
{
 fprintf(stderr,"The IK problem we want to solve has %u groups of subproblems\n",problem->numberOfGroups);
 fprintf(stderr,"It is also ultimately divided into %u kinematic chains\n",problem->numberOfChains);

 for (unsigned int chainID=0; chainID<problem->numberOfChains; chainID++)
 {
   fprintf(stderr,"Chain %u has %u parts : ",chainID,problem->chain[chainID].numberOfParts);
   for (unsigned int partID=0; partID<problem->chain[chainID].numberOfParts; partID++)
   {
     unsigned int jID=problem->chain[chainID].part[partID].jID;

     if (problem->chain[chainID].part[partID].endEffector)
     {
      fprintf(
              stderr,"jID(%s/%u)->EndEffector ",
              problem->mc->jointHierarchy[jID].jointName,
              jID
             );
     } else
     {
     fprintf(
             stderr,"jID(%s/%u)->mID(%u to %u) ",
             problem->mc->jointHierarchy[jID].jointName,
             jID,
             problem->chain[chainID].part[partID].mIDStart,
             problem->chain[chainID].part[partID].mIDEnd
             );
     }
   }
   fprintf(stderr,"\n");
 }

 return 1;
}




float calculateChainLoss(
                         struct ikProblem * problem,
                         unsigned int chainID,
                         unsigned int partIDStart
                        )
{
  unsigned int numberOfSamples=0;
  float loss=0.0;
  if (chainID<problem->numberOfChains)
  {
   //fprintf(stderr,"Chain %u has %u parts : ",chainID,problem->chain[chainID].numberOfParts);

     if (
         bvh_loadTransformForMotionBuffer(
                                          problem->mc,
                                          problem->chain[chainID].currentSolution->motion,
                                          &problem->chain[chainID].current2DProjectionTransform,
                                          0//Dont populate extra structures we dont need them they just take time
                                         )
        )
      {
       #if DISCARD_POSITIONAL_COMPONENT
        bvh_removeTranslationFromTransform(
                                            problem->mc,
                                            &problem->chain[chainID].current2DProjectionTransform
                                          );
       #endif // DISCARD_POSITIONAL_COMPONENT

      if  (bvh_projectTo2D(problem->mc,&problem->chain[chainID].current2DProjectionTransform,problem->renderer,0,0))
      {
       for (unsigned int partID=partIDStart; partID<problem->chain[chainID].numberOfParts; partID++)
       {
         //if ( (partID==partIDStart) || (problem->chain[chainID].part[partID].partParent==partIDStart) )
         {
         unsigned int jID=problem->chain[chainID].part[partID].jID;
         float tX = problem->bvhTarget2DProjectionTransform->joint[jID].pos2D[0];
         float tY = problem->bvhTarget2DProjectionTransform->joint[jID].pos2D[1];

         if ((tX!=0.0) || (tY!=0.0) )
         { //Ignore empty joints ..!
          float thisSquared2DDistance=getSquared2DPointDistance(
                                                                (float) problem->chain[chainID].current2DProjectionTransform.joint[jID].pos2D[0],
                                                                (float) problem->chain[chainID].current2DProjectionTransform.joint[jID].pos2D[1],
                                                                tX,
                                                                tY
                                                               );
         /*
          fprintf(stderr,"%0.2f,%0.2f -> %0.2f,%0.2f : ",
          problem->chain[chainID].current2DProjectionTransform.joint[jID].pos2D[0],
          problem->chain[chainID].current2DProjectionTransform.joint[jID].pos2D[1],
          problem->bvhTarget2DProjectionTransform->joint[jID].pos2D[0],
          problem->bvhTarget2DProjectionTransform->joint[jID].pos2D[1]
          );
          fprintf(stderr,"Joint squared %s distance is %0.2f\n",problem->mc->jointHierarchy[jID].jointName,thisSquared2DDistance);*/
          loss+=thisSquared2DDistance * problem->chain[chainID].part[partID].jointImportance;
          ++numberOfSamples;
         }
         }
       }
     }

      } //Have a valid 2D transform
 } //Have a valid chain

  //I have left 0/0 on purpose to cause NaNs when projection errors occur
  //---------------------------------------------------------------------
  if (numberOfSamples!=0)
   {
     loss = (float) loss/numberOfSamples;
   } else
   {
     loss = NAN;
   }
  //---------------------------------------------------------------------
  return loss;
}


float iteratePartLoss(
                         struct ikProblem * problem,
                         unsigned int chainID,
                         unsigned int partID,
                         float lr,
                         unsigned int epochs,
                         unsigned int tryMaintainingLocalOptima,
                         unsigned int springIgnoresIterativeChanges,
                         unsigned int verbose
                        )
{
 unsigned long startTime = GetTickCountMicrosecondsIK();

 unsigned int consecutiveBadSteps=0;
 unsigned int mIDS[3];
 mIDS[0]= problem->chain[chainID].part[partID].mIDStart;
 mIDS[1]= problem->chain[chainID].part[partID].mIDStart+1;
 mIDS[2]= problem->chain[chainID].part[partID].mIDStart+2;


 float originalValues[3];


 if (springIgnoresIterativeChanges)
 {
  originalValues[0] = problem->initialSolution->motion[mIDS[0]];
  originalValues[1] = problem->initialSolution->motion[mIDS[1]];
  originalValues[2] = problem->initialSolution->motion[mIDS[2]];
 } else
 {
 originalValues[0] = problem->chain[chainID].currentSolution->motion[mIDS[0]];
 originalValues[1] = problem->chain[chainID].currentSolution->motion[mIDS[1]];
 originalValues[2] = problem->chain[chainID].currentSolution->motion[mIDS[2]];
 }

 //This has to happen before the transform economy call (bvh_markJointAsUsefulAndParentsAsUselessInTransform) or all hell will break loose..
 float initialLoss = calculateChainLoss(problem,chainID,partID);

///This is an important call to make sure that we only update this joint and its children but not its parents ( for performance reasons.. )
 bvh_markJointAsUsefulAndParentsAsUselessInTransform(
                                                     problem->mc ,
                                                     &problem->chain[chainID].current2DProjectionTransform,
                                                     problem->chain[chainID].part[partID].jID
                                                    );


 if (verbose) { fprintf(stderr,"\nOptimizing %s (initial loss %0.2f)\n",problem->mc->jointHierarchy[problem->chain[chainID].part[partID].jID].jointName,initialLoss); }


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
#define REMEMBER 0
#if REMEMBER
if (problem->previousSolution!=0)
{
 if (problem->previousSolution->motion!=0)
  {
   //Maybe previous solution is closer to current?
   //previousSolution
   problem->chain[chainID].currentSolution->motion[mIDS[0]] = (float) (problem->previousSolution->motion[mIDS[0]] + originalValues[0]) / 2;
   problem->chain[chainID].currentSolution->motion[mIDS[1]] = (float) (problem->previousSolution->motion[mIDS[1]] + originalValues[1]) / 2;
   problem->chain[chainID].currentSolution->motion[mIDS[2]] = (float) (problem->previousSolution->motion[mIDS[2]] + originalValues[2]) / 2;
   float previousLoss = calculateChainLoss(problem,chainID,partID);
   if (previousLoss<initialLoss)
   {
     fprintf(stderr,"Previous solution loss (%0.2f) is better than current (%0.2f) \n",previousLoss,initialLoss);
     originalValues[0] = problem->chain[chainID].currentSolution->motion[mIDS[0]];
     originalValues[1] = problem->chain[chainID].currentSolution->motion[mIDS[1]];
     originalValues[2] = problem->chain[chainID].currentSolution->motion[mIDS[2]];
     lr*=10;
     initialLoss = previousLoss;
   }
  }
}
#endif // REMEMBER
//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

 float previousValues[3];
 previousValues[0] = originalValues[0];
 previousValues[1] = originalValues[1];
 previousValues[2] = originalValues[2];

 float currentValues[3];
 currentValues[0] = originalValues[0];
 currentValues[1] = originalValues[1];
 currentValues[2] = originalValues[2];

 float bestValues[3];
 bestValues[0] = originalValues[0];
 bestValues[1] = originalValues[1];
 bestValues[2] = originalValues[2];


 float previousLoss[3];
 float currentLoss[3];


 previousLoss[0]=initialLoss;
 previousLoss[1]=initialLoss;
 previousLoss[2]=initialLoss;

 float bestLoss = initialLoss;
 float loss=initialLoss;

 unsigned int maximumConsecutiveBadEpochs=3;
 float e=0.0001;
 float d=lr; //0.005;
 float beta = 0.9; // Momentum
 float gradient;
 float distanceFromInitial;
 float spring = 10.0;


 //Give an initial direction..
 float delta[3]={d,d,d};


 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------
 if (tryMaintainingLocalOptima)
 {
 //Are we at a global optimum? ---------------------------------------------------------------------------------
 //Do we care ? ----------------------------------------------------------------------------------
 unsigned int badLosses=0;
 for (unsigned int i=0; i<3; i++)
 {
  problem->chain[chainID].currentSolution->motion[mIDS[i]] = currentValues[i]+d;
  float lossPlusD=calculateChainLoss(problem,chainID,partID);
  problem->chain[chainID].currentSolution->motion[mIDS[i]] = currentValues[i]-d;
  float lossMinusD=calculateChainLoss(problem,chainID,partID);
  problem->chain[chainID].currentSolution->motion[mIDS[i]] = previousValues[i];

  if (
       (initialLoss<=lossPlusD) && (initialLoss<=lossMinusD)
     )
    {
      if (verbose)
             { fprintf(stderr,"Initial #%u value seems to be locally optimal but nudged..!\n",i); }
      delta[i] = d;
      ++badLosses;
    } else
    {
     if (verbose)
             {  fprintf(stderr,"Initial #%u value can be straight-forwardly changed..!\n",i); }
    }
 }
 if (badLosses==3)
 {
   return initialLoss;
 }

 delta[0] = d;
 delta[1] = d;
 delta[2] = d;
 //-------------------------------------------------------------------------------------------------------------
 }
 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------
 ///--------------------------------------------------------------------------------------------------------------






 if (verbose)
 {
  fprintf(stderr,"  State |   loss   | rX  |  rY  |  rZ \n");
  fprintf(stderr,"Initial | %0.1f | %0.2f  |  %0.2f  |  %0.2f \n",initialLoss,originalValues[0],originalValues[1],originalValues[2]);
 }


 for (unsigned int i=0; i<epochs; i++)
 {
 //-------------------
   problem->chain[chainID].currentSolution->motion[mIDS[0]] = currentValues[0];
   distanceFromInitial=fabs(currentValues[0] - originalValues[0]);
   currentLoss[0]=calculateChainLoss(problem,chainID,partID) + spring * distanceFromInitial * distanceFromInitial;
   problem->chain[chainID].currentSolution->motion[mIDS[0]] = previousValues[0];
 //-------------------
   problem->chain[chainID].currentSolution->motion[mIDS[1]] = currentValues[1];
   distanceFromInitial=fabs(currentValues[1] - originalValues[1]);
   currentLoss[1]=calculateChainLoss(problem,chainID,partID) + spring * distanceFromInitial * distanceFromInitial;
   problem->chain[chainID].currentSolution->motion[mIDS[1]] = previousValues[1];
 //-------------------
   problem->chain[chainID].currentSolution->motion[mIDS[2]] = currentValues[2];
   distanceFromInitial=fabs(currentValues[2] - originalValues[2]);
   currentLoss[2]=calculateChainLoss(problem,chainID,partID) + spring * distanceFromInitial * distanceFromInitial;
   problem->chain[chainID].currentSolution->motion[mIDS[2]] = previousValues[2];
 //-------------------


   //We multiply by 0.5 to do a "One Half Mean Squared Error"
   gradient =  (float) 0.5 * (previousLoss[0] - currentLoss[0]) / (delta[0]+e);
   delta[0] =  beta * delta[0] + (float) lr *  gradient;

   gradient =  (float) 0.5 * (previousLoss[1] - currentLoss[1]) / (delta[1]+e);
   delta[1] =  beta * delta[1] + (float) lr * gradient;

   gradient =  (float) 0.5 * (previousLoss[2] - currentLoss[2]) / (delta[2]+e);
   delta[2] =  beta * delta[2] + (float) lr * gradient;

   if (delta[0]!=delta[0]) { delta[0]=0;}
   if (delta[1]!=delta[1]) { delta[1]=0;}
   if (delta[2]!=delta[2]) { delta[2]=0;}

   previousLoss[0]=currentLoss[0];
   previousLoss[1]=currentLoss[1];
   previousLoss[2]=currentLoss[2];

   previousValues[0]=currentValues[0];
   previousValues[1]=currentValues[1];
   previousValues[2]=currentValues[2];

   currentValues[0]+=delta[0];
   currentValues[1]+=delta[1];
   currentValues[2]+=delta[2];

   problem->chain[chainID].currentSolution->motion[mIDS[0]] = currentValues[0];
   problem->chain[chainID].currentSolution->motion[mIDS[1]] = currentValues[1];
   problem->chain[chainID].currentSolution->motion[mIDS[2]] = currentValues[2];
   loss=calculateChainLoss(problem,chainID,partID);

   if (loss==NAN)
   {
     if (verbose) { fprintf(stderr,RED "%07u | %0.1f | %0.2f  |  %0.2f  |  %0.2f \n" NORMAL,i,loss,currentValues[0],currentValues[1],currentValues[2]); }
     break;
   } else
   if ( (loss<bestLoss) && ( fabs(currentValues[0])<360 ) && ( fabs(currentValues[1])<360 ) && ( fabs(currentValues[2])<360 ) )
   {
     bestLoss=loss;
     bestValues[0]=currentValues[0];
     bestValues[1]=currentValues[1];
     bestValues[2]=currentValues[2];
     consecutiveBadSteps=0;
      if (verbose) { fprintf(stderr,"%07u | %0.1f | %0.2f  |  %0.2f  |  %0.2f \n",i,loss,currentValues[0],currentValues[1],currentValues[2]); }
   } else
   {
     ++consecutiveBadSteps;
     if (verbose) { fprintf(stderr,YELLOW "%07u | %0.1f | %0.2f  |  %0.2f  |  %0.2f \n" NORMAL,i,loss,currentValues[0],currentValues[1],currentValues[2]); }
   }



   if (consecutiveBadSteps>=maximumConsecutiveBadEpochs)
      {
        if (verbose)
             {
              fprintf(stderr,YELLOW "Early Stopping\n" NORMAL);
             }
        break;
      }
 }
 unsigned long endTime = GetTickCountMicrosecondsIK();

        if (verbose)
             {
                fprintf(stderr,"Optimization for joint %s \n", problem->mc->jointHierarchy[problem->chain[chainID].part[partID].jID].jointName);
                fprintf(stderr,"Improved loss from %0.2f to %0.2f ( %0.2f%% ) in %lu microseconds \n",initialLoss,bestLoss, 100 - ( (float) 100* bestLoss/initialLoss ),endTime-startTime);
                fprintf(stderr,"Optimized values changed from %0.2f,%0.2f,%0.2f to %0.2f,%0.2f,%0.2f\n",originalValues[0],originalValues[1],originalValues[2],bestValues[0],bestValues[1],bestValues[2]);
                fprintf(stderr,"correction of %0.2f,%0.2f,%0.2f deg\n",bestValues[0]-originalValues[0],bestValues[1]-originalValues[1],bestValues[2]-originalValues[2]);
             }

   problem->chain[chainID].currentSolution->motion[mIDS[0]] = bestValues[0];
   problem->chain[chainID].currentSolution->motion[mIDS[1]] = bestValues[1];
   problem->chain[chainID].currentSolution->motion[mIDS[2]] = bestValues[2];


 ///This is an important call to make sure that we leave everything as we left it for the next joint ( for performance reasons.. )
 bvh_markJointAndParentsAsUsefulInTransform(
                                            problem->mc ,
                                            &problem->chain[chainID].current2DProjectionTransform,
                                            problem->chain[chainID].part[partID].jID
                                           );
  ///-------------------------------------------------------------------------------------------------------------------------------


  return bestLoss;

}








int iterateChainLoss(
                         struct ikProblem * problem,
                         unsigned int chainID,
                         float lr,
                         unsigned int epochs,
                         unsigned int tryMaintainingLocalOptima,
                         unsigned int springIgnoresIterativeChanges,
                         unsigned int verbose
                        )
{
 //Before we start we will make a copy of the problem->currentSolution to work on improving it..
 copyMotionBuffer(problem->chain[chainID].currentSolution,problem->currentSolution);

 for (unsigned int partID=0; partID<problem->chain[chainID].numberOfParts; partID++)
 {
   if (!problem->chain[chainID].part[partID].endEffector)
   {
    iteratePartLoss(
                    problem,
                    chainID,
                    partID,
                    lr,
                    epochs,
                    tryMaintainingLocalOptima,
                    springIgnoresIterativeChanges,
                    verbose
                   );
   }
 }

 //After we finish we update the problem->currentSolution with what our chain came up with..
 copyMotionBuffer(problem->currentSolution,problem->chain[chainID].currentSolution);

 return 1;
}







int approximateBodyFromMotionBufferUsingInverseKinematics(
                                                          struct BVH_MotionCapture * mc,
                                                          struct simpleRenderer *renderer,
                                                          struct ikConfiguration * ikConfig,
                                                          //---------------------------------
                                                          struct MotionBuffer * previousSolution,
                                                          struct MotionBuffer * solution,
                                                          struct MotionBuffer * groundTruth,
                                                          //---------------------------------
                                                          struct BVH_Transform * bvhTargetTransform,
                                                          //---------------------------------
                                                          float * initialMAEInPixels,
                                                          float * finalMAEInPixels,
                                                          float * initialMAEInMM,
                                                          float * finalMAEInMM
                                                         )
{
  if (ikConfig == 0)
  {
   fprintf(stderr,RED "No configuration provided for IK..\n" NORMAL);
   return 0;
  }


 if (ikConfig->ikVersion != (float) IK_VERSION)
 {
   fprintf(stderr,RED "IK Version mismatch for configuration structure (%0.2f vs %0.2f ) ..\n" NORMAL,ikConfig->ikVersion,IK_VERSION);
   exit(0);
 }

 struct ikProblem * problem= (struct ikProblem * ) malloc(sizeof(struct ikProblem));
 if (problem==0)
     {
        fprintf(stderr,"Failed to allocate memory for our IK problem..\n");
        return 0;
     }

 memset(problem,0,sizeof(struct ikProblem));

  if (!prepareProblem(
                      problem,
                      mc,
                      renderer,
                      previousSolution,
                      solution,
                      bvhTargetTransform
                     )
     )
     {
        fprintf(stderr,"Could not prepare the problem for IK solution\n");
        free(problem);
        return 0;
     }

  //Don't spam console..
  //viewProblem(problem);


  float previousMAEInPixels=1000000; //Big invalid number
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  struct BVH_Transform bvhCurrentTransform={0};


  if (
         bvh_loadTransformForMotionBuffer(
                                          mc,
                                          problem->initialSolution->motion,
                                          &bvhCurrentTransform,
                                          0// We don't need extra structures
                                         )
        )
      {
       #if DISCARD_POSITIONAL_COMPONENT
        bvh_removeTranslationFromTransform(
                                            mc,
                                            &bvhCurrentTransform
                                          );
       #endif // DISCARD_POSITIONAL_COMPONENT


      //----------------------------------------------------
       if (initialMAEInPixels!=0)
       {
        *initialMAEInPixels = meanBVH2DDistace(
                                       mc,
                                       renderer,
                                       1,
                                       0,
                                       &bvhCurrentTransform,
                                       bvhTargetTransform,
                                       ikConfig->verbose
                                      );
       }
      //----------------------------------------------------



      //----------------------------------------------------
      //----------------------------------------------------
      //----------------------------------------------------
      if (problem->previousSolution!=0)
       {
        if (problem->previousSolution->motion!=0)
          {
            struct BVH_Transform bvhPrevioustTransform={0};
              if (
                   bvh_loadTransformForMotionBuffer(
                                                    mc,
                                                    previousSolution->motion,
                                                    &bvhPrevioustTransform,
                                                    0//Dont populate extra structures
                                                   )
                  )
             {
               previousMAEInPixels = meanBVH2DDistace(
                                       mc,
                                       renderer,
                                       1,
                                       0,
                                       &bvhPrevioustTransform,
                                       bvhTargetTransform,
                                       ikConfig->verbose
                                      );
                if (previousMAEInPixels < *initialMAEInPixels)
                {
                  fprintf(stderr,"Previous MAE (%0.2f) seems to be lower than estimation (%0.2f) ..\n",previousMAEInPixels,*initialMAEInPixels);
                  /* THIS WORKS BADLY..!
                  if (!updateProblemSolutionToAllChains(problem,previousSolution))
                  {
                    fprintf(stderr,RED "Failed to updated problem solution..\n" NORMAL);
                    //exit(0);
                  }*/
                  //exit(0);
                }
             }
          }
        }
      //----------------------------------------------------
      //----------------------------------------------------
      //----------------------------------------------------


      if ( (initialMAEInMM!=0) && (groundTruth!=0) )
       {
        *initialMAEInMM = meanBVH3DDistace(
                                           mc,
                                           renderer,
                                           1,
                                           0,
                                           problem->initialSolution->motion,
                                           &bvhCurrentTransform,
                                           groundTruth->motion,
                                           bvhTargetTransform
                                          );
       }
      //----------------------------------------------------

      }

  if (ikConfig->dumpScreenshots)
  {
   dumpBVHToSVGFrame(
                     "initial.svg",
                      mc,
                      &bvhCurrentTransform,
                      0,
                      renderer
                     );
  }
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------




  for (int t=0; t<ikConfig->iterations; t++)
  {
   for (int chainID=0; chainID<problem->numberOfChains; chainID++)
   {
      iterateChainLoss(
                       problem,
                       chainID,
                       ikConfig->learningRate,
                       ikConfig->epochs,
                       ikConfig->tryMaintainingLocalOptima,
                       ikConfig->springIgnoresIterativeChanges,
                       ikConfig->verbose
                      );
   }
  }

   //Retrieve regressed solution
   copyMotionBuffer(solution,problem->currentSolution);






  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------

  if (
         bvh_loadTransformForMotionBuffer(
                                          mc,
                                          solution->motion,
                                          &bvhCurrentTransform,
                                          0// dont use extra structures
                                         )
     )
      {
       #if DISCARD_POSITIONAL_COMPONENT
        bvh_removeTranslationFromTransform(
                                            mc,
                                            &bvhCurrentTransform
                                          );
       #endif // DISCARD_POSITIONAL_COMPONENT


      //----------------------------------------------------
      if (finalMAEInPixels!=0)
       {
        *finalMAEInPixels  = meanBVH2DDistace(
                                       mc,
                                       renderer,
                                       1,
                                       0,
                                       &bvhCurrentTransform,
                                       bvhTargetTransform,
                                       ikConfig->verbose
                                     );
         if (previousMAEInPixels<*finalMAEInPixels)
         {
           if (ikConfig->considerPreviousSolution)
                {
                 fprintf(stderr,RED "After all this work we where not smart enough to understand that previous solution was better all along..\n" NORMAL);
                 copyMotionBuffer(solution,previousSolution);
                }
         }
      }
      //----------------------------------------------------
      if ( (finalMAEInMM!=0) && (groundTruth!=0) )
       {
        *finalMAEInMM = meanBVH3DDistace(
                                       mc,
                                       renderer,
                                       1,
                                       0,
                                       solution->motion,
                                       &bvhCurrentTransform,
                                       groundTruth->motion,
                                       bvhTargetTransform
                                      );
      }
      //----------------------------------------------------


      }
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------

  if (ikConfig->dumpScreenshots)
  {
  dumpBVHToSVGFrame(
                     "target.svg",
                      mc,
                      bvhTargetTransform,
                      1,
                      renderer
                     );

  dumpBVHToSVGFrame(
                     "solution.svg",
                      mc,
                      &bvhCurrentTransform,
                      0,
                      renderer
                     );

 }


 //Cleanup allocations needed for the problem..
 cleanProblem(problem);
 free(problem);
 return 1;
}



// ./BVHTester --from Motions/05_01.bvh --selectJoints 0 23 hip eye.r eye.l abdomen chest neck head rshoulder relbow rhand lshoulder lelbow lhand rhip rknee rfoot lhip lknee lfoot toe1-2.r toe5-3.r toe1-2.l toe5-3.l --testIK 80 4 130 0.001 5 100

int bvhTestIK(
              struct BVH_MotionCapture * mc,
              float lr,
              unsigned int iterations,
              unsigned int epochs,
              unsigned int fIDPrevious,
              unsigned int fIDSource,
              unsigned int fIDTarget
             )
{
  int result=0;

  struct BVH_Transform bvhTargetTransform={0};

  struct simpleRenderer renderer={0};
  simpleRendererDefaults( &renderer, 1920, 1080, 582.18394,   582.52915 );// https://gopro.com/help/articles/Question_Answer/HERO4-Field-of-View-FOV-Information
  simpleRendererInitialize(&renderer);

  fprintf(stderr,"BVH file has motion files with %u elements\n",mc->numberOfValuesPerFrame);

  float initialMAEInPixels=0.0,finalMAEInPixels=0.0;
  float initialMAEInMM=0.0,finalMAEInMM=0.0;

  //Load all motion buffers
  struct MotionBuffer * groundTruth     = mallocNewMotionBuffer(mc);
  struct MotionBuffer * initialSolution = mallocNewMotionBuffer(mc);
  struct MotionBuffer * solution        = mallocNewMotionBuffer(mc);
  struct MotionBuffer * previousSolution= mallocNewMotionBuffer(mc);



  if ( (solution!=0) && (groundTruth!=0) && (initialSolution!=0) && (previousSolution!=0) )
  {
    if (
          ( bvh_copyMotionFrameToMotionBuffer(mc,initialSolution,fIDSource) ) &&
          ( bvh_copyMotionFrameToMotionBuffer(mc,previousSolution,fIDPrevious) ) &&
          ( bvh_copyMotionFrameToMotionBuffer(mc,solution,fIDSource) ) &&
          ( bvh_copyMotionFrameToMotionBuffer(mc,groundTruth,fIDTarget) )
       )
      {
       initialSolution->motion[0]=0;
       initialSolution->motion[1]=0;
       initialSolution->motion[2]=distance;

       previousSolution->motion[0]=0;
       previousSolution->motion[1]=0;
       previousSolution->motion[2]=distance;

       solution->motion[0]=0;
       solution->motion[1]=0;
       solution->motion[2]=distance;


       groundTruth->motion[0]=0;
       groundTruth->motion[1]=0;
       groundTruth->motion[2]=distance;


        if ( bvh_loadTransformForMotionBuffer(mc,groundTruth->motion,&bvhTargetTransform,0) )
         {
           if  (bvh_projectTo2D(mc,&bvhTargetTransform,&renderer,0,0))
           {
            #if DISCARD_POSITIONAL_COMPONENT
            bvh_removeTranslationFromTransform(
                                                mc,
                                                &bvhTargetTransform
                                              );
            #endif // DISCARD_POSITIONAL_COMPONENT

            unsigned int springIgnoresIterativeChanges=0;

            struct ikConfiguration ikConfig={0};
            //------------------------------------
            ikConfig.learningRate = lr;
            ikConfig.iterations = iterations;
            ikConfig.epochs = epochs;
            ikConfig.springIgnoresIterativeChanges = springIgnoresIterativeChanges;
            ikConfig.dumpScreenshots = 1;
            ikConfig.verbose = 1;
            ikConfig.tryMaintainingLocalOptima=1; //Less Jittery but can be stuck at local optima
            ikConfig.ikVersion = IK_VERSION;
            //------------------------------------

            if (
                approximateBodyFromMotionBufferUsingInverseKinematics(
                                                                       mc,
                                                                       &renderer,
                                                                       &ikConfig,
                                                                       //---------------
                                                                       previousSolution,
                                                                       solution,
                                                                       groundTruth,
                                                                       //-------------------
                                                                       &bvhTargetTransform,
                                                                       //-------------------
                                                                       &initialMAEInPixels,
                                                                       &finalMAEInPixels,
                                                                       &initialMAEInMM,
                                                                       &finalMAEInMM
                                                                      )
                )
            {
             //Important :)
             //=======
             result=1;
             //=======

            //-------------------------------------------------------------------------------------------------------------
            //compareMotionBuffers("The problem we want to solve compared to the initial state",initialSolution,groundTruth);
            //compareMotionBuffers("The solution we proposed compared to ground truth",solution,groundTruth);
            //-------------------------------------------------------------------------------------------------------------
            compareTwoMotionBuffers(mc,"Improvement",initialSolution,solution,groundTruth);

            fprintf(stderr,"MAE in 2D Pixels went from %0.2f to %0.2f \n",initialMAEInPixels,finalMAEInPixels);
            fprintf(stderr,"MAE in 3D mm went from %0.2f to %0.2f \n",initialMAEInMM*10,finalMAEInMM*10);


             //-------------------------------------------------------------------------------------------------
             writeHTML(
                       fIDSource,
                       fIDTarget,
                       initialMAEInPixels,
                       finalMAEInPixels,
                       initialMAEInMM,
                       finalMAEInMM,
                       ikConfig.dumpScreenshots
                     );
             //-------------------------------------------------------------------------------------------------
            } else
            {
              fprintf(stderr,"Failed to run IK code..\n");
            }


           } else
            {
              fprintf(stderr,"Could not project 2D points of target..\n");
            }
         }
      }
    freeMotionBuffer(previousSolution);
    freeMotionBuffer(solution);
    freeMotionBuffer(initialSolution);
    freeMotionBuffer(groundTruth);
  }

 return result;
}



























































//https://www.gamasutra.com/blogs/LuisBermudez/20170804/303066/3_Simple_Steps_to_Implement_Inverse_Kinematics.php
//https://groups.csail.mit.edu/drl/journal_club/papers/033005/buss-2004.pdf
//https://simtk-confluence.stanford.edu/display/OpenSim/How+Inverse+Kinematics+Works
int mirrorBVHThroughIK(
                       struct BVH_MotionCapture * mc,
                       struct BVH_Transform * bvhTransform,
                       unsigned int fID,
                       struct simpleRenderer * renderer,
                       BVHJointID jIDA,
                       BVHJointID jIDB
                      )
{
  fprintf(stderr,"NOT IMPLEMENTED YET..");
  //Todo mirror 2D points in 2D and then perform IK..
  return 0;
}




int bvh_MirrorJointsThroughIK(
                               struct BVH_MotionCapture * mc,
                               const char * jointNameA,
                               const char * jointNameB
                             )
{
  BVHJointID jIDA,jIDB;

  if (
       (!bvh_getJointIDFromJointNameNocase(mc,jointNameA,&jIDA)) ||
       (!bvh_getJointIDFromJointNameNocase(mc,jointNameB,&jIDB))
     )
  {
    fprintf(stderr,"bvh_MirrorJointsThroughIK error resolving joints (%s,%s) \n",jointNameA,jointNameB);fprintf(stderr,"Full list of joints is : \n");
    unsigned int jID=0;
     for (jID=0; jID<mc->jointHierarchySize; jID++)
      {
        fprintf(stderr,"   joint %u = %s\n",jID,mc->jointHierarchy[jID].jointName);
      }
    return 0;
  }


 struct BVH_Transform bvhTransform={0};
 struct simpleRenderer renderer={0};
 simpleRendererDefaults(
                        &renderer,
                        1920, 1080, 582.18394,   582.52915 // https://gopro.com/help/articles/Question_Answer/HERO4-Field-of-View-FOV-Information
                       );
 simpleRendererInitialize(&renderer);

 BVHFrameID fID=0;
 for (fID=0; fID<mc->numberOfFrames; fID++)
         {
            mirrorBVHThroughIK(
                                mc,
                                &bvhTransform,
                                fID,
                                &renderer,
                                jIDA,
                                jIDB
                               );
         }


 return 1;
}
