// (C)2005 S2 Games
// c_mesh.h
//
//=============================================================================
#ifndef __C_MESH__
#define __C_MESH__

//=============================================================================
// Definitions
//=============================================================================
typedef uint singleLink_t;

// a mesh is like a "sub object" of a model
// meshes may use any of the 3 following "modes":
//
// MESH_SKINNED_BLENDED
//   each vertex on the mesh is linked to one or more bones
//   if any one vertex in the mesh is linked to more than one bone, this is the mode that gets set
// MESH_SKINNED_NONBLENDED
//   one bone link per vertex
//   this mode gets set for all other meshes, including meshes that did not have physique applied
//   (a bone will be generated for this mesh that all vertices are linked to)
//   even meshes which use keyframe data will have their geometry set (though it may not get used)

typedef enum
{
    MESH_SKINNED_BLENDED = 1,
    MESH_SKINNED_NONBLENDED
}
meshModes_enum;

const uint MESH_GPU_DEFORM = BIT(0);
const uint MESH_INVIS = BIT(1);
const uint MESH_NORMALS = BIT(2);

struct SVertexAttribute
{
    uint    uiType;
    int     iSize;
    int     iOffset;
    bool    bNormalized;

    SVertexAttribute() {}
    SVertexAttribute(uint _uiType, int _iSize, int _iOffset, bool _bNormalized) : uiType(_uiType), iSize(_iSize), iOffset(_iOffset), bNormalized(_bNormalized) {}
};

typedef hash_map<tstring, SVertexAttribute> AttributeMap;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CK2Model;
//=============================================================================

//=============================================================================
// CMesh
//=============================================================================
class K2_API CMesh
{
private:
    tstring         m_sName;
    tstring         m_sDefaultShaderName;

    CK2Model        *m_pModel;      // Owner of this mesh
    vector<float>   m_vFaceAreas;
    float           m_fCumArea;

public:
    ~CMesh();
    CMesh();

    void                PostLoad();
    void                Free();

    const tstring&      GetName() const                                 { return m_sName; }
    void                SetName(const tstring &sName)                   { m_sName = sName; }

    CK2Model*           GetModel() const                                { return m_pModel; }
    void                SetModel(CK2Model *pModel)                      { m_pModel = pModel; }

    const tstring&      GetDefaultShaderName() const                    { return m_sDefaultShaderName; }
    void                SetDefaultShaderName(const tstring &sName)      { m_sDefaultShaderName = sName; }

    vector<float>&      GetFaceAreas()                                  { return m_vFaceAreas; }

    //                     lol?
    float               GetCumArea() const                              { return m_fCumArea; }
    void                SetCumArea(float fCumArea)                      { m_fCumArea = fCumArea; }


    int                 mode;       //see MESH_* defines above

    uint                flags;
    int                 surfflags;
    int                 renderflags;

    vec3_t              bmin;       // bounding box (in MESH coordinates)
    vec3_t              bmax;       // bounding box (in MESH coordinates)

    int                 numFaces;
    uivec3_t            *faceList;

    int                 num_verts;  // number of vertices
    int                 num_uv_channels;
    int                 num_color_channels;

    vec3_t              *verts;     // vertex coords (always in MODEL space)
    vec3_t              *normals;   // vertex normals
    bvec4_t             *colors[MAX_VERTEX_COLOR_CHANNELS]; //vertex colors
    vec2_t              *tverts[MAX_UV_CHANNELS];   // texture coords
    vec3_t              *tangents[MAX_UV_CHANNELS]; // vertex tangents
    byte                *signs[MAX_UV_CHANNELS];    // sign of texture space matrix

    /*
        mode == MESH_SKINNED_BLENDED

        The verts array will store coordinates in MODEL space
        (the world coords of the 3dsmax scene).  Verts are
        then blended via:

        for (v = 0; v < mesh->num_verts; ++v)
        {
            blendedVert_t *blend = &mesh->blended_verts[v];
            M_ClearVec3(outVerts[v]);
            for (link = 0; link < mesh->blended_verts[v].num_links; ++link)
            {
                vec3_t point;
                CBone *bone = &model->bones[blend->links[link]];

                M_TransformPoint(mesh->verts[v], bone->invBase, point);     //get the point into the initial bone space
                M_TransformPoint(point, bone->current_transform, point);    //transform the point into the current bone space
                M_MultVec3(point, blend->weights[link], point);
                M_AddVec3(outVerts[v], point, outVerts[v]);
            }
        }

    */
    byte                *linkPool;      //blendedLinks sets pointers here, rather than allocating its own memory
    struct SBlendedLink *blendedLinks;  //vertBlend only allocated if mode == MESH_SKINNED_BLENDED

    /*
        mode == MESH_SKINNED_NONBLENDED  (or LOD fallback for MESH_SKINNED_BLENDED)

        The verts array will store coordinates in BONE space,
        so the calculation is simple:

        for (v = 0; v < mesh->num_verts; ++v)
        {
            M_TransformPoint(mesh->verts[v], bones[mesh->singleLinks[v]]->current_transform, outVerts[v]);
        }

        We could do this calculation in hardware if we split up the mesh according to link
    */
    singleLink_t        *singleLinks;   //allocated for nonblended meshes.  can also be used as an LOD fallback for blended meshes.

    int                 bonelink;       //if > -1, ALL vertices of this mesh are linked to this bone only, in which case both
                                    //singleLinks and blendedLinks will be NULL.

    int                 sbuffer;                        // index to static vertex buffer
    int                 dbuffer;                        // index to dynamic vertex buffer
    int                 ibuffer;                        // index to index buffer
    int                 vertexStride;                   // vertex stride
    dword               vertexFVF;                      // vertex fvf
    int                 vertexDecl;                     // vertex declaration
    AttributeMap        mapAttributes;                  // vertex attributes (OpenGL)
};
//=============================================================================

#endif // __C_MESH__
