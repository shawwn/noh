#ifndef __MDLSPRITE_T__
#define __MDLSPRITE_T__

struct SMdlSprite
{
	struct model_s *parent;

	char		*name;

	int			type;		//see S2SPRITE_* defines in s2model.h

	float		width;
	float		height;

	int			bone;		//bone the sprite is linked to
};

#endif // __MDLSPRITE_T__
