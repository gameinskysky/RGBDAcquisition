/** @file bvh_loader.h
 *  @brief  BVH file parser
            part of  https://github.com/AmmarkoV/RGBDAcquisition/tree/master/opengl_acquisition_shared_library/opengl_depth_and_color_renderer
            This is a very clean-cut parser of bvh file that will load a struct BVH_MotionCapture using the loadBVH command
            This part of the code does not perform any transformations on the loaded file, if you want to check transformations see bvh_transform.h
 *  @author Ammar Qammaz (AmmarkoV)
 */
#ifndef BVH_LOADER_H_INCLUDED
#define BVH_LOADER_H_INCLUDED


#ifdef __cplusplus
extern "C"
{
#endif

//BVH Code version
static const char BVH_LOADER_VERSION_STRING [] = "0.4";


/**
* @brief MAX_BVH_JOINT_NAME is the maximum label size for Joint names
* @ingroup BVH
*/
#define MAX_BVH_JOINT_NAME 128

/**
* @brief MAX_BVH_JOINT_HIERARCHY_SIZE is the maximum number of Joints
* @ingroup BVH
*/
#define MAX_BVH_JOINT_HIERARCHY_SIZE 1024


/**
* @brief MAX_BVH_FILE_LINE_SIZE is the maximum number of a valid line in an incoming  BVH file
* @ingroup BVH
*/
#define MAX_BVH_FILE_LINE_SIZE 10000



static const char * channelNames[] =
{
    "no rotation channel",
    "Xrotation",
    "Yrotation",
    "Zrotation",
    "Xposition",
    "Yposition",
    "Zposition",
//=================
    "End of Channel Names" ,
    "Unknown"
};


enum CHANNEL_NAMES
{
  BVH_POSITION_NONE=0,
  BVH_ROTATION_X,
  BVH_ROTATION_Y,
  BVH_ROTATION_Z,
  BVH_POSITION_X,
  BVH_POSITION_Y,
  BVH_POSITION_Z,
  //--------------------
  BVH_VALID_CHANNEL_NAMES
};



static const char * rotationOrderNames[] =
{
  "no rotation order",
  "XYZ",
  "XZY",
  "YXZ",
  "YZX",
  "ZXY",
  "ZYX",
//=================
    "End of Channel Rotation Orders" ,
    "Unknown"
};


enum CHANNEL_ROTATION_ORDER
{
  BVH_ROTATION_ORDER_NONE=0,
  BVH_ROTATION_ORDER_XYZ,
  BVH_ROTATION_ORDER_XZY,
  BVH_ROTATION_ORDER_YXZ,
  BVH_ROTATION_ORDER_YZX,
  BVH_ROTATION_ORDER_ZXY,
  BVH_ROTATION_ORDER_ZYX,
  //--------------------
  BVH_VALID_ROTATION_ORDER_NAMES
};




enum ORIENTATION_ENFORCER_IDS
{
  BVH_ENFORCE_NO_ORIENTATION=0,
  BVH_ENFORCE_FRONT_ORIENTATION,
  BVH_ENFORCE_BACK_ORIENTATION,
  BVH_ENFORCE_LEFT_ORIENTATION,
  BVH_ENFORCE_RIGHT_ORIENTATION,
  //--------------------
  BVH_VALID_ENFORCED_ORIENTATION_NAMES
};



extern const char * rotationOrderNames[];

typedef unsigned int BVHJointID;
typedef unsigned int BVHFrameID;
typedef unsigned int BVHMotionChannelID;

struct MotionBuffer
{
  float * motion;
  unsigned int bufferSize;
};

/**
* @brief Each BVH Joint has a number of properties,offsets and channels that are modeled using this structure
* @ingroup BVH
*/
struct BVH_Joint
{
  //--------------------
  char isImmuneToTorsoOcclusions;
  //--------------------
   char isAPartOfLeftFoot;
   char isAPartOfRightFoot;
   char isAPartOfLeftArm;
   char isAPartOfRightArm;
   char isAPartOfLeftHand;
   char isAPartOfRightHand;
   char isAPartOfHead;
   char isAPartOfTorso;
  //--------------------
  char erase2DCoordinates;
  char isRoot;
  char hasBrace;
  char isEndSite;
  char hasEndSite;
  //--------------------
  char jointName[MAX_BVH_JOINT_NAME+1];
  unsigned int jointNameHash;
  //--------------------
  unsigned int parentJoint;
  unsigned int endSiteJoint;
  unsigned int hierarchyLevel;
  //--------------------
  float offset[3];
  float staticTransformation[16];
  //--------------------
  char  channelType[BVH_VALID_CHANNEL_NAMES];
  char  channelRotationOrder;
  unsigned int loadedChannels;
  //--------------------
};



/**
* @brief The lookup table has 1:1 correspondance from joints to the given motions
* @ingroup BVH
*/
struct BVH_JointToMotion_LookupTable
{
  unsigned int jointMotionOffset;
  unsigned int channelIDMotionOffset[BVH_VALID_CHANNEL_NAMES];
};

/**
* @brief The lookup table has 1:1 correspondance from motions to the given joints
* @ingroup BVH
*/
struct BVH_MotionToJoint_LookupTable
{
  unsigned int jointID;
  unsigned int parentID;
  unsigned int channelID;
};


/**
* @brief Each BVH Motion Capture file has a hierarchy of BVH_Joints and an array of motion values that should be accessed using
         the helper functions
* @ingroup BVH
*/
struct BVH_MotionCapture
{
  float scaleWorld;

  //Header
  unsigned int numberOfFrames;
  unsigned int numberOfFramesEncountered;
  float frameTime;

  //Joint Hierarchy
  unsigned int rootJointID;
  unsigned int MAX_jointHierarchySize;
  unsigned int jointHierarchySize;
  struct BVH_Joint jointHierarchy[MAX_BVH_JOINT_HIERARCHY_SIZE+32]; //End Joints need extra space so lets give them +32 bytes

  //We may want to only work with specific selected joints..!
  unsigned int selectionIncludesEndSites;
  unsigned int numberOfJointsWeWantToSelect;
  unsigned int * selectedJoints;
  unsigned int * hideSelectedJoints;

  //Lookup Tables..
  struct BVH_JointToMotion_LookupTable jointToMotionLookup[MAX_BVH_JOINT_HIERARCHY_SIZE];
  struct BVH_MotionToJoint_LookupTable motionToJointLookup[MAX_BVH_JOINT_HIERARCHY_SIZE*6];

  //Motion
  unsigned int numberOfValuesPerFrame;
  unsigned int motionValuesSize;
  float * motionValues;

  //Internal Variables..
  char fileName[2048];
  unsigned int linesParsed;

  //---------
  char debug;
};


int enumerateChannelOrderFromTypes(char typeA,char typeB,char typeC);

int enumerateChannelOrder(struct BVH_MotionCapture * bvhMotion , unsigned int currentJoint);

/**
* @brief Load a BVH file by giving a filename and filling in a BVH_MotionCapture struct
* @ingroup BVH
* @param  C-String with path to BVH File
* @param  pointer to an allocated BVH_MotionCapture struct
* @param  the scale of the world
*/
int bvh_loadBVH(const char * filename , struct BVH_MotionCapture * bvhMotion, float scaleWorld);


/**
* @brief Free the space allocated by a BVH_MotionCapture file
* @ingroup BVH
* @param  BVH Structure
* @return 1=Success/0=Failure
*/
int bvh_free(struct BVH_MotionCapture * bvhMotion);


int bvh_SetPositionRotation(
                            struct BVH_MotionCapture * mc,
                            float * position,
                            float * rotation
                           );


int bvh_OffsetPositionRotation(
                               struct BVH_MotionCapture * mc,
                               float * position,
                               float * rotation
                              );


int bvh_ConstrainRotations(
                           struct BVH_MotionCapture * mc,
                           unsigned int constrainOrientation
                          );



/**
* @brief Get Parent joint .
* @ingroup BVH
* @param  BVH Structure
* @param  Joint ID we want to query
* @return 1=HasParent/0=NoParent-Error
*/
int bhv_getJointParent(struct BVH_MotionCapture * bvhMotion , BVHJointID jID );



int bvh_onlyAnimateGivenJoints(struct BVH_MotionCapture * bvhMotion,unsigned int numberOfArguments,const char **argv);

/**
* @brief Ask if joint has a parent.
* @ingroup BVH
* @param  BVH Structure
* @param  Joint ID we want to query
* @return 1=HasParent/0=NoParent-Error
*/
int bhv_jointHasParent(struct BVH_MotionCapture * bvhMotion , BVHJointID jID );





int bhv_jointGetEndSiteChild(struct BVH_MotionCapture * bvhMotion,BVHJointID jID,BVHJointID * jChildID);


/**
* @brief Ask if joint has rotational component.
* @ingroup BVH
* @param  BVH Structure
* @param  Joint ID we want to query
* @return 1=HasRotation/0=NoRotation-Error
*/
int bhv_jointHasRotation(struct BVH_MotionCapture * bvhMotion , BVHJointID jID);

/**
* @brief Resolve a C-String from a Joint name to a Joint ID
* @ingroup BVH
* @param  BVH Structure
* @param  C-String with Joint Name
* @param  Output Joint ID
* @return 1=Found/0=NotFound
*/
int bvh_getJointIDFromJointName( struct BVH_MotionCapture * bvhMotion , const char * jointName, BVHJointID * jID);

int bvh_getJointIDFromJointNameNocase(
                                      struct BVH_MotionCapture * bvhMotion ,
                                      const char * jointName,
                                      BVHJointID * jID
                                     );

/**
* @brief Get the root joint of a BVH_MotionCapture file
* @ingroup BVH
* @param   BVH Structure
* @param   Output Root Joint ID
* @return  1=Found/0=NotFound
*/
int bvh_getRootJointID(
                       struct BVH_MotionCapture * bvhMotion ,
                       BVHJointID * jID
                      );



/**
* @brief Copy a motion frame of a BVH_MotionCapture file
* @ingroup BVH
* @param  BVH Structure
* @param  Target frame we want to populate with source
* @param  Source frame we want to copy
* @return 1=Success/0=Failure
*/
int bvh_copyMotionFrame(
                         struct BVH_MotionCapture * bvhMotion,
                         BVHFrameID tofID,
                         BVHFrameID fromfID
                        );


/**
* @brief Copy a motion frame to a buffer
* @ingroup BVH
* @param  BVH Structure
* @param  Target frame we want to populate with source
* @param  Source frame we want to copy
* @return 1=Success/0=Failure
*/
int bvh_copyMotionFrameToMotionBuffer(
                                      struct BVH_MotionCapture * bvhMotion,
                                      struct MotionBuffer * motionBuffer,
                                      BVHFrameID fromfID
                                     );

//float * bvh_getJointOffset(struct BVH_MotionCapture * bvhMotion , BVHJointID jID);


int bvh_changeJointDimensions(
                              struct BVH_MotionCapture * bvhMotion,
                              const char * jointName,
                              float xScale,
                              float yScale,
                              float zScale
                             );


int bvh_scaleAllOffsets(
                        struct BVH_MotionCapture * bvhMotion,
                        float scalingRatio
                       );


int bvh_getMotionChannelName(struct BVH_MotionCapture * bvhMotion,BVHMotionChannelID mID,char * target,unsigned int targetLength);

/**
* @brief Request a specific motion channel of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @param  Type of channel we want to access ( see enum CHANNEL_NAMES )
* @return Value of channel for specific joint at specific frame
*/
float bvh_getJointChannelAtFrame(struct BVH_MotionCapture * bvhMotion, BVHJointID jID, BVHFrameID fID, unsigned int channelTypeID);



/**
* @brief Request X Rotation of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return X Rotation for specific joint at specific frame
*/
float  bvh_getJointRotationXAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);

/**
* @brief Request Y Rotation of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return Y Rotation for specific joint at specific frame
*/
float  bvh_getJointRotationYAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);

/**
* @brief Request Z Rotation of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return Z Rotation for specific joint at specific frame
*/
float  bvh_getJointRotationZAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);

/**
* @brief Request X Position of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return X Position for specific joint at specific frame
*/
float  bvh_getJointPositionXAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);

/**
* @brief Request Y Position of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return Y Position for specific joint at specific frame
*/
float  bvh_getJointPositionYAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);

/**
* @brief Request Z Position of a specific joint at a specific frame
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @return Z Position for specific joint at specific frame
*/
float  bvh_getJointPositionZAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID);




//TODO: add doxygen documentation
int bvh_setJointChannelAtFrame(struct BVH_MotionCapture * bvhMotion, BVHJointID jID, BVHFrameID fID, unsigned int channelTypeID,float value);

int bvh_setJointRotationXAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);
int bvh_setJointRotationYAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);
int bvh_setJointRotationZAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);
int bvh_setJointPositionXAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);
int bvh_setJointPositionYAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);
int bvh_setJointPositionZAtFrame(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID,float value);

int bhv_setPosXYZRotXYZ(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID , float * data , unsigned int sizeOfData);



/**
* @brief Request XYZ positions and XYZ rotations that will be written on a float data[6] array
* @ingroup BVH
* @param  BVH Structure
* @param  Joint we want to access ( if we just have the name we can retrieve jID using bvh_getJointIDFromJointName )
* @param  Frame we want to access
* @param  Output float array that will hold the results
* @param  Size of the output float array that should hold at least 6 floats
* @return 1=Success/0=Failure
*/
int bhv_populatePosXYZRotXYZ(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , BVHFrameID fID , float * data , unsigned int sizeOfData);

int bhv_populatePosXYZRotXYZFromMotionBuffer(struct BVH_MotionCapture * bvhMotion , BVHJointID jID , float * motionBuffer, float * data, unsigned int sizeOfData);

/**
* @brief Direct access to the motion data, without Joint hierarchy,Frame separation etc, should not be used unless you really know what you are doing..
* @ingroup BVH
* @param  BVH Structure
* @param  Motion element we want to read
* @return Motion Value
*/
float bvh_getMotionValue(struct BVH_MotionCapture * bvhMotion , unsigned int mID);



int bvh_selectJoints(
                    struct BVH_MotionCapture * mc,
                    unsigned int numberOfValues,
                    unsigned int includeEndSites,
                    const char **argv,
                    unsigned int iplus1
                   );



int bvh_selectJointsToHide2D(
                             struct BVH_MotionCapture * mc,
                             unsigned int numberOfValues,
                             unsigned int includeEndSites,
                             const char **argv,
                             unsigned int iplus1
                            );



 int bvh_removeSelectedFrames(struct BVH_MotionCapture * bvhMotion,unsigned int * framesToRemove);




void freeMotionBuffer(struct MotionBuffer * mb);
int copyMotionBuffer(struct MotionBuffer * dst,struct MotionBuffer * src);
struct MotionBuffer * mallocNewMotionBuffer(struct BVH_MotionCapture * mc);
struct MotionBuffer * mallocNewMotionBufferAndCopy(struct BVH_MotionCapture * mc,struct MotionBuffer * whatToCopy);
void compareMotionBuffers(const char * msg,struct MotionBuffer * guess,struct MotionBuffer * groundTruth);
void compareTwoMotionBuffers(struct BVH_MotionCapture * mc,const char * msg,struct MotionBuffer * guessA,struct MotionBuffer * guessB,struct MotionBuffer * groundTruth);


/**
* @brief Print BVH information on stderr
* @ingroup BVH
* @param  pointer to an allocated BVH_MotionCapture struct
*/
void bvh_printBVH(struct BVH_MotionCapture * bvhMotion);

void bvh_printBVHJointToMotionLookupTable(struct BVH_MotionCapture * bvhMotion);


void bvh_print_C_Header(struct BVH_MotionCapture * bvhMotion);


#ifdef __cplusplus
}
#endif




#endif // BVH_LOADER_H_INCLUDED
