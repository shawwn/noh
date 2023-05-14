#ifndef __SCENEOBJ_T__
#define __SCENEOBJ_T__

#include "../shared/shared_types.h"

#define     SCENEOBJ_NO_SHADOW              0x00000001
//#define       SCENEOBJ_NO_TEXTURE             0x00000002
#define     SCENEOBJ_SOLID_COLOR            0x00000004
#define     SCENEOBJ_SHOW_BOUNDS            0x00000008
#define     SCENEOBJ_MESH_BOUNDS            0x00000010
//#define       SCENEOBJ_NO_LIGHTING            0x00000020
#define     SCENEOBJ_NEVER_CULL             0x00000040
//#define       SCENEOBJ_NO_FOG                 0x00000080
#define     SCENEOBJ_NO_ZWRITE              0x00000100
#define     SCENEOBJ_NO_ZTEST               0x00000200
#define     SCENEOBJ_SELECTABLE             0x00000400
#define     SCENEOBJ_USE_AXIS               0x00000800
#define     SCENEOBJ_USE_ALPHA              0x00001000
#define     SCENEOBJ_BILLBOARD_ORIENTATION  0x00002000
#define     SCENEOBJ_WIREFRAME              0x00004000
#define     SCENEOBJ_POINTS                 0x00008000
//#define       SCENEOBJ_DOUBLE_SIDED           0x00010000
//#define       SCENEOBJ_CULL_FRONT             0x00020000      //cull away front facing polygons (unrelated to SCENEOBJ_NEVER_CULL, which works on the object level)
#define     SCENEOBJ_TRANSLATE_ROTATE       0x00040000      //perform rotation first, then translate
#define     SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME 0x00080000      //loframe specifies the frame of the animated texture
#define     SCENEOBJ_BILLBOARD_ALL_AXES     0x00100000
#define     SCENEOBJ_ORIGINAL_ROTATION      0x00200000      //don't internally rotate the model 180 degrees around Z
#define     SCENEOBJ_SHOW_WIRE              0x00800000
#define     SCENEOBJ_RTS_SILHOUETTE         0x01000000      //draw the silhouette if occluded by something.  uses the sceneobject color to determine the color of the silhouette
#define     SCENEOBJ_SINGLE_MATERIAL            0x02000000      //ignore the skin and use a single shader for meshes
#define     SCENEOBJ_ALWAYS_BLEND           0x04000000

//#define       SCENEOBJ_NO_ANIM_TEX            0x08000000
#define     SCENEOBJ_NO_ADD_TEX             0x10000000
#define     SCENEOBJ_NO_ALPHATEST           0x20000000

#define     SCENEOBJ_SKYBOX (SCENEOBJ_SOLID_COLOR | SCENEOBJ_ALWAYS_BLEND | SCENEOBJ_NO_ALPHATEST)

struct SSceneObject
{
    int         index;          //provide this index to allow the renderer to cache certain elements

    CVec3f      pos;
    CVec3f      angle;
    CVec3f      beamTargetPos;
    CAxis       axis;       //used only if SCENEOBJ_USE_AXIS is set, otherwise 'angle' is used
    float       scale;

    float       creation_time;  //in seconds, just like the cam->time
                                //currently used only for animated shaders
    
    float       alpha;
    
    ResHandle   hModel;
    ResHandle   hMaterial;      //for single polys, or used instead of the skin if SCENEOBJ_SINGLE_MATERIAL is set
    int         hSkin;          //skin index for models

    float       s1;             //used only if it's a billboard
    float       t1;
    float       s2;
    float       t2;
    
    CVec4f      color;          //used if SCENEOBJ_SOLID_COLOR is set
    
    //the following 3 variables are only used if skeleton == nullptr
    int         loframe;        //frame to lerp from (in the default animation)
    int         hiframe;        //frame to lerp to (in the default animation)
    float       lerp_amt;       //amount to lerp between loframe and hiframe

    class CSkeleton *skeleton;      //information about bone positions, for deforming characters
    
    float       width, height;  //for billboards

    int         objtype;

    size_t      zIndex;         //for mouse->object selection

    int         flags;          //see SCENEOBJ_* defines above

    int         *custom_mapping;    //used internally
};

#endif
