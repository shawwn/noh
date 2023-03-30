// (C)2008 S2 Games
// c_clifftool.h
//
//=============================================================================
#ifndef __C_CLIFFTOOL_H__
#define __C_CLIFFTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
#include "../k2/c_cliffdefinitionresource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ECliffMode
{
    CLIFF_CREATE = 0,
    CLIFF_PAINT,
    CLIFF_CLEAR,
    CLIFF_VARIATION,
    CLIFF_DEFINITION,
    CLIFF_RAMPCREATE,
    CLIFF_WC3
};

enum EBlockerFlags
{
    BLOCKER_REMOVE = 0x01,
    BLOCKER_LEFT = 0x02,
    BLOCKER_RIGHT = 0x04,
    BLOCKER_BOT = 0x08,
    BLOCKER_TOP = 0x10
};

    // Ramp Flags per tile.
const uint RAMP_FLAG_SET                    (BIT(0)); // If true this Ramp tile is in use.
    // What direction?
const uint RAMP_FLAG_LEFT_RIGHT_INCLINE     (BIT(1)); // True if the ramp is facing Left to Right. If not its Front to Back.
const uint RAMP_FLAG_INVERT_INCLINE         (BIT(2)); // Determins the facing. If its not set then its Left to Right. If it is set then its Right to Left. Same Idea for Front to Back, Back to Front.
const uint RAMP_FLAG_DIAGONAL               (BIT(7)); // If set then the Ramp will be facing a Diagonal direction. 
    // What type?
    // First and Second are Relitive to the Ramps Facing. If the ramp is Left to Right and non Inverted then Left is First and Right is Second.
    // If none of the flags 2 - 5 are set then the Ramp will not have a model.
const uint RAMP_FLAG_FIRST_INWARD           (BIT(3)); // If Set then the Ramp will have an Inward Ramp model placed here.
const uint RAMP_FLAG_FIRST_OUTWARD          (BIT(4)); // If set then the Ramp will have an Outward Ramp model placed here.
const uint RAMP_FLAG_SECOND_INWARD          (BIT(5)); // If Set then the Ramp will have an Inward Ramp model placed here.
const uint RAMP_FLAG_SECOND_OUTWARD         (BIT(6)); // If set then the Ramp will have an Outward Ramp model placed here. 
    // Removable?
const uint RAMP_FLAG_EX_REMOVABLE           (BIT(8)); // Only used in temporary data.
const uint RAMP_FLAG_EX_FLAT                (BIT(9)); // Tmp only.
const uint RAMP_FLAG_EX_FIRST_DIAGONAL      (BIT(10));
const uint RAMP_FLAG_EX_SECOND_DIAGONAL     (BIT(11));
const uint RAMP_FLAG_EX_DOUBLE_LEFT         (BIT(12));
const uint RAMP_FLAG_EX_DOUBLE_RIGHT        (BIT(13));
//=============================================================================

//=============================================================================
// CCliffTool
//=============================================================================
class CCliffTool : public ITool
{
private:
    bool        m_bValidPosition;
    int         m_iCliffHeight;
    bool        m_bWorking;
    bool        m_bInverse;
    bool        m_bPrimaryDown;
    bool        m_bSecondaryDown;
    bool        m_bFirstLoop;
    uint*       m_uiCliffDrewMap;
    uint        m_uiMapSize;
    CVec3f      m_v3EndPos;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hCliffMaterial;
    ResHandle   m_hFont;
    tstring     m_sLastCliffDef;
    ResHandle   m_CliffDefinitionHandle;
    int         m_iOldX, m_iOldY, m_iXOffset, m_iYOffset, m_iXCliffCenter, m_iYCliffCenter, m_iXPaint, m_iYPaint;

    static void CliffModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);

public:
    CCliffTool();
    ~CCliffTool()               { if (m_uiCliffDrewMap) K2_DELETE_ARRAY(m_uiCliffDrewMap);}

    float   CalculateHeightVertex(int iX, int iY);
    void    CalculateCliffBlockers(CRecti scanArea);
    void    CalcToolProperties();
    void    PaintCliff(float fFrameTime);
    void    CalculateTile(int iXC, int iYC);
    void    CalculateHiddenTiles(CRecti CliffSquare, bool bCliff);
    void    CliffClear();
    void    CliffDraw();
    void    CliffCreate(CVec3f vCliffPos, float fRotation, tstring tModel, int iRotationVertex);
    void    RampCreate(CVec3f vRampPos, float fRotation, tstring tModel, int iRotationVertex);
    void    RotationAdjustment(uint uiEntity, int iRotationVertex);
    void    CliffDelete(int iX, int iY);
    void    CliffDelete(CRecti rectArea);
    void    CliffTilePaint(int iXC, int iYC, bool bAdd = true);
    bool    EnforceHeight(int x, int y, CRecti &enforcedRect);
    void    DefinitionMapSet();
    void    VariationMapSet();
    void    CliffMapAdd(int iCliffHeight);
    int     CliffMapGet();
    void    CliffMapSet(int iCliffHeight);
    void    CliffVariation();
    void    CliffDefinition();
    void    RampPlace();
    void    RampPlaceModel(int iX, int iY, ushort unRampType);
    void    RampErase();
    void    RampErase(int iX, int iY);
    void    CheckForRampsToErase(int iX, int iY);
    void    CreateRampMidPiece();
    int     GetCliffVertHeight(int iXC, int iYC);
    ushort  CanPlaceRamp(int iX, int iY);
    bool    CanPlaceDiagonalRamp(int iX, int iY);
    void    ClampCliffRectToGrid(CRecti *rArea);
    void    ClampRectToGrid(CRecti *rArea);
    bool    CanBlockVert(int iX, int iY);

    void    PrimaryUp();
    void    PrimaryDown();
    void    SecondaryUp();
    void    SecondaryDown();
    void    TertiaryUp();
    void    TertiaryDown();
    void    QuaternaryUp();
    void    QuaternaryDown();

    void    Cancel();
    void    Delete();
    void    Enter();

    void    Frame(float fFrameTime);

    void    Draw();
    void    Render();
    
        // Ramp Flag Tests
    bool    IsRampFlagFirstSet(ushort unFlags)          { return unFlags & RAMP_FLAG_FIRST_INWARD || unFlags & RAMP_FLAG_FIRST_OUTWARD; }
    bool    IsRampFlagSecondSet(ushort unFlags)         { return unFlags & RAMP_FLAG_SECOND_INWARD || unFlags & RAMP_FLAG_SECOND_OUTWARD; }
};
//=============================================================================

#endif // __C_CLIFFTOOL_H__
