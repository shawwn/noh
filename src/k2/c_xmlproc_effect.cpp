// (C)2006 S2 Games
// c_xmlproc_effect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_effect.h"
#include "c_particlesystem.h"
#include "c_effectthread.h"
#include "i_effectcmd.h"
#include "c_soundmanager.h"
#include "c_sample.h"
#include "c_sceneentitymodifier.h"
#include "c_texture.h"
#include "c_xmlprocroot.h"

#include "c_simpleemitter.h"
#include "c_twopointemitter.h"
#include "c_orbiteremitter.h"
#include "c_trackeremitter.h"
#include "c_meshemitter.h"
#include "c_skeletonemitter.h"
#include "c_twopointtrailemitter.h"
#include "c_lightemitter.h"
#include "c_beamemitter.h"
#include "c_groundspriteemitter.h"
#include "c_modelemitter.h"
#include "c_controlleremitter.h"
#include "c_debrisemitter.h"
#include "c_trailemitter.h"
#include "c_billboardemitter.h"
#include "c_traceremitter.h"
#include "c_soundemitter.h"
#include "c_precipemitter.h"
#include "c_terrainemitter.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define TEMPORAL_PROPERTY_RANGE_STRINGS(property) \
    property, \
    _T("start") property, \
    _T("end") property, \
    _T("mid") property, \
    _T("mid") property _T("pos"), \
    _T("min") property, \
    _T("max") property, \
    _T("minstart") property, \
    _T("maxstart") property, \
    _T("minend") property, \
    _T("maxend") property, \
    _T("minmid") property, \
    _T("maxmid") property, \
    _T("minmid") property _T("pos"), \
    _T("maxmid") property _T("pos"), \
    property _T("speed"), \
    _T("min") property _T("speed"), \
    _T("max") property _T("speed")

#define TEMPORAL_PROPERTY_RANGE_ENUMS(property) \
    _##property, \
    _Start_##property, \
    _End_##property, \
    _Mid_##property, \
    _Mid_##property##Pos, \
    _Min_##property, \
    _Max_##property, \
    _MinStart_##property, \
    _MaxStart_##property, \
    _MinEnd_##property, \
    _MaxEnd_##property, \
    _MinMid_##property, \
    _MaxMid_##property, \
    _MinMid_##property##Pos, \
    _MaxMid_##property##Pos, \
    _##property##Speed, \
    _Min_##property##Speed, \
    _Max_##property##Speed
    
tstring g_sTemporalPropertyRange[] = 
{
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("spawnrate")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minparticlelife")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxparticlelife")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("particletimenudge")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("gravity")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("mininheritvelocity")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxinheritvelocity")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("limitinheritvelocity")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minoffsetdirection")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxoffsetdirection")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minoffsetradial")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxoffsetradial")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minoffsetradialangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxoffsetradialangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minorbitangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxorbitangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("mintrackspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxtrackspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("texpostime")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("texposscale")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("texstretchscale")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("size")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("taper")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("tile")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("frame")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("param")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("pitch")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("roll")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("yaw")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("width")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("height")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("scale")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("length")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("speed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("acceleration")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minacceleration")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxacceleration")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("range")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("alpha")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("stickiness")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("anchor")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("widthdistort")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("heightdistort")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("angle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxangle")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("falloffstart")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("falloffend")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("flickeramount")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("flickerfrequency")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("scaleu")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("scalev")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("offsetu")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("offsetv")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("particlealpha")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("particlescale")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("minrotationspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("maxrotationspeed")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("param0")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("param1")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("param2")),
    TEMPORAL_PROPERTY_RANGE_STRINGS(_T("param3")),
};

enum ETemporalPropertyRange
{
    TEMPORAL_PROPERTY_RANGE_ENUMS(SpawnRate),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinParticleLife),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxParticleLife),
    TEMPORAL_PROPERTY_RANGE_ENUMS(ParticleTimeNudge),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Gravity),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinInheritVelocity),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxInheritVelocity),
    TEMPORAL_PROPERTY_RANGE_ENUMS(LimitInheritVelocity),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinOffsetDirection),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxOffsetDirection),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinOffsetRadial),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxOffsetRadial),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinOffsetRadialAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxOffsetRadialAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinOrbitAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxOrbitAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinTrackSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxTrackSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(TexPosTime),
    TEMPORAL_PROPERTY_RANGE_ENUMS(TexPosScale),
    TEMPORAL_PROPERTY_RANGE_ENUMS(TexStretchScale),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Size),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Taper),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Tile),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Frame),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Param),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Pitch),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Roll),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Yaw),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Width),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Height),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Scale),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Length),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Speed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Acceleration),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinAcceleration),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxAcceleration),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Range),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Alpha),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Stickiness),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Anchor),
    TEMPORAL_PROPERTY_RANGE_ENUMS(WidthDistort),
    TEMPORAL_PROPERTY_RANGE_ENUMS(HeightDistort),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Angle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxAngle),
    TEMPORAL_PROPERTY_RANGE_ENUMS(FalloffStart),
    TEMPORAL_PROPERTY_RANGE_ENUMS(FalloffEnd),
    TEMPORAL_PROPERTY_RANGE_ENUMS(FlickerAmount),
    TEMPORAL_PROPERTY_RANGE_ENUMS(FlickerFrequency),
    TEMPORAL_PROPERTY_RANGE_ENUMS(ScaleU),
    TEMPORAL_PROPERTY_RANGE_ENUMS(ScaleV),
    TEMPORAL_PROPERTY_RANGE_ENUMS(OffsetU),
    TEMPORAL_PROPERTY_RANGE_ENUMS(OffsetV),
    TEMPORAL_PROPERTY_RANGE_ENUMS(ParticleAlpha),
    TEMPORAL_PROPERTY_RANGE_ENUMS(ParticleScale),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MinRotationSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(MaxRotationSpeed),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Param0),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Param1),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Param2),
    TEMPORAL_PROPERTY_RANGE_ENUMS(Param3)
};

#define BEGIN_EMITTER_XML_PROCESSOR(name, type) \
BEGIN_XML_PROCESSOR_DECLARATION(name) \
    DECLARE_XML_SUBPROCESSOR(CEffect) \
    DECLARE_XML_SUBPROCESSOR(CParticleSystemDef) \
    DECLARE_XML_SUBPROCESSOR(CSimpleParticleDef) \
    DECLARE_XML_SUBPROCESSOR(CControllerEmitterDef) \
    DECLARE_XML_SUBPROCESSOR(CModelEmitterDef) \
    DECLARE_XML_SUBPROCESSOR(CDebrisEmitterDef) \
END_XML_PROCESSOR_DECLARATION

#define END_EMITTER_XML_PROCESSOR(name, type) \
BEGIN_XML_REGISTRATION(name) \
    REGISTER_XML_PROCESSOR(definitions) \
    REGISTER_XML_PROCESSOR(particlesystem) \
    REGISTER_XML_PROCESSOR(particle) \
    REGISTER_XML_PROCESSOR(controller) \
    REGISTER_XML_PROCESSOR(model) \
    REGISTER_XML_PROCESSOR(debris) \
END_XML_REGISTRATION \
BEGIN_XML_PROCESSOR(name, void) \
    if (pParent->GetName() == _T("definitions")) \
        Process(node, static_cast<CEffect*>(pVoid)); \
    else if (pParent->GetName() == _T("particlesystem")) \
        Process(node, static_cast<CParticleSystemDef*>(pVoid)); \
    else if (pParent->GetName() == _T("particle")) \
        Process(node, static_cast<CSimpleParticleDef*>(pVoid)); \
    else if (pParent->GetName() == _T("controller")) \
        Process(node, static_cast<CControllerEmitterDef*>(pVoid)); \
    else if (pParent->GetName() == _T("model")) \
        Process(node, static_cast<CModelEmitterDef*>(pVoid)); \
    else if (pParent->GetName() == _T("debris")) \
        Process(node, static_cast<CDebrisEmitterDef*>(pVoid)); \
    else \
        return false; \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CEffect) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(node.GetProperty(_T("name")), pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CParticleSystemDef) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CSimpleParticleDef) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CControllerEmitterDef) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CModelEmitterDef) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN \
BEGIN_XML_SUBPROCESSOR(name, CDebrisEmitterDef) \
    C##type##Def *pEmitter(Process##type(node)); \
    if (pEmitter != NULL) \
    { \
        ProcessChildren(node, pEmitter); \
        pObject->AddEmitterDef(pEmitter); \
    } \
END_XML_PROCESSOR_NO_CHILDREN

const tstring g_sSource(_T("source"));
//=============================================================================

#if 0
/*====================
  GetTemporalProperty

  Temporal properties consist of four different values for the
  different combinations of start/end (temporal) and min/max
  (random range). A tag property that contains start/end or
  min/max only will set both of the associated values. So for
  example, "startscale" sets both "minstartscale" and
  "maxstartscale", "scale" would set all four combinations, or
  the individual values can be set directly. bDynamic returns
  whether or not the temporal property is dynamic, this is to
  fix the issue of having two different random numbers being
  generated if both the start/end values of a range set when it
  isn't intended, i.e. if the start/end ranges have been with via
  a solo min/max property.
  ====================*/
#define GET_TEMPORAL_PROPERTY(node, name, fMinStart, fMaxStart, fMinEnd, fMaxEnd, fMinMid, fMaxMid, fMinMidPos, fMaxMidPos, bDynamic) \
{ \
    if (node.HasProperty(name)) \
    { \
        float   fValue(node.GetPropertyFloat(name)); \
        fMinStart = fMaxStart = fMinMid = fMaxMid = fMinEnd = fMaxEnd = fValue; \
    } \
 \
    if (node.HasProperty(_T("start") name)) \
    { \
        float   fStart(node.GetPropertyFloat(_T("start") name)); \
        fMinStart = fMaxStart = fMinMid = fMaxMid = fStart; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("end") name)) \
    { \
        float   fEnd(node.GetPropertyFloat(_T("end") name)); \
        fMinEnd = fMaxEnd = fEnd; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("mid") name)) \
    { \
        float   fMid(node.GetPropertyFloat(_T("mid") name)); \
        fMinMid = fMaxMid = fMid; \
        fMinMidPos = fMaxMidPos = 0.5f; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("mid") name _T("pos"))) \
    { \
        float   fMidPos(node.GetPropertyFloat(_T("mid") name _T("pos"))); \
        fMinMidPos = fMaxMidPos = fMidPos; \
 \
        if (!node.HasProperty(_T("mid") name)) \
        { \
            fMinMid = (fMinStart + fMinEnd) / 2.0f; \
            fMaxMid = (fMaxStart + fMaxEnd) / 2.0f; \
        } \
 \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("min") name)) \
    { \
        float   fMin(node.GetPropertyFloat(_T("min") name)); \
        fMinStart = fMinEnd = fMinMid = fMin; \
    } \
 \
    if (node.HasProperty(_T("max") name)) \
    { \
        float   fMax(node.GetPropertyFloat(_T("max") name)); \
        fMaxStart = fMaxEnd = fMaxMid = fMax; \
    } \
 \
    if (node.HasProperty(_T("minstart") name)) \
    { \
        fMinMid = fMinStart = node.GetPropertyFloat(_T("minstart") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("maxstart") name)) \
    { \
        fMaxMid = fMaxStart = node.GetPropertyFloat(_T("maxstart") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("minend") name)) \
    { \
        fMinEnd = node.GetPropertyFloat(_T("minend") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("maxend") name)) \
    { \
        fMaxEnd = node.GetPropertyFloat(_T("maxend") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("minmid") name)) \
    { \
        fMinMid = node.GetPropertyFloat(_T("minmid") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("maxmid") name)) \
    { \
        fMaxMid = node.GetPropertyFloat(_T("maxmid") name); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("minmid") name _T("pos"))) \
    { \
        fMinMidPos = node.GetPropertyFloat(_T("minmid") name _T("pos")); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(_T("maxmid") name _T("pos"))) \
    { \
        fMaxMidPos = node.GetPropertyFloat(_T("maxmid") name _T("pos")); \
        bDynamic = true; \
    } \
}
#endif


/*====================
  GetTemporalProperty

  Temporal properties consist of four different values for the
  different combinations of start/end (temporal) and min/max
  (random range). A tag property that contains start/end or
  min/max only will set both of the associated values. So for
  example, "startscale" sets both "minstartscale" and
  "maxstartscale", "scale" would set all four combinations, or
  the individual values can be set directly. bDynamic returns
  whether or not the temporal property is dynamic, this is to
  fix the issue of having two different random numbers being
  generated if both the start/end values of a range set when it
  isn't intended, i.e. if the start/end ranges have been with via
  a solo min/max property.
  ====================*/
#define GET_TEMPORAL_PROPERTY(node, property, fMinStart, fMaxStart, fMinEnd, fMaxEnd, fMinMid, fMaxMid, fMinMidPos, fMaxMidPos, bDynamic) \
{ \
    if (node.HasProperty(g_sTemporalPropertyRange[_##property])) \
    { \
        float   fValue(node.GetPropertyFloat(g_sTemporalPropertyRange[_##property])); \
        fMinStart = fMaxStart = fMinMid = fMaxMid = fMinEnd = fMaxEnd = fValue; \
    } \
 \
 if (node.HasProperty(g_sTemporalPropertyRange[_Start_##property])) \
    { \
        float   fStart(node.GetPropertyFloat(g_sTemporalPropertyRange[_Start_##property])); \
        fMinStart = fMaxStart = fMinMid = fMaxMid = fStart; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_End_##property])) \
    { \
        float   fEnd(node.GetPropertyFloat(g_sTemporalPropertyRange[_End_##property])); \
        fMinEnd = fMaxEnd = fEnd; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_Min_##property])) \
    { \
        float   fMin(node.GetPropertyFloat(g_sTemporalPropertyRange[_Min_##property])); \
        fMinStart = fMinEnd = fMinMid = fMin; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_Max_##property])) \
    { \
        float   fMax(node.GetPropertyFloat(g_sTemporalPropertyRange[_Max_##property])); \
        fMaxStart = fMaxEnd = fMaxMid = fMax; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MinStart_##property])) \
    { \
        fMinMid = fMinStart = node.GetPropertyFloat(g_sTemporalPropertyRange[_MinStart_##property]); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MaxStart_##property])) \
    { \
        fMaxMid = fMaxStart = node.GetPropertyFloat(g_sTemporalPropertyRange[_MaxStart_##property]); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MinEnd_##property])) \
    { \
        fMinEnd = node.GetPropertyFloat(g_sTemporalPropertyRange[_MinEnd_##property]); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MaxEnd_##property])) \
    { \
        fMaxEnd = node.GetPropertyFloat(g_sTemporalPropertyRange[_MaxEnd_##property]); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MinMid_##property])) \
    { \
        fMinMid = node.GetPropertyFloat(g_sTemporalPropertyRange[_MinMid_##property]); \
        fMinMidPos = fMaxMidPos = 0.5f; \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MaxMid_##property])) \
    { \
        fMaxMid = node.GetPropertyFloat(g_sTemporalPropertyRange[_MaxMid_##property]); \
        fMinMidPos = fMaxMidPos = 0.5f; \
        bDynamic = true; \
    } \
\
    if (node.HasProperty(g_sTemporalPropertyRange[_Mid_##property])) \
    { \
        float fMid(node.GetPropertyFloat(g_sTemporalPropertyRange[_Mid_##property])); \
        fMinMid = fMaxMid = fMid; \
        fMinMidPos = fMaxMidPos = 0.5f; \
        bDynamic = true; \
    } \
\
    if (node.HasProperty(g_sTemporalPropertyRange[_Mid_##property##Pos])) \
    { \
        float fMidPos(node.GetPropertyFloat(g_sTemporalPropertyRange[_Mid_##property##Pos])); \
        fMinMidPos = fMaxMidPos = fMidPos; \
\
        if (!node.HasProperty(g_sTemporalPropertyRange[_Mid_##property])) \
        { \
            fMinMid = (fMinStart + fMinEnd) / 2.0f; \
            fMaxMid = (fMaxStart + fMaxEnd) / 2.0f; \
        } \
\
        bDynamic = true; \
    } \
\
    if (node.HasProperty(g_sTemporalPropertyRange[_MinMid_##property##Pos])) \
    { \
        fMinMidPos = node.GetPropertyFloat(g_sTemporalPropertyRange[_MinMid_##property##Pos]); \
        bDynamic = true; \
    } \
 \
    if (node.HasProperty(g_sTemporalPropertyRange[_MaxMid_##property##Pos])) \
    { \
        fMaxMidPos = node.GetPropertyFloat(g_sTemporalPropertyRange[_MaxMid_##property##Pos]); \
        bDynamic = true; \
    } \
}


/*====================
  GET_TEMPORAL_PROPERTY_RANGE
  ====================*/
#define GET_TEMPORAL_PROPERTY_RANGE(name, unused, value) \
    float f##name(node.GetPropertyFloat(g_sTemporalPropertyRange[_##name], value)); \
        float fMinStart##name(f##name), fMaxStart##name(f##name), \
            fMinEnd##name(f##name), fMaxEnd##name(f##name), \
            fMinMid##name(f##name), fMaxMid##name(f##name), \
            fMinMid##name##Pos(0.0f), fMaxMid##name##Pos(0.0f); \
        bool bDynamic##name(false); \
        GET_TEMPORAL_PROPERTY(node, name, fMinStart##name, fMaxStart##name, fMinEnd##name, fMaxEnd##name, \
            fMinMid##name, fMaxMid##name, fMinMid##name##Pos, fMaxMid##name##Pos, bDynamic##name); \
        float f##name##Speed(node.GetPropertyFloat(g_sTemporalPropertyRange[_##name##Speed], 0.0f)); \
        float fMin##name##Speed(node.GetPropertyFloat(g_sTemporalPropertyRange[_Min_##name##Speed], f##name##Speed)); \
        float fMax##name##Speed(node.GetPropertyFloat(g_sTemporalPropertyRange[_Max_##name##Speed], f##name##Speed));


/*====================
  GET_TEMPORAL_PROPERTY_RANGE_CHILD
  ====================*/
#define GET_TEMPORAL_PROPERTY_RANGE_CHILD(name, unused, parent) \
        float fMinStart##name(fMinStart##parent), fMaxStart##name(fMaxStart##parent), \
            fMinEnd##name(fMinEnd##parent), fMaxEnd##name(fMaxEnd##parent), \
            fMinMid##name(fMinMid##parent), fMaxMid##name(fMaxMid##parent), \
            fMinMid##name##Pos(fMinMid##parent##Pos), fMaxMid##name##Pos(fMaxMid##parent##Pos); \
        bool bDynamic##name(bDynamic##parent); \
        GET_TEMPORAL_PROPERTY(node, name, fMinStart##name, fMaxStart##name, fMinEnd##name, fMaxEnd##name, \
            fMinMid##name, fMaxMid##name, fMinMid##name##Pos, fMaxMid##name##Pos, bDynamic##name); \
        float fMin##name##Speed(node.GetPropertyFloat(g_sTemporalPropertyRange[_Min_##name##Speed], fMin##parent##Speed)); \
        float fMax##name##Speed(node.GetPropertyFloat(g_sTemporalPropertyRange[_Max_##name##Speed], fMax##parent##Speed));


/*====================
  TEMPORAL_PROPERTY_RANGE_FLOAT
  ====================*/
#define TEMPORAL_PROPERTY_RANGE_FLOAT(name) \
    CTemporalPropertyRangef(fMinStart##name, fMaxStart##name, fMinEnd##name, fMaxEnd##name, \
        fMinMid##name, fMaxMid##name, fMinMid##name##Pos, fMaxMid##name##Pos, bDynamic##name, \
        fMin##name##Speed, fMax##name##Speed)


/*====================
  TEMPORAL_PROPERTY_RANGE_INT
  ====================*/
#define TEMPORAL_PROPERTY_RANGE_INT(name) \
    CTemporalPropertyRangei(int(fMinStart##name), int(fMaxStart##name), int(fMinEnd##name), int(fMaxEnd##name), \
        int(fMinMid##name), int(fMaxMid##name), fMinMid##name##Pos, fMaxMid##name##Pos, bDynamic##name, \
        int(fMin##name##Speed), int(fMax##name##Speed))


/*====================
  GET_TEMPORAL_PROPERTY_VEC3
  ====================*/
#define GET_TEMPORAL_PROPERTY_VEC3(name, property, value) \
    CVec3f  v3##name(node.GetPropertyV3(property, value)); \
    CVec3f  v3Start##name(node.GetPropertyV3(_T("start") property, v3##name)); \
    CVec3f  v3End##name(node.GetPropertyV3(_T("end") property, v3##name)); \
    CVec3f  v3Mid##name(node.GetPropertyV3(_T("mid") property, v3Start##name)); \
    float   fMid##name##Pos(node.GetPropertyFloat(_T("mid") property _T("pos"), 0.0f)); \
    if (!node.HasProperty(_T("mid") property) && node.HasProperty(_T("mid") property _T("pos"))) \
        v3Mid##name = (v3Start##name + v3End##name) / 2.0f; \
    if (node.HasProperty(_T("mid") property) && !node.HasProperty(_T("mid") property _T("pos"))) \
        fMid##name##Pos = 0.5f; \
    CVec3f  v3##name##Speed(node.GetPropertyV3(property _T("speed"), CVec3f(0.0f, 0.0f, 0.0f)));


/*====================
  TEMPORAL_PROPERTY_VEC3
  ====================*/
#define TEMPORAL_PROPERTY_VEC3(name) \
    CTemporalPropertyv3(v3Start##name, v3End##name, v3Mid##name, fMid##name##Pos, v3##name##Speed)


/*====================
  GET_TEMPORAL_PROPERTY_FLOAT
  ====================*/
#define GET_TEMPORAL_PROPERTY_FLOAT(name, property, value) \
    float   f##name(node.GetPropertyFloat(property, value)); \
    float   fStart##name(node.GetPropertyFloat(_T("start") property, f##name)); \
    float   fEnd##name(node.GetPropertyFloat(_T("end") property, f##name)); \
    float   fMid##name(node.GetPropertyFloat(_T("mid") property, fStart##name)); \
    float   fMid##name##Pos(node.GetPropertyFloat(_T("mid") property _T("pos"), 0.0f)); \
    if (!node.HasProperty(_T("mid") property) && node.HasProperty(_T("mid") property _T("pos"))) \
        fMid##name = (fStart##name + fEnd##name) / 2.0f; \
    if (node.HasProperty(_T("mid") property) && !node.HasProperty(_T("mid") property _T("pos"))) \
        fMid##name##Pos = 0.5f; \
    float   f##name##Speed(node.GetPropertyFloat(property _T("speed"), 0.0f));


/*====================
  TEMPORAL_PROPERTY_FLOAT
  ====================*/
#define TEMPORAL_PROPERTY_FLOAT(name) \
    CTemporalPropertyf(fStart##name, fEnd##name, fMid##name, fMid##name##Pos, f##name##Speed)


/*====================
  GetDirectionalSpace
  ====================*/
EDirectionalSpace   GetDirectionalSpace(const tstring &sSpace)
{
    if (sSpace == _T("global"))
        return DIRSPACE_GLOBAL;
    else if (sSpace == _T("local"))
        return DIRSPACE_LOCAL;
    else
    {
        Console.Warn << _T("Unrecognized space ") << SingleQuoteStr(sSpace) << newl;
        return DIRSPACE_GLOBAL;
    }
}


/*====================
  GetSpace
  ====================*/
ESystemSpace    GetSpace(const tstring &sSpace)
{
    if (sSpace == _T("world"))
        return WORLD_SPACE;
    else if (sSpace == _T("entity"))
        return ENTITY_SPACE;
    else if (sSpace == _T("bone"))
        //return BONE_SPACE;
        return ENTITY_SPACE;
    else
    {
        Console.Warn << _T("Unrecognized space ") << SingleQuoteStr(sSpace) << newl;
        return WORLD_SPACE;
    }
}


/*====================
  ProcessSimpleEmitter
  ====================*/
CSimpleEmitterDef*  ProcessSimpleEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    int iParticleLifeAlloc(node.GetPropertyInt(_T("particlelifealloc"), -1));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));
    bool bSubFramePose(node.GetPropertyBool(_T("subframepose"), false));

    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CSimpleEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        iParticleLifeAlloc,
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        bSubFramePose,
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessTwoPointEmitter
  ====================*/
CTwoPointEmitterDef*    ProcessTwoPointEmitter(const CXMLNode &node)
{
    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    int iParticleLifeAlloc(node.GetPropertyInt(_T("particlelifealloc"), -1));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sOwnerA(node.GetProperty(_T("owner_a"), g_sSource));
    const tstring &sOwnerB(node.GetProperty(_T("owner_b"), g_sSource));
    const tstring &sBoneA(node.GetProperty(_T("bone_a")));
    const tstring &sBoneB(node.GetProperty(_T("bone_b")));
    CVec3f  v3PosA(node.GetPropertyV3(_T("position_a"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3PosB(node.GetPropertyV3(_T("position_b"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));
    bool bSubFramePose(node.GetPropertyBool(_T("subframepose"), false));

    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CTwoPointEmitterDef)(
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        iParticleLifeAlloc,
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sOwnerA,
        sOwnerB,
        sBoneA,
        sBoneB,
        v3PosA,
        v3PosB,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        bSubFramePose,
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessOrbiterEmitter
  ====================*/
COrbiterEmitterDef* ProcessOrbiterEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 0.0));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Origin(node.GetPropertyV3(_T("origin"), CVec3f(0.0f, 0.0f, 0.0f)));
    bool    bCylindrical(node.GetPropertyBool(_T("cylindrical"), false));

    GET_TEMPORAL_PROPERTY_VEC3(Offset, _T("offset"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(Orbit, _T("orbit"), CVec3f(1.0f, 0.0f, 0.0f));

    float fOrbitAngle(node.GetPropertyFloat(_T("orbitangle"), 0.0));
    GET_TEMPORAL_PROPERTY_RANGE(MinOrbitAngle, _T("minorbitangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOrbitAngle, _T("maxorbitangle"), fOrbitAngle);

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  COrbiterEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sBone,
        v3Pos,
        V3_ZERO,
        v3Origin,
        TEMPORAL_PROPERTY_VEC3(Offset),
        bCylindrical,
        TEMPORAL_PROPERTY_VEC3(Orbit),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOrbitAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOrbitAngle),
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  GetTrackType
  ====================*/
ETrackType  GetTrackType(const tstring &sTrackType)
{
    if (sTrackType == _T("distance"))
        return TRACK_DISTANCE;
    else if (sTrackType == _T("angular"))
        return TRACK_ANGULAR;
    else if (sTrackType == _T("gravity"))
        return TRACK_GRAVITY;
    else if (sTrackType == _T("cgravity"))
        return TRACK_CGRAVITY;
    else if (sTrackType == _T("target"))
        return TRACK_TARGET;
    else if (sTrackType == _T("lerp"))
        return TRACK_LERP;
    else
    {
        Console.Warn << _T("Unrecognized track type ") << SingleQuoteStr(sTrackType) << newl;
        return TRACK_DISTANCE;
    }
}


/*====================
  ProcessTrackerEmitter
  ====================*/
CTrackerEmitterDef* ProcessTrackerEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    int iParticleLifeAlloc(node.GetPropertyInt(_T("particlelifealloc"), -1));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));
    const tstring &sTargetBone(node.GetProperty(_T("targetbone")));
    CVec3f  v3TargetPos(node.GetPropertyV3(_T("targetposition"), CVec3f(0.0f, 0.0f, 0.0f)));
    const tstring &sTargetOwner(node.GetProperty(_T("targetowner"), g_sSource));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    ETrackType  eTrackType(GetTrackType(node.GetProperty(_T("tracktype"), _T("distance"))));

    float fTrackSpeed(node.GetPropertyFloat(_T("trackspeed"), 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinTrackSpeed, _T("minoffsetdirection"), fTrackSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxTrackSpeed, _T("maxoffsetdirection"), fTrackSpeed);

    bool bDistanceLife(node.GetPropertyBool(_T("distancelife"), true));
    bool bSubFramePose(node.GetPropertyBool(_T("subframepose"), false));

    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CTrackerEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        iParticleLifeAlloc,
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sBone,
        v3Pos,
        v3Offset,
        sTargetBone,
        v3TargetPos,
        sTargetOwner,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        eTrackType,
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinTrackSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxTrackSpeed),
        bDistanceLife,
        bSubFramePose,
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}

    
/*====================
  ProcessMeshEmitter
  ====================*/
CMeshEmitterDef*    ProcessMeshEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sMesh(node.GetProperty(_T("mesh")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));

    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CMeshEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sMesh,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessSkeletonEmitter
  ====================*/
CSkeletonEmitterDef*    ProcessSkeletonEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));
    
    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CSkeletonEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessTwoPointTrailEmitter
  ====================*/
CTwoPointTrailEmitterDef*   ProcessTwoPointTrailEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBoneA(node.GetProperty(_T("bone_a")));
    const tstring &sBoneB(node.GetProperty(_T("bone_b")));
    CVec3f  v3PosA(node.GetPropertyV3(_T("position_a"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3PosB(node.GetPropertyV3(_T("position_b"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    GET_TEMPORAL_PROPERTY_RANGE(TexPosTime, _T("texpostime"), 250.0f);
    GET_TEMPORAL_PROPERTY_RANGE(TexPosScale, _T("texposscale"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(TexStretchScale, _T("texstretchscale"), 0.0f);

    bool bSubFramePose(node.GetPropertyBool(_T("subframepose"), false));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);

    return K2_NEW(ctx_Effects,  CTwoPointTrailEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        fDrag,
        fFriction,
        sBoneA,
        sBoneB,
        v3PosA,
        v3PosB,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_INT(TexPosTime),
        TEMPORAL_PROPERTY_RANGE_FLOAT(TexPosScale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(TexStretchScale),
        bSubFramePose,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha)
    );
}


/*====================
  ProcessTrailEmitter
  ====================*/
CTrailEmitterDef* ProcessTrailEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    GET_TEMPORAL_PROPERTY_RANGE(TexPosTime, _T("texpostime"), 250.0f);
    GET_TEMPORAL_PROPERTY_RANGE(TexPosScale, _T("texposscale"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(TexStretchScale, _T("texstretchscale"), 0.0f);

    bool bSubFramePose(node.GetPropertyBool(_T("subframepose"), false));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CTrailEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        fDrag,
        fFriction,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_INT(TexPosTime),
        TEMPORAL_PROPERTY_RANGE_FLOAT(TexPosScale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(TexStretchScale),
        bSubFramePose,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessLightEmitter
  ====================*/
CLightEmitterDef*   ProcessLightEmitter(const CXMLNode &node)
{
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(FalloffStart, _T("falloffstart"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(FalloffEnd, _T("falloffend"), 500.0f);
    GET_TEMPORAL_PROPERTY_RANGE(FlickerAmount, _T("flickeramount"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(FlickerFrequency, _T("flickerfrequency"), 1.0f);

    return K2_NEW(ctx_Effects,  CLightEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_RANGE_FLOAT(FalloffStart),
        TEMPORAL_PROPERTY_RANGE_FLOAT(FalloffEnd),
        TEMPORAL_PROPERTY_RANGE_FLOAT(FlickerAmount),
        TEMPORAL_PROPERTY_RANGE_FLOAT(FlickerFrequency)
    );
}


/*====================
  ProcessBeamEmitter
  ====================*/
CBeamEmitterDef*    ProcessBeamEmitter(const CXMLNode &node)
{
    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sOwnerA(node.GetProperty(_T("owner_a"), g_sSource));
    const tstring &sOwnerB(node.GetProperty(_T("owner_b"), g_sSource));
    const tstring &sBoneA(node.GetProperty(_T("bone_a")));
    const tstring &sBoneB(node.GetProperty(_T("bone_b")));
    CVec3f  v3PosA(node.GetPropertyV3(_T("position_a"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3PosB(node.GetPropertyV3(_T("position_b"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Size, _T("size"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Taper, _T("taper"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Tile, _T("tile"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Frame, _T("frame"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param, _T("param"), 0.0f);
    
    const tstring &sMaterial(node.GetProperty(_T("material")));

    return K2_NEW(ctx_Effects,  CBeamEmitterDef)(
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        sOwnerA,
        sOwnerB,
        sBoneA,
        sBoneB,
        v3PosA,
        v3PosB,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Size),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Taper),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Tile),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Frame),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL)
    );
}


/*====================
  ProcessGroundSpriteEmitter
  ====================*/
CGroundSpriteEmitterDef*    ProcessGroundSpriteEmitter(const CXMLNode &node)
{
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Roll, _T("roll"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Size, _T("size"), 64.0f);
    GET_TEMPORAL_PROPERTY_RANGE_CHILD(Width, _T("width"), Size);
    GET_TEMPORAL_PROPERTY_RANGE_CHILD(Height, _T("height"), Size);

    GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Frame, _T("frame"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param, _T("param"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));

    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("global"))));

    return K2_NEW(ctx_Effects,  CGroundSpriteEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Roll),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Width),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Height),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Frame),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        eDirectionalSpace
    );
}


/*====================
  ProcessBillboardEmitter
  ====================*/
CBillboardEmitterDef*   ProcessBillboardEmitter(const CXMLNode &node)
{
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Roll, _T("roll"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Size, _T("size"), 64.0f);
    GET_TEMPORAL_PROPERTY_RANGE_CHILD(Width, _T("width"), Size);
    GET_TEMPORAL_PROPERTY_RANGE_CHILD(Height, _T("height"), Size);

    GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Frame, _T("frame"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param, _T("param"), 0.0f);
    
    GET_TEMPORAL_PROPERTY_RANGE(ScaleU, _T("scaleu"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ScaleV, _T("scalev"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(OffsetU, _T("offsetu"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(OffsetV, _T("offsetv"), 0.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias")));

    uint uiFlags(0);
    if (node.GetPropertyBool(_T("lockup"), false))
        uiFlags |= BBOARD_LOCK_UP;
    if (node.GetPropertyBool(_T("lockright"), false))
        uiFlags |= BBOARD_LOCK_RIGHT;
    if (node.GetPropertyBool(_T("turn"), false))
        uiFlags |= BBOARD_TURN;
    if (node.GetPropertyBool(_T("flare"), false))
        uiFlags |= BBOARD_FLARE;
    if (node.GetPropertyBool(_T("alphacolor"), false))
        uiFlags |= BBOARD_ALPHACOLOR;

    tstring sMaterial(node.GetProperty(_T("material")));

    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("global"))));

    return K2_NEW(ctx_Effects,  CBillboardEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Roll),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Width),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Height),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Frame),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param),
        fDepthBias,
        uiFlags,
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        eDirectionalSpace,
        TEMPORAL_PROPERTY_RANGE_FLOAT(ScaleU),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ScaleV),
        TEMPORAL_PROPERTY_RANGE_FLOAT(OffsetU),
        TEMPORAL_PROPERTY_RANGE_FLOAT(OffsetV)
    );
}


/*====================
  ProcessModelEmitter
  ====================*/
CModelEmitterDef*   ProcessModelEmitter(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_T("name")));
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));
    
    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Roll, _T("roll"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Param0, _T("param0"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param1, _T("param1"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param2, _T("param2"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param3, _T("param3"), 0.0f);

    const tstring &sModel(node.GetProperty(_T("model")));
    const tstring &sSkin(node.GetProperty(_T("skin")));
    const tstring &sMaterial(node.GetProperty(_T("material")));
    const tstring &sAnim(node.GetProperty(_T("anim")));

    ResHandle hModel(g_ResourceManager.Register(sModel, RES_MODEL));
    SkinHandle hSkin(g_ResourceManager.GetSkin(hModel, sSkin));
    g_ResourceManager.PrecacheSkin(hModel, hSkin);

    ResHandle hMaterial(g_ResourceManager.Register(sMaterial, RES_MATERIAL));

    bool bParentModel(node.GetPropertyBool(_T("parentmodel"), false));
    bool bParentSkeleton(node.GetPropertyBool(_T("parentskeleton"), false));

    static tsvector vEmitters;
    vEmitters.clear();

    if (node.HasProperty(_T("emitter")))
        vEmitters.push_back(node.GetProperty(_T("emitter")));
    for (int i(0); i < 32; ++i)
    {
        tstring sName(_T("emitter") + XtoA(i));
        
        if (!node.HasProperty(sName))
            break;

        vEmitters.push_back(node.GetProperty(sName));
    }

    return K2_NEW(ctx_Effects,  CModelEmitterDef)(
        sName,
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        eDirectionalSpace,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Roll),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param0),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param1),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param2),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param3),
        hModel,
        hSkin,
        hMaterial,
        sAnim,
        bParentModel,
        bParentSkeleton,
        vEmitters
    );
}


/*====================
  ProcessControllerEmitter
  ====================*/
CControllerEmitterDef*  ProcessControllerEmitter(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_T("name")));
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));
    
    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Roll, _T("roll"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);

    static tsvector vEmitters;
    vEmitters.clear();

    if (node.HasProperty(_T("emitter")))
        vEmitters.push_back(node.GetProperty(_T("emitter")));
    for (int i(0); i < 32; ++i)
    {
        tstring sName(_T("emitter") + XtoA(i));
        
        if (!node.HasProperty(sName))
            break;

        vEmitters.push_back(node.GetProperty(sName));
    }

    bool bLookAt(node.GetPropertyBool(_T("lookat"), false));
    CVec3f v3LookAtPos(node.GetPropertyV3(_T("lookatposition"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f v3LookAtOffset(node.GetPropertyV3(_T("lookatoffset"), CVec3f(0.0f, 0.0f, 0.0f)));
    const tstring &sLookAtOwner(node.GetProperty(_T("lookatowner"), g_sSource));
    const tstring &sLookAtBone(node.GetProperty(_T("lookatbone")));
    EDirectionalSpace eLookAtDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("lookatdirectionalspace"), _T("local"))));

    return K2_NEW(ctx_Effects,  CControllerEmitterDef)(
        sName,
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        eDirectionalSpace,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Roll),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
        vEmitters,
        bLookAt,
        v3LookAtPos,
        v3LookAtOffset,
        sLookAtOwner,
        sLookAtBone,
        eLookAtDirectionalSpace
    );
}


/*====================
  ProcessDebrisEmitter
  ====================*/
CDebrisEmitterDef*  ProcessDebrisEmitter(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_T("name")));
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));
    
    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Roll, _T("roll"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Param0, _T("param0"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param1, _T("param1"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param2, _T("param2"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param3, _T("param3"), 0.0f);

    const tstring &sModel(node.GetProperty(_T("model")));
    const tstring &sSkin(node.GetProperty(_T("skin")));
    const tstring &sMaterial(node.GetProperty(_T("material")));
    const tstring &sAnim(node.GetProperty(_T("anim")));

    ResHandle hModel(g_ResourceManager.Register(sModel, RES_MODEL));
    SkinHandle hSkin(g_ResourceManager.GetSkin(hModel, sSkin));
    g_ResourceManager.PrecacheSkin(hModel, hSkin);

    ResHandle hMaterial(g_ResourceManager.Register(sMaterial, RES_MATERIAL));

    static tsvector vEmitters;
    vEmitters.clear();

    if (node.HasProperty(_T("emitter")))
        vEmitters.push_back(node.GetProperty(_T("emitter")));
    for (int i(0); i < 32; ++i)
    {
        tstring sName(_T("emitter") + XtoA(i));
        
        if (!node.HasProperty(sName))
            break;

        vEmitters.push_back(node.GetProperty(sName));
    }

    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);
    
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));

    float fRotationSpeed(node.GetPropertyFloat(_T("rotationspeed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinRotationSpeed, _T("minrotationspeed"), fRotationSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxRotationSpeed, _T("maxrotationspeed"), fRotationSpeed);

    float   fBounce(node.GetPropertyFloat(_T("bounce"), 0.0f));
    float   fReflect(node.GetPropertyFloat(_T("reflect"), 0.0f));

    bool bAnimPose(node.GetPropertyBool(_T("animpose"), false));
    bool bUseAnim(node.GetPropertyBool(_T("useanim"), false));

    return K2_NEW(ctx_Effects,  CDebrisEmitterDef)(
        sName,
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        eDirectionalSpace,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Roll),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param0),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param1),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param2),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param3),
        hModel,
        hSkin,
        hMaterial,
        sAnim,
        vEmitters,
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        v3Dir,
        fDrag,
        fFriction,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinRotationSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxRotationSpeed),
        fBounce,
        fReflect,
        bAnimPose,
        bUseAnim
    );
}


/*====================
  ProcessTracerEmitter
  ====================*/
CTracerEmitterDef*  ProcessTracerEmitter(const CXMLNode &node)
{
    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sOwnerA(node.GetProperty(_T("owner_a"), g_sSource));
    const tstring &sOwnerB(node.GetProperty(_T("owner_b"), g_sSource));
    const tstring &sBoneA(node.GetProperty(_T("bone_a")));
    const tstring &sBoneB(node.GetProperty(_T("bone_b")));
    CVec3f  v3PosA(node.GetPropertyV3(_T("position_a"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3PosB(node.GetPropertyV3(_T("position_b"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

    GET_TEMPORAL_PROPERTY_RANGE(Width, _T("width"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Length, _T("length"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Speed, _T("speed"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Acceleration, _T("acceleration"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Taper, _T("taper"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Tile, _T("tile"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Frame, _T("frame"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Param, _T("param"), 0.0f);
    
    const tstring &sMaterial(node.GetProperty(_T("material")));

    return K2_NEW(ctx_Effects,  CTracerEmitterDef)(
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        sOwnerA,
        sOwnerB,
        sBoneA,
        sBoneB,
        v3PosA,
        v3PosB,
        TEMPORAL_PROPERTY_VEC3(Color),
        TEMPORAL_PROPERTY_FLOAT(Alpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Width),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Length),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Speed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Acceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Taper),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Tile),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Frame),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Param),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL)
    );
}


/*====================
  ProcessSoundEmitter
  ====================*/
CSoundEmitterDef*   ProcessSoundEmitter(const CXMLNode &node)
{
    const tstring &sSample(node.GetProperty(_T("sample")));
    if (sSample.empty())
        return NULL;

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    float fFalloff(node.GetPropertyFloat(_T("falloff"), 1.0f));
    float fMinFalloff(node.GetPropertyFloat(_T("minfalloff"), fFalloff));
    float fMaxFalloff(node.GetPropertyFloat(_T("maxfalloff"), fFalloff));

    float fFalloffStart(node.GetPropertyFloat(_T("falloffstart"), 1.0f));
    float fMinFalloffStart(node.GetPropertyFloat(_T("minfalloffstart"), fFalloffStart));
    float fMaxFalloffStart(node.GetPropertyFloat(_T("maxfalloffstart"), fFalloffStart));

    float fFalloffEnd(node.GetPropertyFloat(_T("falloffend"), 1.0f));
    float fMinFalloffEnd(node.GetPropertyFloat(_T("minfalloffend"), fFalloffEnd));
    float fMaxFalloffEnd(node.GetPropertyFloat(_T("maxfalloffend"), fFalloffEnd));

    float fVolume(node.GetPropertyFloat(_T("volume"), 1.0f));
    float fMinVolume(node.GetPropertyFloat(_T("minvolume"), fVolume));
    float fMaxVolume(node.GetPropertyFloat(_T("maxvolume"), fVolume));
    GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);

    uint uiFlags(0);
    if (node.GetPropertyBool(_T("sound2d"), false))
        uiFlags |= SND_2D;
    if (node.GetPropertyBool(_T("soundloop"), false))
        uiFlags |= SND_LOOP;
    if (node.GetPropertyBool(_T("linearfalloff"), false))
        uiFlags |= SND_LINEARFALLOFF;
    if (node.GetPropertyBool(_T("squaredfalloff"), false))
        uiFlags |= SND_SQUAREDFALLOFF;

    int iFadeIn(node.GetPropertyInt(_T("fadein"), 0));
    int iFadeOutStartTime(node.GetPropertyInt(_T("fadeoutstarttime"), 0));
    int iFadeOut(node.GetPropertyInt(_T("fadeout"), 0));

    // playback speed
    float fSpeed1(node.GetPropertyFloat(_T("speed1"), 1.0));
    float fMinSpeed1(node.GetPropertyFloat(_T("minspeed1"), fSpeed1));
    float fMaxSpeed1(node.GetPropertyFloat(_T("maxspeed1"), fSpeed1));
    float fSpeed2(node.GetPropertyFloat(_T("speed2"), 1.0));
    float fMinSpeed2(node.GetPropertyFloat(_T("minspeed2"), fSpeed2));
    float fMaxSpeed2(node.GetPropertyFloat(_T("maxspeed2"), fSpeed2));
    int iSpeedUpTime(node.GetPropertyInt(_T("speeduptime"), 0));
    int iSlowDownTime(node.GetPropertyInt(_T("slowdowntime"), 0));

    ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_Effects,  CSample)(sSample, uiFlags), RES_SAMPLE));
    if (sSample.empty() || hSample == INVALID_RESOURCE)
        return NULL;

    return K2_NEW(ctx_Effects,  CSoundEmitterDef)(
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        iFadeIn,
        iFadeOutStartTime,
        iFadeOut,
        sBone,
        v3Pos,
        v3Offset,
        CRangef(fMinFalloff, fMaxFalloff),
        CRangef(fMinVolume, fMaxVolume),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
        uiFlags,
        hSample,
        CRangef(fMinSpeed1, fMaxSpeed1),
        CRangef(fMinSpeed2, fMaxSpeed2),
        iSpeedUpTime,
        iSlowDownTime,
        CRangef(fMinFalloffStart, fMaxFalloffStart),
        CRangef(fMinFalloffEnd, fMaxFalloffEnd)
    );
}

    
/*====================
  ProcessPrecipEmitter
  ====================*/
CPrecipEmitterDef*  ProcessPrecipEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    int iParticleLifeAlloc(node.GetPropertyInt(_T("particlelifealloc"), -1));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("global"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));

    float   fDrawDistance(node.GetPropertyFloat(_T("drawdistance"), 100.0f));
    float   fMinDrawDistance(node.GetPropertyFloat(_T("mindrawdistance"), fDrawDistance));
    float   fMaxDrawDistance(node.GetPropertyFloat(_T("maxdrawdistance"), fDrawDistance));

    bool bCollide(node.GetPropertyBool(_T("collide"), false));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CPrecipEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        iParticleLifeAlloc,
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        CRangef(fMinDrawDistance, fMaxDrawDistance),
        bCollide,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


/*====================
  ProcessTerrainEmitter
  ====================*/
CTerrainEmitterDef* ProcessTerrainEmitter(const CXMLNode &node)
{
    // Owner
    const tstring &sOwner(node.GetProperty(_T("owner"), g_sSource));

    // Life
    int iLife(node.GetPropertyInt(_T("life"), -1));
    int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
    int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

    // Expire Life
    int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
    int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
    int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

    // Count
    int iCount(node.GetPropertyInt(_T("count"), -1));
    int iMinCount(node.GetPropertyInt(_T("mincount"), iCount));
    int iMaxCount(node.GetPropertyInt(_T("maxcount"), iCount));

    // TimeNudge
    int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
    int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
    int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

    // Delay
    int iDelay(node.GetPropertyInt(_T("delay"), 0));
    int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
    int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

    // Loop
    bool bLoop(node.GetPropertyBool(_T("loop"), false));

    GET_TEMPORAL_PROPERTY_RANGE(SpawnRate, _T("spawnrate"), 0.0f);
    float fParticleLife(node.GetPropertyFloat(_T("particlelife"), -1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinParticleLife, _T("minparticlelife"), fParticleLife);
    GET_TEMPORAL_PROPERTY_RANGE(MaxParticleLife, _T("maxparticlelife"), fParticleLife);
    int iParticleLifeAlloc(node.GetPropertyInt(_T("particlelifealloc"), -1));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleTimeNudge, _T("particletimenudge"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(Gravity, _T("gravity"), 0.0f);

    float fSpeed(node.GetPropertyFloat(_T("speed"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinSpeed, _T("minspeed"), fSpeed);
    GET_TEMPORAL_PROPERTY_RANGE(MaxSpeed, _T("maxspeed"), fSpeed);

    float fAcceleration(node.GetPropertyFloat(_T("acceleration"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAcceleration, _T("minacceleration"), fAcceleration);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAcceleration, _T("maxacceleration"), fAcceleration);

    float fAngle(node.GetPropertyFloat(_T("angle"), 180.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinAngle, _T("minangle"), 0.0f);
    GET_TEMPORAL_PROPERTY_RANGE(MaxAngle, _T("maxangle"), fAngle);

    float fInheritVelocity(node.GetPropertyFloat(_T("inheritvelocity"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinInheritVelocity, _T("mininheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(MaxInheritVelocity, _T("maxinheritvelocity"), fInheritVelocity);
    GET_TEMPORAL_PROPERTY_RANGE(LimitInheritVelocity, _T("limitinheritvelocity"), 0.0f);

    const tstring &sMaterial(node.GetProperty(_T("material")));
    CVec3f  v3Dir(node.GetPropertyV3(_T("direction"), CVec3f(0.0f, 0.0f, 1.0f)));
    EDirectionalSpace eDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("directionalspace"), _T("local"))));
    float   fDrag(node.GetPropertyFloat(_T("drag"), 0.0f));
    float   fFriction(node.GetPropertyFloat(_T("friction"), 0.0f));
    const tstring &sBone(node.GetProperty(_T("bone")));
    CVec3f  v3Pos(node.GetPropertyV3(_T("position"), CVec3f(0.0f, 0.0f, 0.0f)));
    CVec3f  v3Offset(node.GetPropertyV3(_T("offset"), CVec3f(0.0f, 0.0f, 0.0f)));

    GET_TEMPORAL_PROPERTY_VEC3(OffsetSphere, _T("offsetsphere"), CVec3f(0.0f, 0.0f, 0.0f));
    GET_TEMPORAL_PROPERTY_VEC3(OffsetCube, _T("offsetcube"), CVec3f(0.0f, 0.0f, 0.0f));

    float fOffsetDirection(node.GetPropertyFloat(_T("offsetdirection"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetDirection, _T("minoffsetdirection"), fOffsetDirection);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetDirection, _T("maxoffsetdirection"), fOffsetDirection);

    float fOffsetRadial(node.GetPropertyFloat(_T("offsetradial"), 0.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadial, _T("minoffsetradial"), fOffsetRadial);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadial, _T("maxoffsetradial"), fOffsetRadial);

    float fOffsetRadialAngle(node.GetPropertyFloat(_T("offsetradialangle"), 90.0f));
    GET_TEMPORAL_PROPERTY_RANGE(MinOffsetRadialAngle, _T("minoffsetradialangle"), fOffsetRadialAngle);
    GET_TEMPORAL_PROPERTY_RANGE(MaxOffsetRadialAngle, _T("maxoffsetradialangle"), fOffsetRadialAngle);

    bool bCollide(node.GetPropertyBool(_T("collide"), false));

    GET_TEMPORAL_PROPERTY_RANGE(Range, _T("range"), 0.0f);

    EDirectionalSpace eParticleDirectionalSpace(GetDirectionalSpace(node.GetProperty(_T("particledirectionalspace"), _T("global"))));

    GET_TEMPORAL_PROPERTY_VEC3(ParticleColor, _T("particlecolor"), CVec3f(1.0f, 1.0f, 1.0f));
    GET_TEMPORAL_PROPERTY_RANGE(ParticleAlpha, _T("particlealpha"), 1.0f);
    GET_TEMPORAL_PROPERTY_RANGE(ParticleScale, _T("particlescale"), 1.0f);

    float fDepthBias(node.GetPropertyFloat(_T("depthbias"), 0.0f));

    return K2_NEW(ctx_Effects,  CTerrainEmitterDef)(
        sOwner,
        CRangei(iMinLife, iMaxLife),
        CRangei(iMinExpireLife, iMaxExpireLife),
        CRangei(iMinCount, iMaxCount),
        CRangei(iMinTimeNudge, iMaxTimeNudge),
        CRangei(iMinDelay, iMaxDelay),
        bLoop,
        TEMPORAL_PROPERTY_RANGE_FLOAT(SpawnRate),
        TEMPORAL_PROPERTY_RANGE_INT(MinParticleLife),
        TEMPORAL_PROPERTY_RANGE_INT(MaxParticleLife),
        iParticleLifeAlloc,
        TEMPORAL_PROPERTY_RANGE_INT(ParticleTimeNudge),
        TEMPORAL_PROPERTY_RANGE_FLOAT(Gravity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxSpeed),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAcceleration),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxInheritVelocity),
        TEMPORAL_PROPERTY_RANGE_FLOAT(LimitInheritVelocity),
        g_ResourceManager.Register(sMaterial, RES_MATERIAL),
        v3Dir,
        eDirectionalSpace,
        fDrag,
        fFriction,
        sBone,
        v3Pos,
        v3Offset,
        TEMPORAL_PROPERTY_VEC3(OffsetSphere),
        TEMPORAL_PROPERTY_VEC3(OffsetCube),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetDirection),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadial),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MinOffsetRadialAngle),
        TEMPORAL_PROPERTY_RANGE_FLOAT(MaxOffsetRadialAngle),
        bCollide,
        TEMPORAL_PROPERTY_RANGE_FLOAT(Range),
        eParticleDirectionalSpace,
        TEMPORAL_PROPERTY_VEC3(ParticleColor),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleAlpha),
        TEMPORAL_PROPERTY_RANGE_FLOAT(ParticleScale),
        fDepthBias
    );
}


//=============================================================================
// CSpawnParticleSystem
//=============================================================================
class CSpawnParticleSystem : public IEffectCmd
{
private:
    tstring     m_sInstance;
    tstring     m_sParticleSystem;

public:
    CSpawnParticleSystem
    (
        const tstring &sInstance,
        const tstring &sParticleSystem
    ) :
    m_sInstance(sInstance),
    m_sParticleSystem(sParticleSystem)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        PROFILE("SpawnParticleSystem");

        if (pEffectThread == NULL)
            return true;

        CParticleSystemDef *pParticleSystemDef = pEffectThread->GetEffect()->GetParticleSystemDef(m_sParticleSystem);

        if (!pParticleSystemDef)
            return true;

        CParticleSystem *pNewParticleSystem = K2_NEW(ctx_Effects,  CParticleSystem)(uiMilliseconds, pEffectThread, *pParticleSystemDef);

        pEffectThread->AddInstance(m_sInstance, pNewParticleSystem);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CWait
//=============================================================================
class CWait : public IEffectCmd
{
private:
    uint    m_uiDuration;

public:
    CWait(uint uiDuration) :
    m_uiDuration(uiDuration)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        pEffectThread->Wait(m_uiDuration);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CWaitForDeath
//=============================================================================
class CWaitForDeath : public IEffectCmd
{
private:
    tstring     m_sInstance;

public:
    CWaitForDeath
    (
        const tstring &sInstance
    ) :
    m_sInstance(sInstance)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        IEffectInstance *pParticleSystem(pEffectThread->GetInstance(m_sInstance));

        if (!pParticleSystem || pParticleSystem->IsDead())
            return true;
        else
            return false;
    }
};
//=============================================================================


//=============================================================================
// CPlaySound
//=============================================================================
class CPlaySound : public IEffectCmd
{
private:
    ResHandle       m_hSample;
    int             m_iChannel;
    float           m_fFalloff;
    float           m_fVolume;

public:
    CPlaySound
    (
        const tstring &sSound,
        int iChannel,
        float fFalloff,
        float fVolume
    ) :
    m_hSample(g_ResourceManager.Register(K2_NEW(ctx_Effects,  CSample)(sSound, 0), RES_SAMPLE)),
    m_iChannel(iChannel),
    m_fFalloff(fFalloff),
    m_fVolume(fVolume)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        if (m_hSample == INVALID_RESOURCE)
            return true;

        K2SoundManager.PlaySFXSound(m_hSample, &pEffectThread->GetSourcePosition(), NULL, m_fVolume, m_fFalloff, m_iChannel);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CCameraShake
//=============================================================================
class CCameraShake : public IEffectCmd
{
private:
    float   m_fFalloffStart;
    float   m_fFalloffEnd;
    float   m_fFrequency;
    float   m_fScale;
    uint    m_uiDuration;

public:
    CCameraShake
    (
        float fFalloffStart,
        float fFalloffEnd,
        float fFrequency,
        float fScale,
        uint uiDuration
    ) :
    m_fFalloffStart(fFalloffStart),
    m_fFalloffEnd(fFalloffEnd),
    m_fFrequency(fFrequency),
    m_fScale(fScale),
    m_uiDuration(uiDuration)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        if (pEffectThread->GetCustomVisibility())
            pEffectThread->StartCameraShake(uiMilliseconds, m_fFalloffStart, m_fFalloffEnd, m_fFrequency, m_fScale, m_uiDuration);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CCameraShake2
//=============================================================================
class CCameraShake2 : public IEffectCmd
{
private:
    float   m_fFalloffStart;
    float   m_fFalloffEnd;
    float   m_fFrequency;
    float   m_fScale;
    uint    m_uiDuration;

public:
    CCameraShake2
    (
        float fFalloffStart,
        float fFalloffEnd,
        float fFrequency,
        float fScale,
        uint uiDuration
    ) :
    m_fFalloffStart(fFalloffStart),
    m_fFalloffEnd(fFalloffEnd),
    m_fFrequency(fFrequency),
    m_fScale(fScale),
    m_uiDuration(uiDuration)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        if (pEffectThread->GetCustomVisibility())
            pEffectThread->StartCameraShake2(uiMilliseconds, m_fFalloffStart, m_fFalloffEnd, m_fFrequency, m_fScale, m_uiDuration);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CCameraKick
//=============================================================================
class CCameraKick : public IEffectCmd
{
private:
    CRangef     m_rfPitch;
    float       m_fTurn;
    CRangef     m_rfBack;
    CRangef     m_rfUp;
    CRangef     m_rfRight;
    float       m_fHalfLife;

public:
    CCameraKick
    (
        CRangef rfPitch,
        float fTurn,
        CRangef rfBack,
        CRangef rfUp,
        CRangef rfRight,
        float fHalfLife
    ) :
    m_rfPitch(rfPitch),
    m_fTurn(fTurn),
    m_rfBack(rfBack),
    m_rfUp(rfUp),
    m_rfRight(rfRight),
    m_fHalfLife(fHalfLife)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        pEffectThread->StartCameraKick(uiMilliseconds, m_rfPitch, m_fTurn, m_rfBack, m_rfUp, m_rfRight, m_fHalfLife);
        return true;
    }
};
//=============================================================================


//=============================================================================
// COverlay
//=============================================================================
class COverlay : public IEffectCmd
{
private:
    ResHandle               m_hMaterial;
    CTemporalPropertyv3     m_tv3Color;
    CTemporalPropertyf      m_tfAlpha;
    uint                    m_uiDuration;

public:
    COverlay
    (
        const tstring &sMaterialPath,
        const CTemporalPropertyv3 &tv3Color,
        const CTemporalPropertyf &tfAlpha,
        uint uiDuration
    ) :
    m_hMaterial(g_ResourceManager.Register(K2_NEW(ctx_Effects,  CTexture)(sMaterialPath, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE)),
    m_tv3Color(tv3Color),
    m_tfAlpha(tfAlpha),
    m_uiDuration(uiDuration)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        pEffectThread->StartOverlay(uiMilliseconds, m_tv3Color, m_tfAlpha, m_hMaterial, m_uiDuration);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CSpawnModifier
//=============================================================================
class CSpawnModifier : public IEffectCmd
{
private:
    tstring     m_sInstance;
    tstring     m_sModifier;

public:
    CSpawnModifier
    (
        const tstring &sInstance,
        const tstring &sModifier
    ) :
    m_sInstance(sInstance),
    m_sModifier(sModifier)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        CModifierDef *pModifierDef = pEffectThread->GetEffect()->GetModifierDef(m_sModifier);

        if (!pModifierDef)
            return true;

        CSceneEntityModifier *pNewModifier = K2_NEW(ctx_Effects,  CSceneEntityModifier)(uiMilliseconds, pEffectThread, *pModifierDef);

        pEffectThread->AddInstance(m_sInstance, pNewModifier);
        return true;
    }
};
//=============================================================================


//=============================================================================
// CWaitForKick
//=============================================================================
class CWaitForKick : public IEffectCmd
{
private:
    tstring     m_sInstance;

public:
    CWaitForKick
    (
        const tstring &sInstance
    ) :
    m_sInstance(sInstance)
    {
    }

    bool    Execute(CEffectThread *pEffectThread, uint uiMilliseconds)
    {
        if (pEffectThread->GetKickTime() == INVALID_TIME)
            return true;
        else
            return false;
    }
};
//=============================================================================


namespace XMLEffects
{
    // <effect>
    DECLARE_XML_PROCESSOR(effect)
    BEGIN_XML_REGISTRATION(effect)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(effect, CEffect)
        pObject->Setup(
            node.GetProperty(_T("name")),
            node.GetPropertyBool(_T("deferred"), false),
            node.GetPropertyBool(_T("persistent"), false),
            node.GetPropertyBool(_T("pausable"), false),
            node.GetPropertyBool(_T("useentityeffectscale"), false)
        );
    END_XML_PROCESSOR(pObject)


    // <definitions>
    DECLARE_XML_PROCESSOR(definitions)
    BEGIN_XML_REGISTRATION(definitions)
        REGISTER_XML_PROCESSOR(effect)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(definitions, CEffect)
    END_XML_PROCESSOR(pObject)


    // <particlesystem>
    DECLARE_XML_PROCESSOR(particlesystem)
    BEGIN_XML_REGISTRATION(particlesystem)
        REGISTER_XML_PROCESSOR(definitions)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(particlesystem, CEffect)
        ESystemSpace eSpace(GetSpace(node.GetProperty(_T("space"), _T("world"))));
        float fScale(node.GetPropertyFloat(_T("scale"), 1.0f));
        CVec3f v3Color(node.GetPropertyV3(_T("color"), CVec3f(1.0f, 1.0f, 1.0f)));
        CParticleSystemDef *pParticleSystemDef(K2_NEW(ctx_Effects,  CParticleSystemDef)(eSpace, fScale, v3Color));

        ProcessChildren(node, pParticleSystemDef);

        pObject->AddParticleSystemDef(node.GetProperty(_T("name")), pParticleSystemDef);
    END_XML_PROCESSOR_NO_CHILDREN


    // <modifier>
    DECLARE_XML_PROCESSOR(modifier)
    BEGIN_XML_REGISTRATION(modifier)
        REGISTER_XML_PROCESSOR(definitions)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(modifier, CEffect)
        const tstring &sName(node.GetProperty(_T("name")));

        // Life
        int iLife(node.GetPropertyInt(_T("life"), -1));
        int iMinLife(node.GetPropertyInt(_T("minlife"), iLife));
        int iMaxLife(node.GetPropertyInt(_T("maxlife"), iLife));

        // Expire Life
        int iExpireLife(node.GetPropertyInt(_T("expirelife"), 0));
        int iMinExpireLife(node.GetPropertyInt(_T("minexpirelife"), iExpireLife));
        int iMaxExpireLife(node.GetPropertyInt(_T("maxexpirelife"), iExpireLife));

        // TimeNudge
        int iTimeNudge(node.GetPropertyInt(_T("timenudge"), 0));
        int iMinTimeNudge(node.GetPropertyInt(_T("mintimenudge"), iTimeNudge));
        int iMaxTimeNudge(node.GetPropertyInt(_T("maxtimenudge"), iTimeNudge));

        // Delay
        int iDelay(node.GetPropertyInt(_T("delay"), 0));
        int iMinDelay(node.GetPropertyInt(_T("mindelay"), iDelay));
        int iMaxDelay(node.GetPropertyInt(_T("maxdelay"), iDelay));

        // Loop
        bool bLoop(node.GetPropertyBool(_T("loop"), false));

        GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
        GET_TEMPORAL_PROPERTY_FLOAT(Alpha, _T("alpha"), 1.0f);

        GET_TEMPORAL_PROPERTY_FLOAT(Param0, _T("param0"), 0.0f);
        GET_TEMPORAL_PROPERTY_FLOAT(Param1, _T("param1"), 0.0f);
        GET_TEMPORAL_PROPERTY_FLOAT(Param2, _T("param2"), 0.0f);
        GET_TEMPORAL_PROPERTY_FLOAT(Param3, _T("param3"), 0.0f);

        tstring tsMaterial(node.GetProperty(_T("material")));

        const tstring &sMaterial(tsMaterial);

        GET_TEMPORAL_PROPERTY_VEC3(Offset, _T("offset"), CVec3f(0.0f, 0.0f, 0.0f));

        const tstring &sSkin(node.GetProperty(_T("skin")));

        CModifierDef *pModifierDef(K2_NEW(ctx_Effects,  CModifierDef)(
            CRangei(iMinLife, iMaxLife),
            CRangei(iMinExpireLife, iMaxExpireLife),
            CRangei(iMinTimeNudge, iMaxTimeNudge),
            CRangei(iMinDelay, iMaxDelay),
            bLoop,
            TEMPORAL_PROPERTY_VEC3(Color),
            TEMPORAL_PROPERTY_FLOAT(Alpha),
            TEMPORAL_PROPERTY_FLOAT(Param0),
            TEMPORAL_PROPERTY_FLOAT(Param1),
            TEMPORAL_PROPERTY_FLOAT(Param2),
            TEMPORAL_PROPERTY_FLOAT(Param3),
            g_ResourceManager.Register(sMaterial, RES_MATERIAL),
            TEMPORAL_PROPERTY_VEC3(Offset),
            sSkin
        ));

        ProcessChildren(node, pModifierDef);

        pObject->AddModifierDef(sName, pModifierDef);
    END_XML_PROCESSOR_NO_CHILDREN

    // <particle>
    DECLARE_XML_PROCESSOR(particle)
    BEGIN_XML_PROCESSOR(particle, IEmitterDef)
        GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
        GET_TEMPORAL_PROPERTY_RANGE(Alpha, _T("alpha"), 1.0f);

        GET_TEMPORAL_PROPERTY_RANGE(Size, _T("size"), 2.0f);
        GET_TEMPORAL_PROPERTY_RANGE_CHILD(Width, _T("width"), Size);
        GET_TEMPORAL_PROPERTY_RANGE_CHILD(Height, _T("height"), Size);

        GET_TEMPORAL_PROPERTY_RANGE(Scale, _T("scale"), 1.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Angle, _T("angle"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Pitch, _T("pitch"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Yaw, _T("yaw"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Frame, _T("frame"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Param, _T("param"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Stickiness, _T("stickiness"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(Anchor, _T("anchor"), 0.0f);

        GET_TEMPORAL_PROPERTY_RANGE(WidthDistort, _T("widthdistort"), 0.0f);
        GET_TEMPORAL_PROPERTY_RANGE(HeightDistort, _T("heightdistort"), 0.0f);

        float fScaleU(node.GetPropertyFloat(_T("scaleu"), 1.0f));
        float fScaleV(node.GetPropertyFloat(_T("scalev"), 1.0f));
        float fOffsetU(node.GetPropertyFloat(_T("offsetu"), 0.0f));
        float fOffsetV(node.GetPropertyFloat(_T("offsetv"), 0.0f));
        
        CVec2f v2Center(node.GetPropertyV2(_T("center"), CVec2f(0.5f, 0.5f)));

        float fSelectionWeight(node.GetPropertyFloat(_T("weight"), 1.0f));

        uint uiFlags(0);

        if (node.GetPropertyBool(_T("lockup"), false))
            uiFlags |= BBOARD_LOCK_UP;
        if (node.GetPropertyBool(_T("lockright"), false))
            uiFlags |= BBOARD_LOCK_RIGHT;
        if (node.GetPropertyBool(_T("turn"), false))
            uiFlags |= BBOARD_TURN;
        if (node.GetPropertyBool(_T("flare"), false))
            uiFlags |= BBOARD_FLARE;
        if (node.GetPropertyBool(_T("alphacolor"), false))
            uiFlags |= BBOARD_ALPHACOLOR;
        if (node.GetPropertyBool(_T("offcenter"), false))
            uiFlags |= BBOARD_OFFCENTER;

        static tsvector vEmitters;
        vEmitters.clear();

        if (node.HasProperty(_T("emitter")))
            vEmitters.push_back(node.GetProperty(_T("emitter")));
        for (int i(0); i < 32; ++i)
        {
            tstring sName(_T("emitter") + XtoA(i));
            
            if (!node.HasProperty(sName))
                break;

            vEmitters.push_back(node.GetProperty(sName));
        }

        CSimpleParticleDef *pParticle(K2_NEW(ctx_Effects,  CSimpleParticleDef)(
            TEMPORAL_PROPERTY_VEC3(Color),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Alpha),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Width),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Height),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Scale),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Angle),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Pitch),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Yaw),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Frame),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Param),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Stickiness),
            TEMPORAL_PROPERTY_RANGE_FLOAT(Anchor),
            TEMPORAL_PROPERTY_RANGE_FLOAT(WidthDistort),
            TEMPORAL_PROPERTY_RANGE_FLOAT(HeightDistort),
            fScaleU,
            fScaleV,
            fOffsetU,
            fOffsetV,
            v2Center,
            fSelectionWeight,
            uiFlags,
            vEmitters
        ));

        ProcessChildren(node, pParticle);

        pObject->AddParticleDef(pParticle);
    END_XML_PROCESSOR_NO_CHILDREN

    // <controller>
    BEGIN_EMITTER_XML_PROCESSOR(controller, ControllerEmitter)

    // <model>
    BEGIN_EMITTER_XML_PROCESSOR(model, ModelEmitter)

    // <debris>
    BEGIN_EMITTER_XML_PROCESSOR(debris, DebrisEmitter)

    // <simpleemitter>
    BEGIN_EMITTER_XML_PROCESSOR(simpleemitter, SimpleEmitter)
    END_EMITTER_XML_PROCESSOR(simpleemitter, SimpleEmitter)

    // <twopointemitter>
    BEGIN_EMITTER_XML_PROCESSOR(twopointemitter, TwoPointEmitter)
    END_EMITTER_XML_PROCESSOR(twopointemitter, TwoPointEmitter)

    // <orbiteremitter>
    BEGIN_EMITTER_XML_PROCESSOR(orbiteremitter, OrbiterEmitter)
    END_EMITTER_XML_PROCESSOR(orbiteremitter, OrbiterEmitter)

    // <trackeremitter>
    BEGIN_EMITTER_XML_PROCESSOR(trackeremitter, TrackerEmitter)
    END_EMITTER_XML_PROCESSOR(trackeremitter, TrackerEmitter)

    // <meshemitter>
    BEGIN_EMITTER_XML_PROCESSOR(meshemitter, MeshEmitter)
    END_EMITTER_XML_PROCESSOR(meshemitter, MeshEmitter)

    // <skeletonemitter>
    BEGIN_EMITTER_XML_PROCESSOR(skeletonemitter, SkeletonEmitter)
    END_EMITTER_XML_PROCESSOR(skeletonemitter, SkeletonEmitter)

    // <twopointtrailemitter>
    BEGIN_EMITTER_XML_PROCESSOR(twopointtrailemitter, TwoPointTrailEmitter)
    END_EMITTER_XML_PROCESSOR(twopointtrailemitter, TwoPointTrailEmitter)

    // <trailemitter>
    BEGIN_EMITTER_XML_PROCESSOR(trailemitter, TrailEmitter)
    END_EMITTER_XML_PROCESSOR(trailemitter, TrailEmitter)

    // <light>
    BEGIN_EMITTER_XML_PROCESSOR(light, LightEmitter)
    END_EMITTER_XML_PROCESSOR(light, LightEmitter)

    // <beam>
    BEGIN_EMITTER_XML_PROCESSOR(beam, BeamEmitter)
    END_EMITTER_XML_PROCESSOR(beam, BeamEmitter)

    // <groundsprite>
    BEGIN_EMITTER_XML_PROCESSOR(groundsprite, GroundSpriteEmitter)
    END_EMITTER_XML_PROCESSOR(groundsprite, GroundSpriteEmitter)

    // <billboard>
    BEGIN_EMITTER_XML_PROCESSOR(billboard, BillboardEmitter)
    END_EMITTER_XML_PROCESSOR(billboard, BillboardEmitter)

    // <tracer>
    BEGIN_EMITTER_XML_PROCESSOR(tracer, TracerEmitter)
    END_EMITTER_XML_PROCESSOR(tracer, TracerEmitter)

    // <sound>
    BEGIN_EMITTER_XML_PROCESSOR(sound, SoundEmitter)
    END_EMITTER_XML_PROCESSOR(sound, SoundEmitter)

    // <precipemitter>
    BEGIN_EMITTER_XML_PROCESSOR(precipemitter, PrecipEmitter)
    END_EMITTER_XML_PROCESSOR(precipemitter, PrecipEmitter)

    // <terrainemitter>
    BEGIN_EMITTER_XML_PROCESSOR(terrainemitter, TerrainEmitter)
    END_EMITTER_XML_PROCESSOR(terrainemitter, TerrainEmitter)

    // <controller>
    END_EMITTER_XML_PROCESSOR(controller, ControllerEmitter)

    // <model>  
    END_EMITTER_XML_PROCESSOR(model, ModelEmitter)

    // <debris> 
    END_EMITTER_XML_PROCESSOR(debris, DebrisEmitter)

    BEGIN_XML_REGISTRATION(particle)
        REGISTER_XML_PROCESSOR(effect)
        REGISTER_XML_PROCESSOR(simpleemitter)
        REGISTER_XML_PROCESSOR(orbiteremitter)
        REGISTER_XML_PROCESSOR(trackeremitter)
        REGISTER_XML_PROCESSOR(meshemitter)
        REGISTER_XML_PROCESSOR(skeletonemitter)
        REGISTER_XML_PROCESSOR(twopointtrailemitter)
        REGISTER_XML_PROCESSOR(twopointemitter)
        REGISTER_XML_PROCESSOR(trailemitter)
        REGISTER_XML_PROCESSOR(precipemitter)
        REGISTER_XML_PROCESSOR(terrainemitter)
    END_XML_REGISTRATION

    // <thread>
    DECLARE_XML_PROCESSOR(thread)
    BEGIN_XML_REGISTRATION(thread)
        REGISTER_XML_PROCESSOR(effect)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(thread, CEffect)
        CEffectThread *pEffectThread(K2_NEW(ctx_Effects,  CEffectThread)(pObject));
        ProcessChildren(node, pEffectThread);
        pObject->SetEffectThread(pEffectThread);
    END_XML_PROCESSOR_NO_CHILDREN


    // <spawnparticlesystem>
    DECLARE_XML_PROCESSOR(spawnparticlesystem)
    BEGIN_XML_REGISTRATION(spawnparticlesystem)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(spawnparticlesystem, CEffectThread)
        const tstring &sInstance(node.GetProperty(_T("instance")));
        const tstring &sParticleSystem(node.GetProperty(_T("particlesystem")));
        pObject->AddCmd(K2_NEW(ctx_Effects,  CSpawnParticleSystem)(sInstance, sParticleSystem));
    END_XML_PROCESSOR_NO_CHILDREN


    // <wait>
    DECLARE_XML_PROCESSOR(wait)
    BEGIN_XML_REGISTRATION(wait)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(wait, CEffectThread)
        pObject->AddCmd(K2_NEW(ctx_Effects,  CWait)(node.GetPropertyInt(_T("duration"), 0)));
    END_XML_PROCESSOR_NO_CHILDREN


    // <waitfordeath>
    DECLARE_XML_PROCESSOR(waitfordeath)
    BEGIN_XML_REGISTRATION(waitfordeath)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(waitfordeath, CEffectThread)
        pObject->AddCmd(K2_NEW(ctx_Effects,  CWaitForDeath)(node.GetProperty(_T("instance"))));
    END_XML_PROCESSOR_NO_CHILDREN


    // <playsound>
    DECLARE_XML_PROCESSOR(playsound)
    BEGIN_XML_REGISTRATION(playsound)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(playsound, CEffectThread)
        const tstring &sSound(node.GetProperty(_T("sound")));
        int iChannel(node.GetPropertyInt(_T("channel"), -1));
        float fFalloff(node.GetPropertyFloat(_T("falloff"), -1));
        float fVolume(node.GetPropertyFloat(_T("volume"), 1.0f));
        pObject->AddCmd(K2_NEW(ctx_Effects,  CPlaySound)(sSound, iChannel, fFalloff, fVolume));
    END_XML_PROCESSOR_NO_CHILDREN


    // <camerashake>
    DECLARE_XML_PROCESSOR(camerashake)
    BEGIN_XML_REGISTRATION(camerashake)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(camerashake, CEffectThread)
        float   fFalloffStart(node.GetPropertyFloat(_T("falloffstart"), 0.0f));
        float   fFalloffEnd(node.GetPropertyFloat(_T("falloffend"), 700.0f));
        float   fFrequency(node.GetPropertyFloat(_T("frequency"), 0.0f));
        float   fScale(node.GetPropertyFloat(_T("scale"), 0.0f));
        uint    uiDuration(node.GetPropertyInt(_T("duration"), 0));
        pObject->AddCmd(K2_NEW(ctx_Effects,  CCameraShake)(fFalloffStart, fFalloffEnd, fFrequency, fScale, uiDuration));
    END_XML_PROCESSOR_NO_CHILDREN


    // <camerashake2>
    DECLARE_XML_PROCESSOR(camerashake2)
    BEGIN_XML_REGISTRATION(camerashake2)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(camerashake2, CEffectThread)
        float   fFalloffStart(node.GetPropertyFloat(_T("falloffstart"), 0.0f));
        float   fFalloffEnd(node.GetPropertyFloat(_T("falloffend"), 700.0f));
        float   fFrequency(node.GetPropertyFloat(_T("frequency"), 0.0f));
        float   fScale(node.GetPropertyFloat(_T("scale"), 0.0f));
        uint    uiDuration(node.GetPropertyInt(_T("duration"), 0));
        pObject->AddCmd(K2_NEW(ctx_Effects,  CCameraShake2)(fFalloffStart, fFalloffEnd, fFrequency, fScale, uiDuration));
    END_XML_PROCESSOR_NO_CHILDREN


    // <camerakick>
    DECLARE_XML_PROCESSOR(camerakick)
    BEGIN_XML_REGISTRATION(camerakick)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(camerakick, CEffectThread)
        float   fPitch(node.GetPropertyFloat(_T("pitch"), 0.0f));
        float   fPitchRange(node.GetPropertyFloat(_T("pitchrange"), 0.0f));
        CRangef rfPitch(node.GetPropertyFloat(_T("minpitch"), fPitch - fPitchRange), node.GetPropertyFloat(_T("maxpitch"), fPitch + fPitchRange));

        float   fTurn(node.GetPropertyFloat(_T("turn"), 0.0f));
        
        float   fBack(node.GetPropertyFloat(_T("back"), 0.0f));
        float   fBackRange(node.GetPropertyFloat(_T("backrange"), 0.0f));
        CRangef rfBack(node.GetPropertyFloat(_T("minback"), fBack - fBackRange), node.GetPropertyFloat(_T("maxback"), fBack + fBackRange));

        float   fUp(node.GetPropertyFloat(_T("up"), 0.0f));
        float   fUpRange(node.GetPropertyFloat(_T("uprange"), 0.0f));
        CRangef rfUp(node.GetPropertyFloat(_T("minup"), fUp - fUpRange), node.GetPropertyFloat(_T("maxup"), fUp + fPitchRange));
        
        float   fRight(node.GetPropertyFloat(_T("right"), 0.0f));
        float   fRightRange(node.GetPropertyFloat(_T("rightrange"), 0.0f));
        CRangef rfRight(node.GetPropertyFloat(_T("minright"), fRight - fRightRange), node.GetPropertyFloat(_T("maxright"), fRight + fRightRange));

        float   fHalfLife(node.GetPropertyFloat(_T("halflife"), 0.1f));

        pObject->AddCmd(K2_NEW(ctx_Effects,  CCameraKick)(rfPitch, fTurn, rfBack, rfUp, rfRight, fHalfLife));
    END_XML_PROCESSOR_NO_CHILDREN


    // <overlay>
    DECLARE_XML_PROCESSOR(overlay)
    BEGIN_XML_REGISTRATION(overlay)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(overlay, CEffectThread)
        GET_TEMPORAL_PROPERTY_VEC3(Color, _T("color"), CVec3f(1.0f, 1.0f, 1.0f));
        GET_TEMPORAL_PROPERTY_RANGE(Alpha, _T("alpha"), 1.0f);

        const tstring &sMaterialPath(node.GetProperty(_T("material")));
        uint uiDuration(node.GetPropertyInt(_T("duration")));

        IEffectCmd *pCmd(
            K2_NEW(ctx_Effects,  COverlay)(
                sMaterialPath,
                TEMPORAL_PROPERTY_VEC3(Color),
                TEMPORAL_PROPERTY_RANGE_FLOAT(Alpha),
                uiDuration
            )
        );

        pObject->AddCmd(pCmd);
    END_XML_PROCESSOR_NO_CHILDREN


    // <spawnmodifier>
    DECLARE_XML_PROCESSOR(spawnmodifier)
    BEGIN_XML_REGISTRATION(spawnmodifier)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(spawnmodifier, CEffectThread)
        const tstring &sInstance(node.GetProperty(_T("instance")));
        const tstring &sModifier(node.GetProperty(_T("modifier")));

        pObject->AddCmd(K2_NEW(ctx_Effects,  CSpawnModifier)(sInstance, sModifier));
    END_XML_PROCESSOR_NO_CHILDREN


    // <waitforkick>
    DECLARE_XML_PROCESSOR(waitforkick)
    BEGIN_XML_REGISTRATION(waitforkick)
        REGISTER_XML_PROCESSOR(thread)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(waitforkick, CEffectThread)
        pObject->AddCmd(K2_NEW(ctx_Effects,  CWaitForKick)(node.GetProperty(_T("instance"))));
    END_XML_PROCESSOR_NO_CHILDREN
}
