// (C)2010 S2 Games
// c_resourcemanager_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourcemanager.h"
#include "i_resourcelibrary.h"
//=============================================================================

//=============================================================================
// CVars
//=============================================================================
//=============================================================================

/*--------------------
  ReloadRes
  --------------------*/
CMD(ReloadRes)
{
    if (vArgList.empty())
        return false;

    g_ResourceManager.Reload(g_ResourceManager.LookUpPath(vArgList[0]));
    return true;
}


/*--------------------
  ListResources
  --------------------*/
CMD(ListResources)
{
    tstring sWildcard(_T("*"));
    if (vArgList.size() >= 1)
        sWildcard = vArgList[0];

    g_ResourceManager.ExecCommand(sWildcard, _T("list"));
    return true;
}


/*--------------------
  ListResourceUsage
  --------------------*/
CMD(ListResourceUsage)
{
    g_ResourceManager.ListResourceUsage();
    return true;
}


/*--------------------
  ReloadInterfaces
  --------------------*/
CMD(ReloadInterfaces)
{
    g_ResourceManager.GetLib(RES_INTERFACE)->ReloadAll();
    return true;
}


/*--------------------
  ReloadSamples
  --------------------*/
CMD(ReloadSamples)
{
    g_ResourceManager.GetLib(RES_SAMPLE)->ReloadAll();
    return true;
}


/*--------------------
  ReloadFontMaps
  --------------------*/
CMD(ReloadFontMaps)
{
    g_ResourceManager.GetLib(RES_FONTMAP)->ReloadAll();
    return true;
}


/*--------------------
  ReloadModels
  --------------------*/
CMD(ReloadModels)
{
    g_ResourceManager.GetLib(RES_MODEL)->ReloadAll();
    return true;
}


#if 0
/*--------------------
  PurgeHeroResources
  --------------------*/
CMD(PurgeHeroResources)
{
    if (!res_purgeResources)
        return true;

    Console << _T("^m==== purging hero resources ====^*") << newl;

    tstring sPurgeWildcard;

    // purge all hero-specific resources.
    sPurgeWildcard.append(_T("*/heroes/*"));

    // don't purge entity files, since we're loading every *.entity when joining a lobby.
    sPurgeWildcard.append(_T("|"));
    sPurgeWildcard.append(_T("-*.entity"));

    // don't purge hero icons.
    sPurgeWildcard.append(_T("|"));
    sPurgeWildcard.append(_T("-*/icon*.tga"));
    sPurgeWildcard.append(_T("|"));
    sPurgeWildcard.append(_T("-*/icons/*.tga"));
    sPurgeWildcard.append(_T("|"));
    sPurgeWildcard.append(_T("-*/rocky/hero.tga"));

    Console << _T("     ^bExecuting command:  ResourceCmd purge ") << sPurgeWildcard << newl << newl;
    g_ResourceManager.ExecCommand(sPurgeWildcard, _T("unregister"));

    return true;
}


/*--------------------
  PurgeResources
  --------------------*/
CMD(PurgeResources)
{
    if (vArgList.size() < 1)
    {
        Console << _T("example syntax (purges all hero resources except entity resources):")
            << newl << _T("     ") << _T("PurgeResources */heroes/*|-*.entity") << newl;
        return false;
    }

    // arg 0 (string): wildcard match
    tstring sWildcardMatch(vArgList[0]);
    g_ResourceManager.ExecCommand(sWildcardMatch, _T("purge"));
    return true;
}
#endif


/*--------------------
  ResourceCmd
  --------------------*/
CMD(ResourceCmd)
{
    tstring sWildcard(_T("*"));
    tstring sCmd;

    if (vArgList.size() >= 1)
        sWildcard = vArgList[0];

    if (vArgList.size() >= 2)
        sCmd = vArgList[1];

    g_ResourceManager.ExecCommand(sWildcard, sCmd);

    return true;
}


/*--------------------
  RegisterRes
  --------------------*/
CMD(RegisterRes)
{
    if (vArgList.size() < 2)
    {
        Console << _T("RegisterRes <path> <type>") << newl;
        Console << _T("where <type> is one of: material, vertexshader, pixelshader, texture, ramp, ") << newl;
        Console << _T("  cliffdef, model, clip, fontface, fontmap, sample, stringtable, interface, ") << newl;
        Console << _T("  effect, posteffect, bitmap, cursor, reference, entity, or gamemechanics") << newl;
        return false;
    }

    const tstring &sPath(vArgList[0]);
    const tstring &sType(LowerString(vArgList[1]));

    int iType(-1);
    if (sType == _T("material"))
        iType = RES_MATERIAL;
    else if (sType == _T("vertexshader"))
        iType = RES_VERTEX_SHADER;
    else if (sType == _T("pixelshader"))
        iType = RES_PIXEL_SHADER;
    else if (sType == _T("texture"))
        iType = RES_TEXTURE;
    else if (sType == _T("ramp"))
        iType = RES_RAMP;
    else if (sType == _T("cliffdef") || sType == _T("cliff"))
        iType = RES_CLIFFDEF;
    else if (sType == _T("model"))
        iType = RES_MODEL;
    else if (sType == _T("clip"))
        iType = RES_CLIP;
    else if (sType == _T("fontface"))
        iType = RES_FONTFACE;
    else if (sType == _T("fontmap"))
        iType = RES_FONTMAP;
    else if (sType == _T("sample"))
        iType = RES_SAMPLE;
    else if (sType == _T("stringtable"))
        iType = RES_STRINGTABLE;
    else if (sType == _T("interface"))
        iType = RES_INTERFACE;
    else if (sType == _T("effect"))
        iType = RES_EFFECT;
    else if (sType == _T("posteffect"))
        iType = RES_POST_EFFECT;
    else if (sType == _T("bitmap"))
        iType = RES_BITMAP;
    else if (sType == _T("cursor"))
        iType = RES_K2CURSOR;
    else if (sType == _T("reference"))
        iType = RES_REFERENCE;
    else if (sType == _T("entity"))
        iType = NUM_RESOURCE_TYPES;
    else if (sType == _T("gamemechanics"))
        iType = NUM_RESOURCE_TYPES + 1;

    if (iType <= 0)
    {
        Console << _T("Unknown resource type.") << newl;
        return false;
    }

    ResHandle hResource(g_ResourceManager.Register(sPath, (uint)iType));
    if (hResource == INVALID_RESOURCE)
        Console << _T("Failed to register.") << newl;
    else
        Console << _T("Registered resource: ") << XtoA(hResource, 0, 0, 16) << newl;

    return true;
}

