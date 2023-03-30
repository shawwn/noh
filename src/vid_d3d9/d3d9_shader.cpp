// (C)2005 S2 Games
// d3d9_shader.cpp
//
// Direct3D shader functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
#include "d3d9_shader.h"
#include "d3d9_state.h"
#include "d3d9_util.h"
#include "d3d9_texture.h"
#include "d3d9_scene.h"
#include "d3d9_material.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_shadervar.h"
#include "c_shadervarregistry.h"
#include "c_shadersamplerregistry.h"
#include "c_shadowmap.h"
#include "c_reflectionmap.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"
#include "c_velocitymap.h"
#include "c_shadercache.h"

#include "../k2/c_filemanager.h"
#include "../k2/c_material.h"
#include "../k2/c_vertexshader.h"
#include "../k2/c_pixelshader.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_filechangecallback.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
typedef map<dword, int>     VertexDeclarationMap;

struct SPixelShaderVersion
{
    tstring sName;
    int     major;
    int     minor;
};

struct SVertexShaderVersion
{
    tstring sName;
    int     major;
    int     minor;
};
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_STRING (vid_shaderDefinitions,         "");
CVAR_BOOLF  (vid_specularLookup,            false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderDebug,               false,  CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shaderDebugOutputDisasm,   false);
CVAR_BOOL   (vid_shaderDebugOutputBin,      false);
CVAR_BOOL   (vid_shaderDebugOutputAsm,      false);
CVAR_BOOLF  (vid_treeSmoothNormals,         false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_terrainDerepeat,           true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_terrainAlphamap,           true,   CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowFalloff,             true);
CVAR_BOOLF  (vid_shaderSmoothSelfOcclude,   true,   CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderLightingQuality,     0,      CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderFalloffQuality,      0,      CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderFogQuality,          0,      CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderPartialPrecision,    false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderLegacyCompiler,      false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderPrecache,            true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderCRC,                 true,   CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shaderAmbientOcclusion,    false);
CVAR_BOOL   (vid_shaderGroundAmbient,       true);
CVAR_BOOLF  (vid_shaderTexkill,             false,  CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shaderTexkillColorOnly,    false);
CVAR_BOOL   (vid_shaderRXGBNormalmap,       true);

IDirect3DVertexDeclaration9 *g_pVertexDeclarations[NUM_VERTEX_TYPES + MAX_MESHES];

SVertexShaderSlot           g_aVertexShaderSlots[MAX_SHADERS];
SPixelShaderSlot            g_aPixelShaderSlots[MAX_SHADERS];

int                         g_iMaxVertexShader;
int                         g_iMaxPixelShader;

ResHandle                   g_hNullMeshVS, g_hNullMeshPS;

CMaterial                   g_SimpleMaterial3D(TSNULL, _T("simple_material"));
CMaterial                   g_SimpleMaterial3DLit(TSNULL, _T("simple_material_lit"));
CMaterial                   g_SimpleMaterial3DColored(TSNULL, _T("simple_material_colored"));
CMaterial                   g_SimpleMaterial3DColoredBias(TSNULL, _T("simple_material_colored_bias"));
CMaterial                   g_MaterialGUI(TSNULL, _T("gui_material"));
CMaterial                   g_MaterialGUIGrayScale(TSNULL, _T("gui_grayscale_material"));
CMaterial                   g_MaterialGUIBlur(TSNULL, _T("gui_blur_material"));

VertexDeclarationMap        g_mapVertexDeclarations;

SPixelShaderVersion g_PixelShaderVersions[] =
{
    { _T("ps_3_0"), 3, 0 },
    { _T("ps_2_0"), 2, 0 },
    { _T("ps_1_4"), 1, 4 },
    { _T("ps_1_1"), 1, 1 },
    { _T(""), 0, 0 }
};

SVertexShaderVersion g_VertexShaderVersions[] =
{
    { _T("vs_3_0"), 3, 0 },
    { _T("vs_2_0"), 2, 0 },
    { _T("vs_1_1"), 1, 1 },
    { _T(""), 0, 0 }
};

uint g_auiMaxAniso[NUM_TEXTUREFILTERING_MODES] =
{
    1,
    1,
    1,
    2,
    4,
    6,
    8,
    12,
    16,
    32
};
//=============================================================================


/*====================
  D3D_RegisterVertexDeclaration
  ====================*/
int     D3D_RegisterVertexDeclaration(dword fvf)
{
    if (g_pd3dDevice == NULL)
        return -1;

    // See if this vertex declaration is already declared
    VertexDeclarationMap::iterator findit = g_mapVertexDeclarations.find(fvf);

    if (findit != g_mapVertexDeclarations.end())
        return findit->second; // return the instance that's already in memory

    int i;

    // Find empty decl slot
    for (i = NUM_VERTEX_TYPES; i < NUM_VERTEX_TYPES + MAX_MESHES; ++i)
    {
        if (g_pVertexDeclarations[i] == NULL)
            break;
    }

    if (i == NUM_VERTEX_TYPES + MAX_MESHES)
        return -1;

    // WORD    Stream;     // Stream index
    // WORD    Offset;     // Offset in the stream in bytes
    // BYTE    Type;       // Data type
    // BYTE    Method;     // Processing method
    // BYTE    Usage;      // Semantics
    // BYTE    UsageIndex; // Semantic index

    int d = 0;
    int iOffset = 0;


    D3DVERTEXELEMENT9 decl[13];

    //
    // Position
    //

    if (fvf & D3DFVF_XYZ)
    {
        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = D3DDECLTYPE_FLOAT3;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_POSITION;
        decl[d].UsageIndex = 0;
        ++d;
        iOffset += 12;
    }

    //
    // Normal
    //

    if (fvf & D3DFVF_NORMAL)
    {
        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = D3DDECLTYPE_FLOAT3;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_NORMAL;
        decl[d].UsageIndex = 0;
        ++d;
        iOffset += 12;
    }

    //
    // 4 Byte Normal
    //

    if (fvf & D3DFVF_NORMAL4B)
    {
        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = D3DDECLTYPE_UBYTE4;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_NORMAL;
        decl[d].UsageIndex = 0;
        ++d;
        iOffset += 4;
    }

    //
    // Color0
    //

    if (fvf & D3DFVF_DIFFUSE)
    {
        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = D3DDECLTYPE_D3DCOLOR;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_COLOR;
        decl[d].UsageIndex = 0;
        ++d;
        iOffset += 4;
    }

    //
    // Color1
    //

    if (fvf & D3DFVF_SPECULAR)
    {
        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = D3DDECLTYPE_D3DCOLOR;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_COLOR;
        decl[d].UsageIndex = 1;
        ++d;
        iOffset += 4;
    }

    //
    // TexcoordX
    //
    int iNumTexcoords = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;

    for (int n = 0; n < iNumTexcoords; ++n)
    {
        int eType = (fvf >> (n*2 + 16)) & 0x3;
        int iType;
        int iSize;

        switch (eType)
        {
        case D3DFVF_TEXTUREFORMAT1:
            iType = D3DDECLTYPE_UBYTE4;
            iSize = 4;
            break;
        default:
        case D3DFVF_TEXTUREFORMAT2:
            iType = D3DDECLTYPE_FLOAT2;
            iSize = 8;
            break;
        case D3DFVF_TEXTUREFORMAT3:
            iType = D3DDECLTYPE_FLOAT3;
            iSize = 12;
            break;
        case D3DFVF_TEXTUREFORMAT4:
            iType = D3DDECLTYPE_FLOAT4;
            iSize = 16;
            break;
        }

        decl[d].Stream = 0;
        decl[d].Offset = iOffset;
        decl[d].Type = iType;
        decl[d].Method = D3DDECLMETHOD_DEFAULT;
        decl[d].Usage = D3DDECLUSAGE_TEXCOORD;
        decl[d].UsageIndex = n;
        ++d;
        iOffset += iSize;
    }

    //
    // Terminator
    //

    decl[d].Stream = 0xff;
    decl[d].Offset = 0;
    decl[d].Type = D3DDECLTYPE_UNUSED;
    decl[d].Method = 0;
    decl[d].Usage = 0;
    decl[d].UsageIndex = 0;

    g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[i]);

    g_mapVertexDeclarations[fvf] = i;

    return i;
}


/*====================
  CInclude
  ====================*/
class CInclude : public ID3DXInclude
{
public:
    HRESULT WINAPI Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        string sTemp(pFileName);
        tstring sFileName(StringToTString(sTemp));
        CFileHandle hFile(sFileName, FILE_READ | FILE_BINARY | FILE_ALLOW_CUSTOM);

        if (!hFile.IsOpen())
        {
            Console.Warn << "Failed to open " << QuoteStr(pFileName) << newl;
            return E_FAIL;
        }

        uint uiBufferSize;
        const char *pBuffer = hFile.GetBuffer(uiBufferSize);

        byte *pData = K2_NEW_ARRAY(ctx_D3D9, byte, uiBufferSize);
        if (!pData)
            return E_OUTOFMEMORY;

        MemManager.Copy(pData, pBuffer, uiBufferSize);

        *ppData = pData;
        *pBytes = uiBufferSize;

        return S_OK;
    }

    HRESULT WINAPI Close(THIS_ LPCVOID pData)
    {
        K2_DELETE_ARRAY(pData);

        return S_OK;
    }
} g_ShaderInclude;


/*====================
  CIncludeSystem
  ====================*/
class CIncludeSystem : public ID3DXInclude
{
public:
    HRESULT WINAPI Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        TCHAR szOldDir[_MAX_PATH];
        GetCurrentDirectory(_MAX_PATH, szOldDir);

        char szFullPathName[_MAX_PATH];
        char *szFilePart;

        SetCurrentDirectory(FileManager.GetSystemPath(FileManager.GetWorkingDirectory()).c_str());

        if (GetFullPathNameA(pFileName, _MAX_PATH, szFullPathName, &szFilePart) == 0)
        {
            Console.Warn << "CIncludeSystem - GetFullPathName failed on " << QuoteStr(pFileName) << newl;

            SetCurrentDirectory(szOldDir);
            return E_FAIL;
        }

        string sTemp(szFullPathName);
        tstring sFileName(StringToTString(sTemp));
        CFileHandle hFile(FileManager.GetGamePath(sFileName), FILE_READ | FILE_BINARY);

        if (!hFile.IsOpen())
        {
            Console.Warn << "CIncludeSystem - Failed to open " << QuoteStr(pFileName) << newl;

            SetCurrentDirectory(szOldDir);
            return E_FAIL;
        }

        uint uiBufferSize;
        const char *pBuffer = hFile.GetBuffer(uiBufferSize);

        byte *pData = K2_NEW_ARRAY(ctx_D3D9, byte, uiBufferSize);
        if (!pData)
            return E_OUTOFMEMORY;

        MemManager.Copy(pData, pBuffer, uiBufferSize);

        *ppData = pData;
        *pBytes = uiBufferSize;

        SetCurrentDirectory(szOldDir);

        return S_OK;
    }

    HRESULT WINAPI Close(THIS_ LPCVOID pData)
    {
        K2_DELETE_ARRAY(pData);

        return S_OK;
    }
} g_ShaderIncludeSystem;


/*====================
  CVertexShaderFileCallback
  ====================*/
class CVertexShaderFileCallback : public IFileChangeCallback
{
private:
    int     m_iShaderIndex;

public:
    ~CVertexShaderFileCallback() {}
    CVertexShaderFileCallback(const tstring &sPath, int iShaderIndex) :
    IFileChangeCallback(sPath),
    m_iShaderIndex(iShaderIndex)
    {
    }

    void    Execute()
    {
        D3D_FreeVertexShader(m_iShaderIndex);
    }
};


/*====================
  D3D_LoadVertexShader
  ====================*/
bool    D3D_LoadVertexShader(const tstring &sName, const tstring &sFunction, int &iIndex)
{
    PROFILE("D3D_LoadVertexShader");

    DWORD dwShaderFlags = 0;

    if (vid_shaderLegacyCompiler)
        dwShaderFlags |= D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;

    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif

    if (vid_shaderPartialPrecision)
        dwShaderFlags |= D3DXSHADER_PARTIALPRECISION;

    if (iIndex == -1)
    {
        // Find empty vertex shader slot
        for (iIndex = NUM_VERTEX_TYPES; iIndex < MAX_SHADERS; ++iIndex)
        {
            if (!g_aVertexShaderSlots[iIndex].bActive)
                break;
        }

        if (iIndex == MAX_SHADERS)
            return false;
    }

    // Search for the best shader we can use
    int i;

    for (i = 0; ; ++i)
    {
        if (D3DVS_VERSION(g_VertexShaderVersions[i].major, g_VertexShaderVersions[i].minor) <= g_DeviceCaps.dwMaxVertexShaderVersion)
        {
            if (FileManager.Exists(_TS("/core/shaders/") + g_VertexShaderVersions[i].sName + _T("/") + sName + _T(".vsh"), FILE_NOARCHIVES))
                break;
        }

        if (g_VertexShaderVersions[i].major == 0 && g_VertexShaderVersions[i].minor == 0)
        {
            Console.Warn << _T("Vertex shader ") << QuoteStr(sName) << _T(" doesn't exist") << newl;
            return false;
        }
    }

    // Bump shader highest supported version
#if 1
    LPCSTR szProfile(D3DXGetVertexShaderProfile(g_pd3dDevice));
#else
    LPCSTR szProfile("vs_2_0");
#endif

    tstring sProfile;
    StrToTString(sProfile, szProfile);

    tstring sDir = _TS("/core/shaders/") + g_VertexShaderVersions[i].sName + _T("/");
    tstring sFilename = sDir + sName + _T(".vsh");

    CBufferDynamic cBuffer;
    ID3DXBuffer *pCode(NULL);
    uint uiCRC32(0);

    if (g_ShaderCache.LoadShader(sFilename, cBuffer, uiCRC32))
    {
        // See if our CRC matches any loaded shader
        for (int i(NUM_VERTEX_TYPES); i < g_iMaxVertexShader; ++i)
        {
            if (!g_aVertexShaderSlots[i].bActive)
                continue;

            if (g_aVertexShaderSlots[i].uiCRC32 == uiCRC32)
            {
                Console.Video << _T("Skipping duplicate vertex shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;
                iIndex = i;
                return true;
            }
        }

        Console.Video << _T("Loading cached vertex shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;

        const char *pBuffer(cBuffer.Get(0));
        uint uiSize(cBuffer.GetLength());

        if (FAILED(D3DXCreateBuffer(uiSize, &pCode)))
        {
            Console.Err << _T("D3D_LoadVertexShader: D3DXCreateBuffer failed") << newl;
            return false;
        }

        MemManager.Copy(pCode->GetBufferPointer(), pBuffer, uiSize);
    }
    else
    {
        Console.Video << _T("Compiling vertex shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;
        
        ID3DXBuffer *pErrorMsgs = NULL;

        D3DXMACRO   *pMacros = g_ShaderPreprocessor.AllocMacroArray();

        {
            PROFILE("Compile");

            if (vid_shaderDebug)
            {
                // Use FromFile so we get proper error/warning messages with line numbers
                tstring sOldDir = FileManager.GetWorkingDirectory();
                FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

                // Assemble the pixel shader from the file
                char szFuncName[1024];
                MemManager.Set(szFuncName, 0, 1024);
                TCHARToSingle(szFuncName, 1023, sFunction.c_str(), 1023);

                char szProfileName[1024];
                MemManager.Set(szProfileName, 0, 1024);
                TCHARToSingle(szProfileName, 1023, g_VertexShaderVersions[i].sName.c_str(), 1023);

                const tstring &sSystemPath(FileManager.GetSystemPath(sFilename));

                D3DXCompileShaderFromFile(sSystemPath.c_str(), pMacros, &g_ShaderIncludeSystem, szFuncName, szProfile, dwShaderFlags, &pCode, &pErrorMsgs, NULL);

                FileManager.SetWorkingDirectory(sOldDir);
            }
            else
            {
                tstring sOldDir = FileManager.GetWorkingDirectory();
                FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

                CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | FILE_ALLOW_CUSTOM);

                if (!hFile.IsOpen())
                {
                    FileManager.SetWorkingDirectory(sOldDir);
                    Console.Warn << _T("Failed to open ") << QuoteStr(sFilename) << newl;
                    return false;
                }

                uint uiBufferSize;
                const char *pBuffer = hFile.GetBuffer(uiBufferSize);

                char szFuncName[1024];
                MemManager.Set(szFuncName, 0, 1024);
                TCHARToSingle(szFuncName, 1023, sFunction.c_str(), 1023);

                char szProfileName[1024];
                MemManager.Set(szProfileName, 0, 1024);
                TCHARToSingle(szProfileName, 1023, g_VertexShaderVersions[i].sName.c_str(), 1023);

                D3DXCompileShader(pBuffer, uiBufferSize, pMacros, &g_ShaderInclude, szFuncName, szProfile, dwShaderFlags, &pCode, &pErrorMsgs, NULL);

                FileManager.SetWorkingDirectory(sOldDir);
            }
        }

        K2_DELETE_ARRAY(pMacros);

        if (!pCode)
        {
            if (pErrorMsgs)
                Console.Err << static_cast<char *>(pErrorMsgs->GetBufferPointer());
            else
                Console.Err << sFilename << _T(" : ") _T("error X0000: ") _T("Unknown error") << newl;

            SAFE_RELEASE(pErrorMsgs);
            return false;
        }

        SAFE_RELEASE(pErrorMsgs);

        uiCRC32 = M_GetCRC32((byte *)pCode->GetBufferPointer(), pCode->GetBufferSize());

        g_ShaderCache.CacheShader(sFilename, (byte *)pCode->GetBufferPointer(), pCode->GetBufferSize(), uiCRC32);
    }

    if (vid_shaderDebugOutputDisasm && pCode)
    {
        ID3DXBuffer *pDisasmCode = NULL;
        D3DXDisassembleShader((DWORD*)pCode->GetBufferPointer(), FALSE, NULL, &pDisasmCode);
        const char *szCode = (const char *)pDisasmCode->GetBufferPointer();

        CFileHandle hOutfile(_TS("~") + sFilename + _T(".") + XtoA(g_ShaderRegistry.GenerateVertexShaderKey(0) / MAX_SHADER_SLOTS) + _T(".txt"), FILE_WRITE | FILE_TEXT);

        hOutfile << g_ShaderPreprocessor.GetDefinitionHeaderString();
        hOutfile << szCode;

        SAFE_RELEASE(pDisasmCode);
    }

    // See if our CRC matches any loaded shader
    if (pCode && vid_shaderCRC)
    {       
        for (int i(NUM_VERTEX_TYPES); i < g_iMaxVertexShader; ++i)
        {
            if (!g_aVertexShaderSlots[i].bActive)
                continue;

            if (g_aVertexShaderSlots[i].uiCRC32 == uiCRC32)
            {
                iIndex = i;
                SAFE_RELEASE(pCode);
                return true;
            }
        }
        
        g_aVertexShaderSlots[iIndex].uiCRC32 = uiCRC32;
    }

    if (pCode)
    {
        PROFILE("CreateVertexShader");

        if (FAILED(g_pd3dDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &g_aVertexShaderSlots[iIndex].pShader)))
        {
            Console.Err << _T("D3D_LoadVertexShader: CreateVertexShader failed") << newl;
            return false;
        }

        if (FAILED(D3DXGetShaderConstantTableEx((DWORD*)pCode->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &g_aVertexShaderSlots[iIndex].pConstantTable)))
        {
            Console.Err << _T("D3D_LoadVertexShader: D3DXGetShaderConstantTable failed") << newl;
            return false;
        }
    }
    else
    {
        Console.Err << _T("D3D_LoadVertexShader: unknown error") << newl;
        return false;
    }

    SAFE_RELEASE(pCode);

    //
    // Vertex Shader Parameters
    //

    g_aVertexShaderSlots[iIndex].uiNumRegisters = 0;

    D3DXCONSTANTTABLE_DESC tableDesc;

    g_aVertexShaderSlots[iIndex].pConstantTable->GetDesc(&tableDesc);

    for (UINT i(0); i < tableDesc.Constants; ++i)
    {
        D3DXHANDLE hHandle(g_aVertexShaderSlots[iIndex].pConstantTable->GetConstant(NULL, i));

        if (hHandle)
        {
            g_aVertexShaderSlots[iIndex].vConstant.push_back(SVertexShaderConstant());
            SVertexShaderConstant &cConstant(g_aVertexShaderSlots[iIndex].vConstant.back());

            D3DXCONSTANT_DESC constantDesc;
            UINT iCount(1);

            g_aVertexShaderSlots[iIndex].pConstantTable->GetConstantDesc(hHandle, &constantDesc, &iCount);

            tstring sName(StringToTString(string(constantDesc.Name)));
            
            cConstant.hHandle = hHandle;
            cConstant.sName = sName;
            cConstant.pShaderVar = CShaderVarRegistry::GetInstance()->GetShaderVar(sName);
            cConstant.uiRegisterIndex = constantDesc.RegisterIndex;
            cConstant.uiSize = constantDesc.RegisterCount / constantDesc.Elements;

            g_aVertexShaderSlots[iIndex].uiNumRegisters += constantDesc.RegisterCount;
        }
    }

    g_aVertexShaderSlots[iIndex].uiNumConstants = uint(g_aVertexShaderSlots[iIndex].vConstant.size());
    g_aVertexShaderSlots[iIndex].bActive = true;
    g_iMaxVertexShader = MAX(g_iMaxVertexShader, iIndex + 1);

    g_aVertexShaderSlots[iIndex].pFileCallback = K2_NEW(ctx_D3D9,   CVertexShaderFileCallback)(sFilename, iIndex);
    g_ResourceManager.RegisterFileChangeCallback(g_aVertexShaderSlots[iIndex].pFileCallback);

    return true;
}


/*====================
  D3D_FreeVertexShader
  ====================*/
void    D3D_FreeVertexShader(int iShaderIndex)
{
    if (!g_aVertexShaderSlots[iShaderIndex].bActive)
        return;

    if (D3D_GetVertexShader() && D3D_GetVertexShader() == g_aVertexShaderSlots[iShaderIndex].pShader)
        D3D_SetVertexShader(NULL);

    g_aVertexShaderSlots[iShaderIndex].bActive = false;
    SAFE_RELEASE(g_aVertexShaderSlots[iShaderIndex].pShader);
    SAFE_RELEASE(g_aVertexShaderSlots[iShaderIndex].pConstantTable);
    g_aVertexShaderSlots[iShaderIndex].vConstant.clear();

    g_ResourceManager.UnregisterFileChangeCallback(g_aVertexShaderSlots[iShaderIndex].pFileCallback);
    SAFE_DELETE(g_aVertexShaderSlots[iShaderIndex].pFileCallback);

    g_ShaderRegistry.FreeVertexShaderInstance(iShaderIndex);
}


/*====================
  CPixelShaderFileCallback
  ====================*/
class CPixelShaderFileCallback : public IFileChangeCallback
{
private:
    int     m_iShaderIndex;

public:
    ~CPixelShaderFileCallback() {}
    CPixelShaderFileCallback(const tstring &sPath, int iShaderIndex) :
    IFileChangeCallback(sPath),
    m_iShaderIndex(iShaderIndex)
    {
    }

    void    Execute()
    {
        D3D_FreePixelShader(m_iShaderIndex);
    }
};


/*====================
  D3D_LoadPixelShader
  ====================*/
bool    D3D_LoadPixelShader(const tstring &sName, const tstring &sFunction, int &iIndex)
{
    PROFILE("D3D_LoadPixelShader");

    DWORD dwShaderFlags = 0;

    if (vid_shaderLegacyCompiler)
        dwShaderFlags |= D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;

    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    if (vid_shaderPartialPrecision)
        dwShaderFlags |= D3DXSHADER_PARTIALPRECISION;

    if (iIndex == -1)
    {
        // Find empty shader slot
        for (iIndex = NUM_VERTEX_TYPES; iIndex < MAX_SHADERS; ++iIndex)
        {
            if (!g_aPixelShaderSlots[iIndex].bActive)
                break;
        }

        if (iIndex == MAX_SHADERS)
            return false;
    }

    // Search for the best shader we can use
    int i;

    for (i = 0; ; ++i)
    {
        if (D3DPS_VERSION(g_PixelShaderVersions[i].major, g_PixelShaderVersions[i].minor) <= g_DeviceCaps.dwMaxPixelShaderVersion)
        {
            if (FileManager.Exists(_TS("/core/shaders/") + g_PixelShaderVersions[i].sName + _T("/") + sName + _T(".psh"), FILE_NOARCHIVES))
                break;
        }

        if (g_PixelShaderVersions[i].major == 0 && g_PixelShaderVersions[i].minor == 0)
        {
            Console.Warn << _T("Pixel shader ") << QuoteStr(sName) << _T(" doesn't exist") << newl;
            return false;
        }
    }

    // Bump shader highest supported version
#if 1
    LPCSTR szProfile(D3DXGetPixelShaderProfile(g_pd3dDevice));
#else
    LPCSTR szProfile("ps_2_0");
#endif

    tstring sProfile;
    StrToTString(sProfile, szProfile);

    tstring sDir = _TS("/core/shaders/") + g_PixelShaderVersions[i].sName + _T("/");
    tstring sFilename = sDir + sName + _T(".psh");

    CBufferDynamic cBuffer;
    ID3DXBuffer *pCode(NULL);
    uint uiCRC32(0);

    if (g_ShaderCache.LoadShader(sFilename, cBuffer, uiCRC32))
    {
        // See if our CRC matches any loaded shader
        for (int i(NUM_VERTEX_TYPES); i < g_iMaxPixelShader; ++i)
        {
            if (!g_aPixelShaderSlots[i].bActive)
                continue;

            if (g_aPixelShaderSlots[i].uiCRC32 == uiCRC32)
            {
                Console.Video << _T("Skipping duplicate pixel shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;
                iIndex = i;
                return true;
            }
        }

        Console.Video << _T("Loading cached pixel shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;

        const char *pBuffer(cBuffer.Get());
        uint uiSize(cBuffer.GetLength());

        if (FAILED(D3DXCreateBuffer(uiSize, &pCode)))
        {
            Console.Err << _T("D3D_LoadPixelShader: D3DXCreateBuffer failed") << newl;
            return false;
        }

        MemManager.Copy(pCode->GetBufferPointer(), pBuffer, uiSize);
    }
    else
    {
        Console.Video << _T("Compiling pixel shader: ") << Filename_StripPath(sFilename) << _T(" (") << sProfile << _T(")") << newl;
        
        ID3DXBuffer *pErrorMsgs = NULL;

        D3DXMACRO   *pMacros = g_ShaderPreprocessor.AllocMacroArray();

        {
            PROFILE("Compile");

            if (vid_shaderDebug)
            {
                // Use FromFile so we get proper error/warning messages with line numbers
                tstring sOldDir = FileManager.GetWorkingDirectory();
                FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

                // Assemble the pixel shader from the file
                char szFuncName[1024];
                MemManager.Set(szFuncName, 0, 1024);
                TCHARToSingle(szFuncName, 1023, sFunction.c_str(), 1023);

                char szProfileName[1024];
                MemManager.Set(szProfileName, 0, 1024);
                TCHARToSingle(szProfileName, 1023, g_PixelShaderVersions[i].sName.c_str(), 1023);

                D3DXCompileShaderFromFile(FileManager.GetSystemPath(sFilename).c_str(), pMacros, &g_ShaderIncludeSystem, szFuncName, szProfile, dwShaderFlags, &pCode, &pErrorMsgs, NULL);

                FileManager.SetWorkingDirectory(sOldDir);
            }
            else
            {
                tstring sOldDir = FileManager.GetWorkingDirectory();
                FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

                CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | FILE_ALLOW_CUSTOM);

                if (!hFile.IsOpen())
                {
                    FileManager.SetWorkingDirectory(sOldDir);
                    Console.Warn << _T("Failed to open ") << QuoteStr(sFilename) << newl;
                    return false;
                }

                uint uiBufferSize;
                const char *pBuffer = hFile.GetBuffer(uiBufferSize);

                char szFuncName[1024];
                MemManager.Set(szFuncName, 0, 1024);
                TCHARToSingle(szFuncName, 1023, sFunction.c_str(), 1023);

                char szProfileName[1024];
                MemManager.Set(szProfileName, 0, 1024);
                TCHARToSingle(szProfileName, 1023, g_PixelShaderVersions[i].sName.c_str(), 1023);

                D3DXCompileShader(pBuffer, uiBufferSize, pMacros, &g_ShaderInclude, szFuncName, szProfile, dwShaderFlags, &pCode, &pErrorMsgs, NULL);

                FileManager.SetWorkingDirectory(sOldDir);
            }
        }

        K2_DELETE_ARRAY(pMacros);

        if (!pCode)
        {
            if (pErrorMsgs)
                Console.Err << static_cast<char *>(pErrorMsgs->GetBufferPointer());
            else
                Console.Err << sFilename << _T(" : ") _T("error X0000: ") _T("Unknown error") << newl;

            SAFE_RELEASE(pErrorMsgs);

            return false;
        }

        SAFE_RELEASE(pErrorMsgs);

        uiCRC32 = M_GetCRC32((byte *)pCode->GetBufferPointer(), pCode->GetBufferSize());

        g_ShaderCache.CacheShader(sFilename, (byte *)pCode->GetBufferPointer(), pCode->GetBufferSize(), uiCRC32);
    }

    if (vid_shaderDebugOutputDisasm && pCode)
    {
        ID3DXBuffer *pDisasmCode = NULL;
        D3DXDisassembleShader((DWORD*)pCode->GetBufferPointer(), FALSE, NULL, &pDisasmCode);
        const char *szCode = (const char *)pDisasmCode->GetBufferPointer();

        CFileHandle hOutfile(_TS("~") + sFilename + _T(".") + XtoA(g_ShaderRegistry.GeneratePixelShaderKey(0) / MAX_SHADER_SLOTS) + _T(".txt"), FILE_WRITE | FILE_TEXT);

        hOutfile << g_ShaderPreprocessor.GetDefinitionHeaderString();
        hOutfile << szCode;

        SAFE_RELEASE(pDisasmCode);
    }

    if (vid_shaderDebugOutputBin && pCode)
    {
        CFileHandle hOutfile(_TS("~") + sFilename + _T(".") + XtoA(g_ShaderRegistry.GeneratePixelShaderKey(0) / MAX_SHADER_SLOTS) + _T(".bin"), FILE_WRITE | FILE_BINARY);

        hOutfile.Write((byte *)pCode->GetBufferPointer(), pCode->GetBufferSize());
    }

    if (vid_shaderDebugOutputAsm && pCode)
    {
        ID3DXBuffer *pDisasmCode = NULL;
        D3DXDisassembleShader((DWORD*)pCode->GetBufferPointer(), FALSE, NULL, &pDisasmCode);
        const char *szCode = (const char *)pDisasmCode->GetBufferPointer();

        CFileHandle hOutfile(_TS("~") + sFilename + _T(".") + XtoA(g_ShaderRegistry.GeneratePixelShaderKey(0) / MAX_SHADER_SLOTS) + _T(".ps"), FILE_WRITE | FILE_TEXT | FILE_ASCII);

        hOutfile << szCode;

        SAFE_RELEASE(pDisasmCode);
    }

    // See if our CRC matches any loaded shader
    if (pCode && vid_shaderCRC)
    {       
        for (int i(NUM_VERTEX_TYPES); i < g_iMaxPixelShader; ++i)
        {
            if (!g_aPixelShaderSlots[i].bActive)
                continue;

            if (g_aPixelShaderSlots[i].uiCRC32 == uiCRC32)
            {
                iIndex = i;
                SAFE_RELEASE(pCode);
                return true;
            }
        }
        
        g_aPixelShaderSlots[iIndex].uiCRC32 = uiCRC32;
    }

    if (pCode)
    {
        PROFILE("CreatePixelShader");

        if (FAILED(g_pd3dDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &g_aPixelShaderSlots[iIndex].pShader)))
        {
            Console.Err << _T("D3D_LoadPixelShader: CreatePixelShader failed") << newl;
            return true;
        }

        if (FAILED(D3DXGetShaderConstantTableEx((DWORD*)pCode->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &g_aPixelShaderSlots[iIndex].pConstantTable)))
        {
            Console.Err << _T("D3D_LoadPixelShader: D3DXGetShaderConstantTable failed") << newl;
            return false;
        }
    }
    else
    {
        Console.Err << _T("D3D_LoadPixelShader: unknown error") << newl;
        return false;
    }

    SAFE_RELEASE(pCode);

    //
    // Pixel Shader Parameters
    //

    g_aPixelShaderSlots[iIndex].uiNumRegisters = 0;

    D3DXCONSTANTTABLE_DESC tableDesc;

    g_aPixelShaderSlots[iIndex].pConstantTable->GetDesc(&tableDesc);

    for (UINT i = 0; i < tableDesc.Constants; ++i)
    {
        D3DXHANDLE hHandle = g_aPixelShaderSlots[iIndex].pConstantTable->GetConstant(NULL, i);

        if (hHandle)
        {
            g_aPixelShaderSlots[iIndex].vConstant.push_back(SPixelShaderConstant());
            SPixelShaderConstant &cConstant(g_aPixelShaderSlots[iIndex].vConstant.back());

            D3DXCONSTANT_DESC constantDesc;
            UINT iCount = 1;

            g_aPixelShaderSlots[iIndex].pConstantTable->GetConstantDesc(hHandle, &constantDesc, &iCount);

            tstring sName(StringToTString(string(constantDesc.Name)));

            if (sName.length() > 2 && sName[sName.length() - 2] == _T('_'))
            {
                cConstant.uiSubTexture = sName[sName.length() - 1] - _T('0');
                sName = sName.substr(0, sName.length() - 2);
            }
            else
                cConstant.uiSubTexture = 0;

            cConstant.hHandle = hHandle;
            cConstant.sName = sName;
            cConstant.pShaderVar = NULL;
            cConstant.pShaderSamplers = NULL;
            cConstant.eRegisterSet = constantDesc.RegisterSet;
            cConstant.uiRegisterIndex = constantDesc.RegisterIndex;
            cConstant.uiSize = constantDesc.RegisterCount / constantDesc.Elements;

            switch (constantDesc.RegisterSet)
            {
            case D3DXRS_SAMPLER:
                g_aPixelShaderSlots[iIndex].vConstant.back().pShaderSamplers = CShaderSamplerRegistry::GetInstance()->GetShaderSampler(sName);
                break;
            case D3DXRS_INT4:
            case D3DXRS_FLOAT4:
            case D3DXRS_BOOL:
                g_aPixelShaderSlots[iIndex].vConstant.back().pShaderVar = CShaderVarRegistry::GetInstance()->GetShaderVar(sName);
                g_aPixelShaderSlots[iIndex].uiNumRegisters += constantDesc.RegisterCount;
                break;
            }
        }
    }

    g_aPixelShaderSlots[iIndex].uiNumConstants = uint(g_aPixelShaderSlots[iIndex].vConstant.size());
    g_aPixelShaderSlots[iIndex].bActive = true;
    g_iMaxPixelShader = MAX(g_iMaxPixelShader, iIndex + 1);

    g_aPixelShaderSlots[iIndex].pFileCallback = K2_NEW(ctx_D3D9,   CPixelShaderFileCallback)(sFilename, iIndex);
    g_ResourceManager.RegisterFileChangeCallback(g_aPixelShaderSlots[iIndex].pFileCallback);

    return true;
}


/*====================
  D3D_FreePixelShader
  ====================*/
void    D3D_FreePixelShader(int iShaderIndex)
{
    if (!g_aPixelShaderSlots[iShaderIndex].bActive)
        return;

    if (D3D_GetPixelShader() && D3D_GetPixelShader() == g_aPixelShaderSlots[iShaderIndex].pShader)
        D3D_SetPixelShader(NULL);

    g_aPixelShaderSlots[iShaderIndex].bActive = false;
    SAFE_RELEASE(g_aPixelShaderSlots[iShaderIndex].pShader);
    SAFE_RELEASE(g_aPixelShaderSlots[iShaderIndex].pConstantTable);
    g_aPixelShaderSlots[iShaderIndex].vConstant.clear();

    g_ResourceManager.UnregisterFileChangeCallback(g_aPixelShaderSlots[iShaderIndex].pFileCallback);
    SAFE_DELETE(g_aPixelShaderSlots[iShaderIndex].pFileCallback);

    g_ShaderRegistry.FreePixelShaderInstance(iShaderIndex);
}


/*====================
  D3D_RegisterVertexShader
  ====================*/
int     D3D_RegisterVertexShader(CVertexShader *pVertexShader)
{
    ShaderHandle hShader(g_ShaderRegistry.RegisterVertexShader(pVertexShader->GetName(), pVertexShader->GetShaderFlags()));

    pVertexShader->SetIndex(hShader);

    return 0;
}


/*====================
  D3D_UnregisterVertexShader
  ====================*/
void    D3D_UnregisterVertexShader(CVertexShader *pVertexShader)
{
    g_ShaderRegistry.UnregisterVertexShader(pVertexShader->GetName());
}


/*====================
  D3D_RegisterPixelShader
  ====================*/
int     D3D_RegisterPixelShader(CPixelShader *pPixelShader)
{
    ShaderHandle hShader(g_ShaderRegistry.RegisterPixelShader(pPixelShader->GetName(), pPixelShader->GetShaderFlags()));

    pPixelShader->SetIndex(hShader);

    return 0;
}


/*====================
  D3D_UnregisterPixelShader
  ====================*/
void    D3D_UnregisterPixelShader(CPixelShader *pPixelShader)
{
    g_ShaderRegistry.UnregisterVertexShader(pPixelShader->GetName());
}


/*====================
  D3D_RegisterShaderPair
  ====================*/
void    D3D_RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader)
{   
}


/*====================
  D3D_InitShader
  ====================*/
void    D3D_InitShader()
{
    //
    // Shader Definitions
    //

    g_ShaderPreprocessor.Define("NUM_POINT_LIGHTS", TStringToString(XtoA(MAX_POINT_LIGHTS)));
    g_ShaderPreprocessor.Define("NUM_BONE_WEIGHTS", TStringToString(XtoA(ICvar::GetInteger(_T("geom_maxBoneWeights")))));
    g_ShaderPreprocessor.Define("LIGHTING_QUALITY", TStringToString(XtoA(vid_shaderLightingQuality)));
    g_ShaderPreprocessor.Define("FALLOFF_QUALITY", TStringToString(XtoA(vid_shaderFalloffQuality)));
    g_ShaderPreprocessor.Define("FOG_QUALITY", TStringToString(XtoA(vid_shaderFogQuality)));
    g_ShaderPreprocessor.Define("SHADOWMAP_TYPE", TStringToString(XtoA(vid_shadowmapType)));
    g_ShaderPreprocessor.Define("SHADOWMAP_FILTER_WIDTH", TStringToString(XtoA(vid_shadowmapFilterWidth)));

    if (vid_shaderSmoothSelfOcclude)
        g_ShaderPreprocessor.Define("SMOOTH_SELF_OCCLUDE");

    if (vid_treeSmoothNormals)
        g_ShaderPreprocessor.Define("SMOOTH_TREE_NORMALS");

    if (vid_specularLookup)
        g_ShaderPreprocessor.Define("SPECULAR_LOOKUP");

    if (vid_terrainDerepeat)
        g_ShaderPreprocessor.Define("TERRAIN_DEREPEAT");

    if (vid_terrainAlphamap)
        g_ShaderPreprocessor.Define("TERRAIN_ALPHAMAP");

    if (vid_shaderAmbientOcclusion)
        g_ShaderPreprocessor.Define("AMBIENT_OCCLUSION");

    if (vid_shaderGroundAmbient)
        g_ShaderPreprocessor.Define("GROUND_AMBIENT");

    if (vid_shaderPartialPrecision)
        g_ShaderPreprocessor.Define("PARTIAL_PRECISION");

    if (gfx_clouds)
        g_ShaderPreprocessor.Define("CLOUDS");

    if (vid_shadowFalloff)
        g_ShaderPreprocessor.Define("SHADOW_FALLOFF");

    if (vid_shaderRXGBNormalmap)
        g_ShaderPreprocessor.Define("RXGB_NORMALMAP");

    g_hCloudTexture = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CTexture)(gfx_cloudTexture, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);

    gfx_fogType.SetModified(false);
    vid_treeSmoothNormals.SetModified(false);
    gfx_clouds.SetModified(false);
    gfx_cloudTexture.SetModified(false);
    vid_terrainDerepeat.SetModified(false);
    vid_terrainAlphamap.SetModified(false);
    vid_shadows.SetModified(false);
    vid_shadowmapType.SetModified(false);
    vid_shadowmapSize.SetModified(false);
    vid_shadowFalloff.SetModified(false);
    vid_shaderLightingQuality.SetModified(false);
    vid_shaderFalloffQuality.SetModified(false);
    vid_shaderFogQuality.SetModified(false);
    vid_shaderSmoothSelfOcclude.SetModified(false);
    vid_sceneBuffer.SetModified(false);
    vid_sceneBufferMipmap.SetModified(false);
    vid_postEffects.SetModified(false);
    vid_shaderAmbientOcclusion.SetModified(false);
    vid_shaderGroundAmbient.SetModified(false);
    vid_shaderPartialPrecision.SetModified(false);

#if 0
    // Parse user shader definitions
    vector<StringPair>  vPairs;
    ParseDefinitions(vid_shaderDefinitions, vPairs);

    for (vector<StringPair>::iterator it = vPairs.begin(); it != vPairs.end(); ++it)
        g_ShaderPreprocessor.Define(TStringToString(it->first), TStringToString(it->second));
#endif

    g_iNumActivePointLights = 0;
    g_iNumActiveBones = 0;
    g_bLighting = true;
    g_iTexcoords = 1;
    g_bTexkill = false;

    //
    // Error Shaders
    //

    g_hNullMeshVS = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CVertexShader)(_T("simple"), 0), RES_VERTEX_SHADER);
    g_hNullMeshPS = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CPixelShader)(_T("simple"), 0), RES_PIXEL_SHADER);

    //
    // GUI Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_GUI]);
    }

    //
    // Foliage Vertex Declaration
    //

    {
#if 0
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            { 0, 48, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
            D3DDECL_END()
        };
#else
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 16, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            D3DDECL_END()
        };
#endif

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_FOLIAGE]);
    }

    //
    // Effect Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_EFFECT]);
    }

    //
    // Terrain Vertex Declaration
    //

    {
#if 0
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            { 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
            D3DDECL_END()
        };
#else
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 4, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 8, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 12, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            D3DDECL_END()
        };
#endif

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_TERRAIN]);
    }



    //
    // Position Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_POSITION]);
    }

    //
    // Line/Box Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_LINE]);
    }

    //
    // Skybox Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_SKYBOX]);
    }

    //
    // Extended Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 28, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_EXTENDED]);
    }

    //
    // Tree Billboard Vertex Declaration
    //

    {
        D3DVERTEXELEMENT9 decl[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        g_pd3dDevice->CreateVertexDeclaration(decl, &g_pVertexDeclarations[VERTEX_TREE_BILLBOARD]);
    }


    //
    // Setup "simple" 3D shader for lines, boxes, and such
    //
    {
        g_SimpleMaterial3D.SetGlossiness(16.0f);
        g_SimpleMaterial3D.SetSpecularLevel(1.0f);
        g_SimpleMaterial3D.SetBumpLevel(1.0f);
        g_SimpleMaterial3D.SetReflect(0.0f);
        g_SimpleMaterial3D.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_SimpleMaterial3D.SetOpacity(1.0f);

        //
        // Phase 0 (depth)
        //
        CMaterialPhase  phaseDepth(PHASE_DEPTH, _T("mesh_depth"), _T("mesh_depth"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
        g_SimpleMaterial3D.AddPhase(phaseDepth);

        //
        // Phase 1 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("simple"), _T("simple"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
        g_SimpleMaterial3D.AddPhase(phaseColor);
    }


    //
    // Setup "simple" 3D shader with lighting
    //
    {
        g_SimpleMaterial3DLit.SetGlossiness(16.0f);
        g_SimpleMaterial3DLit.SetSpecularLevel(1.0f);
        g_SimpleMaterial3DLit.SetBumpLevel(1.0f);
        g_SimpleMaterial3DLit.SetReflect(0.0f);
        g_SimpleMaterial3DLit.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_SimpleMaterial3DLit.SetOpacity(1.0f);

        //
        // Phase 0 (depth)
        //
        CMaterialPhase  phaseDepth(PHASE_DEPTH, _T("mesh_depth"), _T("mesh_depth"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
        g_SimpleMaterial3DLit.AddPhase(phaseDepth);

        //
        // Phase 1 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("simple_color_flat"), _T("simple_color_flat"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
        g_SimpleMaterial3DLit.AddPhase(phaseColor);
    }


    //
    // Setup "simple" 3D shader for lines, boxes, and such
    //
    {
        g_SimpleMaterial3DColored.SetGlossiness(16.0f);
        g_SimpleMaterial3DColored.SetSpecularLevel(1.0f);
        g_SimpleMaterial3DColored.SetBumpLevel(1.0f);
        g_SimpleMaterial3DColored.SetReflect(0.0f);
        g_SimpleMaterial3DColored.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_SimpleMaterial3DColored.SetOpacity(1.0f);

        //
        // Phase 0 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("line"), _T("line"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
        g_SimpleMaterial3DColored.AddPhase(phaseColor);
    }

    //
    // Setup biased 3D shader for lines, boxes, and such
    //
    {
        g_SimpleMaterial3DColoredBias.SetGlossiness(16.0f);
        g_SimpleMaterial3DColoredBias.SetSpecularLevel(1.0f);
        g_SimpleMaterial3DColoredBias.SetBumpLevel(1.0f);
        g_SimpleMaterial3DColoredBias.SetReflect(0.0f);
        g_SimpleMaterial3DColoredBias.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_SimpleMaterial3DColoredBias.SetOpacity(1.0f);

        //
        // Phase 0 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("line"), _T("line"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 2, 0, 0.0f, PHASE_TRANSLUCENT | PHASE_COLOR_WRITE | PHASE_ALPHA_WRITE);
        g_SimpleMaterial3DColoredBias.AddPhase(phaseColor);
    }

    //
    // Setup default GUI material
    //
    {
        g_MaterialGUI.SetGlossiness(16.0f);
        g_MaterialGUI.SetSpecularLevel(1.0f);
        g_MaterialGUI.SetBumpLevel(1.0f);
        g_MaterialGUI.SetReflect(0.0f);
        g_MaterialGUI.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_MaterialGUI.SetOpacity(1.0f);

        //
        // Phase 0 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("gui"), _T("gui"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
        CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
        phaseColor.AddSampler(samplerDiffuse);

        g_MaterialGUI.AddPhase(phaseColor);
    }

    
    //
    // Setup grayscale GUI material
    //
    {
        g_MaterialGUIGrayScale.SetGlossiness(16.0f);
        g_MaterialGUIGrayScale.SetSpecularLevel(1.0f);
        g_MaterialGUIGrayScale.SetBumpLevel(1.0f);
        g_MaterialGUIGrayScale.SetReflect(0.0f);
        g_MaterialGUIGrayScale.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_MaterialGUIGrayScale.SetOpacity(1.0f);

        //
        // Phase 0 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("gui"), _T("gui_grayscale"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
        CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
        phaseColor.AddSampler(samplerDiffuse);

        g_MaterialGUIGrayScale.AddPhase(phaseColor);
    }

    //
    // Setup blur GUI material
    //
    {
        g_MaterialGUIBlur.SetGlossiness(16.0f);
        g_MaterialGUIBlur.SetSpecularLevel(1.0f);
        g_MaterialGUIBlur.SetBumpLevel(1.0f);
        g_MaterialGUIBlur.SetReflect(0.0f);
        g_MaterialGUIBlur.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
        g_MaterialGUIBlur.SetOpacity(1.0f);

        //
        // Phase 0 (color)
        //
        CMaterialPhase  phaseColor(PHASE_COLOR, _T("gui"), _T("gui_blur"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
        CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
        phaseColor.AddSampler(samplerDiffuse);

        g_MaterialGUIBlur.AddPhase(phaseColor);
    }
}


bool    g_bReloadShaders(false);

/*====================
  D3D_UpdateDefineCvarInt
  ====================*/
void    D3D_UpdateDefineCvarInt(ICvar *pCvar, const string &sDefine)
{
    if (pCvar->IsModified())
    {
        pCvar->SetModified(false);

        g_ShaderPreprocessor.Define(sDefine, TStringToString(pCvar->GetString()));

        g_bReloadShaders = true;
    }
}


/*====================
  D3D_UpdateDefineCvarBool
  ====================*/
void    D3D_UpdateDefineCvarBool(CCvar<bool> *pCvar, const string &sDefine)
{
    if (pCvar->IsModified())
    {
        pCvar->SetModified(false);

        if (*pCvar)
            g_ShaderPreprocessor.Define(sDefine);
        else
            g_ShaderPreprocessor.Undefine(sDefine);

        g_bReloadShaders = true;
    }
}


/*====================
  D3D_FrameShader
  ====================*/
void    D3D_FrameShader()
{
    for (dword i(0); i < g_dwNumSamplers; ++i)
        D3D_SetSamplerState(i, D3DSAMP_MAXANISOTROPY, g_auiMaxAniso[CLAMP<int>(vid_textureFiltering, 0, NUM_TEXTUREFILTERING_MODES - 1)]);

    if (vid_shadows.IsModified() ||
        vid_shadowmapSize.IsModified() ||
        vid_shadowmapType.IsModified())
    {
        vid_shadows.SetModified(false);
        vid_shadowmapSize.SetModified(false);
        vid_shadowmapType.SetModified(false);

        g_bReloadShaders = true;

        g_Shadowmap.Release();
        g_Shadowmap.Initialize();
    }

    if (vid_reflections.IsModified())
    {
        vid_reflections.SetModified(false);

        g_bReloadShaders = true;

        g_ReflectionMap.Release();
        g_ReflectionMap.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
    }

    if (vid_sceneBuffer.IsModified())
    {
        vid_sceneBuffer.SetModified(false);

        g_bReloadShaders = true;

        g_SceneBuffer.Release();
        g_SceneBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
    }

    if (vid_sceneBufferMipmap.IsModified())
    {
        vid_sceneBufferMipmap.SetModified(false);

        g_SceneBuffer.Release();
        g_SceneBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
    }

    if (vid_postEffects.IsModified())
    {
        vid_postEffects.SetModified(false);

        g_PostBuffer.Release();
        g_PostBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
    }

    if (vid_motionBlur.IsModified())
    {
        vid_motionBlur.SetModified(false);

        g_bReloadShaders = true;

        g_VelocityMap.Release();
        g_VelocityMap.Initialize();
    }

    if (gfx_cloudTexture.IsModified())
    {
        gfx_cloudTexture.SetModified(false);

        g_hCloudTexture = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CTexture)(gfx_cloudTexture, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    D3D_UpdateDefineCvarBool(&vid_treeSmoothNormals, "SMOOTH_TREE_NORMALS");
    D3D_UpdateDefineCvarBool(&vid_terrainDerepeat, "TERRAIN_DEREPEAT");
    D3D_UpdateDefineCvarBool(&vid_terrainAlphamap, "TERRAIN_ALPHAMAP");
    D3D_UpdateDefineCvarInt(&vid_shadowmapFilterWidth, "SHADOWMAP_FILTER_WIDTH");
    D3D_UpdateDefineCvarBool(&gfx_clouds,"CLOUDS");
    D3D_UpdateDefineCvarBool(&vid_shadowFalloff, "SHADOW_FALLOFF");
    D3D_UpdateDefineCvarInt(&vid_shaderLightingQuality, "LIGHTING_QUALITY");
    D3D_UpdateDefineCvarInt(&vid_shaderFalloffQuality, "FALLOFF_QUALITY");
    D3D_UpdateDefineCvarInt(&vid_shaderFogQuality, "FOG_QUALITY");
    D3D_UpdateDefineCvarBool(&vid_shaderSmoothSelfOcclude, "SMOOTH_SELF_OCCLUDE");
    D3D_UpdateDefineCvarInt(&gfx_fogType, "FOG_TYPE");
    D3D_UpdateDefineCvarBool(&vid_shaderAmbientOcclusion, "AMBIENT_OCCLUSION");
    D3D_UpdateDefineCvarBool(&vid_shaderGroundAmbient, "GROUND_AMBIENT");
    D3D_UpdateDefineCvarBool(&vid_shaderPartialPrecision, "PARTIAL_PRECISION");

    if (g_bReloadShaders)
    {
        g_bReloadShaders = false;
        g_ShaderRegistry.ReloadShaders();
    }
}


/*====================
  D3D_DestroyShader
  ====================*/
void    D3D_DestroyShader()
{
    g_mapTextures.clear();
    g_mapVertexDeclarations.clear();

    for (int i = 0; i < NUM_VERTEX_TYPES + MAX_MESHES; ++i)
    {
        SAFE_RELEASE(g_pVertexDeclarations[i]);
    }

    for (int i = 0; i < MAX_SHADERS; ++i)
    {
        SAFE_RELEASE(g_aVertexShaderSlots[i].pShader);
        SAFE_RELEASE(g_aPixelShaderSlots[i].pShader);
        SAFE_RELEASE(g_aVertexShaderSlots[i].pConstantTable);
        SAFE_RELEASE(g_aPixelShaderSlots[i].pConstantTable);
    }

    for (int n = 0; n < MAX_TEXTURES; ++n)
    {
        SAFE_RELEASE(g_pTextures2D[n]);
        SAFE_RELEASE(g_pTexturesCube[n]);
        SAFE_RELEASE(g_pTexturesVolume[n]);
        g_pTextures[n] = NULL;
    }
}


/*====================
  D3D_ShutdownShader
  ====================*/
void    D3D_ShutdownShader()
{
    D3D_DestroyShader();

    for (int i = 0; i < MAX_SHADERS; ++i)
    {
        g_ResourceManager.UnregisterFileChangeCallback(g_aVertexShaderSlots[i].pFileCallback);
        SAFE_DELETE(g_aVertexShaderSlots[i].pFileCallback);

        g_ResourceManager.UnregisterFileChangeCallback(g_aPixelShaderSlots[i].pFileCallback);
        SAFE_DELETE(g_aPixelShaderSlots[i].pFileCallback);
    }
}

