// (C)2008 S2 Games
// c_gfxshaders.cpp
//
// Shaders
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include "c_gfxshaders.h"

#include "c_gfxmaterials.h"
#include "c_gfxshaders.h"
#include "c_glslpreprocessor.h"
#include "c_proceduralregistry.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_shadervar.h"
#include "c_shadervarregistry.h"
#include "c_shadersamplerregistry.h"
#include "c_shadowmap.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"

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

SINGLETON_INIT(CGfxShaders)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxShaders *GfxShaders(CGfxShaders::GetInstance());

SVertexShaderSlot           g_aVertexShaderSlots[MAX_SHADERS];
SPixelShaderSlot            g_aPixelShaderSlots[MAX_SHADERS];
SShaderProgramSlot          g_aShaderProgramSlots[MAX_SHADERS];

ResHandle                   g_hNullMeshVS, g_hNullMeshPS;
ResHandle                   g_hCloudTexture;

VertexDeclarationMap        g_mapVertexDeclarations;

int g_aiMaxAniso[] =
{
    1,
    1,
    1,
    2,
    4,
    6,
    8,
    12,
    16
};
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_STRING (vid_shaderDefinitions,         "");
CVAR_BOOLF  (vid_specularLookup,            false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderDebug,               false,  CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shaderDebugOutputDisasm,   false);
CVAR_BOOLF  (vid_treeSmoothNormals,         false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_terrainDerepeat,           true,   CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowFalloff,             true);
CVAR_BOOLF  (vid_shaderSmoothSelfOcclude,   true,   CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderLightingQuality,     0,      CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderFalloffQuality,      0,      CVAR_SAVECONFIG);
CVAR_INTF   (vid_shaderFogQuality,          0,      CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderPartialPrecision,    false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderPrecache,            true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_shaderCRC,                 true,   CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shaderAmbientOcclusion,    false);
CVAR_BOOL   (vid_shaderGroundAmbient,       true);
CVAR_BOOL   (vid_shaderRXGBNormalmap,       true);
CVAR_BOOLF  (vid_useDriverGLSLPreprocessor, false,  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CGfxShaders::~CGfxShaders
  ====================*/
CGfxShaders::~CGfxShaders()
{
}


/*====================
  CGfxShaders::CGfxShaders
  ====================*/
CGfxShaders::CGfxShaders() :
m_bReloadShaders(false)
{
}


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
        CGfxShaders::GetInstance()->FreeVertexShader(m_iShaderIndex);
    }
};


/*====================
  CGfxShaders::LoadVertexShader
  ====================*/
bool    CGfxShaders::LoadVertexShader(const tstring &sName, int &iIndex)
{
    PROFILE("CGfxShaders::LoadVertexShader");

    if (iIndex == -1)
    {
        // Find empty vertex shader slot
        for (iIndex = 0; iIndex < MAX_SHADERS; ++iIndex)
        {
            if (!g_aVertexShaderSlots[iIndex].bActive)
                break;
        }

        if (iIndex == MAX_SHADERS)
            return false;
    }

    tstring sDir(_TS("/core/shaders/vs_glsl/"));
    tstring sFilename = sDir + sName + _T(".vsh");

    if (!FileManager.Exists(sFilename))
    {
        Console.Warn << _T("Vertex shader ") << QuoteStr(sFilename) << _T(" doesn't exist") << newl;
        return false;
    }

    CBufferDynamic cBuffer;
    uint uiShader;

    if (0/*g_ShaderCache.LoadShader(sFilename, cBuffer)*/)
    {
        Console.Video << _T("Loading cached vertex shader: ") << Filename_StripPath(sFilename) << _T(" (vs_glsl)") << newl;

        //const char *pBuffer(cBuffer.Get(0));
        //uint uiSize(cBuffer.GetLength());
    }
    else
    {
        Console.Video << _T("Compiling vertex shader: ") << Filename_StripPath(sFilename) << _T(" (vs_glsl)") << newl;

        const char *szMacros(g_ShaderPreprocessor.AllocMacroArray());

        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

        if (!hFile.IsOpen())
        {
            FileManager.SetWorkingDirectory(sOldDir);
            Console.Warn << _T("Failed to open ") << QuoteStr(sFilename) << newl;
            return false;
        }

        uint uiBufferSize;
        const char *pBuffer(hFile.GetBuffer(uiBufferSize));

        char *szBuffer(K2_NEW_ARRAY(ctx_GL2, char, uiBufferSize+1));
        MemManager.Copy(szBuffer, pBuffer, uiBufferSize);
        szBuffer[uiBufferSize] = '\0';

        uiShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        if (!vid_useDriverGLSLPreprocessor)
        {
            CGLSLPreprocessor pp;
            pp.AddSource(szMacros);
            pp.AddSource(szBuffer);
            
            K2_DELETE_ARRAY(szBuffer);
            szBuffer = pp.AllocSourceArray();
            
            const char *pszSources[1] = 
            {
                szBuffer
            };
            glShaderSourceARB(uiShader, 1, pszSources, NULL);
        }
        else
        {
            const char *pszSources[2] = 
            {
                szMacros,
                szBuffer
            };
            glShaderSourceARB(uiShader, 2, pszSources, NULL);
        }
        glCompileShaderARB(uiShader);

        GLint iResult;
        glGetObjectParameterivARB(uiShader, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);

        if (!iResult)
        {
            K2_DELETE_ARRAY(szBuffer);  
            K2_DELETE_ARRAY(szMacros);
            
            GLint iLogLength;
            glGetObjectParameterivARB(uiShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &iLogLength);

            if (iLogLength <= 1)
            {
                Console.Err << sFilename << _T(" : ") _T("error X0000: ") _T("Unknown error") << newl;
            }
            else
            {
                char *szLog(K2_NEW_ARRAY(ctx_GL2, char, iLogLength));

                glGetInfoLogARB(uiShader, iLogLength, NULL, szLog);

                Console.Err << szLog;

                K2_DELETE_ARRAY(szLog);
            }
            
            FileManager.SetWorkingDirectory(sOldDir);

            return false;
        }

        FileManager.SetWorkingDirectory(sOldDir);

        K2_DELETE_ARRAY(szBuffer);  
        K2_DELETE_ARRAY(szMacros);
    }

    g_aVertexShaderSlots[iIndex].uiShader = uiShader;
    g_aVertexShaderSlots[iIndex].bActive = true;

    g_aVertexShaderSlots[iIndex].pFileCallback = K2_NEW(ctx_GL2,    CVertexShaderFileCallback)(sFilename, iIndex);
    g_ResourceManager.RegisterFileChangeCallback(g_aVertexShaderSlots[iIndex].pFileCallback);

    return true;
}


/*====================
  CGfxShaders::FreeVertexShader
  ====================*/
void    CGfxShaders::FreeVertexShader(int iShaderIndex)
{
    if (!g_aVertexShaderSlots[iShaderIndex].bActive)
        return;

    g_aVertexShaderSlots[iShaderIndex].bActive = false;
    
    glDeleteObjectARB(g_aVertexShaderSlots[iShaderIndex].uiShader);
    g_aVertexShaderSlots[iShaderIndex].uiShader = 0;

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
        CGfxShaders::GetInstance()->FreePixelShader(m_iShaderIndex);
    }
};


/*====================
  CGfxShaders::LoadPixelShader
  ====================*/
bool    CGfxShaders::LoadPixelShader(const tstring &sName, int &iIndex)
{
    PROFILE("CGfxShaders::LoadPixelShader");

    if (iIndex == -1)
    {
        // Find empty pixel shader slot
        for (iIndex = 0; iIndex < MAX_SHADERS; ++iIndex)
        {
            if (!g_aPixelShaderSlots[iIndex].bActive)
                break;
        }

        if (iIndex == MAX_SHADERS)
            return false;
    }

    tstring sDir(_TS("/core/shaders/ps_glsl/"));
    tstring sFilename = sDir + sName + _T(".psh");

    if (!FileManager.Exists(sFilename))
    {
        Console.Warn << _T("Pixel shader ") << QuoteStr(sFilename) << _T(" doesn't exist") << newl;
        return false;
    }

    CBufferDynamic cBuffer;
    uint uiShader;

    if (0/*g_ShaderCache.LoadShader(sFilename, cBuffer)*/)
    {
        Console.Video << _T("Loading cached pixel shader: ") << Filename_StripPath(sFilename) << _T(" (ps_glsl)") << newl;

        //const char *pBuffer(cBuffer.Get(0));
        //uint uiSize(cBuffer.GetLength());
    }
    else
    {
        Console.Video << _T("Compiling pixel shader: ") << Filename_StripPath(sFilename) << _T(" (ps_glsl)") << newl;

        const char *szMacros(g_ShaderPreprocessor.AllocMacroArray());

        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(FileManager.SanitizePath(sDir));

        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

        if (!hFile.IsOpen())
        {
            FileManager.SetWorkingDirectory(sOldDir);
            Console.Warn << _T("Failed to open ") << QuoteStr(sFilename) << newl;
            return false;
        }

        uint uiBufferSize;
        const char *pBuffer(hFile.GetBuffer(uiBufferSize));

        char *szBuffer(K2_NEW_ARRAY(ctx_GL2, char, uiBufferSize+1));
        MemManager.Copy(szBuffer, pBuffer, uiBufferSize);
        szBuffer[uiBufferSize] = '\0';
        
        uiShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        if (!vid_useDriverGLSLPreprocessor)
        {
            CGLSLPreprocessor pp;
            pp.AddSource(szMacros);
            pp.AddSource(szBuffer);
            
            K2_DELETE_ARRAY(szBuffer);
            szBuffer = pp.AllocSourceArray();
            
            const char *pszSources[1] = 
            {
                szBuffer
            };
            glShaderSourceARB(uiShader, 1, pszSources, NULL);
        }
        else
        {
            const char *pszSources[2] = 
            {
                szMacros,
                szBuffer
            };
            glShaderSourceARB(uiShader, 2, pszSources, NULL);
        }
        
        glCompileShaderARB(uiShader);
        
        GLint iResult;
        glGetObjectParameterivARB(uiShader, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);

        if (!iResult)
        {
            K2_DELETE_ARRAY(szBuffer);  
            K2_DELETE_ARRAY(szMacros);

            GLint iLogLength;
            glGetObjectParameterivARB(uiShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &iLogLength);

            if (iLogLength <= 1)
            {
                Console.Err << sFilename << _T(" : ") _T("error X0000: ") _T("Unknown error") << newl;
            }
            else
            {
                char *szLog(K2_NEW_ARRAY(ctx_GL2, char, iLogLength));

                glGetInfoLogARB(uiShader, iLogLength, NULL, szLog);

                if (szLog[iLogLength - 1] == _T(' ')) // Because ATI is retarded
                    szLog[iLogLength - 1] = _T('\n');

                Console.Err << szLog;

                if (szLog[iLogLength - 1] != _T('\n'))
                    Console.Err << newl;

                K2_DELETE_ARRAY(szLog);
            }
            
            FileManager.SetWorkingDirectory(sOldDir);

            return false;
        }

        FileManager.SetWorkingDirectory(sOldDir);

        K2_DELETE_ARRAY(szBuffer);  
        K2_DELETE_ARRAY(szMacros);
    }

    g_aPixelShaderSlots[iIndex].uiShader = uiShader;
    g_aPixelShaderSlots[iIndex].bActive = true;

    g_aPixelShaderSlots[iIndex].pFileCallback = K2_NEW(ctx_GL2,    CPixelShaderFileCallback)(sFilename, iIndex);
    g_ResourceManager.RegisterFileChangeCallback(g_aPixelShaderSlots[iIndex].pFileCallback);

    return true;
}


/*====================
  CGfxShaders::FreePixelShader
  ====================*/
void    CGfxShaders::FreePixelShader(int iShaderIndex)
{
    if (!g_aPixelShaderSlots[iShaderIndex].bActive)
        return;

    g_aPixelShaderSlots[iShaderIndex].bActive = false;
    
    glDeleteObjectARB(g_aPixelShaderSlots[iShaderIndex].uiShader);
    g_aPixelShaderSlots[iShaderIndex].uiShader = 0;

    g_ResourceManager.UnregisterFileChangeCallback(g_aPixelShaderSlots[iShaderIndex].pFileCallback);
    SAFE_DELETE(g_aPixelShaderSlots[iShaderIndex].pFileCallback);

    g_ShaderRegistry.FreePixelShaderInstance(iShaderIndex);
}


/*====================
  CGfxShaders::FreeShaderProgram
  ====================*/
void    CGfxShaders::FreeShaderProgram(int iShaderIndex)
{
    if (!g_aShaderProgramSlots[iShaderIndex].bActive)
        return;

    if (g_iCurrentShaderProgram == iShaderIndex)
    {
        glUseProgramObjectARB(0);
        g_iCurrentShaderProgram = -1;
    }

    g_aShaderProgramSlots[iShaderIndex].bActive = false;
    
    glDeleteObjectARB(g_aShaderProgramSlots[iShaderIndex].uiProgram);
    g_aShaderProgramSlots[iShaderIndex].uiProgram = 0;
    g_aShaderProgramSlots[iShaderIndex].vAttributes.clear();
    g_aShaderProgramSlots[iShaderIndex].vUniforms.clear();
    g_aShaderProgramSlots[iShaderIndex].iNumTextureStages = 0;

    g_ShaderRegistry.FreeShaderProgramInstance(iShaderIndex);
}


/*====================
  CGfxShaders::RegisterVertexShader
  ====================*/
int     CGfxShaders::RegisterVertexShader(CVertexShader *pVertexShader)
{
    ShaderHandle hShader(g_ShaderRegistry.RegisterVertexShader(pVertexShader->GetName(), pVertexShader->GetShaderFlags()));

    pVertexShader->SetIndex(hShader);

    return 0;
}


/*====================
  CGfxShaders::UnregisterVertexShader
  ====================*/
void    CGfxShaders::UnregisterVertexShader(CVertexShader *pVertexShader)
{
    g_ShaderRegistry.UnregisterVertexShader(pVertexShader->GetName());
}


/*====================
  CGfxShaders::RegisterPixelShader
  ====================*/
int     CGfxShaders::RegisterPixelShader(CPixelShader *pPixelShader)
{
    ShaderHandle hShader(g_ShaderRegistry.RegisterPixelShader(pPixelShader->GetName(), pPixelShader->GetShaderFlags()));

    pPixelShader->SetIndex(hShader);

    return 0;
}


/*====================
  CGfxShaders::UnregisterPixelShader
  ====================*/
void    CGfxShaders::UnregisterPixelShader(CPixelShader *pPixelShader)
{
    g_ShaderRegistry.UnregisterVertexShader(pPixelShader->GetName());
}


/*====================
  CGfxShaders::RegisterShaderPair
  ====================*/
void    CGfxShaders::RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader)
{
    g_ShaderRegistry.RegisterShaderPair(pVertexShader->GetIndex(), pPixelShader->GetIndex());
}


/*====================
  CGfxShaders::LinkShaderProgram
  ====================*/
bool    CGfxShaders::LinkShaderProgram(int iVertexShader, int iPixelShader, int &iIndex)
{
    PROFILE("CGfxShaders::LinkShaderProgram");

    if (!g_aVertexShaderSlots[iVertexShader].bActive ||
        !g_aPixelShaderSlots[iPixelShader].bActive)
    {
        Console.Video << _T("Attempt to link invalid shaders (glsl)") << newl;
        return false;
    }

    if (iIndex == -1)
    {
        // Find empty pixel shader slot
        for (iIndex = 0; iIndex < MAX_SHADERS; ++iIndex)
        {
            if (!g_aShaderProgramSlots[iIndex].bActive)
                break;
        }

        if (iIndex == MAX_SHADERS)
            return false;
    }

    Console.Video << _T("Linking shaders: ")
        << Filename_StripPath(g_aVertexShaderSlots[iVertexShader].pFileCallback->GetPath())
        << _T(" and ")
        << Filename_StripPath(g_aPixelShaderSlots[iPixelShader].pFileCallback->GetPath())
        << _T(" (glsl)") << newl;

    GLuint uiProgram(glCreateProgramObjectARB());

    glAttachObjectARB(uiProgram, g_aVertexShaderSlots[iVertexShader].uiShader);
    glAttachObjectARB(uiProgram, g_aPixelShaderSlots[iPixelShader].uiShader);
    
    bool bAttrib0(true), bRelinked(false);
        
    do
    {
        bAttrib0 = true;
        
        glLinkProgramARB(uiProgram);
    
        GLint iResult;
        glGetObjectParameterivARB(uiProgram, GL_OBJECT_LINK_STATUS_ARB, &iResult);
    
        if (!iResult)
        {
            GLint iLogLength;
            glGetObjectParameterivARB(uiProgram, GL_OBJECT_INFO_LOG_LENGTH_ARB, &iLogLength);
    
            if (iLogLength <= 1)
                Console.Err << _T("CGfxShaders::LinkShaderProgram") << _T(" : ") _T("error X0000: ") _T("Unknown error") << newl;
            if (iLogLength > 1)
            {
                char *szLog(K2_NEW_ARRAY(ctx_GL2, char, iLogLength));
        
                glGetInfoLogARB(uiProgram, iLogLength, NULL, szLog);
        
                Console.Err << _T("CGfxShaders::LinkShaderProgram") << newl;
        
                Console.Err << szLog;
        
                 // Because ATI is retarded
                size_t iStrLen(strlen(szLog));
        
                if (szLog[iStrLen - 1] != _T('\n'))
                    Console.Err << newl;
        
                K2_DELETE_ARRAY(szLog);
            }
    
            return false;
        }
    
        g_aShaderProgramSlots[iIndex].bActive = true;
        g_aShaderProgramSlots[iIndex].uiProgram = uiProgram;
    
        //
        // Map attributes
        //
        GLint iNumAttributes;
        glGetObjectParameterivARB(uiProgram, GL_OBJECT_ACTIVE_ATTRIBUTES_ARB, &iNumAttributes);
        if (vid_shaderDebug)
            Console.Video << XtoA(iNumAttributes) << _T(" active attributes:") << newl;
    
        for (int i(0); i < iNumAttributes; ++i)
        {
            GLsizei iLength;
            GLint iSize;
            GLenum eType;
            GLchar szName[256];
    
            glGetActiveAttribARB(uiProgram, i, sizeof(szName), &iLength, &iSize, &eType, szName);
            if (vid_shaderDebug)
                Console.Video << _T("  ") << XtoA(i) << _T(": ") << szName << newl;
            
            if (strncmp(szName, "gl_", 3) == 0)
                continue;
            
            if (strncmp(szName, "a_fHeight", 9) == 0 && glGetAttribLocationARB(uiProgram, szName) != 0)
                bAttrib0 = false;
    
            g_aShaderProgramSlots[iIndex].vAttributes.push_back(SShaderAttribute());
            SShaderAttribute &cAttribute(g_aShaderProgramSlots[iIndex].vAttributes.back());
            
            cAttribute.sName = StringToTString(string(szName));
            cAttribute.eType = eType;
            cAttribute.iLocation = glGetAttribLocationARB(uiProgram, szName);
            if (cAttribute.iLocation == -1)
                Console.Warn << _T("Attribute ") << szName << _T(" is not active.") << newl;
        }
        
        if (!bAttrib0)
        {
            if (bRelinked)
            {
                Console.Warn << _T("CGfxShaders::LinkShaderProgram() - no attributes bound at location 0") << newl;
                break;
            }
            
            // assume a_fHeight is used in all shaders that don't use gl_Vertex
            glBindAttribLocationARB(uiProgram, 0, "a_fHeight");
            g_aShaderProgramSlots[iIndex].vAttributes.clear();
            bRelinked = true;
        }
    }
    while (!bAttrib0);

    //
    // Map uniforms
    //

    GLint iNumUniforms;
    glGetObjectParameterivARB(uiProgram, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &iNumUniforms);
    if (vid_shaderDebug)
        Console.Video << XtoA(iNumUniforms) << _T(" active uniforms:") << newl;

    int iTextureStage(0);
    
    for (int i(0); i < iNumUniforms; ++i)
    {
        GLsizei iLength;
        GLint iSize;
        GLenum eType;
        GLchar szName[256];

        glGetActiveUniformARB(uiProgram, i, sizeof(szName), &iLength, &iSize, &eType, szName);
        if (vid_shaderDebug)
            Console.Video << _T("  ") << XtoA(i) << _T(": ") << szName << newl;

        if (strncmp(szName, "gl_", 3) == 0)
            continue;

        // Strip brackets (because ATI is retarded)
        for (GLchar *psz(szName); *psz != NULL; ++psz)
        {
            if (*psz == _T('['))
            {
                *psz = NULL;
                break;
            }
        }
        
        g_aShaderProgramSlots[iIndex].vUniforms.push_back(SShaderUniform());
        SShaderUniform &cUniform(g_aShaderProgramSlots[iIndex].vUniforms.back());

        tstring sName(StringToTString(string(szName)));

        if (sName.length() > 2 && sName[sName.length() - 2] == _T('_'))
        {
            cUniform.uiSubTexture = sName[sName.length() - 1] - _T('0');
            sName = sName.substr(0, sName.length() - 2);
        }
        else
            cUniform.uiSubTexture = 0;
    
        cUniform.sName = sName;
        cUniform.eType = eType;
        cUniform.iLocation = glGetUniformLocationARB(uiProgram, szName);
        if (cUniform.iLocation == -1)
            Console.Warn << _T("Uniform ") << sName << _T(" is not active.") << newl;

        switch (cUniform.eType)
        {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_SHADOW:
            cUniform.eTextureType = GL_TEXTURE_1D;
            cUniform.pShaderSampler = CShaderSamplerRegistry::GetInstance()->GetShaderSampler(cUniform.sName);
            cUniform.iTextureStage = iTextureStage;
            ++iTextureStage;
            break;
        case GL_SAMPLER_3D:
            cUniform.eTextureType = GL_TEXTURE_3D;
            cUniform.pShaderSampler = CShaderSamplerRegistry::GetInstance()->GetShaderSampler(cUniform.sName);
            cUniform.iTextureStage = iTextureStage;
            ++iTextureStage;
            break;
        case GL_SAMPLER_CUBE:
            cUniform.eTextureType = GL_TEXTURE_CUBE_MAP;
            cUniform.pShaderSampler = CShaderSamplerRegistry::GetInstance()->GetShaderSampler(cUniform.sName);
            cUniform.iTextureStage = iTextureStage;
            ++iTextureStage;
            break;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
            cUniform.eTextureType = GL_TEXTURE_2D;
            cUniform.pShaderSampler = CShaderSamplerRegistry::GetInstance()->GetShaderSampler(cUniform.sName);
            cUniform.iTextureStage = iTextureStage;
            ++iTextureStage;
            break;
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
            cUniform.eTextureType = GL_NONE;
            cUniform.pShaderVar = CShaderVarRegistry::GetInstance()->GetShaderVar(cUniform.sName);
            break;
        default:
            cUniform.eTextureType = GL_NONE;
            cUniform.pShaderVar = NULL;
            break;
        }
    }

    g_aShaderProgramSlots[iIndex].iNumTextureStages = iTextureStage;
    
#ifdef __APPLE__
    glUseProgramObjectARB(uiProgram);
    
    GLint fragmentGPUProcessing, vertexGPUProcessing;
    CGLGetParameter (CGLGetCurrentContext(), kCGLCPGPUFragmentProcessing,
                                             &fragmentGPUProcessing);
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUVertexProcessing,
                                             &vertexGPUProcessing);
    if (!fragmentGPUProcessing || !vertexGPUProcessing)
    {
        tstring sShader;
        if (!fragmentGPUProcessing && !vertexGPUProcessing)
            sShader = _T("Fragment and vertex shaders");
        else if (!fragmentGPUProcessing)
            sShader = _T("Fragment shader");
        else
            sShader = _T("Vertex shader");
        Console.Warn << _T("CGfxShaders::LinkShaderProgram() - ") << sShader << _T(" will not be executed on the GPU") << newl;
    }
    
    glUseProgramObjectARB(0);
#endif

    return true;
}


/*====================
  CGfxShaders::Frame
  ====================*/
void    CGfxShaders::Frame()
{
    if (vid_shadows.IsModified())
    {
        vid_shadows.SetModified(false);

        m_bReloadShaders = true;

        g_Shadowmap.Release();
        g_Shadowmap.Initialize(EShadowmapType(int(vid_shadowmapType)));
    }

    if (vid_shadowmapSize.IsModified())
    {
        vid_shadowmapSize.SetModified(false);

        g_Shadowmap.Release();
        g_Shadowmap.Initialize(EShadowmapType(int(vid_shadowmapType)));
    }

    if (vid_shadowmapType.IsModified())
    {
        vid_shadowmapType.SetModified(false);

        g_Shadowmap.Release();
        g_Shadowmap.Initialize(EShadowmapType(int(vid_shadowmapType)));
    }

    if (vid_sceneBuffer.IsModified())
    {
        vid_sceneBuffer.SetModified(false);

        m_bReloadShaders = true;

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

    UpdateDefineCvarBool(&vid_treeSmoothNormals, "SMOOTH_TREE_NORMALS");
    UpdateDefineCvarBool(&vid_terrainDerepeat, "TERRAIN_DEREPEAT");
    UpdateDefineCvarInt(&vid_shadowmapFilterWidth, "SHADOWMAP_FILTER_WIDTH");
    //UpdateDefineCvarBool(&gfx_clouds, "CLOUDS");
    UpdateDefineCvarBool(&vid_shadowFalloff, "SHADOW_FALLOFF");
    UpdateDefineCvarInt(&vid_shaderLightingQuality, "LIGHTING_QUALITY");
    UpdateDefineCvarInt(&vid_shaderFalloffQuality, "FALLOFF_QUALITY");
    UpdateDefineCvarInt(&vid_shaderFogQuality, "FOG_QUALITY");
    UpdateDefineCvarBool(&vid_shaderSmoothSelfOcclude, "SMOOTH_SELF_OCCLUDE");
    UpdateDefineCvarInt(&gfx_fogType, "FOG_TYPE");
    UpdateDefineCvarBool(&vid_shaderGroundAmbient, "GROUND_AMBIENT");
    //UpdateDefineCvarInt(ICvar::GetCvar(_T("geom_maxBoneWeights")), "NUM_BONE_WEIGHTS");

    if (m_bReloadShaders)
    {
        m_bReloadShaders = false;
        g_ShaderRegistry.ReloadShaders();
    }
}


/*====================
  CGfxShaders::Init
  ====================*/
void    CGfxShaders::Init()
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

    if (vid_shaderAmbientOcclusion)
        g_ShaderPreprocessor.Define("AMBIENT_OCCLUSION");

    if (vid_shaderGroundAmbient)
        g_ShaderPreprocessor.Define("GROUND_AMBIENT");

#if 0
    if (gfx_clouds)
        g_ShaderPreprocessor.Define("CLOUDS");
#endif

    if (vid_shadowFalloff)
        g_ShaderPreprocessor.Define("SHADOW_FALLOFF");

    if (vid_shaderRXGBNormalmap)
        g_ShaderPreprocessor.Define("RXGB_NORMALMAP");

    if (g_DeviceCaps.bNonSquareMatrix)
        g_ShaderPreprocessor.Define("NON_SQUARE_MATRIX");

    g_ShaderPreprocessor.Define("MAX_VARYING_FLOATS", XtoS(g_DeviceCaps.iMaxVaryingFloats));

    g_hCloudTexture = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(gfx_cloudTexture, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);

    gfx_fogType.SetModified(false);
    vid_treeSmoothNormals.SetModified(false);
    vid_shadowmapFilterWidth.SetModified(false);
    gfx_clouds.SetModified(false);
    gfx_cloudTexture.SetModified(false);
    vid_terrainDerepeat.SetModified(false);
    vid_shadowFalloff.SetModified(false);
    vid_shaderLightingQuality.SetModified(false);
    vid_shaderFalloffQuality.SetModified(false);
    vid_shaderFogQuality.SetModified(false);
    vid_shaderSmoothSelfOcclude.SetModified(false);
    vid_sceneBuffer.SetModified(false);
    vid_sceneBufferMipmap.SetModified(false);
    vid_postEffects.SetModified(false);

    g_iMaxDynamicLights = (vid_dynamicLights && vid_shaderLightingQuality == 0) ? MIN(MAX_POINT_LIGHTS, int(vid_maxDynamicLights)) : 0;
    g_iNumActivePointLights = 0;
    g_iNumActiveBones = 0;
    g_bLighting = true;
    g_bTexkill = false;

    //
    // Error Shaders
    //

    g_hNullMeshVS = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CVertexShader)(_T("simple"), 0), RES_VERTEX_SHADER);
    g_hNullMeshPS = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CPixelShader)(_T("simple"), 0), RES_PIXEL_SHADER);
}


/*====================
  CGfxShaders::UpdateDefineCvarInt
  ====================*/
void    CGfxShaders::UpdateDefineCvarInt(ICvar *pCvar, const string &sDefine)
{
    if (pCvar->IsModified())
    {
        pCvar->SetModified(false);

        g_ShaderPreprocessor.Define(sDefine, TStringToString(pCvar->GetString()));

        m_bReloadShaders = true;
    }
}


/*====================
  CGfxShaders::UpdateDefineCvarBool
  ====================*/
void    CGfxShaders::UpdateDefineCvarBool(CCvar<bool> *pCvar, const string &sDefine)
{
    if (pCvar->IsModified())
    {
        pCvar->SetModified(false);

        if (*pCvar)
            g_ShaderPreprocessor.Define(sDefine);
        else
            g_ShaderPreprocessor.Undefine(sDefine);

        m_bReloadShaders = true;
    }
}


/*====================
  CGfxShaders::Shutdown
  ====================*/
void    CGfxShaders::Shutdown()
{
    for (int i = 0; i < MAX_SHADERS; ++i)
    {
        FreeShaderProgram(i);
        FreeVertexShader(i);
        FreePixelShader(i);

        g_ResourceManager.UnregisterFileChangeCallback(g_aVertexShaderSlots[i].pFileCallback);
        SAFE_DELETE(g_aVertexShaderSlots[i].pFileCallback);

        g_ResourceManager.UnregisterFileChangeCallback(g_aPixelShaderSlots[i].pFileCallback);
        SAFE_DELETE(g_aPixelShaderSlots[i].pFileCallback);
    }

    PRINT_GLERROR_BREAK();
}
