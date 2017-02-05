#ifndef ASSIMP_LOADER_H_INCLUDED
#define ASSIMP_LOADER_H_INCLUDED


#include "../../src/ModelLoader/model_loader_tri.h"

unsigned int countNumberOfNodes(struct aiScene *scene  , struct aiMesh * mesh );

int convertAssimpToTRI(const char * filename  , struct TRI_Model * triModel , struct TRI_Model * originalModel , int selectMesh);


#endif // ASSIMP_LOADER_H_INCLUDED
