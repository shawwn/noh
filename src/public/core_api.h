// (C)2005 S2 Games
// core_api.h
//
// Core engine interface functions
//=============================================================================
#ifndef __CORE_API_H__
#define __CORE_API_H__

//=============================================================================
// Headers
//=============================================================================
#include "../shared/shared_types.h"

#include "../public/worldlight_t.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IModel;
class CModel;
class CTreeModel;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SGameLibInfo
{
    tstring sName;
    int     iMajorVersion;
    int     iMinorVersion;

    bool    (*Init)();
    void    (*Frame)(uint uiGameTime);
    void    (*Shutdown)();

    SGameLibInfo() :
    iMajorVersion(0),
    iMinorVersion(0),
    Init(nullptr),
    Frame(nullptr),
    Shutdown(nullptr)
    {
    }
};
//=============================================================================

typedef struct coreAPI_shared_s
{
    //media
    residx_t    (*Res_LoadStringTable)(const tstring &sName);

    //string table
    char*       (*Str_Get)(residx_t stringtable, const char *stringname);
}
coreAPI_shared_t;

struct SFoliagemapTile;
struct SFoliagemapVertex;

typedef struct coreAPI_client_s
{
    // World management
    void        (*World_SampleGround)(float xpos, float ypos, struct pointinfo_s *result);
    void        (*World_GetBounds)(vec3_t bmin, vec3_t bmax);
    void        (*World_Destroy)();
    void        (*World_DeformGround)(int xpos, int ypos, heightmap_t newheight);
    bool        (*World_TraceBox)(struct traceinfo_s *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
    bool        (*World_TraceBoxEx)(struct traceinfo_s *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);
    void        (*World_LinkObject)(int iObjIndex, const CVec3f &v3ObjPos, const CVec3f &v3ObjBMin, const CVec3f &v3ObjBMax, int iObjFlags, int iObjSurfFlags);
    void        (*World_UnlinkObject)(int iObjIndex);
    void        (*World_FitPolyToTerrain)(SSceneFaceVert *verts, int num_verts, residx_t shader, int flags, void (*clipCallback)(int num_verts, SSceneFaceVert *verts, residx_t shader, int flags));
    float       (*World_CalcMaxZ)(float x1, float y1, float x2, float y2);
    float       (*WorldToGrid)(float coord);
    float       (*GridToWorld)(float gridcoord);
    void        (*World_GetOccluder)(int num, struct occluder_s *out);
    bool        (*World_UpdateOccluder)(int num, const struct occluder_s *occ);
    int         (*World_GetNumOccluders)();
    void        (*World_ClearOccluders)();
    int         (*World_AddOccluder)(struct occluder_s *out);
    void        (*World_RemoveOccluder)(int num);
    bool        (*World_UpdateTerrain)(int iStartX, int iStartY, int iWidth, int iHeight);
    bool        (*World_GetShadermapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestWidth, int iLayer);
    bool        (*World_SetShadermapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceWidth, int iLayer);
    bool        (*World_GetHeightmapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestWidth);
    bool        (*World_SetHeightmapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceWidth);
    bool        (*World_GetColormapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestWidth);
    bool        (*World_SetColormapRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceWidth);
    bool        (*World_GetFoliagemapTileRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestWidth);
    bool        (*World_SetFoliagemapTileRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceWidth);
    bool        (*World_GetFoliagemapVertexRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestWidth);
    bool        (*World_SetFoliagemapVertexRegion)(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceWidth);
    void        (*World_GetLight)(int num, struct SSceneLight *out);
    bool        (*World_UpdateLight)(int num, const struct SSceneLight *in);
    int         (*World_GetNumLights)();
    void        (*World_ClearLights)();
    int         (*World_AddLight)(struct SSceneLight *light);
    void        (*World_RemoveLight)(int num);
    bool        (*World_Save)(const tstring &sName);


    // World rendering
    void        (*WR_SetFoliagemapVertex)(int x, int y, const SFoliagemapVertex &inVertex);

    float       (*WR_FarClip)();

//  void        (*WR_FitQuad)(SSceneFaceVert quadverts[4], residx_t shader, int flags);

    // World Objects
    int         (*WO_GetNumObjdefs)();
    tstring     (*WO_GetObjdefName)(int n);
    int         (*WO_RegisterObjectVar)(const char *objclass, const char *varname);
    tstring     (*WO_GetObjdefVar)(int objdef_id, const char *varname);
    float       (*WO_GetObjdefVarValue)(int objdef_id, const char *varname);
    bool        (*WO_UseObjdef)(const tstring &sObjDefName);
    bool        (*WO_GetObjectPos)(int id, SObjectPosition *objpos);
    residx_t    (*WO_GetObjectSound)(int id);
    void        (*WO_CreateObjectClass)(const char *objclass);
    bool        (*WO_CreateObject)(int objdef, const char *optmodel, const char *optskin, SObjectPosition *pos, int id, uint uiSeed);
    void        (*WO_ClearObjects)();
    bool        (*WO_GetObjectBounds)(int id, vec3_t bmin, vec3_t bmax);
    void        (*WO_DeleteObject)(int id);
    int         (*WO_GetObjectObjdef)(int id);
    ResHandle   (*WO_GetObjectModel)(int id);
    residx_t    (*WO_GetObjectSkin)(int id);
    int         (*WO_GetObjdefId)(const tstring &sName);
    uint    (*WO_GetObjectSeed)(int id);
    void        (*WO_SetObjectSeed)(int id, uint uiSeed);
    bool        (*WO_IsObjectReference)(int id);
    void        (*WO_SetObjectModelAndSkin)(int id, ResHandle hModel, residx_t skin);
    void        (*WO_RenderObjects)(vec3_t cameraPos);

    // Scene management
    void        (*Scene_AddLight)(SSceneLight *light);
    
    // Host functions
    int         (*Milliseconds)();  
    int         (*FrameMilliseconds)();

    ResHandle   (*GetWhiteTexture)();

    void                (*Client_SendMessageToServer)(const tstring &sMsg);
    void                (*Client_RequestStateString)(int id, int num);
    void                (*Client_GetRealPlayerState)(SPlayerState *ps);
    int                 (*Client_GetOwnClientNum)();
    bool                (*Client_ObjectsUpdated)(int *updatetime);
    class CStateString& (*Client_GetStateString)(size_t zID);
    int                 (*Client_GetConnectionState)();
    void                (*Client_SendInputStates)(struct SInputState *pInputState, size_t zCount);
    bool                (*Client_ConnectionProblems)();
    bool                (*Client_IsPlayingDemo)();

    int     (*MasterServer_GetGameList)();
    int     (*MasterServer_GetGameList_Frame)(SServerInfo *servers, int max_servers, int *num_servers);
    bool    (*MasterServer_GetGameInfo)(const tstring &sIP, int port, int *ping, char **coreInfo, char **gameInfo);
    int     (*MasterServer_FindUsers)(char *name);
    int     (*MasterServer_GetUserList_Frame)(SUserInfo *users, int max_users, int *num_users);
    int     (*MasterServer_GetUserInfo)(char *cookie, char *server, int port);
    int     (*MasterServer_GetUserInfo_Frame)(char *info, int maxlen);

    int    (*HTTP_GetText)(char *addr, char *textbuf, int buf_size);
    int    (*HTTP_GetFile)(const tstring &sUrl, const tstring &sFilename);

    bool    (*Speak)(const char *string);

    void    (*TL_DrawSunRays)(const class CCamera &cam);
}
coreAPI_client_t;

typedef struct coreAPI_server_s
{
    //world management
    void        (*World_SampleGround)(float xpos, float ypos, struct pointinfo_s *result);
    void        (*World_GetBounds)(vec3_t bmin, vec3_t bmax);
    void        (*World_Destroy)();
    void        (*World_DeformGround)(int xpos, int ypos, heightmap_t newheight);
    bool        (*World_TraceBox)(struct traceinfo_s *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
    bool        (*World_TraceBoxEx)(struct traceinfo_s *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);
    void        (*World_LinkObject)(int iObjIndex, const CVec3f &v3ObjPos, const CVec3f &v3ObjBMin, const CVec3f &v3ObjBMax, int iObjFlags, int iObjSurfFlags);
    void        (*World_UnlinkObject)(int iObjIndex);
    bool        (*World_IsLinked)(int iObjIndex);
    float       (*World_CalcMaxZ)(float x1, float y1, float x2, float y2);
    float       (*WorldToGrid)(float coord);
    float       (*GridToWorld)(float gridcoord);

    int         (*WO_GetNumObjdefs)();
    tstring     (*WO_GetObjdefName)(int n);
    int         (*WO_RegisterObjectVar)(const char *objclass, const char *varname);
    tstring     (*WO_GetObjdefVar)(int objdef_id, const char *varname);
    float       (*WO_GetObjdefVarValue)(int objdef_id, const char *varname);
    bool        (*WO_UseObjdef)(const tstring &sObjDefName);
    bool        (*WO_GetObjectPos)(int id, SObjectPosition *objpos);
    void        (*WO_CreateObjectClass)(const char *objclass);
    bool        (*WO_CreateObject)(int objdef, const char *optmodel, const char *optskin, SObjectPosition *pos, int id, uint uiSeed);
    void        (*WO_ClearObjects)();
    bool        (*WO_GetObjectBounds)(int id, vec3_t bmin, vec3_t bmax);
    void        (*WO_DeleteObject)(int id);
    int         (*WO_GetObjectObjdef)(int id);
    residx_t    (*WO_GetObjectModel)(int id);
    residx_t    (*WO_GetObjectSkin)(int id);
    int         (*WO_GetObjdefId)(const tstring &sName);
    int         (*WO_FindReferenceObject)(int startindex, const char *refname, SReferenceObject *refobj);

    //navrep functions
    void        (*NavRep_CSGSubtract)(int iObjIndex, const CVec3f &v3ObjPos, const CVec3f &v3ObjBMin, const CVec3f &v3ObjBMax, const CVec3f &v3ObjAngles);
    void        (*NavRep_CSGAdd)(int iObjIndex);
    bool        (*NavRep_Trace)(navmeshsize_t navmeshsize, const vec2_t src, const vec2_t dest);
    bool        (*NavRep_CanPlaceBuilding)(const vec3_t bminw, const vec3_t bmaxw);

    //navrep path functions
    CNavPath*   (*NavRep_PathCreateToPosition)(navmeshsize_t navmeshsize, const vec3_t src, struct navpoly_s* navPolySrc, const vec3_t dest);
    CNavPath*   (*NavRep_PathCreateToObject)(navmeshsize_t navmeshsize, const vec3_t src, struct navpoly_s* navPolySrc, int iObjIndex, CVec3f v3ObjPos, CVec3f v3ObjAngles, bool close);
    void        (*NavRep_PathDestroy)(navmeshsize_t navmeshsize, CNavPath* navpath);
    bool        (*NavRep_PathOptimize)(navmeshsize_t navmeshsize, CNavPath* navpath);

    //host functions
    int         (*Milliseconds)();  

    //packet functions
    void    (*Pkt_Clear)(struct packet_s *pkt);
    bool    (*Pkt_WriteInt)(struct packet_s *pkt, int i);
    bool    (*Pkt_WriteShort)(struct packet_s *pkt, short i);
    bool    (*Pkt_WriteByte)(struct packet_s *pkt, byte b);
    bool    (*Pkt_WriteString)(struct packet_s *pkt, const char *s);
    bool    (*Pkt_WriteCmd)(struct packet_s *pkt, byte cmd);
    bool    (*Pkt_WriteCoord)(struct packet_s *pkt, float coord);
    bool    (*Pkt_WriteByteAngle)(struct packet_s *pkt, float angle);
    bool    (*Pkt_WriteWordAngle)(struct packet_s *pkt, float angle);
    bool    (*Pkt_WriteFloat)(struct packet_s *pkt, float f);

    void            (*Server_BroadcastMessage)(int sender, const tstring &sMsg);
    void            (*Server_SendMessage)(int sender, int client, const tstring &sMsg);
    void            (*Server_SendUnreliableMessage)(int sender, int client, const tstring &sMsg);
    void            (*Server_GameObjectPointer)(void *base, int stride, int num_objects);
    void            (*Server_ClientExclusionListPointer)(void *base, int stride, int num_clients);
    void            (*Server_UpdatePlayerState)(int clientnum, SPlayerState *ps);
    char*           (*Server_GetClientCookie)(int client_id);
    uint    (*Server_GetClientUID)(int client_id);
    uint    (*Server_GetClientGUID)(int client_id);
    void            (*Server_SetStateString)(size_t zID, const tstring &sStr);
    char*           (*Server_GetStateString)(int id, char *buf, int bufsize);
    void            (*Server_SetRequestOnly)(int id);
    void            (*Server_SendRequestString)(int clientnum, int id);
    int             (*Server_GetClientPing)(int clientnum);
    const char*     (*Server_GetClientIP)(int clientnum);
    int             (*Server_AllocVirtualClientSlot)();
    bool            (*Server_StartStats)(const tstring &sStats);
    bool            (*Server_AddClientStats)(char *cookie, const tstring &sStats);
    bool            (*Server_SendStats)();
}
coreAPI_server_t;
//=============================================================================

#endif // __CORE_API_H__
