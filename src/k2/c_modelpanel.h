// (C)2006 S2 Games
// c_modelpanel.h
//
//=============================================================================
#ifndef __C_MODELPANEL_H__
#define __C_MODELPANEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
class ICvar;
class CSkeleton;
class CEffectThread;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CModelPanel
//=============================================================================
class CModelPanel : public IWidget
{
protected:
	struct SModel
	{
		tstring*		sModel;
		tstring*		sEffect;
		tstring*		sAnim;

		ResHandle		hModel;
		SkinHandle		hSkin;
		CSkeleton		*pSkeleton;
		CEffectThread	*pEffectThread;

		CVec3f			v3ModelPos;
		CVec3f			v3ModelAngles;
		float			fModelScale;

		SModel() :
		sModel(NULL),
		sEffect(NULL),
		sAnim(NULL),
		hModel(INVALID_RESOURCE),
		hSkin(0),
		pSkeleton(NULL),
		pEffectThread(NULL),
		v3ModelPos(0.0f, 0.0f, 0.0f),
		v3ModelAngles(0.0f, 0.0f, 0.0f),
		fModelScale(1.0f)
		{}

		~SModel()
		{
			SAFE_DELETE(sModel);
			SAFE_DELETE(sEffect);
			SAFE_DELETE(sAnim);
		}
	};
	typedef hash_map<int, SModel>	ModelMap;

	ModelMap		m_mapModels;

	tstring*		m_sRestoreModel;
	tstring*		m_sRestoreEffect;
	tstring*		m_sRestoreAnim;

	CCamera			m_Camera;

	ResHandle		m_hModel;
	SkinHandle		m_hSkin;
	CSkeleton*		m_pSkeleton;
	CEffectThread*	m_pEffectThread;

	CVec3f			m_v3ModelPos;
	CVec3f			m_v3ModelAngles;
	float			m_fModelScale;
	
	CVec3f			m_v3ModelStartPos;
	CVec3f			m_v3ModelEndPos;
	uint			m_uiModelMoveStartTime;
	uint			m_uiModelMoveEndTime;

	CVec3f			m_v3ModelStartAngles;
	CVec3f			m_v3ModelEndAngles;
	uint			m_uiModelRotationStartTime;
	uint			m_uiModelRotationEndTime;

	bool			m_bOrtho;
	bool			m_bLookAt;
	bool			m_bShadows;
	bool			m_bDepthClear;
	bool			m_bDepthCompress;
	bool			m_bPostEffects;
	bool			m_bReflections;
	bool			m_bSceneBuffer;

	tstring			m_sAnim;
	tstring			m_sSkin;

	CVec3f			m_v3CameraPos;
	float			m_fCameraDist;

	CVec3f			m_v3SunColor;
	CVec3f			m_v3AmbientColor;
	float			m_fSunAltitude;
	float			m_fSunAzimuth;

	bool			m_bFog;
	CVec3f			m_v3FogColor;
	float			m_fFogNear;
	float			m_fFogFar;
	float			m_fFogScale;
	float			m_fFogDensity;

	bool			m_bFovY;
	bool			m_bLookUpResource;

	float			m_fCameraNear;
	float			m_fCameraFar;

	CVec4f			m_v4TeamColor;

	void			UpdateEffect(CEffectThread *&pEffectThread, const CVec3f &v3ModelPos, const CVec3f &v3ModelAngles, float fModelScale, ResHandle hModel);

	void			ReadModelProperties(const CWidgetStyle& style, int i);

public:
	~CModelPanel();
	CModelPanel(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

	void	RenderWidget(const CVec2f &vOrigin, float fFade);

	void	SetTeamColor(const CVec4f &v4Color)		{ m_v4TeamColor = v4Color; }

	void	SetAnim(const tstring &sAnim);
	void	SetModel(const tstring &sModel);
	void	SetEffect(const tstring &sEffect);

	void	SetAnim(int i, const tstring &sAnim);
	void	SetModel(int i, const tstring &sModel);
	void	SetEffect(int i, const tstring &sEffect);

	const tstring&	GetAnim() const;
	const tstring&	GetModel() const;
	const tstring&	GetEffect() const;

	const tstring&	GetAnim(int i) const;
	const tstring&	GetModel(int i) const;
	const tstring&	GetEffect(int i) const;
	
	void	RecalculateSize();

	const	CVec3f&	GetModelPos() const	{ return m_v3ModelPos; }
	
	void	MoveModel(const CVec3f &v3Start, const CVec3f &v3End, uint uiTime);
	void	RotateModel(const CVec3f &v3Start, const CVec3f &v3End, uint uiTime);

	void	SetModelPos(const CVec3f &v3ModelPos);
	void	SetModelAngles(const CVec3f &v3ModelAngles);
	void	SetModelScale(float fModelScale);
};
//=============================================================================
#endif //__C_MODELPANEL_H__
