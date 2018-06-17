

#ifndef MATRIXOPENGL_H_INCLUDED
#define MATRIXOPENGL_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif



/**
* @brief build Projection Matrix using Rodriguez Rotation and a translation ( typically coming from OpenCV )
* @ingroup AmMatrix
* @param  Output Array 4x4 of resulting matrix
* @param  Input Rodriguez Coordinates
* @param  Input Translation Coordinate
* @param  Input Unit Scale
* @retval 0=Failure,1=Success
*/
int convertRodriguezAndTranslationTo4x4DUnprojectionMatrix(double * result4x4, double * rodriguez , double * translation , double scaleToDepthUnit);

/**
* @brief build OpenGL Projection Matrix using Rodriguez Rotation and a translation ( typically coming from OpenCV )
* @ingroup AmMatrix
* @param  Output Array 4x4 of resulting matrix
* @param  Input Rodriguez Coordinates
* @param  Input Translation Coordinate
* @param  Input Unit Scale
* @retval 0=Failure,1=Success
*/
int convertRodriguezAndTranslationToOpenGL4x4DProjectionMatrix(double * result4x4, double * rodriguez , double * translation , double scaleToDepthUnit);




/**
* @brief build OpenGL Projection Matrix simulating a "real" camera
* @ingroup AmMatrix
* @param  Output Array 4x1 of resulting relative position ( X,Y,Z,W )
* @param  Input Array 4x1 of object Position ( X,Y,Z,W )
* @param  Input Array 3x3 of object Rotation
* @param  Input Array 4x1 of absolute 3D position of the point ( X,Y,Z,W )
* @retval 0=Failure,1=Success
*/
void buildOpenGLProjectionForIntrinsics   (
                                             double * frustum,
                                             int * viewport ,
                                             double fx,
                                             double fy,
                                             double skew,
                                             double cx, double cy,
                                             unsigned int imageWidth, unsigned int imageHeight,
                                             double nearPlane,
                                             double farPlane
                                           );






/**
* @brief Calculate an OpenGL frustrum matrix using floats
* @ingroup OGLTools
* @param Output Matrix
* @param Left Limit
* @param Right Limit
* @param Bottom Limit
* @param Top Limit
* @param Z Near
* @param Z Far
*/
void glhFrustumf2(
                  float *matrix,
                  float left,
                  float right,
                  float bottom,
                  float top,
                  float znear,
                  float zfar
                 );




/**
* @brief Calculate an OpenGL perspective matrix using floats
* @ingroup OGLTools
* @param Output Matrix
* @param Field Of View in Degrees
* @param Aspect Ratio
* @param Z Near
* @param Z Far
*/
void glhPerspectivef2(
                      float *matrix,
                      float fovyInDegrees,
                      float aspectRatioV,
                      float znear,
                      float zfar
                     );




/**
* @brief Calculate an OpenGL perspective matrix using doubles
* @ingroup OGLTools
* @param Output Matrix
* @param Field Of View in Degrees
* @param Aspect Ratio
* @param Z Near
* @param Z Far
*/
void gldPerspective(
                     double * matrix,
                     double fovx,
                     double aspect,
                     double zNear,
                     double zFar
                   );
















#ifdef __cplusplus
}
#endif


#endif //MATRIXOPENGL_H_INCLUDED
