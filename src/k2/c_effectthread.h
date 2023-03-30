// (C)2006 S2 Games
// c_effectthread.h
//
//=============================================================================
#ifndef __C_EFFECTTHREAD_H__
#define __C_EFFECTTHREAD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resourcewatcher2.h"
#include "c_temporalproperty.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class IEffectCmd;
class CEffect;
class IEffectInstance;
class CParticleSystem;
class CSkeleton;
class CCamera;
class CWorld;

typedef	vector<IEffectCmd *>				EffectCmdBuffer;
typedef	map<tstring, IEffectInstance *>		InstanceMap;
//=============================================================================

//=============================================================================
// CEffectThread
//=============================================================================
class CEffectThread : public IResourceWatcher
{
private:
	CEffect*						m_pEffect;
	ResHandle						m_hEffect;
	uint							m_uiStartTime;
	uint							m_uiLastUpdateTime;
	uint							m_uiWaitTime;
	EffectCmdBuffer					m_vCmdBuffer;		// Only valid in an effect thread defintion (inside CEffect)
	const EffectCmdBuffer*			m_pvCmdBuffer;		// Effect thread instances store a pointer
														// to the cmd buffer of the effect's
														// definition, not an actual m_vCmdBuffer
	EffectCmdBuffer::const_iterator	m_itStartCmd;
	EffectCmdBuffer::const_iterator	m_itCurrentCmd;

	InstanceMap						m_mapInstances;

	bool							m_bActive;
	bool							m_bExpire;

	CCamera*						m_pCamera;
	CWorld*							m_pWorld;

	// Camera Effects
	CVec3f							m_v3CameraOffset;
	CVec3f							m_v3CameraAngleOffset;

	// Shake
	uint							m_uiCameraShakeTime;
	uint							m_uiCameraShakeDuration;
	float							m_fCameraShakeFrequency;
	CVec3f							m_v3CameraShake;

	// Kick
	uint							m_uiCameraKickTime;
	float							m_fCameraKickHalfLife;
	CVec3f							m_v3CameraKickAngles;
	CVec3f							m_v3CameraKickPosition;

	// Overlay
	bool							m_bActiveOverlay;
	uint							m_uiOverlayStartTime;
	uint							m_uiOverlayDuration;
	CTemporalPropertyv3				m_tv3OverlayColor;
	CTemporalPropertyf				m_tfOverlayAlpha;
	CVec4f							m_v4OverlayColor;
	ResHandle						m_hOverlayMaterial;

	CSkeleton*						m_pSourceSkeleton;
	CModel*							m_pSourceModel;
	CVec3f							m_v3SourcePos;
	CAxis							m_aSourceAxis;
	float							m_fSourceScale;
	float							m_fSourceEffectScale;
	bool							m_bSourceVisibility;
	
	CSkeleton*						m_pTargetSkeleton;
	CModel*							m_pTargetModel;
	CVec3f							m_v3TargetPos;
	CAxis							m_aTargetAxis;
	float							m_fTargetScale;
	float							m_fTargetEffectScale;
	bool							m_bTargetVisibility;

	bool							m_bCustomVisibility;

	CVec3f							m_v3Color;

	CVec3f		GetBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime);
	void		GetBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
	CVec3f		GetRandomPositionOnMesh(CModel *pModel, const tstring &sMesh);
	CVec3f		GetRandomPositionWithNormalOnMesh(CModel *pModel, const tstring &sMesh, CVec3f &v3Normal);
	CVec3f		GetRandomPositionOnMesh(CModel *pModel, const tstring &sMesh, const CVec3f &v3Dir);
	CVec3f		GetRandomPositionOnSkeleton(CSkeleton *pSkeleton);
	matrix43_t*	GetBoneTransform(CSkeleton *pSkeleton, const tstring &sBone);
	bool		GetVisibility(CSkeleton *pSkeleton, const tstring &sBone);
	
	void		UpdateCameraShake(uint uiMilliseconds);
	void		UpdateCameraKick(uint uiMilliseconds);

	bool		Rewind(uint uiMilliseconds);

public:
	K2_API ~CEffectThread();
	CEffectThread(CEffect *pEffect);
	CEffectThread(uint uiStartTime, const CEffectThread &etSettings);

	void				Rebuild(ResHandle hResource);

	void				AddCmd(IEffectCmd *pCmd);
	CEffect*			GetEffect()								{ return m_pEffect; }

	K2_API bool			Execute(uint uiMilliseconds);
	K2_API void			Cleanup();

	void				AddInstance(const tstring &sName, IEffectInstance *pParticleSystem);
	IEffectInstance*	GetInstance(const tstring &sName);

	const InstanceMap&	GetInstances() const					{ return m_mapInstances; }

	void				SetActive(bool bActive)					{ m_bActive = bActive; }
	bool				GetActive() const						{ return m_bActive; }

	K2_API void			Expire(uint uiMilliseconds);
	bool				GetExpire() const						{ return m_bExpire; }

	K2_API bool			IsDeferred();
	K2_API bool			IsPersistent();
	K2_API bool			IsPausable();
	K2_API bool			GetUseEntityEffectScale();

	void				SetCamera(CCamera *pCamera)				{ m_pCamera = pCamera; }
	CCamera*			GetCamera() const						{ return m_pCamera; }
	
	void				SetWorld(CWorld *pWorld)				{ m_pWorld = pWorld; }
	CWorld*				GetWorld() const						{ return m_pWorld; }
	
	const CVec3f&		GetCameraOffset() const					{ return m_v3CameraOffset; }
	const CVec3f&		GetCameraAngleOffset() const			{ return m_v3CameraAngleOffset; }

	void				SetSourceSkeleton(CSkeleton *pSkeleton)	{ m_pSourceSkeleton = pSkeleton; }
	void				SetSourceModel(CModel *pModel)			{ m_pSourceModel = pModel; }

	void				SetTargetSkeleton(CSkeleton *pSkeleton)	{ m_pTargetSkeleton = pSkeleton; }
	void				SetTargetModel(CModel *pModel)			{ m_pTargetModel = pModel; }

	CVec3f				GetSourceBonePosition(const tstring &sBone, uint uiTime);
	void				GetSourceBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
	CVec3f				GetSourceMeshPosition(const tstring &sMesh);
	CVec3f				GetSourceRandomPositionOnMesh(const tstring &sMesh);
	CVec3f				GetSourceRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Normal);
	CVec3f				GetSourceRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir);
	CVec3f				GetSourceRandomPositionOnSkeleton();
	matrix43_t*			GetSourceBoneTransform(const tstring &sBone);
	bool				GetSourceVisibility(const tstring &sBone);
	CSkeleton*			GetSourceSkeleton()						{ return m_pSourceSkeleton; }
	CModel*				GetSourceModel()						{ return m_pSourceModel; }
	
	CVec3f				GetTargetBonePosition(const tstring &sBone, uint uiTime);
	void				GetTargetBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
	CVec3f				GetTargetMeshPosition(const tstring &sMesh);
	CVec3f				GetTargetRandomPositionOnMesh(const tstring &sMesh);
	CVec3f				GetTargetRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Normal);
	CVec3f				GetTargetRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir);
	CVec3f				GetTargetRandomPositionOnSkeleton();
	matrix43_t*			GetTargetBoneTransform(const tstring &sBone);
	bool				GetTargetVisibility(const tstring &sBone);
	CSkeleton*			GetTargetSkeleton()						{ return m_pTargetSkeleton; }
	CModel*				GetTargetModel()						{ return m_pTargetModel; }

	CVec3f				GetCustomBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime);
	bool				GetCustomVisibility(CSkeleton *pSkeleton, const tstring &sBone);
	void				GetCustomBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
		
	CAxis				SourceTransformAxis(const CAxis &aAxis);
	CVec3f				SourceTransformPosition(const CVec3f &v3Pos);
	
	CAxis				TargetTransformAxis(const CAxis &aAxis);
	CVec3f				TargetTransformPosition(const CVec3f &v3Pos);
	
	void				SetSourcePos(const CVec3f &v3Pos)		{ m_v3SourcePos = v3Pos; }
	void				SetSourceAxis(const CAxis &aAxis)		{ m_aSourceAxis = aAxis; }
	void				SetSourceScale(float fScale)			{ m_fSourceScale = fScale; }
	void				SetSourceEffectScale(float fScale)		{ m_fSourceEffectScale = fScale; }
	void				SetSourceVisibility(bool bVisibility)	{ m_bSourceVisibility = bVisibility; }
	void				SetCustomVisibility(bool bVisibility)	{ m_bCustomVisibility = bVisibility; }
	
	const CVec3f&		GetSourcePosition() const				{ return m_v3SourcePos; }
	const CAxis&		GetSourceAxis() const					{ return m_aSourceAxis; }
	float				GetSourceScale() const					{ return m_fSourceScale; }
	float				GetSourceEffectScale() const			{ return m_fSourceEffectScale; }
	bool				GetSourceVisibility() const				{ return m_bSourceVisibility; }
	bool				GetCustomVisibility() const				{ return m_bCustomVisibility; }
	
	void				SetTargetPos(const CVec3f &v3Pos)		{ m_v3TargetPos = v3Pos; }
	void				SetTargetAxis(const CAxis &aAxis)		{ m_aTargetAxis = aAxis; }
	void				SetTargetScale(float fScale)			{ m_fTargetScale = fScale; }
	void				SetTargetEffectScale(float fScale)		{ m_fTargetEffectScale = fScale; }
	void				SetTargetVisibility(bool bVisibility)	{ m_bTargetVisibility = bVisibility; }
	
	const CVec3f&		GetTargetPosition() const				{ return m_v3TargetPos; }
	const CAxis&		GetTargetAxis() const					{ return m_aTargetAxis; }
	float				GetTargetScale() const					{ return m_fTargetScale; }
	float				GetTargetEffectScale() const			{ return m_fTargetEffectScale; }
	bool				GetTargetVisibility() const				{ return m_bTargetVisibility; }

	void				Wait(uint uiDuration)					{ m_uiWaitTime += uiDuration; }

	void				StartCameraShake(uint uiMilliseconds, float fFalloffStart, float fFalloffEnd, float fFrequency, float fScale, uint uiDuration);
	void				StartCameraKick(uint uiMilliseconds, const CRangef &rfPitch, float fTurn, const CRangef &rfBack, const CRangef &rfUp, const CRangef &rfRight, float fHalfLife);
	void				StartCameraShake2(uint uiMilliseconds, float fFalloffStart, float fFalloffEnd, float fFrequency, float fScale, uint uiDuration);

	uint				GetKickTime() const						{ return m_uiCameraKickTime; }

	void				StartOverlay(uint uiMilliseconds, const CTemporalPropertyv3 &tv3Color, const CTemporalPropertyf &tfAlpha, ResHandle hMaterial, uint uiDuration);
	void				UpdateOverlay(uint uiMilliseconds);
	bool				HasActiveOverlay() const				{ return m_bActiveOverlay; }
	CVec4f				GetOverlayColor() const					{ return m_v4OverlayColor; }
	ResHandle			GetOverlayMaterial() const				{ return m_hOverlayMaterial; }

	const CVec3f&		GetColor() const						{ return m_v3Color; }
	void				SetColor(const CVec3f &v3Color)			{ m_v3Color = v3Color; }
};
//=============================================================================

#endif	//__C_EFFECTTHREAD_H__
