// (C)2005 S2 Games
// mv.h
//
// Modelviewer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "../k2/k2_types.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_effectthread.h"
//=============================================================================

const int NUM_MV_SOUNDS(64);
const int NUM_MV_EFFECTS(256);

typedef struct mvres_s
{
	ResHandle	mainCursor;
}
mvres_t;

enum eMVButton
{
	MV_BUTTON_LEFT = BIT(0),
	MV_BUTTON_RIGHT = BIT(1),
	MV_BUTTON_MIDDLE = BIT(2),
	MV_BUTTON_SHIFT = BIT(3),
	MV_BUTTON_CTRL = BIT(4),
	MV_BUTTON_ALT = BIT(5),
	MV_BUTTON_FORWARD = BIT(6),
	MV_BUTTON_BACKWARD = BIT(7),
	MV_BUTTON_STRAFE_LEFT = BIT(8),
	MV_BUTTON_STRAFE_RIGHT = BIT(9)
};

struct SMv
{
	int			button;

	CCamera		camera;

	ResHandle		hActiveModel;
	uint	lastFrame;
	float			frametime;

	SoundHandle		ahSounds[NUM_MV_SOUNDS];
	CEffectThread*	apEffectThread[NUM_MV_EFFECTS];
};

extern SMv mv;
extern mvres_t res;
