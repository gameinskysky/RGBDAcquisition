/** @file main.c
 *  @brief  A library that can parse BVH files and perform various processing options as a commandline tool
 *          X86 compilation: gcc -o -L/usr/X11/lib   main main.c
 *          X64 compilation: gcc -o -L/usr/X11/lib64 main main.c
 *  @author Ammar Qammaz (AmmarkoV)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "../../Library/TrajectoryParser/TrajectoryParserDataStructures.h"
#include "../../Library/MotionCaptureLoader/bvh_loader.h"
#include "../../Library/MotionCaptureLoader/calculate/bvh_to_tri_pose.h"

#include "../../Library/MotionCaptureLoader/export/bvh_to_trajectoryParserTRI.h"
#include "../../Library/MotionCaptureLoader/export/bvh_to_trajectoryParserPrimitives.h"
#include "../../Library/MotionCaptureLoader/export/bvh_export.h"
#include "../../Library/MotionCaptureLoader/export/bvh_to_bvh.h"

#include "../../Library/MotionCaptureLoader/edit/bvh_cut_paste.h"
#include "../../Library/MotionCaptureLoader/edit/bvh_randomize.h"
#include "../../Library/MotionCaptureLoader/edit/bvh_filter.h"
#include "../../Library/MotionCaptureLoader/edit/bvh_rename.h"
#include "../../Library/MotionCaptureLoader/edit/bvh_merge.h"
#include "../../Library/MotionCaptureLoader/edit/bvh_interpolate.h"

#include "../../Library/MotionCaptureLoader/ik/bvh_inverseKinematics.h"

#include "../../Library/MotionCaptureLoader/tests/test.h"


#include  "../../../../../tools/AmMatrix/matrix4x4Tools.h"
#include  "../../../../../tools/AmMatrix/matrixOpenGL.h"



#define NORMAL   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */

void prepare4x4Human36MRotationMatrix(float * rotationMatrix,float rX,float rY,float rZ)
{
    float rXM[16],rYM[16],rZM[16];

    //R1x=np.matrix([[1,0,0],    [0,np.cos(Rx),-np.sin(Rx)], [0,np.sin(Rx),np.cos(Rx)] ]) #[1 0 0; 0 cos(obj.Params(1)) -sin(obj.Params(1)); 0 sin(obj.Params(1)) cos(obj.Params(1))]
    rXM[0]=1.0;  rXM[1]=0.0;     rXM[2]=0.0;        rXM[3]=0.0;
    rXM[4]=0.0;  rXM[5]=cos(rX); rXM[6]=-sin(rX);   rXM[7]=0.0;
    rXM[8]=0.0;  rXM[9]=sin(rX); rXM[10]=cos(rX);   rXM[11]=0.0;
    rXM[12]=0.0; rXM[13]=0.0;    rXM[14]=0.0;       rXM[15]=1.0;

    //R1y=np.matrix([[np.cos(Ry),0,np.sin(Ry)], [0,1,0], [-np.sin(Ry),0,np.cos(Ry)]])     #[cos(obj.Params(2)) 0 sin(obj.Params(2)); 0 1 0; -sin(obj.Params(2)) 0 cos(obj.Params(2))]
    rYM[0]=cos(rY);  rYM[1]=0.0;  rYM[2]=sin(rY);   rYM[3]=0.0;
    rYM[4]=0.0;      rYM[5]=1.0;  rYM[6]=0.0;       rYM[7]=0.0;
    rYM[8]=-sin(rY); rYM[9]=0.0;  rYM[10]=cos(rY);  rYM[11]=0.0;
    rYM[12]=0.0;     rYM[13]=0.0; rYM[14]=0.0;      rYM[15]=1.0;

    //R1z=np.matrix([[np.cos(Rz),-np.sin(Rz),0], [np.sin(Rz),np.cos(Rz),0], [0,0,1]])     #[cos(obj.Params(3)) -sin(obj.Params(3)) 0; sin(obj.Params(3)) cos(obj.Params(3)) 0; 0 0 1]
    rZM[0]=cos(rZ); rZM[1]=-sin(rZ); rZM[2]=0.0;    rZM[3]=0.0;
    rZM[4]=sin(rZ); rZM[5]=cos(rZ);  rZM[6]=0.0;    rZM[7]=0.0;
    rZM[8]=0.0;     rZM[9]=0.0;      rZM[10]=1.0;   rZM[11]=0.0;
    rYM[12]=0.0;    rZM[13]=0.0;     rZM[14]=0.0;   rZM[15]=1.0;

    multiplyThree4x4FMatrices(
                              rotationMatrix ,
                              rXM ,
                              rYM,
                              rZM
                            );

}



void haltOnError(unsigned int haltingSwitch,const char * message)
{
  fprintf(stderr,RED "=======================================\n");
  fprintf(stderr,"=======================================\n");
  fprintf(stderr,"Encountered error during procedure %s \n",message);
  fprintf(stderr,"=======================================\n");
  fprintf(stderr,"=======================================\n" NORMAL);

  if (haltingSwitch)
    {
       fprintf(stderr,RED "Halting because of --haltonerror switch\n" NORMAL);
       exit(1);
    }
}


void incorrectArguments()
{
  fprintf(stderr,RED "Incorrect number of arguments.. \n" NORMAL);
  exit(1);
}


int testMultipleLoad(const char * filename)
{
 struct BVH_MotionCapture bvhMotion={0};

 unsigned int done=0;
 unsigned int fileNumber=0;
 ssize_t read;
 FILE * fp = fopen(filename,"r");
    if (fp!=0)
        {
            char * line = NULL;
            size_t len = 0;

            while ( (!done) && ( (read = getline(&line, &len, fp)) != -1) )
                {
                  if (line!=0)
                  {
                    int lineLength = strlen(line);
                    if (lineLength>=1)
                    {
                      if (line[lineLength-1]==10) { line[lineLength-1]=0; }
                      if (line[lineLength-1]==13) { line[lineLength-1]=0; }
                    }
                    if (lineLength>=2)
                    {
                      if (line[lineLength-2]==10) { line[lineLength-2]=0; }
                      if (line[lineLength-2]==13) { line[lineLength-2]=0; }
                    }

                  fprintf(stderr,"Next file is `%s`\n",line);
                  if ( bvh_loadBVH(line, &bvhMotion, 1.0) )
                   {
                      fprintf(stderr,"Loaded file `%s`\n",line);
                      //Change joint names..
                      bvh_renameJointsForCompatibility(&bvhMotion);
                      fprintf(stderr,"Did rename `%s`\n",line);
                      bvh_free(&bvhMotion);
                      fprintf(stderr,"Freed file `%s`\n",line);
                   }
                  }

                  ++fileNumber;
                  if (fileNumber==10) { done=1; }
                }

          if (line!=0) { free(line); }
          fclose(fp);
          return 1;
        }
  return 0;
}



int main(int argc,const char **argv)
{
    unsigned int immediatelyHaltOnError=0;
    //----------------------------------------------
    const char * fromBVHFile="Motions/example.bvh";
    const char * toBVHFile="Motions/bvh.bvh";
    const char * toSceneFile="Scenes/bvh.conf";
    const char * toSceneFileTRI="Scenes/bvhTRI.conf";
    const char * toSVGDirectory="tmp/";
    const char * toCSVFilename="data.csv";
    unsigned int convertToSVG=0;
    unsigned int convertToCSV=0;
    unsigned int useCSV_2D_Output=1,useCSV_3D_Output=1,useCSV_BVH_Output=1;
    unsigned int maxFrames = 0;
    unsigned int occlusions = 0;
    float scaleWorld=1.0;
    //unsigned int flipOrientation = 0;
    //unsigned int flipRandomizationOrientation = 0;


    unsigned int regularOrientation = BVH_ENFORCE_NO_ORIENTATION;
    unsigned int randomizedOrientation =   BVH_ENFORCE_NO_ORIENTATION;
    unsigned int csvOrientation = BVH_ENFORCE_NO_ORIENTATION;
    unsigned int filterBehindCamera=1;
    unsigned int filterIfAnyJointOutsideof2DFrame=1;
    unsigned int filterTopWeirdRandomSkeletons=1;

    struct BVH_MotionCapture bvhMotion={0};

    struct BVH_RendererConfiguration renderingConfiguration={0};

    // Emulate GoPro by default..
    // https://gopro.com/help/articles/Question_Answer/HERO4-Field-of-View-FOV-Information
    renderingConfiguration.width=1920;
    renderingConfiguration.height=1080;
    renderingConfiguration.cX=(float)renderingConfiguration.width/2;
    renderingConfiguration.cY=(float)renderingConfiguration.height/2;
    renderingConfiguration.fX=582.18394;
    renderingConfiguration.fY=582.52915;
    //640,480 , 575.57 , 575.57, //Kinect
    //-------------------------------------------------------------------------------------

    unsigned int i=0;
    for (i=0; i<argc; i++)
    {
        //-----------------------------------------------------
        if (strcmp(argv[i],"--printparams")==0)
        {
           fprintf(stderr,"Utility was called using following parameters :\n");
           unsigned int z=0;
           for (z=0; z<argc; z++)
            {
              fprintf(stderr," %s",argv[z]);
            }
           fprintf(stderr,"\n");
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--debug")==0)
        {
          bvhMotion.debug=1;
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--filtergimballocks")==0)
        {
           if (i+1>=argc)  { incorrectArguments(); }
           filterOutPosesThatAreGimbalLocked(&bvhMotion,atof(argv[i+1]));
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--haltonerror")==0)
        {
          immediatelyHaltOnError=1;
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--occlusions")==0)
        {
          occlusions=1;
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--print")==0)
        {
          bvh_printBVH(&bvhMotion);
        } else
        if (strcmp(argv[i],"--printc")==0)
        {
          bvh_print_C_Header(&bvhMotion);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--test")==0)
        {
          bvh_testConstrainRotations();
          exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--testmultiple")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          testMultipleLoad(argv[i+1]);
          exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--testIK")==0)
        {
          // ./BVHTester --from Motions/05_01.bvh --selectJoints 0 23 hip eye.r eye.l abdomen chest neck head rshoulder relbow rhand lshoulder lelbow lhand rhip rknee rfoot lhip lknee lfoot toe1-2.r toe5-3.r toe1-2.l toe5-3.l --testIK 80 4 130 0.001 5 100

          if (i+6>=argc)  { incorrectArguments(); }

          unsigned int previousFrame=atoi(argv[i+1]);
          unsigned int sourceFrame=atoi(argv[i+2]);
          unsigned int targetFrame=atoi(argv[i+3]);
          float        learningRate = atof(argv[i+4]);
          unsigned int iterations=atoi(argv[i+5]);
          unsigned int epochs=atoi(argv[i+6]);

           bvhTestIK(
                      &bvhMotion,
                      learningRate,
                      iterations,
                      epochs,
                      previousFrame,
                      sourceFrame,
                      targetFrame
                    );

          exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--probefilter")==0)
        {
          //Filter using 2D rules
          //./BVHTester --from Motions/05_01.bvh --probefilter 0 0 -130.0 0 0 0 1920 1080 570.7 570.3 3 rhand lhip 10 12 rhand rhip 5 8 rhand lhand 20 25
          probeForFilterRules(&bvhMotion,argc-i-1,&argv[i+1]);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--filterout")==0)
        {
          //Filter using 2D rules
          //./BVHTester --from Motions/05_01.bvh --filterout 0 0 -130.0 0 0 0 1920 1080 570.7 570.3 3 rhand lhip 140.0 145.0 rhand rhip 65 70 rhand lhand 190 205
          //./BVHTester --printparams --haltonerror --from Motions/05_01.bvh --selectJoints 1 13 hip eye.r eye.l abdomen chest neck head rshoulder relbow rhand lshoulder lelbow lhand --hide2DLocationOfJoints 0 4 abdomen chest eye.r eye.l --perturbJointAngles 4 38.0 rshoulder lshoulder rhip lhip --perturbJointAngles 8 10.0 rhand relbow lelbow lhand lknee rknee lfoot rfoot --repeat 0 --filterout 0 0 -130.0 0 90 0 1920 1080 570.7 570.3 6 rhand lhip 0 120 rhand rhip 0 120 rhand lhand 0 150 lhand rhip 0 120 lhand lhip 0 120 lhand rhand 0 150 --randomize2D 1000 4000 -35 -179.999999 -35 35 180 35 --occlusions --csv ./ upperbody_all.csv 2d+bvh

          if (!filterOutPosesThatAreCloseToRules(&bvhMotion,argc-i-1,&argv[i+1]))
            {
              haltOnError(immediatelyHaltOnError,"Error while filtering out joints..");
            }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--nofilter")==0)
        {
          filterBehindCamera=0;
          filterIfAnyJointOutsideof2DFrame=0;
          filterTopWeirdRandomSkeletons=0;
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--renderingConfiguration")==0)
        {
          if (i+17>=argc)  { incorrectArguments(); }
          //-----------------------------------------
          float rX=atof(argv[i+1]);
          float rY=atof(argv[i+2]);
          float rZ=atof(argv[i+3]);
          renderingConfiguration.T[0]=atof(argv[i+4]);
          renderingConfiguration.T[1]=atof(argv[i+5]);
          renderingConfiguration.T[2]=atof(argv[i+6]);
          renderingConfiguration.fX=atof(argv[i+7]);
          renderingConfiguration.fY=atof(argv[i+8]);
          renderingConfiguration.cX=atof(argv[i+9]);
          renderingConfiguration.cY=atof(argv[i+10]);
          renderingConfiguration.k1=atof(argv[i+11]);
          renderingConfiguration.k2=atof(argv[i+12]);
          renderingConfiguration.k3=atof(argv[i+13]);
          renderingConfiguration.p1=atof(argv[i+14]);
          renderingConfiguration.p2=atof(argv[i+15]);
          unsigned int width=atoi(argv[i+16]);
          unsigned int height=atoi(argv[i+17]);
          //-------------------------------------------------------------------------------
          prepare4x4Human36MRotationMatrix(renderingConfiguration.viewMatrix,rX,rY,rZ);
          //copy3x3FMatrixTo4x4F(renderingConfiguration.viewMatrix,renderingConfiguration.R);

          // 0  1  2  3
          // 4  5  6  7
          // 8  9  10 11
          // 12 13 14 15
          //----------------------------------------------------------------
          renderingConfiguration.viewMatrix[3] =renderingConfiguration.T[0];
          renderingConfiguration.viewMatrix[7] =renderingConfiguration.T[1];
          renderingConfiguration.viewMatrix[11]=renderingConfiguration.T[2];
          //----------------------------------------
          renderingConfiguration.viewport[0]=0;
          renderingConfiguration.viewport[1]=0;
          renderingConfiguration.viewport[2]=width;
          renderingConfiguration.viewport[3]=height;
          renderingConfiguration.width=width;
          renderingConfiguration.height=height;
          //----------------------------------------
          buildOpenGLProjectionForIntrinsics(
                                             renderingConfiguration.projection ,
                                             renderingConfiguration.viewport ,
                                             renderingConfiguration.fX,
                                             renderingConfiguration.fY,
                                             1.0,//sr->skew,
                                             renderingConfiguration.cX,
                                             renderingConfiguration.cY,
                                             width,
                                             height,
                                             1.0, //Near
                                             10000.0 //Far
                                            );
          //----------------------------------------
          renderingConfiguration.isDefined=1;
          exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--changeJointDimensions")==0)
        {
          if (i+4>=argc)  { incorrectArguments(); }
          if (
              !bvh_changeJointDimensions(
                                       &bvhMotion,
                                       argv[i+1],
                                       atof(argv[i+2]),
                                       atof(argv[i+3]),
                                       atof(argv[i+4])
                                       )
             )
          {
              fprintf(stderr,RED "failed to change `%s` joint dimensions\n",argv[i+1]);
              haltOnError(immediatelyHaltOnError,"Error while changing joint dimensions..");
          }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--onlyAnimateGivenJoints")==0)
        {
          unsigned int numberOfArguments=atoi(argv[i+1]);
          bvh_onlyAnimateGivenJoints(&bvhMotion,numberOfArguments,argv+i+2);
          //
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--scale")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          scaleWorld=atof(argv[i+1]);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--scaleOffsets")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          float scaleRatio = atof(argv[i+1]);
          fprintf(stderr,"Offset scaling ratio = %0.2f \n",scaleRatio);
          bvh_scaleAllOffsets(
                              &bvhMotion,
                              scaleRatio
                              );
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--onlyFirstFrame")==0)
        {
          bvh_copyMotionFrame(&bvhMotion, 0, 1 );
          bvhMotion.numberOfFrames=2; //Just Render one frame..
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--maxFrames")==0)
        {
          if (i+1>=argc)  { incorrectArguments();}
          maxFrames=atoi(argv[i+1]);
          //We can limit the number of frames
          if (maxFrames!=0)
            {
             //Only reducing number of frames
             if (bvhMotion.numberOfFrames>maxFrames)
               {
                 bvhMotion.numberOfFrames = maxFrames;
               }
            }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--from")==0)
        {
          if (i+1>=argc)  { incorrectArguments();}
          fromBVHFile=argv[i+1];
          //First of all we need to load the BVH file
          if (!bvh_loadBVH(fromBVHFile, &bvhMotion, scaleWorld))
          {
            haltOnError(immediatelyHaltOnError,"Error loading bvh file..");
          }

          //Change joint names..
          bvh_renameJointsForCompatibility(&bvhMotion);
          bvh_ConstrainRotations(&bvhMotion,regularOrientation);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--merge")==0)
        {
          if (i+2>=argc)  { incorrectArguments();}
          const char * BVHPathToFileToMerge = argv[i+1];
          const char * pathToMergeRules = argv[i+2];

          struct BVH_MotionCapture bvhMotionToMerge={0};
          if ( bvh_loadBVH(BVHPathToFileToMerge, &bvhMotionToMerge, scaleWorld) )
          {
            bvh_renameJointsForCompatibility(&bvhMotionToMerge);
            if (
                !bvh_mergeWith(
                               &bvhMotion,
                               &bvhMotionToMerge,
                               pathToMergeRules
                              )
               )
               {
                 fprintf(stderr,"Failed to merge files (%s and %s)..\n",fromBVHFile,BVHPathToFileToMerge);
               }
          } else
          {
            fprintf(stderr,"Could not open BVH file that was requested to be merged (%s)..\n",BVHPathToFileToMerge);
          }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--swap")==0)
        {
          if (i+2>=argc)  { incorrectArguments(); }
          bvh_GrowMocapFileBySwappingJointAndItsChildren(
                                                          &bvhMotion,
                                                          argv[i+1],
                                                          argv[i+2],
                                                          0
                                                         );
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--360")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }

          bvh_GrowMocapFileByGeneratingPoseFromAllViewingAngles(
                                                                &bvhMotion,
                                                                atoi(argv[i+1])
                                                               );
        } else
        //-----------------------------------------------------
        //     This does not work..
        //if (strcmp(argv[i],"--mirror")==0)
        //{
        //  if (i+2>=argc)  { incorrectArguments(); }
        //  bvh_MirrorJointsThroughIK(
        //                            &bvhMotion,
        //                            argv[i+1],
        //                            argv[i+2]
        //                          );
        //} else
        //-----------------------------------------------------

        //-----------------------------------------------------
        if (strcmp(argv[i],"--interpolate")==0)
        {
          bvh_InterpolateMotion(
                                &bvhMotion
                               );
        } else
        //-----------------------------------------------------

        //-----------------------------------------------------
        if (strcmp(argv[i],"--repeat")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          bvh_GrowMocapFileByCopyingExistingMotions(
                                                     &bvhMotion,
                                                     atoi(argv[i+1])
                                                   );
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--setPositionRotation")==0)
        {
          if (i+6>=argc)  { incorrectArguments(); }
          float cameraPositionOffset[3];
          float cameraRotationOffset[3];

          cameraPositionOffset[0]=-1*atof(argv[i+1])/10;
          cameraPositionOffset[1]=-1*atof(argv[i+2])/10;
          cameraPositionOffset[2]=-1*atof(argv[i+3])/10;
          cameraRotationOffset[0]=atof(argv[i+4]);
          cameraRotationOffset[1]=atof(argv[i+5]);
          cameraRotationOffset[2]=atof(argv[i+6]);
          bvh_SetPositionRotation(
                                  &bvhMotion,
                                  cameraPositionOffset,
                                  cameraRotationOffset
                                 );

          bvh_ConstrainRotations(&bvhMotion,regularOrientation);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--offsetPositionRotation")==0)
        {
          if (i+6>=argc)  { incorrectArguments(); }
          float cameraPositionOffset[3];
          float cameraRotationOffset[3];

          cameraPositionOffset[0]=-1*atof(argv[i+1])/10;
          cameraPositionOffset[1]=-1*atof(argv[i+2])/10;
          cameraPositionOffset[2]=-1*atof(argv[i+3])/10;
          cameraRotationOffset[0]=atof(argv[i+4]);
          cameraRotationOffset[1]=atof(argv[i+5]);
          cameraRotationOffset[2]=atof(argv[i+6]);
          bvh_OffsetPositionRotation(
                                     &bvhMotion,
                                     cameraPositionOffset,
                                     cameraRotationOffset
                                    );

          bvh_ConstrainRotations(&bvhMotion,regularOrientation);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--randomizedOrientation")==0)
        {
             //flipRandomizationOrientation=1;
             if (strcmp("front",argv[i+1])==0)   {  randomizedOrientation=BVH_ENFORCE_FRONT_ORIENTATION;    } else
             if (strcmp("back",argv[i+1])==0)    {  randomizedOrientation=BVH_ENFORCE_BACK_ORIENTATION;     } else
             if (strcmp("left",argv[i+1])==0)    {  randomizedOrientation=BVH_ENFORCE_LEFT_ORIENTATION;     } else
             if (strcmp("right",argv[i+1])==0)   {  randomizedOrientation=BVH_ENFORCE_RIGHT_ORIENTATION;    } else
                                                 {  haltOnError(immediatelyHaltOnError,"Randomized Orientation offered is wrong "); }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--csvOrientation")==0)
        {
             //flipRandomizationOrientation=1;
             if (strcmp("front",argv[i+1])==0)   {  csvOrientation=BVH_ENFORCE_FRONT_ORIENTATION;    } else
             if (strcmp("back",argv[i+1])==0)    {  csvOrientation=BVH_ENFORCE_BACK_ORIENTATION;     } else
             if (strcmp("left",argv[i+1])==0)    {  csvOrientation=BVH_ENFORCE_LEFT_ORIENTATION;     } else
             if (strcmp("right",argv[i+1])==0)   {  csvOrientation=BVH_ENFORCE_RIGHT_ORIENTATION;    } else
                                                 {  haltOnError(immediatelyHaltOnError,"CSV Orientation offered is wrong "); }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--perturbJointAngles")==0)
        {
          if (i+2>=argc)  { incorrectArguments(); }
          unsigned int numberOfValues=atoi(argv[i+1]);
          float deviation=atof(argv[i+2]);
          srand(time(NULL));
          if (i+2+numberOfValues>=argc)  { incorrectArguments(); } else
            {

              if (
                   !bvh_PerturbJointAngles(
                                     &bvhMotion,
                                     numberOfValues,
                                     deviation,
                                     argv,
                                     i+2
                                    )
                 )  { haltOnError(immediatelyHaltOnError,"Error while perturbing joint angles"); }
            }
            //exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--randomizeJointAngles")==0)
        {
          // ./BVHTester --from Motions/DAZFriendlyCMUPlusHeadAndHandsAndFeet.bvh --repeat 100 --randomizeJointAngles 15 -65 0 1 finger5-1.l finger5-2.l finger5-3.l finger4-1.l finger4-2.l finger4-3.l finger3-1.l finger3-2.l finger3-3.l finger2-1.l finger2-2.l finger2-3.l finger1-1.l finger1-2.l finger1-3.l --randomizeJointAngles 5 -90 0 1 finger5-1.l finger4-1.l finger3-1.l finger2-1.l finger1-1.l --randomizeJointAngles 2 -75 75 2 finger1-1.l finger1-2.l --randomizeJointAngles 1 -45 0 3 finger1-1.l --bvh restR.bvh
          // ./BVHTester --from Motions/DAZFriendlyCMUPlusHeadAndHandsAndFeet.bvh --repeat 100 --randomizeJointAngles 15 0 65 1 finger5-1.r finger5-2.r finger5-3.r finger4-1.r finger4-2.r finger4-3.r finger3-1.r finger3-2.r finger3-3.r finger2-1.r finger2-2.r finger2-3.r finger1-1.r finger1-2.r finger1-3.r --randomizeJointAngles 5 0 90 1 finger5-1.r finger4-1.r finger3-1.r finger2-1.r finger1-1.r --randomizeJointAngles 2 -75 75 2 finger1-1.r finger1-2.r --randomizeJointAngles 1 0 45 3 finger1-1.r --bvh restR.bvh
          if (i+2>=argc)  { incorrectArguments(); }
          unsigned int numberOfValues=atoi(argv[i+1]);
          float startOfRandomization=atof(argv[i+2]);
          float endOfRandomization=atof(argv[i+3]);
          unsigned int specificChannelRandomization=atoi(argv[i+4]);
          srand(time(NULL));
          if (i+2+numberOfValues>=argc)  { incorrectArguments(); } else
            {
              if (
                   !bvh_PerturbJointAnglesRange(
                                                &bvhMotion,
                                                numberOfValues,
                                                startOfRandomization,
                                                endOfRandomization,
                                                specificChannelRandomization,
                                                argv,
                                                i+4
                                               )
                 )  { haltOnError(immediatelyHaltOnError,"Error while randomizing joint angles"); }
            }
            //exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--selectJoints")==0)
        {
          if (i+2>=argc)  { incorrectArguments(); }
          unsigned int includeEndJoints=atoi(argv[i+1]);
          unsigned int numberOfValues=atoi(argv[i+2]);
          if (i+2+numberOfValues>=argc)  { incorrectArguments(); } else
            {
              if (
                   !bvh_selectJoints(
                                     &bvhMotion,
                                     numberOfValues,
                                     includeEndJoints,//include End Joints
                                     argv,
                                     i+2
                                    )
                 ) { haltOnError(immediatelyHaltOnError,"Error while selecting Joints"); }
            }
            //exit(0);
        } else
        //-----------------------------------------------------
        //-----------------------------------------------------
        if (strcmp(argv[i],"--hide2DLocationOfJoints")==0)
        {
          if (i+2>=argc)  { incorrectArguments(); }
          unsigned int includeEndJoints=atoi(argv[i+1]);
          unsigned int numberOfValues=atoi(argv[i+2]);
          if (i+2+numberOfValues>=argc)  { incorrectArguments(); } else
            {
               if (
                   !bvh_selectJointsToHide2D(
                                             &bvhMotion,
                                             numberOfValues,
                                             includeEndJoints,
                                             argv,
                                             i+2
                                            )
                 ) { haltOnError(immediatelyHaltOnError,"Error while selecting 2D Joints"); }
            }
            //exit(0);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--eraseJoints")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          unsigned int numberOfValues=atoi(argv[i+1]);
          if (i+1+numberOfValues>=argc)  { incorrectArguments(); } else
            {
              if (
                   !bvh_eraseJoints(
                              &bvhMotion,
                              numberOfValues,
                              1,//include End Joints
                              argv,
                              i+1
                             )
                 ) { haltOnError(immediatelyHaltOnError,"Error while selecting joints to erase"); }
            }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--testRandomizationLimits")==0)
        {
          // ./BVHTester --from Motions/02_03.bvh --testRandomizationLimits -1400 -300 1000 1400 300 5000 --svg tmp/
          if (i+6>=argc)  { incorrectArguments(); }
          float minimumPositionTest[3];
          float maximumPositionTest[3];
          //----
          minimumPositionTest[0]=-1*atof(argv[i+1])/10;
          minimumPositionTest[1]=-1*atof(argv[i+2])/10;
          minimumPositionTest[2]=-1*atof(argv[i+3])/10;
          //----
          maximumPositionTest[0]=-1*atof(argv[i+4])/10;
          maximumPositionTest[1]=-1*atof(argv[i+5])/10;
          maximumPositionTest[2]=-1*atof(argv[i+6])/10;

          bvh_TestRandomizationLimitsXYZ(
                                         &bvhMotion,
                                         minimumPositionTest,
                                         maximumPositionTest
                                        );

          bvh_ConstrainRotations(&bvhMotion,randomizedOrientation);
        } else
         //-----------------------------------------------------
        if (strcmp(argv[i],"--randomize2D")==0)
        {
          // ./BVHTester --from Motions/02_03.bvh --randomize2D 1400 5000 -35 -90 -35 35 90 35 --occlusions --svg tmp/
          if (i+8>=argc)  { incorrectArguments(); }
          float minimumRotation[3];
          float maximumRotation[3];

          float minimumDepth=-1*atof(argv[i+1])/10;
          float maximumDepth=-1*atof(argv[i+2])/10;
          //----
          minimumRotation[0]=atof(argv[i+3]);
          minimumRotation[1]=atof(argv[i+4]);
          minimumRotation[2]=atof(argv[i+5]);
          //----
          maximumRotation[0]=atof(argv[i+6]);
          maximumRotation[1]=atof(argv[i+7]);
          maximumRotation[2]=atof(argv[i+8]);
          //----

          bvh_RandomizePositionFrom2D(
                                      &bvhMotion,
                                      minimumRotation,
                                      maximumRotation,
                                      minimumDepth,
                                      maximumDepth,
                                      renderingConfiguration.fX,
                                      renderingConfiguration.fY,
                                      renderingConfiguration.cX,
                                      renderingConfiguration.cY,
                                      renderingConfiguration.width,
                                      renderingConfiguration.height
                                     );

          bvh_ConstrainRotations(&bvhMotion,randomizedOrientation);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--randomize")==0)
        {
          if (i+12>=argc)  { incorrectArguments(); }
          srand(time(NULL));

          float minimumPosition[3];
          float minimumRotation[3];
          float maximumPosition[3];
          float maximumRotation[3];

          //----
          minimumPosition[0]=-1*atof(argv[i+1])/10;
          minimumPosition[1]=-1*atof(argv[i+2])/10;
          minimumPosition[2]=-1*atof(argv[i+3])/10;
          //----
          minimumRotation[0]=atof(argv[i+4]);
          minimumRotation[1]=atof(argv[i+5]);
          minimumRotation[2]=atof(argv[i+6]);
          //----
          maximumPosition[0]=-1*atof(argv[i+7])/10;
          maximumPosition[1]=-1*atof(argv[i+8])/10;
          maximumPosition[2]=-1*atof(argv[i+9])/10;
          //----
          maximumRotation[0]=atof(argv[i+10]);
          maximumRotation[1]=atof(argv[i+11]);
          maximumRotation[2]=atof(argv[i+12]);


          bvh_RandomizePositionRotation(
                                         &bvhMotion,
                                         minimumPosition,
                                         minimumRotation,
                                         maximumPosition,
                                         maximumRotation
                                       );

          bvh_ConstrainRotations(&bvhMotion,randomizedOrientation);
        } else
         //-----------------------------------------------------
        if (strcmp(argv[i],"--randomize2Dranges")==0)
        {
          // ./BVHTester --from Motions/02_03.bvh --randomize2Dranges 1400 5000 -35 -179.999999 -35 35 -90 35 -35 90 -35 35 180 35 --occlusions --svg tmp/
          if (i+14>=argc)  { incorrectArguments(); }
          float minimumRotationRangeA[3];
          float maximumRotationRangeA[3];
          float minimumRotationRangeB[3];
          float maximumRotationRangeB[3];

          float minimumDepth=-1*atof(argv[i+1])/10;
          float maximumDepth=-1*atof(argv[i+2])/10;
          //----
          minimumRotationRangeA[0]=atof(argv[i+3]);
          minimumRotationRangeA[1]=atof(argv[i+4]);
          minimumRotationRangeA[2]=atof(argv[i+5]);
          //----
          maximumRotationRangeA[0]=atof(argv[i+6]);
          maximumRotationRangeA[1]=atof(argv[i+7]);
          maximumRotationRangeA[2]=atof(argv[i+8]);
          //----
          minimumRotationRangeB[0]=atof(argv[i+9]);
          minimumRotationRangeB[1]=atof(argv[i+10]);
          minimumRotationRangeB[2]=atof(argv[i+11]);
          //----
          maximumRotationRangeB[0]=atof(argv[i+12]);
          maximumRotationRangeB[1]=atof(argv[i+13]);
          maximumRotationRangeB[2]=atof(argv[i+14]);
          //----

          bvh_RandomizePositionFrom2DRotation2Ranges(
                                      &bvhMotion,
                                      minimumRotationRangeA,
                                      maximumRotationRangeA,
                                      minimumRotationRangeB,
                                      maximumRotationRangeB,
                                      minimumDepth,
                                      maximumDepth,
                                      renderingConfiguration.fX,
                                      renderingConfiguration.fY,
                                      renderingConfiguration.cX,
                                      renderingConfiguration.cY,
                                      renderingConfiguration.width,
                                      renderingConfiguration.height
                                     );

          bvh_ConstrainRotations(&bvhMotion,randomizedOrientation);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--randomizeranges")==0)
        {
          if (i+24>=argc)  { incorrectArguments(); }
          srand(time(NULL));

          float minimumPositionRangeA[3];
          float minimumRotationRangeA[3];
          float maximumPositionRangeA[3];
          float maximumRotationRangeA[3];

          float minimumPositionRangeB[3];
          float minimumRotationRangeB[3];
          float maximumPositionRangeB[3];
          float maximumRotationRangeB[3];

          //----
          minimumPositionRangeA[0]=-1*atof(argv[i+1])/10;
          minimumPositionRangeA[1]=-1*atof(argv[i+2])/10;
          minimumPositionRangeA[2]=-1*atof(argv[i+3])/10;
          //----
          minimumRotationRangeA[0]=atof(argv[i+4]);
          minimumRotationRangeA[1]=atof(argv[i+5]);
          minimumRotationRangeA[2]=atof(argv[i+6]);
          //----
          maximumPositionRangeA[0]=-1*atof(argv[i+7])/10;
          maximumPositionRangeA[1]=-1*atof(argv[i+8])/10;
          maximumPositionRangeA[2]=-1*atof(argv[i+9])/10;
          //----
          maximumRotationRangeA[0]=atof(argv[i+10]);
          maximumRotationRangeA[1]=atof(argv[i+11]);
          maximumRotationRangeA[2]=atof(argv[i+12]);

          //----
          minimumPositionRangeB[0]=-1*atof(argv[i+13])/10;
          minimumPositionRangeB[1]=-1*atof(argv[i+14])/10;
          minimumPositionRangeB[2]=-1*atof(argv[i+15])/10;
          //----
          minimumRotationRangeB[0]=atof(argv[i+16]);
          minimumRotationRangeB[1]=atof(argv[i+17]);
          minimumRotationRangeB[2]=atof(argv[i+18]);
          //----
          maximumPositionRangeB[0]=-1*atof(argv[i+19])/10;
          maximumPositionRangeB[1]=-1*atof(argv[i+20])/10;
          maximumPositionRangeB[2]=-1*atof(argv[i+21])/10;
          //----
          maximumRotationRangeB[0]=atof(argv[i+22]);
          maximumRotationRangeB[1]=atof(argv[i+23]);
          maximumRotationRangeB[2]=atof(argv[i+24]);

          bvh_RandomizePositionRotation2Ranges(
                                                &bvhMotion,
                                                minimumPositionRangeA,
                                                minimumRotationRangeA,
                                                maximumPositionRangeA,
                                                maximumRotationRangeA,
                                                minimumPositionRangeB,
                                                minimumRotationRangeB,
                                                maximumPositionRangeB,
                                                maximumRotationRangeB
                                              );

          bvh_ConstrainRotations(&bvhMotion,randomizedOrientation);
        }  else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--to")==0)
        {
          //./BVHTester --from Motions/02_03.bvh --to Motions/cmuTomakehuman.profile test.conf
          if (i+2>=argc)  { incorrectArguments(); }
          const char * retargetProfile=argv[i+1];//"Motions/cmu.profile";
          toSceneFile=argv[i+2];
          //toSceneFileTRI

           struct bvhToTRI bvhtri={0};
           bvh_loadBVHToTRIAssociationFile(retargetProfile,&bvhtri,&bvhMotion);
           dumpBVHToTrajectoryParserTRI(toSceneFileTRI,&bvhMotion,&bvhtri,1/*USE Irugubak oisutuib*/,0);
           dumpBVHToTrajectoryParserPrimitives(toSceneFile,&bvhMotion);
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--bvh")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          toBVHFile=argv[i+1];
          if (
               !dumpBVHToBVH(
                             toBVHFile,
                             &bvhMotion
                            )
             ) { haltOnError(immediatelyHaltOnError,"Error while outputing a BVH file.."); }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--csv")==0)
        {
          if (i+3>=argc)  { incorrectArguments(); }
          toSVGDirectory=argv[i+1];
          toCSVFilename=argv[i+2];
          convertToCSV=1;
          if (strcmp(argv[i+3],"2d+bvh")==0 ) { useCSV_2D_Output=1; useCSV_3D_Output=0; useCSV_BVH_Output=1; } else
          if (strcmp(argv[i+3],"2d")==0 )     { useCSV_2D_Output=1; useCSV_3D_Output=0; useCSV_BVH_Output=0; } else
          if (strcmp(argv[i+3],"3d")==0 )     { useCSV_2D_Output=0; useCSV_3D_Output=1; useCSV_BVH_Output=0; } else
          if (strcmp(argv[i+3],"bvh")==0 )    { useCSV_2D_Output=0; useCSV_3D_Output=0; useCSV_BVH_Output=1; } else
                                              { useCSV_2D_Output=1; useCSV_3D_Output=1; useCSV_BVH_Output=1; }
        } else
        //-----------------------------------------------------
        if (strcmp(argv[i],"--svg")==0)
        {
          if (i+1>=argc)  { incorrectArguments(); }
          toSVGDirectory=argv[i+1];

          char removeOldSVGFilesCommand[512];
          snprintf(removeOldSVGFilesCommand,512,"rm %s/*.svg",toSVGDirectory);
          int res = system(removeOldSVGFilesCommand);
          if (res!=0) { fprintf(stderr,"Could not clean svg files in %s",toSVGDirectory); }
          convertToSVG=1;
        }
    }

    //SVG or CSV output ..
    if ( (convertToSVG) || (convertToCSV) )
    {
     struct filteringResults filterStats={0};

     dumpBVHToSVGCSV(
                     toSVGDirectory,
                     toCSVFilename,
                     convertToSVG,
                     convertToCSV,useCSV_2D_Output,useCSV_3D_Output,useCSV_BVH_Output,
                     &bvhMotion,
                     csvOrientation,
                     &renderingConfiguration,
                     &filterStats,
                     occlusions,
                     filterBehindCamera,//Filter out all poses where even one joint is behind camera
                     filterIfAnyJointOutsideof2DFrame,//Filter out all poses where even one joint is outside of 2D frame
                     filterTopWeirdRandomSkeletons,//Filter top left weird random skelingtons ( skeletons )
                     0//We don't want to convert to radians
                 );
    }


    bvh_free(&bvhMotion);

    return 0;
}
