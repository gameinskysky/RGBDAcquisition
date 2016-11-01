/** @file model_loader_transform_joints.c
 *  @brief  Recursive node transformations for TRIModels part of
            https://github.com/AmmarkoV/RGBDAcquisition/tree/master/opengl_acquisition_shared_library/opengl_depth_and_color_renderer
 *  @author Ammar Qammaz (AmmarkoV)
 */
#include "model_loader_tri.h"
#include "model_loader_transform_joints.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../../../tools/AmMatrix/matrix4x4Tools.h"

#define NORMAL   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */


static float _triTrans_degrees_to_rad(float degrees)
{
    return degrees * (M_PI /180.0 );
}


static void _triTrans_create4x4MatrixFromEulerAnglesXYZ(float * m ,float eulX, float eulY, float eulZ)
{
    float x = _triTrans_degrees_to_rad(eulX);
    float y = _triTrans_degrees_to_rad(eulY);
    float z = _triTrans_degrees_to_rad(eulZ);

	float cr = cos( x );
	float sr = sin( x );
	float cp = cos( y );
	float sp = sin( y );
	float cy = cos( z );
	float sy = sin( z );

	m[0] = cp*cy ;
	m[1] = cp*sy;
	m[2] = -sp ;
	m[3] = 0; // 4x4


	float srsp = sr*sp;
	float crsp = cr*sp;

	m[4] = srsp*cy-cr*sy ;
	m[5] = srsp*sy+cr*cy ;
	m[6] = sr*cp ;
	m[7] = 0; // 4x4

	m[8] =  crsp*cy+sr*sy ;
	m[9] =  crsp*sy-sr*cy ;
	m[10]= cr*cp ;
    m[11]= 0; // 4x4

    m[12]= 0;
    m[13]= 0;
    m[14]= 0;
    m[15]= 1.0;
}


/// Clamp a value to 0-255
int Clamp(int i)
{
  if (i < 0) return 0;
  if (i > 255) return 255;
  return i;
}


/// h is from 0-360
/// s,v values are 0-1
/// r,g,b values are 0-255
void HsvToRgb(double h, double S, double V, float * r, float * g, float * b)
{
  // ######################################################################
  // T. Nathan Mundhenk
  // mundhenk@usc.edu
  // C/C++ Macro HSV to RGB

  double H = h;
  while (H < 0) { H += 360; };
  while (H >= 360) { H -= 360; };
  double R, G, B;
  if (V <= 0)
    { R = G = B = 0; }
  else if (S <= 0)
  {
    R = G = B = V;
  }
  else
  {
    double hf = H / 60.0;
    int i = (int) hf;
    double f = hf - i;
    double pv = V * (1 - S);
    double qv = V * (1 - S * f);
    double tv = V * (1 - S * (1 - f));
    switch (i)
    {

      // Red is the dominant color

      case 0:
        R = V;
        G = tv;
        B = pv;
        break;

      // Green is the dominant color

      case 1:
        R = qv;
        G = V;
        B = pv;
        break;
      case 2:
        R = pv;
        G = V;
        B = tv;
        break;

      // Blue is the dominant color

      case 3:
        R = pv;
        G = qv;
        B = V;
        break;
      case 4:
        R = tv;
        G = pv;
        B = V;
        break;

      // Red is the dominant color

      case 5:
        R = V;
        G = pv;
        B = qv;
        break;

      // Just in case we overshoot on our math by a little, we put these here. Since its a switch it won't slow us down at all to put these here.

      case 6:
        R = V;
        G = tv;
        B = pv;
        break;
      case -1:
        R = V;
        G = pv;
        B = qv;
        break;

      // The color is not defined, we should throw an error.

      default:
        //LFATAL("i Value error in Pixel conversion, Value is %d", i);
        R = G = B = V; // Just pretend its black/white
        break;
    }
  }
  *r = (float) Clamp((int)(R * 255.0));
  *g = (float) Clamp((int)(G * 255.0));
  *b = (float) Clamp((int)(B * 255.0));
}


void getDistinctColor3F_ForID(unsigned int id,unsigned maxID , float *oR,float *oG,float *oB)
{

  unsigned int sCoef=10;
  unsigned int vCoef=40;



  unsigned int hStep = (unsigned int) 360/maxID;
  unsigned int sStep = (unsigned int) sCoef/maxID;
  unsigned int vStep = (unsigned int) vCoef/maxID;


  // assumes hue [0, 360), saturation [0, 100), lightness [0, 100)
  float h = id * hStep ;
  float s = (100-sCoef) + ( (sStep-sCoef) * sStep );
  float v = (100-vCoef) + ( (vStep-vCoef) * vStep );

  HsvToRgb(h, (float) s/100, (float) v/100 , oR , oG , oB);


  *oR = (float) *oR / 255;
  *oG = (float) *oG / 255;
  *oB = (float) *oB / 255;
}


struct TRI_Bones_Per_Vertex * allocTransformTRIBonesToVertexBoneFormat(struct TRI_Model * in)
{
  struct TRI_Bones_Per_Vertex * out = (struct TRI_Bones_Per_Vertex *) malloc(sizeof(struct TRI_Bones_Per_Vertex));
  if (out==0) { return 0; }

  out->numberOfBones = in->header.numberOfBones;
  out->numberOfVertices = in->header.numberOfVertices;
  out->bonesPerVertex = (struct TRI_Bones_Per_Vertex_Vertice_Item *)   malloc(sizeof(struct TRI_Bones_Per_Vertex_Vertice_Item) *  in->header.numberOfVertices);
  if (out->bonesPerVertex==0) { return 0; }

  memset(out->bonesPerVertex,0,sizeof(struct TRI_Bones_Per_Vertex_Vertice_Item) *  in->header.numberOfVertices);

  unsigned int b=0 , w=0 , outOfSpace=0;
  for (b=0; b<in->header.numberOfBones; b++)
  {
      for (w=0; w<in->bones[b].info->boneWeightsNumber; w++)
      {
         unsigned int boneIndex = in->bones[b].weightIndex[w];
         struct TRI_Bones_Per_Vertex_Vertice_Item * bone =  &out->bonesPerVertex[boneIndex];


          if (bone->bonesOfthisVertex < MAX_BONES_PER_VERTICE)
          {
            bone->indicesOfThisVertex[bone->bonesOfthisVertex] = in->bones[b].weightIndex[w];
            bone->weightsOfThisVertex[bone->bonesOfthisVertex] = in->bones[b].weightValue[w];
            bone->boneIDOfThisVertex[bone->bonesOfthisVertex]  = b;

            ++bone->bonesOfthisVertex;
          } else
          {
            ++outOfSpace;
          }
      }
  }

  if (outOfSpace>0)
  {
    fprintf(stderr,"Vertices are set up to accomodate at most %u bones , %u vertices where too small for our input .. \n",MAX_BONES_PER_VERTICE,outOfSpace);
  }
 return out;
}





void freeTransformTRIBonesToVertexBoneFormat(struct TRI_Bones_Per_Vertex * in)
 {
   if (in!=0)
   {
     if (in->bonesPerVertex!=0)
     {
      free(in->bonesPerVertex);
     }
    free(in);
   }
 }


float * generatePalette(struct TRI_Model * in)
{
  unsigned int maxNumBones = in->header.numberOfBones;
  fprintf(stderr,"generating palette for model with %u bones \n",maxNumBones);
  unsigned int maxNumWeightBones=0;
  unsigned int i=0 , numW =0;
  float * gp = (float*) malloc(sizeof(float)*maxNumBones * 3);
  if (gp!=0)
  {
   for (i=0; i<maxNumBones; i++)
   {
     getDistinctColor3F_ForID(
                              i,
                              maxNumBones,
                              &gp[3*i+0],
                              &gp[3*i+1],
                              &gp[3*i+2]
                             );
   }

/*
   for (i=0; i<maxNumBones; i++)
   {
     if ( in->bones[i].info->boneWeightsNumber > 0 )
     {
         ++maxNumWeightBones;
     }
   }


   for (i=0; i<maxNumBones; i++)
   {
    if ( in->bones[i].info->boneWeightsNumber > 0 )
    {
      getDistinctColor3F_ForID(
                                numW,
                                maxNumWeightBones,
                                &gp[3*numW+0],
                                &gp[3*numW+1],
                                &gp[3*numW+2]
                              );
      ++numW;
    } else
    {
      gp[3*numW+0]=0.0;
      gp[3*numW+1]=0.0;
      gp[3*numW+2]=0.0;
    }
   }
   */

  }

 return gp;
}



void colorCodeBones(struct TRI_Model * in)
{
   if (in->bones==0)
   {
     fprintf(stderr,"No bones to colorcode \n");
     return;
   }


  struct TRI_Bones_Per_Vertex * bpv = allocTransformTRIBonesToVertexBoneFormat(in);
  if (bpv!=0)
  {
   float * gp = generatePalette(in);
   if (gp!=0)
   {
    if (in->colors==0)
    {
      in->header.numberOfColors = in->header.numberOfVertices;
      in->colors = (float*) malloc(sizeof(float) * in->header.numberOfColors );
    }

   if (
        (in->colors!=0) &&
        (in->header.numberOfColors!=0) &&
        (in->header.numberOfVertices!=0)
       )
   {
   unsigned int i=0;
   for (i=0; i<in->header.numberOfVertices; i++)
    {
      struct TRI_Bones_Per_Vertex_Vertice_Item * bone =  &bpv->bonesPerVertex[i];

      float maxWeight = 0.0;
      unsigned int z=0 , b=0;
      for (z=0; z<bone->bonesOfthisVertex; z++)
      {
        if ( bone->weightsOfThisVertex[z]>maxWeight )
            {
              maxWeight=bone->weightsOfThisVertex[z];
              b=z;
            }
      }

      unsigned int indxID=bone->indicesOfThisVertex[b];
      unsigned int boneID=bone->boneIDOfThisVertex[b];
      if (boneID>in->header.numberOfVertices)
      {
         fprintf(stderr,"Error bug detected \n"); boneID=0;
      }

      in->colors[indxID*3+0]=gp[boneID*3+0];
      in->colors[indxID*3+1]=gp[boneID*3+1];
      in->colors[indxID*3+2]=gp[boneID*3+2];
    }
   }
   free(gp);
   }
   freeTransformTRIBonesToVertexBoneFormat(bpv);
  }

}



void transformTRIJoint(
                                 struct TRI_Model * in ,
                                 float * jointData ,
                                 unsigned int jointDataSize ,

                                 unsigned int jointToChange ,
                                 float rotEulerX ,
                                 float rotEulerY ,
                                 float rotEulerZ
                               )
{
  float * mat = &jointData[16*jointToChange];
  _triTrans_create4x4MatrixFromEulerAnglesXYZ(mat,rotEulerX,rotEulerY,rotEulerZ);
}




float * mallocModelTransformJoints(
                                    struct TRI_Model * triModelInput ,
                                    unsigned int * jointDataSizeOutput
                                   )
{
  float * returnMat = (float * ) malloc(sizeof(float) * 16 * triModelInput->header.numberOfBones);
  if (returnMat)
  {
     *jointDataSizeOutput =  triModelInput->header.numberOfBones;
     unsigned int i=0;
     for (i=0; i<(*jointDataSizeOutput); i++)
     {
       float * mat = &returnMat[16*i];
       create4x4IdentityFMatrix(mat);
     }
  }
  return returnMat;
}

void recursiveJointHeirarchyTransformer(
                                         struct TRI_Model * in  ,
                                         int curBone ,
                                         double * parentLocalTransformationUntouched ,
                                         float * jointData , unsigned int jointDataSize ,
                                         unsigned int recursionLevel
                                       )
{
  if (recursionLevel>=in->header.numberOfBones+1)
        { fprintf(stderr,RED "BUG : REACHED RECURSION LIMIT (%u/%u)\n" NORMAL,recursionLevel,in->header.numberOfBones); return; }


   unsigned int i=0;
   double parentLocalTransformation[16] , globalTransformation[16] , currentNodeLocalTransformation[16];
   copy4x4Matrix(parentLocalTransformation,parentLocalTransformationUntouched);
   copy4x4Matrix(currentNodeLocalTransformation,in->bones[curBone].info->localTransformation);


  //These prevent to recalculate nodes where there does not appear to be
  //change..
 if ( in->bones[curBone].info->boneWeightsNumber>0 )
  {
    if (in->bones[curBone].info->altered)
      {
      //print4x4DMatrixTRI("mTransformation was .. \n",in->bones[curBone].info->localTransformation);
      double translation[16] , rotation[16] , scaling[16];
      create4x4IdentityMatrix(translation);
      create4x4IdentityMatrix(rotation);
      create4x4IdentityMatrix(scaling);

      copy4x4FMatrixToD(rotation,&jointData[curBone*16]);

      //Get Translation
      translation[3] =in->bones[curBone].info->localTransformation[3];
      translation[7] =in->bones[curBone].info->localTransformation[7];
      translation[11]=in->bones[curBone].info->localTransformation[11];

      multiplyThree4x4Matrices( currentNodeLocalTransformation, translation,rotation,scaling);
      //print4x4DMatrixTRI("Translation was .. ",translation);
      //print4x4DMatrixTRI("Scaling was .. ",scaling);
      //print4x4DMatrixTRI("Rotation was .. ",rotation);
      //print4x4DMatrixTRI("Node Transformation is now.. \n",currentNodeLocalTransformation);
      }

      multiplyTwo4x4Matrices(globalTransformation,parentLocalTransformation,currentNodeLocalTransformation);
      multiplyThree4x4Matrices(
                                 in->bones[curBone].info->finalVertexTransformation ,
                                 in->header.boneGlobalInverseTransform ,
                                 globalTransformation,
                                 in->bones[curBone].info->matrixThatTransformsFromMeshSpaceToBoneSpaceInBindPose
                              );

     for ( i = 0 ; i < in->bones[curBone].info->numberOfBoneChildren; i++)
      {
        unsigned int curBoneChild=in->bones[curBone].boneChild[i];
        recursiveJointHeirarchyTransformer(
                                           in  ,
                                           curBoneChild ,
                                           globalTransformation ,
                                           jointData , jointDataSize ,
                                           recursionLevel+1
                                         );
      }
    } else
    {
      multiplyTwo4x4Matrices(globalTransformation,parentLocalTransformation,currentNodeLocalTransformation);
      for ( i = 0 ; i < in->bones[curBone].info->numberOfBoneChildren; i++)
       {
        unsigned int curBoneChild=in->bones[curBone].boneChild[i];
        recursiveJointHeirarchyTransformer(
                                           in  ,
                                           curBoneChild ,
                                           globalTransformation ,
                                           jointData , jointDataSize ,
                                           recursionLevel+1
                                         );
       }
    }
}




int applyVertexTransformation( struct TRI_Model * triModelOut , struct TRI_Model * triModelIn )
{
  double transformedPosition[4]={0} ,transformedNormal[4]={0} , position[4]={0} , normal[4]={0};
  unsigned int i,k;
 //We NEED to clear the vertices and normals since they are added uppon , not having
  //the next two lines results in really weird and undebuggable visual behaviour
  memset(triModelOut->vertices, 0, triModelOut->header.numberOfVertices  * sizeof(float));
  memset(triModelOut->normal  , 0, triModelOut->header.numberOfNormals   * sizeof(float));

   for (k=0; k<triModelIn->header.numberOfBones; k++ )
   {
     for (i=0; i<triModelIn->bones[k].info->boneWeightsNumber; i++ )
     {
       //V is the vertice we will be working in this loop
       unsigned int v = triModelIn->bones[k].weightIndex[i];
       //W is the weight that we have for the specific bone
       float w = triModelIn->bones[k].weightValue[i];

       //We load our input into position/normal
       position[0] = triModelIn->vertices[v*3+0];
       position[1] = triModelIn->vertices[v*3+1];
       position[2] = triModelIn->vertices[v*3+2];
       position[3] = 1.0;

       normal[0]   = triModelIn->normal[v*3+0];
       normal[1]   = triModelIn->normal[v*3+1];
       normal[2]   = triModelIn->normal[v*3+2];
       normal[3]   = 1.0;

       //We transform input (initial) position with the transform we computed to get transformedPosition
       transform3DPointVectorUsing4x4Matrix(transformedPosition, triModelIn->bones[k].info->finalVertexTransformation ,position);
	   triModelOut->vertices[v*3+0] += (float) transformedPosition[0] * w;
	   triModelOut->vertices[v*3+1] += (float) transformedPosition[1] * w;
	   triModelOut->vertices[v*3+2] += (float) transformedPosition[2] * w;

       //We transform input (initial) normal with the transform we computed to get transformedNormal
       transform3DPointVectorUsing4x4Matrix(transformedNormal, triModelIn->bones[k].info->finalVertexTransformation ,normal);
	   triModelOut->normal[v*3+0] += (float) transformedNormal[0] * w;
	   triModelOut->normal[v*3+1] += (float) transformedNormal[1] * w;
	   triModelOut->normal[v*3+2] += (float) transformedNormal[2] * w;
     }
   }
 return 1;
}










int doModelTransform(
                      struct TRI_Model * triModelOut ,
                      struct TRI_Model * triModelIn ,
                      float * jointData ,
                      unsigned int jointDataSize ,
                      unsigned int autodetectAlteredMatrices ,
                      unsigned int performVertexTransform
                    )
{
  if (triModelIn==0)
                     { fprintf(stderr,"doModelTransform called without input TRI Model \n"); return 0; }
  if ( ( triModelIn->vertices ==0 ) || ( triModelIn->header.numberOfVertices ==0 ) )
                     { fprintf(stderr,RED "Number of vertices is zero so can't do model transform using weights..\n" NORMAL); return 0; }

 if ( (jointData==0) || (jointDataSize==0) )
 {
   fprintf(stderr,"doModelTransform called without joints to transform , ");
   fprintf(stderr,"so it will be just returning a null transformed copy of");
   fprintf(stderr,"the input mesh , hope this is what you intended..\n");
   return 1;
 }

 if (!autodetectAlteredMatrices)
     {
        fprintf(stderr,"disabled autodetection of altered matrices might result in strange transformations being executed.. \n");
     }

 unsigned int i=0;
 for (i=0; i<triModelIn->header.numberOfBones; i++)
   {
     float * jointI = &jointData[i*16];

     if (autodetectAlteredMatrices)
     {
       if (!is4x4FIdentityMatrix(jointI))  { triModelIn->bones[i].info->altered=1; } else
                                           { triModelIn->bones[i].info->altered=0; }
     } else
     {
       //All matrices considered altered
       triModelIn->bones[i].info->altered=1;
     }
   }

  double initialParentTransform[16]={0};
  create4x4IdentityMatrix(initialParentTransform) ; //Initial "parent" transform is Identity

  //This recursively calculates all matrix transforms and prepares the correct matrices
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   recursiveJointHeirarchyTransformer( triModelIn , triModelIn->header.rootBone  , initialParentTransform , jointData , jointDataSize , 0 /*First call 0 recursion*/ );
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (performVertexTransform)
  {
    //Past checks..
   copyModelTri( triModelOut , triModelIn , 1 /*We also want bone data*/);
   applyVertexTransformation( triModelOut ,  triModelIn );
  }

 return 1;
}
