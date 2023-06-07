// (C)2008 S2 Games
// game_shared_types.h
//
//=============================================================================
#ifndef __GAME_SHARED_TYPES_H__
#define __GAME_SHARED_TYPES_H__

//=============================================================================
// Declarations
//=============================================================================
class CPlayer;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EVCType
{
    VC_TEAM,
    VC_ALL
};

typedef map<int, class CPlayer*>        PlayerMap;
typedef PlayerMap::iterator             PlayerMap_it;
typedef PlayerMap::const_iterator       PlayerMap_cit;
typedef PlayerMap::reverse_iterator     PlayerMap_rit;

template<class T> T                 GetDefaultEmptyValue()                      {}
template<> inline float             GetDefaultEmptyValue<float>()               { return 0.0f; }
template<> inline byte              GetDefaultEmptyValue<byte>()                { return 0; }
template<> inline int               GetDefaultEmptyValue<int>()                 { return 0; }
template<> inline uint              GetDefaultEmptyValue<uint>()                { return 0; }
template<> inline bool              GetDefaultEmptyValue<bool>()                { return false; }
template<> inline CVec2f            GetDefaultEmptyValue<CVec2f>()              { return V2_ZERO; }
template<> inline CVec3f            GetDefaultEmptyValue<CVec3f>()              { return V_ZERO; }
template<> inline CVec4f            GetDefaultEmptyValue<CVec4f>()              { return V4_ZERO; }
template<> inline tstring           GetDefaultEmptyValue<tstring>()             { return TSNULL; }
template<> inline const tstring&    GetDefaultEmptyValue<const tstring&>()      { return TSNULL; }
template<> inline tsvector          GetDefaultEmptyValue<tsvector>()            { return VSNULL; }
template<> inline const tsvector&   GetDefaultEmptyValue<const tsvector&>()     { return VSNULL; }
//=============================================================================

#endif //__GAME_SHARED_TYPES_H__
