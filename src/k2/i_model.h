// (C)2005 S2 Games
// i_model.h
//
//=============================================================================
#ifndef __I_MODEL_H__
#define __I_MODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSkin;
class CModelEffect;
class CXMLNode;
class CModel;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EModelType
{
	MODEL_K2,			// Models from our Max exporter
	MODEL_SPEEDTREE,	// SpeedTree model

	NUM_MODEL_TYPES
};

typedef vector<CConvexPolyhedron>	SurfVector;
typedef vector<CSkin*>				SkinVector;
//=============================================================================

//=============================================================================
// IModel
//=============================================================================
class IModel
{
protected:
	tstring			m_sName;
	EModelType		m_eType;

	uint			m_uiSeed;

	SurfVector		m_vCollisionSurfs;	// surfaces used for collision detection

	CBBoxf			m_bbBounds;

	SkinVector		m_vSkins;
	tsvector		m_vSpawnEvents;

	IModel*			m_pBaseLod;
	vector<IModel*>	m_vLods;
	float			m_fLodDistance;

	CModel			*m_pModel;

	IModel();

public:
	virtual ~IModel();
	IModel(EModelType eType) : m_eType(eType), m_uiSeed(1), m_pBaseLod(NULL), m_fLodDistance(0.0f), m_pModel(NULL)	{}

	const tstring&		GetName() const								{ return m_sName; }
	void				SetName(const tstring &sName)				{ m_sName = sName; }

	CModel*				GetModel() const							{ return m_pModel; }
	void				SetModel(CModel *pModel)					{ m_pModel = pModel; }

	EModelType			GetType() const								{ return m_eType; }

	uint				GetSeed()									{ return m_uiSeed; }
	virtual void		SetSeed(uint uiSeed)						{ m_uiSeed = uiSeed; }

	const CBBoxf&		GetBounds() const							{ return m_bbBounds; }
	CBBoxf&				GetBounds()									{ return m_bbBounds; }
	void				SetBounds(const CBBoxf &bb)					{ m_bbBounds = bb; }

	K2_API void			AddCollisionSurf(const CConvexPolyhedron &cSurf);
	CConvexPolyhedron&	AllocCollisionSurf()					{ m_vCollisionSurfs.push_back(CConvexPolyhedron()); return m_vCollisionSurfs.back(); }
	SurfVector&			GetSurfs()								{ return m_vCollisionSurfs; }

	K2_API CBBoxf		GetModelSurfaceBounds();
	K2_API CBBoxf		GetModelVisualBounds();

	virtual bool		Load(const tstring &sFileName, uint uiIgnoreFlags) = 0;
	virtual void		ProcessProperties(const CXMLNode &node) = 0;
	virtual void		PostLoad() = 0;

	// Skins
	virtual uint		GetNumMaterials() const = 0;

	K2_API SkinHandle	GetSkinHandle(const tstring &sName) const;
	CSkin*				GetSkin(SkinHandle hSkin);
	K2_API void			AddSkin(const CSkin &skin);
	K2_API void			ClearSkins();
	uint				NumSkins() const;

	void				AddSpawnEvent(const tstring &sCmd)	{ m_vSpawnEvents.push_back(sCmd); }
	const tsvector&		GetSpawnEvents()						{ return m_vSpawnEvents; }

	uint				GetNumLods()		{ return uint(m_vLods.size()); }
	IModel*				GetLod(uint ui)		{ return m_vLods[ui]; }
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  IModel::GetSkin
  ====================*/
inline
CSkin*	IModel::GetSkin(SkinHandle hSkin)
{
	return m_vSkins[hSkin];
}


/*====================
  IModel::NumSkins
  ====================*/
inline
uint	IModel::NumSkins() const
{
	return int(m_vSkins.size());
}

//=============================================================================

#endif //__I_MODEL_H__
