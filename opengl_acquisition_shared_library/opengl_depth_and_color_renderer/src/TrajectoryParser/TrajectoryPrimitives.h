#ifndef TRAJECTORYPRIMITIVES_H_INCLUDED
#define TRAJECTORYPRIMITIVES_H_INCLUDED


enum { 
 TRAJECTORYPRIMITIVES_EMPTY=0,
 TRAJECTORYPRIMITIVES_OBJ,
 TRAJECTORYPRIMITIVES_DEBUG,
 TRAJECTORYPRIMITIVES_TIMESTAMP,
 TRAJECTORYPRIMITIVES_AUTOREFRESH,
 TRAJECTORYPRIMITIVES_INTERPOLATE_TIME,
 TRAJECTORYPRIMITIVES_BACKGROUND,
 TRAJECTORYPRIMITIVES_OBJECTTYPE,
 TRAJECTORYPRIMITIVES_OBJECT,
 TRAJECTORYPRIMITIVES_COMPOSITEOBJECT,
 TRAJECTORYPRIMITIVES_ARROW,
 TRAJECTORYPRIMITIVES_POS,
 TRAJECTORYPRIMITIVES_EVENT,
 TRAJECTORYPRIMITIVES_PROJECTION_MATRIX,
 TRAJECTORYPRIMITIVES_EMULATE_PROJECTION_MATRIX,
 TRAJECTORYPRIMITIVES_MODELVIEW_MATRIX,
 TRAJECTORYPRIMITIVES_SCALE_WORLD,
 TRAJECTORYPRIMITIVES_OFFSET_ROTATIONS,
 TRAJECTORYPRIMITIVES_MAP_ROTATIONS,
 TRAJECTORYPRIMITIVES_,
 TRAJECTORYPRIMITIVES_END_OF_ITEMS
};



int scanFor_TrajectoryPrimitives(char * str,unsigned int strLength); 

#endif