#ifndef __MOUSEPOS_T__
#define __MOUSEPOS_T__

typedef struct mousepos_s
{
	int x;
	int y;

	int deltax;
	int deltay;

	float realpitch;
	float realyaw;
}
mousepos_t;

#endif // __MOUSEPOS_T__