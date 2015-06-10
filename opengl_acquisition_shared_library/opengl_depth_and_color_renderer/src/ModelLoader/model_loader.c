
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>    /* this includes the necessary X headers */

#include "model_loader.h"
#include "model_loader_obj.h"
#include "../tools.h"

#define DISABLE_GL_CALL_LIST 1
#if DISABLE_GL_CALL_LIST
 #warning "Please note that glCallList is disabled and that has a really bad effect on graphics card performance"
#endif // DISABLE_GL_CALL_LIST


#define PIE 3.14159265358979323846
#define degreeToRadOLD(deg) (deg)*(PIE/180)

const GLfloat defaultAmbient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat defaultDiffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat defaultSpecular[]   = { 0.1f, 0.1f, 0.1f, 1.0f };
const GLfloat defaultShininess[] = { 5.0f };



int drawAxis(float x, float y , float z, float scale)
{
 glLineWidth(6.0);
 glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0); glVertex3f(x,y,z); glVertex3f(x+scale,y,z);
  glColor3f(0.0,1.0,0.0); glVertex3f(x,y,z); glVertex3f(x,y+scale,z);
  glColor3f(0.0,0.0,1.0); glVertex3f(x,y,z); glVertex3f(x,y,z+scale);
 glEnd();
 glLineWidth(1.0);
 return 1;
}

int drawObjPlane(float x,float y,float z,float dimension)
{
   // glNewList(1, GL_COMPILE_AND_EXECUTE);
    /* front face */
    glBegin(GL_QUADS);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(x-dimension, 0.0, z-dimension);
      glVertex3f(x+dimension, 0.0, z-dimension);
      glVertex3f(x+dimension, 0.0, z+dimension);
      glVertex3f(x-dimension, 0.0, z+dimension);
    glEnd();
    //glEndList();
    return 1;
}



int drawGridPlane(float x,float y,float z , float scale)
{
 glBegin(GL_LINES);
 glNormal3f(0.0,1.0,0.0);

  signed int i;

 float floorWidth = 500;
 for(i=-floorWidth; i<=floorWidth; i++)
 {
    glVertex3f(x+-floorWidth*scale,y,z+i*scale);
    glVertex3f(x+floorWidth*scale,y,z+i*scale);

    glVertex3f(x+i*scale,y,z-floorWidth*scale);
    glVertex3f(x+i*scale,y,z+floorWidth*scale);
 };
glEnd();
 return 1;
}

void drawSphere()
{
//    double r=1.0;
    int lats=100;
    int longs=100;
  //---------------
    int i, j;
    for(i = 0; i <= lats; i++) {
       double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
       double z0  = sin(lat0);
       double zr0 =  cos(lat0);

       double lat1 = M_PI * (-0.5 + (double) i / lats);
       double z1 = sin(lat1);
       double zr1 = cos(lat1);

       glBegin(GL_QUAD_STRIP);
       for(j = 0; j <= longs; j++) {
           double lng = 2 * M_PI * (double) (j - 1) / longs;
           double x = cos(lng);
           double y = sin(lng);

           glNormal3f(x * zr0, y * zr0, z0);
           glVertex3f(x * zr0, y * zr0, z0);
           glNormal3f(x * zr1, y * zr1, z1);
           glVertex3f(x * zr1, y * zr1, z1);
       }
       glEnd();
   }
}



int drawCube()
{
   // glNewList(1, GL_COMPILE_AND_EXECUTE);
    /* front face */
    glBegin(GL_QUADS);
      //glColor3f(0.0, 0.7, 0.1);  /* green */
      glVertex3f(-1.0, 1.0, 1.0);
      glVertex3f(1.0, 1.0, 1.0);
      glVertex3f(1.0, -1.0, 1.0);
      glVertex3f(-1.0, -1.0, 1.0);

      /* back face */
      //glColor3f(0.9, 1.0, 0.0);  /* yellow */
      glVertex3f(-1.0, 1.0, -1.0);
      glVertex3f(1.0, 1.0, -1.0);
      glVertex3f(1.0, -1.0, -1.0);
      glVertex3f(-1.0, -1.0, -1.0);

      /* top side face */
      //glColor3f(0.2, 0.2, 1.0);  /* blue */
      glVertex3f(-1.0, 1.0, 1.0);
      glVertex3f(1.0, 1.0, 1.0);
      glVertex3f(1.0, 1.0, -1.0);
      glVertex3f(-1.0, 1.0, -1.0);

      /* bottom side face */
      //glColor3f(0.7, 0.0, 0.1);  /* red */
      glVertex3f(-1.0, -1.0, 1.0);
      glVertex3f(1.0, -1.0, 1.0);
      glVertex3f(1.0, -1.0, -1.0);
      glVertex3f(-1.0, -1.0, -1.0);
    glEnd();
   // glEndList();
    return 1;
}


void drawPyramid()
{
  // draw a pyramid (in smooth coloring mode)
  glBegin(GL_POLYGON);				// start drawing a pyramid

  // front face of pyramid
  //glColor3f(1.0f,0.0f,0.0f);			// Set The Color To Red
  glVertex3f(0.0f, 1.0f, 0.0f);		        // Top of triangle (front)
  //glColor3f(0.0f,1.0f,0.0f);			// Set The Color To Green
  glVertex3f(-1.0f,-1.0f, 1.0f);		// left of triangle (front)
  //glColor3f(0.0f,0.0f,1.0f);			// Set The Color To Blue
  glVertex3f(1.0f,-1.0f, 1.0f);		        // right of traingle (front)

  // right face of pyramid
  //glColor3f(1.0f,0.0f,0.0f);			// Red
  glVertex3f( 0.0f, 1.0f, 0.0f);		// Top Of Triangle (Right)
  //glColor3f(0.0f,0.0f,1.0f);			// Blue
  glVertex3f( 1.0f,-1.0f, 1.0f);		// Left Of Triangle (Right)
  //glColor3f(0.0f,1.0f,0.0f);			// Green
  glVertex3f( 1.0f,-1.0f, -1.0f);		// Right Of Triangle (Right)

  // back face of pyramid
  //glColor3f(1.0f,0.0f,0.0f);			// Red
  glVertex3f( 0.0f, 1.0f, 0.0f);		// Top Of Triangle (Back)
  //glColor3f(0.0f,1.0f,0.0f);			// Green
  glVertex3f( 1.0f,-1.0f, -1.0f);		// Left Of Triangle (Back)
  //glColor3f(0.0f,0.0f,1.0f);			// Blue
  glVertex3f(-1.0f,-1.0f, -1.0f);		// Right Of Triangle (Back)

  // left face of pyramid.
  //glColor3f(1.0f,0.0f,0.0f);			// Red
  glVertex3f( 0.0f, 1.0f, 0.0f);		// Top Of Triangle (Left)
  //glColor3f(0.0f,0.0f,1.0f);			// Blue
  glVertex3f(-1.0f,-1.0f,-1.0f);		// Left Of Triangle (Left)
  //glColor3f(0.0f,1.0f,0.0f);			// Green
  glVertex3f(-1.0f,-1.0f, 1.0f);		// Right Of Triangle (Left)

  glEnd();					// Done Drawing The Pyramid
}


struct Model * loadModel(char * directory,char * modelname)
{
  if ( (directory==0) || (modelname==0) )
  {
    fprintf(stderr,"loadModel failing , no modelname given");
    return 0;
  }

  struct Model * mod = ( struct Model * ) malloc(sizeof(struct Model));
  if ( mod == 0 )  { fprintf(stderr,"Could not allocate enough space for model %s \n",modelname);  return 0; }
  memset(mod , 0 , sizeof(struct Model));

  if ( strcmp(modelname,"plane") == 0 )   {  mod->type = OBJ_PLANE;     mod->model = 0; }  else
  if ( strcmp(modelname,"grid") == 0 )    {  mod->type = OBJ_GRIDPLANE; mod->model = 0; }  else
  if ( strcmp(modelname,"cube") == 0 )    {  mod->type = OBJ_CUBE;      mod->model = 0; }  else
  if ( strcmp(modelname,"pyramid") == 0 ) {  mod->type = OBJ_PYRAMID;   mod->model = 0; }  else
  if ( strcmp(modelname,"axis") == 0 )    {  mod->type = OBJ_AXIS;      mod->model = 0; }  else
  if ( strcmp(modelname,"sphere") == 0 )  {  mod->type = OBJ_SPHERE;    mod->model = 0; }  else

  if ( strstr(modelname,".obj") != 0 )
    {
      mod->type = OBJMODEL;
      mod->model = (struct  OBJ_Model * ) loadObj(directory,modelname);
      if (mod->model ==0 ) { free(mod); return 0 ;}
    } else
    {
      fprintf(stderr,"Could not understand how to load object %s \n",modelname);
      fprintf(stderr,"Searched in directory %s \n",directory);
      fprintf(stderr,"Object %s was also not one of the hardcoded shapes\n",modelname);
      if (mod->model ==0 ) { free(mod); return 0 ;}
    }

  return mod;
}

void unloadModel(struct Model * mod)
{
   if (mod == 0 ) { return ; }

    switch ( mod->type )
    {
      case OBJMODEL :
          unloadObj( (struct  OBJ_Model * ) mod->model);
      break;
    };
}

int drawConnector(
                  float * posA,
                  float * posB,
                  float * scale ,
                  unsigned char R , unsigned char G , unsigned char B , unsigned char Alpha )
{
 glPushMatrix();
    glLineWidth(*scale);
    glColor3f(R,G,B);
     glBegin(GL_LINES);
       glVertex3f(posA[0],posA[1],posA[2]);
       glVertex3f(posB[0],posB[1],posB[2]);
     glEnd();
 glPopMatrix();
 return 1;
}


int drawModelAt(struct Model * mod,float x,float y,float z,float heading,float pitch,float roll)
{
 if (mod==0)
  {
    fprintf(stderr,"Cannot draw model at position %0.2f %0.2f %0.2f , it doesnt exist \n",x,y,z);
    return 0;
  }

  if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt called while on an erroneous state :(\n"); }
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  //glEnable(GL_NORMALIZE);
  /*If scale factors other than 1 are applied to the modelview matrix
    and lighting is enabled, lighting often appears wrong.
    In that case, enable automatic normalization of normals by
    calling glEnable with the argument GL_NORMALIZE.*/

  if (mod->nocull) { glDisable(GL_CULL_FACE); }
  glTranslatef(x,y,z);
  if ( roll!=0 )    { glRotatef(roll,0.0,0.0,1.0); }
  if ( heading!=0 ) { glRotatef(heading,0.0,1.0,0.0); }
  if ( pitch!=0 )   { glRotatef(pitch,1.0,0.0,0.0); }

  if ( (mod->scaleX!=1.0) ||
       (mod->scaleY!=1.0) ||
       (mod->scaleZ!=1.0) )
                       {
                         glScalef( mod->scaleX , mod->scaleY , mod->scaleZ );
                         if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"Could not scale :(\n"); }
                         //fprintf(stderr,"Scaling model by %f %f %f\n",mod->scaleX,mod->scaleY,mod->scaleZ);
                       }

  if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after specifying dimensions \n"); }

       // MAGIC NO COLOR VALUE :P MEANS NO COLOR SELECTION
      if (mod->nocolor!=0)  { glDisable(GL_COLOR_MATERIAL);   } else
      {//We Have a color to set
        //glEnable(GL_COLOR); <- not even an opengl command :P
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after enabling color\n"); }
        glEnable(GL_COLOR_MATERIAL);
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after enabling color material\n"); }

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,    defaultAmbient);
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error regarding color/materials\n"); }
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,    defaultDiffuse);
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error regarding color/materials\n"); }
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,   defaultSpecular);
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error regarding color/materials\n"); }
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,   defaultShininess);
        if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error regarding color/materials\n"); }

        if (mod->transparency==0.0)
        {
          //fprintf(stderr,"SET RGB(%0.2f,%0.2f,%0.2f)\n",mod->colorR, mod->colorG, mod->colorB);
          glColor3f(mod->colorR,mod->colorG,mod->colorB);
        }
        else
        { glEnable(GL_BLEND);			// Turn Blending On
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          //fprintf(stderr,"SET RGBA(%0.2f,%0.2f,%0.2f,%0.2f)\n",mod->colorR, mod->colorG, mod->colorB, mod->transparency);
          glColor4f(mod->colorR,mod->colorG,mod->colorB,mod->transparency);
        }
      }

  if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after specifying color/materials\n"); }

    //fprintf(stderr,"Drawing RGB(%0.2f,%0.2f,%0.2f) , Transparency %0.2f , ColorDisabled %u\n",mod->colorR, mod->colorG, mod->colorB, mod->transparency,mod->nocolor );

    switch ( mod->type )
    {
      case OBJ_PLANE :     drawObjPlane(0,0,0, 0.5);             break;
      case OBJ_GRIDPLANE : drawGridPlane( 0.0 , 0.0 , 0.0, 1.0); break;
      case OBJ_AXIS :      drawAxis(0,0,0,1.0);                  break;
      case OBJ_CUBE :      drawCube();                           break;
      case OBJ_PYRAMID :   drawPyramid();                        break;
      case OBJ_SPHERE  :   drawSphere();                         break;
      case OBJMODEL :
      {
         if (mod->model!=0)
         {
           //A model has been created , and it can be served
           GLuint objlist  =  getObjOGLList( ( struct OBJ_Model * ) mod->model);
           if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"OpenGL error after getObjOGLList\n"); }


           if ( (objlist!=0) && (!DISABLE_GL_CALL_LIST) )
             { //We have compiled a list of the triangles for better performance
               glCallList(objlist);
               if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after drawing glCallList(%u)\n",objlist); }
             }  else
             { //Just feed the triangles to open gl one by one ( slow )
               drawOBJMesh( ( struct OBJ_Model * ) mod->model);
               if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after drawing all the triangles\n"); }
             }
         } else
         { fprintf(stderr,"Could not draw unspecified model\n"); }
         glDisable(GL_TEXTURE_2D); //TODO : <-- change drawOBJMesh , Calllist so that they dont leave textures on! :P
      }
      break;
    };

  if (checkOpenGLError(__FILE__, __LINE__)) { fprintf(stderr,"drawModelAt error after drawing geometry\n"); }

  if (mod->transparency!=0.0) {glDisable(GL_BLEND);  }
  if (mod->nocolor) {glEnable(GL_COLOR); glEnable(GL_COLOR_MATERIAL); }
  if (mod->nocull)  {glEnable(GL_CULL_FACE); }

  glTranslatef(-x,-y,-z);
  //glDisable(GL_NORMALIZE);
  glPopMatrix();
 return 1;
}


int drawModel(struct Model * mod)
{
    if (mod == 0) { fprintf(stderr,"Cannot draw model , it doesnt exist \n"); return 0; } //If mod = 0 accesing the fields below will lead in crashing..
    return drawModelAt(mod,mod->x,mod->y,mod->z,mod->heading,mod->pitch,mod->roll);
}


int addToModelCoordinates(struct Model * mod,float x,float y,float z,float heading,float pitch,float roll)
{
  if (mod==0) { return 0; }
  mod->x+=x; mod->y+=y; mod->z+=z;

  mod->heading+=heading; mod->pitch+=pitch; mod->roll+=roll;
  fprintf(stderr,"Model(%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",mod->x,mod->y,mod->z,mod->heading,mod->pitch,mod->roll);
  return 1;
}

int addToModelCoordinatesNoSTACK(struct Model * mod,float *x,float *y,float *z,float *heading,float *pitch,float *roll)
{
  if (mod==0) { return 0; }
  mod->x+=*x; mod->y+=*y; mod->z+=*z;

  mod->heading+=*heading; mod->pitch+=*pitch; mod->roll+=*roll;
  fprintf(stderr,"Model(%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",mod->x,mod->y,mod->z,mod->heading,mod->pitch,mod->roll);
  return 1;
}

int setModelCoordinates(struct Model * mod,float x,float y,float z,float heading,float pitch,float roll)
{
  if (mod==0) { return 0; }
  fprintf(stderr,"Model SET Got params(%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",x,y,z,heading,pitch,roll);

  mod->x=x; mod->y=y; mod->z=z;

  mod->heading=heading; mod->pitch=pitch; mod->roll=roll;
  fprintf(stderr,"Model SET (%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",mod->x,mod->y,mod->z,mod->heading,mod->pitch,mod->roll);
  return 1;
}

int setModelCoordinatesNoSTACK(struct Model * mod,float * x,float* y,float *z,float *heading,float *pitch,float* roll)
{
  if (mod==0) { return 0; }
  //fprintf(stderr,"Model SET NoSTACK Got params(%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",*x,*y,*z,*heading,*pitch,*roll);

  mod->x=*x; mod->y=*y; mod->z=*z;

  mod->heading=*heading; mod->pitch=*pitch; mod->roll=*roll;
  //fprintf(stderr,"Model SET NoSTACK (%0.2f %0.2f %0.2f - %0.4f %0.4f %0.4f)\n",mod->x,mod->y,mod->z,mod->heading,mod->pitch,mod->roll);
  return 1;
}



int setModelColor(struct Model * mod,float *R,float *G,float *B,float *transparency,unsigned char * noColor)
{
 if (mod==0) { return 0; }

 //fprintf(stderr,"Seting color to  %0.2f %0.2f %0.2f trans %0.2f \n",*R,*G,*B,*transparency);
 mod->colorR = *R;
 mod->colorG = *G;
 mod->colorB = *B;
 mod->transparency = *transparency;
 mod->nocolor = *noColor;
 //fprintf(stderr,"Seting color to  %0.2f %0.2f %0.2f trans %0.2f \n",mod->colorR,mod->colorG,mod->colorB,mod->transparency);
 return 1;
}

int getModelBBox(struct Model *mod , float * minX,  float * minY , float * minZ , float * maxX , float * maxY , float * maxZ)
{
 struct OBJ_Model * objMod = (struct OBJ_Model * ) mod->model;
 * minX = objMod->boundBox.min.x;
 * minY = objMod->boundBox.min.y;
 * minZ = objMod->boundBox.min.z;
 * maxX = objMod->boundBox.max.x;
 * maxY = objMod->boundBox.max.y;
 * maxZ = objMod->boundBox.max.z;
 return 1;
}


int getModel3dSize(struct Model *mod , float * sizeX , float * sizeY , float * sizeZ )
{
  float minX,minY,minZ,maxX,maxY,maxZ;
  getModelBBox(mod,&minX,&minY,&minZ,&maxX,&maxY,&maxZ);

  *sizeX = maxX - minX;
  *sizeY = maxY - minY;
  *sizeZ = maxZ - minZ;

 return 1;
}

