#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/Library/ModelLoader/model_loader_tri.h"

#include "assimp_loader.h"
#include "assimp_bvh.h"






int main (int argc, char *argv[])
{
 if (argc<4)   { fprintf(stderr,"Not enough arguments , use as : \nassimpTester --convert source.dae target.tri \n"); return 0; }


 fprintf(stderr,"assimpTester %s %s %s \n",argv[1],argv[2],argv[3]);


 if ( (strstr(argv[2],".dae")!=0) ||
    (strstr(argv[2],".obj")!=0) )

 {

 struct TRI_Model *flatModel    =  allocateModelTri();
 struct TRI_Model *originalModel=  allocateModelTri();

 int selectMesh=0;
 if (argc>=5)
  {
     selectMesh=atoi(argv[4]);
     fprintf(stderr,"Selecting mesh %u \n",selectMesh);
   }


 convertAssimpToTRI(argv[2],flatModel,originalModel,selectMesh);


    for (int i=0; i<argc; i++)
        {
           if (strcmp(argv[i],"--paint")==0)
            {
               int r = atoi(argv[i+1]);
               int g = atoi(argv[i+2]);
               int b = atoi(argv[i+3]);

               fprintf(stderr,"Will paint mesh (RGB) (%u/%u/%u) \n",r,g,b);
               paintTRI(
                        originalModel,
                        r,
                        g,
                        b
                       );
            }
        }

 saveModelTri(argv[3], originalModel);


   if (strcmp(argv[1],"--test")==0)
   {
    struct TRI_Model *reloadedModel=  allocateModelTri();
    loadModelTri( argv[3] , reloadedModel);
    saveModelTri( "resave.tri", reloadedModel);

    freeModelTri(reloadedModel);
   }

 freeModelTri(flatModel);
 freeModelTri(originalModel);


 } else
 if (strstr(argv[2],".bvh")!=0)
 {
   doBVHConversion(argv[2]);

 }


 return 0;

}
