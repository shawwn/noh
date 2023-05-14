// (C)2007 S2 Games
// c_triggermanager.h
//
//=============================================================================
#ifndef __C_TRIGGERMANAGER_H__
#define __C_TRIGGERMANAGER_H__

//=============================================================================
// Headers
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
extern class CTriggerManager &TriggerManager;

#define TRIGGER_CMD(name, enttype, minargs) \
    static bool cmd##name##FnTrigger(const tsvector &vArgList, I##enttype##Entity *pEnt);\
CMD(name)\
{\
    if (vArgList.size() < minargs)\
    {\
        Console.Script << _T(#name) << _T(": Not enough arguments. ") << INT_SIZE(vArgList.size()) << _T(" provided, ") << _T(#minargs) << _T(" required.") << newl;\
        return false;\
    }\
\
    bool bReturn(false);\
    IGameEntity* pEnt(nullptr);\
\
    IGame *pGame(Game.GetCurrentGamePointer());\
    Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
    tstring sIndex;\
    size_t zEndPos(0);\
    size_t zStartPos(0);\
\
    while (zEndPos != tstring::npos)\
    {\
        zEndPos = vArgList[0].find(_T(','), zStartPos);\
\
        if (zEndPos == tstring::npos)\
            sIndex = vArgList[0].substr(zStartPos, vArgList[0].length() - zStartPos);\
        else\
            sIndex = vArgList[0].substr(zStartPos, zEndPos - zStartPos);\
\
        if (!sIndex.empty() && isdigit(sIndex[0]))\
        {\
            pEnt = GameServer.GetEntity(AtoI(sIndex));\
    \
            if (pEnt != nullptr && pEnt->Is##enttype())\
            {\
                bReturn = cmd##name##FnTrigger(vArgList, (I##enttype##Entity *)pEnt);\
    \
                if (!bReturn)\
                {\
                    Console.Script << _T(#name) << _T(": Failed to execute command on entity index ") << pEnt->GetIndex() << _T(".") << newl;\
                    Console.Script << _T(#name) << _T(": Command was: ") << ConcatinateArgs(vArgList) << newl;\
                }\
            }\
            else if (pEnt == nullptr)\
                Console.Script << _T(#name) << _T(": Entity index ") << sIndex << _T(" was not found.") << newl;\
            else\
                Console.Script << _T(#name) << _T(": Entity index ") << sIndex << _T(" was not of type ") << _T(#enttype) << _T(".") << newl;\
\
        }\
        else if (!sIndex.empty())\
        {\
            WorldEntList map(GameServer.GetWorldEntityList());\
            WorldEntList_it it(map.begin());\
            bool bFound(false);\
    \
            while (it != map.end())\
            {\
                CWorldEntity *pWorldEntity(GameServer.GetWorldEntity(it->first));\
                if (!pWorldEntity)\
                {\
                    it++;\
                    continue;\
                }\
    \
                if (pWorldEntity->GetName() == sIndex)\
                {\
                    pEnt = GameServer.GetEntity(pWorldEntity->GetGameIndex());\
    \
                    if (pEnt != nullptr && pEnt->Is##enttype())\
                    {\
                        bReturn = cmd##name##FnTrigger(vArgList, (I##enttype##Entity *)pEnt);\
    \
                        if (!bReturn)\
                        {\
                            Console.Script << _T(#name) << _T(": Failed to execute command on entity ") << vArgList[0] << _T(" (Index: ") << pEnt->GetIndex() << _T(").") << newl;\
                            Console.Script << _T(#name) << _T(": Command was: ") << ConcatinateArgs(vArgList) << newl;\
                        }\
    \
                        bFound = true;\
                    }\
                }\
    \
                it++;\
            }\
    \
            if (!bFound)\
                Console.Script << _T(#name) << _T(": No entities named ") << sIndex << _T(" found that are of type ") << _T(#enttype) << _T(".") << newl;\
        }\
    \
        zStartPos = zEndPos + 1;\
    }\
\
    Game.SetCurrentGamePointer(pGame);\
\
    return bReturn;\
}\
static bool cmd##name##FnTrigger(const tsvector &vArgList, I##enttype##Entity *pEnt)




#define TRIGGER_FCN(name, enttype, minargs) \
    static tstring  fn##name##FnTrigger(const tsvector &vArgList, I##enttype##Entity *pEnt);\
FUNCTION(name)\
{\
    if (vArgList.size() < minargs)\
    {\
        Console.Script << _T(#name) << _T(": Not enough arguments. ") << INT_SIZE(vArgList.size()) << _T(" provided, ") << _T(#minargs) << _T(" required.") << newl;\
        return _T("");\
    }\
\
    tstring sReturn(_T(""));\
    IGameEntity* pEnt(nullptr);\
\
    IGame *pGame(Game.GetCurrentGamePointer());\
    Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
    tstring sIndex;\
    size_t zEndPos(0);\
    size_t zStartPos(0);\
\
    while (zEndPos != tstring::npos)\
    {\
        zEndPos = vArgList[0].find(_T(','), zStartPos);\
\
        if (zEndPos == tstring::npos)\
            sIndex = vArgList[0].substr(zStartPos, vArgList[0].length() - zStartPos);\
        else\
            sIndex = vArgList[0].substr(zStartPos, zEndPos - zStartPos);\
\
        if (!sIndex.empty() && isdigit(sIndex[0]))\
        {\
            pEnt = GameServer.GetEntity(AtoI(sIndex));\
    \
            if (pEnt != nullptr && pEnt->Is##enttype())\
                sReturn = fn##name##FnTrigger(vArgList, (I##enttype##Entity *)pEnt);\
            else if (pEnt == nullptr)\
                Console.Script << _T(#name) << _T(": Entity index ") << sIndex << _T(" was not found.") << newl;\
            else\
                Console.Script << _T(#name) << _T(": Entity index ") << sIndex << _T(" was not of type ") << _T(#enttype) << _T(".") << newl;\
\
        }\
        else if (!sIndex.empty())\
        {\
            WorldEntList map(GameServer.GetWorldEntityList());\
            WorldEntList_it it(map.begin());\
            bool bFound(false);\
    \
            while (it != map.end())\
            {\
                CWorldEntity *pWorldEntity(GameServer.GetWorldEntity(it->first));\
                if (!pWorldEntity)\
                {\
                    it++;\
                    continue;\
                }\
    \
                if (pWorldEntity->GetName() == sIndex)\
                {\
                    pEnt = GameServer.GetEntity(pWorldEntity->GetGameIndex());\
    \
                    if (pEnt != nullptr && pEnt->Is##enttype())\
                    {\
                        sReturn = fn##name##FnTrigger(vArgList, (I##enttype##Entity *)pEnt);\
                        bFound = true;\
                    }\
                }\
    \
                it++;\
            }\
    \
            if (!bFound)\
                Console.Script << _T(#name) << _T(": No entities named ") << sIndex << _T(" found that are of type ") << _T(#enttype) << _T(".") << newl;\
        }\
    \
        zStartPos = zEndPos + 1;\
    }\
\
    Game.SetCurrentGamePointer(pGame);\
\
    return sReturn;\
}\
static tstring  fn##name##FnTrigger(const tsvector &vArgList, I##enttype##Entity *pEnt)

//=============================================================================
// CTriggerManager
//=============================================================================

class CTriggerManager
{
    SINGLETON_DEF(CTriggerManager)

private:

    tsmapts                 m_mapTriggerParams;
    map<uint, tsmapts>      m_mapEntityScripts;
    tsmapts                 m_mapGlobalScripts;

public:
    ~CTriggerManager()      {}

    void        RegisterEntityScript(uint uiIndex, const tstring &sName, const tstring &sScript);
    void        RegisterGlobalScript(const tstring &sName, const tstring &sScript)  { m_mapGlobalScripts[sName] = sScript; }

    void        CopyEntityScripts(uint uiFromIndex, uint uiToIndex);

    void        ClearEntityScripts(uint uiIndex)                                    { m_mapEntityScripts.erase(uiIndex); }
    void        ClearAllEntityScripts()                                             { m_mapEntityScripts.clear(); }
    void        ClearGlobalScripts()                                                { m_mapGlobalScripts.clear(); }

    bool        TriggerEntityScript(uint uiIndex, const tstring &sName);
    bool        TriggerGlobalScript(const tstring &sName);

    void        RegisterTriggerParam(const tstring &sName, const tstring &sValue);
};

#endif // __C_TRIGGERMANAGER_H__
