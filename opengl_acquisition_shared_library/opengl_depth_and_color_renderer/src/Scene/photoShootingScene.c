#include "photoShootingScene.h"
#include "../tiledRenderer.h"



#include <stdio.h>
#include <stdlib.h>











int setupPhotoshoot(
                        void * scene,
                        void * modelStorage,
                        void * context,
                        int objID,
                        unsigned int columns , unsigned int rows ,
                        float distance,
                        float angleX,float angleY,float angleZ ,
                        float angXVariance ,float angYVariance , float angZVariance
                       )
{

  struct tiledRendererConfiguration * configuration = (struct tiledRendererConfiguration *) context;

  configuration->columns=columns;
  configuration->rows=rows;
  configuration->objID=objID;
  configuration->distance=distance;
  configuration->angleX=angleX;
  configuration->angleY=angleY;
  configuration->angleZ=angleZ;
  configuration->angXVariance=angXVariance;
  configuration->angYVariance=angYVariance;
  configuration->angZVariance=angZVariance;

  configuration->scenePTR = (void *) scene;
  configuration->modelPTR = (void *) modelStorage;
  return 1;
}

void * createPhotoshoot(
                        void * scene,
                        void * modelStorage,
                        int objID,
                        unsigned int columns , unsigned int rows ,
                        float distance,
                        float angleX,float angleY,float angleZ ,
                        float angXVariance ,float angYVariance , float angZVariance
                       )
{

  struct tiledRendererConfiguration * configuration = 0;

  configuration = (struct tiledRendererConfiguration * ) malloc(sizeof( struct tiledRendererConfiguration));
  if (configuration == 0) { fprintf(stderr,"Could not allocate a configuration structure\n"); return 0; }


  configuration->columns=columns;
  configuration->rows=rows;
  configuration->objID=objID;
  configuration->distance=distance;
  configuration->angleX=angleX;
  configuration->angleY=angleY;
  configuration->angleZ=angleZ;
  configuration->angXVariance=angXVariance;
  configuration->angYVariance=angYVariance;
  configuration->angZVariance=angZVariance;

  configuration->scenePTR = (void *) scene;
  configuration->modelPTR = (void *) modelStorage;


  return (void*) configuration;

}



int renderPhotoshoot( void * context  )
{
  struct tiledRendererConfiguration * configuration=context;

  fprintf(stderr," renderPhotoshoot Rows/Cols %u/%u  Distance %0.2f , Angles %0.2f %0.2f %0.2f\n",configuration->rows,configuration->columns,configuration->distance,configuration->angleX,configuration->angleY,configuration->angleZ);
  fprintf(stderr,"Angle Variance %0.2f %0.2f %0.2f\n",configuration->angXVariance,configuration->angYVariance,configuration->angZVariance);


  int i= tiledRenderer_Render(configuration);

  if (i) {
           //framesRendered++;
           return 1;
         }
  return 0;
}


