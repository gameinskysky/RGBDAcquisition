#include "simpleRenderer.h"
#include "matrix4x4Tools.h"
#include "matrixOpenGL.h"
#include <stdio.h>

int simpleRendererRender(
                         struct simpleRenderer * sr ,
                         float * position3D,
                         float * center3D,
                         float * objectRotation,
                         unsigned int rotationOrder,
                         ///---------------
                         float * output2DX,
                         float * output2DY,
                         float * output2DW
                        )
{
 float  modelTransformationF[16];
 ///--------------------------------------------------------------------
 ///                       CAMERA MATRICES ETC
 ///--------------------------------------------------------------------
 create4x4FModelTransformation(
                              modelTransformationF,
                              //Rotation Component
                              (float) sr->cameraOffsetRotation[0],//heading,
                              (float) sr->cameraOffsetRotation[1],//pitch,
                              (float) sr->cameraOffsetRotation[2],//roll,
                              ROTATION_ORDER_RPY,
                              //Translation Component
                              (float) sr->cameraOffsetPosition[0],
                              (float) sr->cameraOffsetPosition[1],
                              (float) sr->cameraOffsetPosition[2],
                              //Scale Component
                              (float) 1.0,
                              (float) 1.0,
                              (float) 1.0
                             );
 ///--------------------------------------------------------------------
 multiplyTwo4x4FMatrices(sr->modelViewMatrix,sr->viewMatrix,modelTransformationF);
 ///--------------------------------------------------------------------





 ///--------------------------------------------------------------------
 ///                       OBJECT MATRICES ETC
 ///--------------------------------------------------------------------
  float objectMatrixRotation[16];

  if (objectRotation==0)
    {
      create4x4FIdentityMatrix(objectMatrixRotation);
    } else
  if ( (objectRotation[0]==0) && (objectRotation[1]==0) && (objectRotation[2]==0) )
    {
      create4x4FIdentityMatrix(objectMatrixRotation);
    } else
  if (rotationOrder==ROTATION_ORDER_RPY)
    {
     //This is the old way to do this rotation
     doRPYTransformationF(
                          objectMatrixRotation,
                          (float) objectRotation[0],
                          (float) objectRotation[1],
                          (float) objectRotation[2]
                         );
    } else
    {
     //fprintf(stderr,"Using new model transform code\n");
     create4x4FMatrixFromEulerAnglesWithRotationOrder(
                                                      objectMatrixRotation ,
                                                      (float) objectRotation[0],
                                                      (float) objectRotation[1],
                                                      (float) objectRotation[2],
                                                      rotationOrder
                                                     );
    }
 ///--------------------------------------------------------------------



   float point3D[4];
   float resultPoint3D[4];
   point3D[0]=(float) (position3D[0]-center3D[0]);
   point3D[1]=(float) (position3D[1]-center3D[1]);
   point3D[2]=(float) (position3D[2]-center3D[2]);
   point3D[3]=(float) (1.0);

   transform3DPointFVectorUsing4x4FMatrix(
                                          resultPoint3D,
                                          objectMatrixRotation,
                                          point3D
                                         );


  float final3DPosition[4];

  if (sr->removeObjectPosition)
   {
    final3DPosition[0]=(float) resultPoint3D[0]+sr->cameraOffsetPosition[0];
    final3DPosition[1]=(float) resultPoint3D[1]+sr->cameraOffsetPosition[1];
    final3DPosition[2]=(float) resultPoint3D[2]+sr->cameraOffsetPosition[2];
   } else
   {
    final3DPosition[0]=(float) resultPoint3D[0]+center3D[0]+sr->cameraOffsetPosition[0];
    final3DPosition[1]=(float) resultPoint3D[1]+center3D[1]+sr->cameraOffsetPosition[1];
    final3DPosition[2]=(float) resultPoint3D[2]+center3D[2]+sr->cameraOffsetPosition[2];
   }
  final3DPosition[3]=(float) 1.0;//resultPoint3D[3];
 ///--------------------------------------------------------------------




 ///--------------------------------------------------------------------
 ///                         FINAL PROJECTION
 ///--------------------------------------------------------------------
  float windowCoordinates[3]={0};

  if (
       !_glhProjectf(
                     final3DPosition,
                     sr->modelViewMatrix,
                     sr->projectionMatrix,
                     sr->viewport,
                     windowCoordinates
                    )
     )
     {
        //If you reach here make sure you have called simpleRendererInitialize
        /*
        print4x4FMatrix("modelViewMatrix",sr->modelMatrix,1);
        print4x4FMatrix("projectionMatrix",sr->projectionMatrix,1);
        fprintf(stderr,"simpleRendererRender: Could not project 3D Point (%0.2f,%0.2f,%0.2f)\n",final3DPosition[0],final3DPosition[1],final3DPosition[2]);
        */
     }
 ///--------------------------------------------------------------------
  *output2DX = windowCoordinates[0];//windowCoordinates[2];
  *output2DY = windowCoordinates[1];//windowCoordinates[2];
  *output2DW = windowCoordinates[2];
  return 1;
}




int simpleRendererRenderUsingPrecalculatedMatrices(
                                                    struct simpleRenderer * sr ,
                                                    float * position3D,
                                                    ///---------------
                                                    float * output2DX,
                                                    float * output2DY,
                                                    float * output2DW
                                                  )
{
 ///--------------------------------------------------------------------
 ///                         FINAL PROJECTION
 ///--------------------------------------------------------------------
  float windowCoordinates[3]={0};

  create4x4FIdentityMatrix(sr->modelViewMatrix);

  if (
       !_glhProjectf(
                     position3D,
                     sr->modelViewMatrix,
                     sr->projectionMatrix,
                     sr->viewport,
                     windowCoordinates
                    )
     )
     {
         //fprintf(stderr,"simpleRendererRenderUsingPrecalculatedMatrices: Could not project 3D Point (%0.2f,%0.2f,%0.2f)\n",position3D[0],position3D[1],position3D[2]);
     }
 ///--------------------------------------------------------------------
  *output2DX = windowCoordinates[0];//windowCoordinates[2];
  *output2DY = windowCoordinates[1];//windowCoordinates[2];
  *output2DW = windowCoordinates[2];
  return 1;
}


int simpleRendererDefaults(
                            struct simpleRenderer * sr,
                            unsigned int width,
                            unsigned int height,
                            float fX,
                            float fY
                            )
{
  sr->width=width;
  sr->height=height;
  sr->fx = fX;
  sr->fy = fY;
  sr->skew = 0.0;
  sr->cx = (float) width/2;
  sr->cy = (float) height/2;
  sr->near = 1.0;
  sr->far = 10000.0;

  //-----------------------------------
  sr->cameraOffsetPosition[0]=0.0;
  sr->cameraOffsetPosition[1]=0.0;
  sr->cameraOffsetPosition[2]=0.0;
  sr->cameraOffsetPosition[3]=0.0;
  //-----------------------------------
  sr->cameraOffsetRotation[0]=0.0;
  sr->cameraOffsetRotation[1]=0.0;
  sr->cameraOffsetRotation[2]=0.0;
  sr->cameraOffsetRotation[3]=0.0;
  //-----------------------------------
  return 1;
}


int simpleRendererInitialize(struct simpleRenderer * sr)
{
  sr->viewport[0]=0;
  sr->viewport[1]=0;
  sr->viewport[2]=sr->width;
  sr->viewport[3]=sr->height;

  buildOpenGLProjectionForIntrinsics(
                                      sr->projectionMatrix ,
                                      sr->viewport ,
                                      sr->fx,
                                      sr->fy,
                                      sr->skew,
                                      sr->cx,
                                      sr->cy,
                                      sr->width,
                                      sr->height,
                                      sr->near,
                                      sr->far
                                     );

   double viewMatrixD[16];
   create4x4DIdentityMatrix(viewMatrixD);

   create4x4DScalingMatrix(viewMatrixD,0.01,0.01,-0.01);
   copy4x4DMatrixTo4x4F(sr->viewMatrix,viewMatrixD);

   //Initialization of matrices not yet used
   create4x4FIdentityMatrix(sr->modelMatrix);
   create4x4FIdentityMatrix(sr->modelViewMatrix);

 return 1;
}








int simpleRendererInitializeFromExplicitConfiguration(struct simpleRenderer * sr)
{
  //We basically have to do nothing to initialize this way since we have an explicit configuration populating our struct
  //This call is made to basically track when we try to use an explicit configuration
  return 1;
}








