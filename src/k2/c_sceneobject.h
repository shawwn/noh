// (C)2005 S2 Games
// c_sceneobject.h
//
//=============================================================================
#ifndef __C_SCENEOBJECT_H__
#define __C_SCENEOBJECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_axis.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define     SCENEOBJ_NO_SHADOW                      0x00000001

#define     SCENEOBJ_SOLID_COLOR                    0x00000004
#define     SCENEOBJ_SHOW_BOUNDS                    0x00000008
#define     SCENEOBJ_MESH_BOUNDS                    0x00000010

#define     SCENEOBJ_NEVER_CULL                     0x00000040

#define     SCENEOBJ_NO_ZWRITE                      0x00000100
#define     SCENEOBJ_NO_ZTEST                       0x00000200
#define     SCENEOBJ_SELECTABLE                     0x00000400
#define     SCENEOBJ_USE_AXIS                       0x00000800
#define     SCENEOBJ_USE_ALPHA                      0x00001000
#define     SCENEOBJ_BILLBOARD_ORIENTATION          0x00002000
#define     SCENEOBJ_WIREFRAME                      0x00004000



#define     SCENEOBJ_TRANSLATE_ROTATE               0x00040000      //perform rotation first, then translate
#define     SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME 0x00080000      //loframe specifies the frame of the animated texture
#define     SCENEOBJ_BILLBOARD_ALL_AXES             0x00100000
#define     SCENEOBJ_ORIGINAL_ROTATION              0x00200000      //don't internally rotate the model 180 degrees around Z

#define     SCENEOBJ_SHOW_WIRE                      0x00800000
#define     SCENEOBJ_RTS_SILHOUETTE                 0x01000000      //draw the silhouette if occluded by something.  uses the sceneobject color to determine the color of the silhouette
#define     SCENEOBJ_SINGLE_MATERIAL                    0x02000000      //ignore the skin and use a single shader for meshes
#define     SCENEOBJ_ALWAYS_BLEND                   0x04000000
#define     SCENEOBJ_NO_ALPHATEST                   0x08000000

#define     SCENEOBJ_SKYBOX (SCENEOBJ_SOLID_COLOR | SCENEOBJ_NO_ALPHATEST)
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// CSceneObject
// Todo: convert this to a class
//=============================================================================
struct SSceneObj
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
    
    residx_t    model;
    residx_t    shader;         //for single polys, or used instead of the skin if SCENEOBJ_SINGLE_MATERIAL is set
    int         skin;           //skin index for models (FIXME: we should allow a string to be given here, too)

    float       s1;             //used only if it's a billboard
    float       t1;
    float       s2;
    float       t2;
    
    vec4_t      color;          //used if SCENEOBJ_SOLID_COLOR is set
    
    //the following 3 variables are only used if skeleton == NULL
    int         loframe;        //frame to lerp from (in the default animation)
    int         hiframe;        //frame to lerp to (in the default animation)
    float       lerp_amt;       //amount to lerp between loframe and hiframe

    SSkeleton   *skeleton;      //information about bone positions, for deforming characters
    
    float       width, height;  //for billboards

    int         objtype;

    int         selection_id;   //for mouse->object selection
    int         particle_id;    // for particle system selection

    int         flags;          //see SCENEOBJ_* defines above

    int         *custom_mapping;    //used internally
    int         blocknum;
};
//=============================================================================
#endif //__C_SCENEOBJECT_H__
