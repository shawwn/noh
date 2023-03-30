// (C)2005 S2 Games
// k2_mathlib.cpp
//
// general 2d / 3d math functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "k2_mathlib.h"

#include <zlib.h> // crc32

#include "c_plane.h"
#include "c_axis.h"
#include "intersection.h"
#include "k2_randlib.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================

// From "Texturing and Modeling, a Procedural Approach", chapter 6
const int BB = 0x100;
const int BM = 0xff;

const int N = 0x1000;
const int NP = 12;   // 2^N
const int NM = 0xfff;

static int p[BB + BB + 2];
static float g3[BB + BB + 2][3];
static float g2[BB + BB + 2][2];
static float g1[BB + BB + 2];

#define s_curve(t) (t * t * (3.0f - 2.0f * t))
#define at2(rx,ry) (rx * q[0] + ry * q[1])
#define at3(rx, ry, rz) (rx * q[0] + ry * q[1] + rz * q[2])

#define NOISE_SETUP(i, b0, b1, r0, r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0 + 1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.0f;

int sin_tmp;
float lattice[128][128];

static uint s_auiCRC32[256];
//=============================================================================

/*====================
  M_NextPowerOfTwo
  ====================*/
int		M_NextPowerOfTwo(int x)
{
	// Check that it isn't already a power of two
	for (int n = 0; n < 32; ++n)
	{
		if (x == (1 << n))
			return x;
	}

	// Find the highest bit
	int c = 0;
	while (x > 0)
	{
		x >>= 1;
		++c;
	}
	
	return (1 << c);
}


/*====================
  M_InitNoise
  ====================*/
static void	M_InitNoise(void)
{
	srand(0);

	int i, j;
	for (i = 0; i < BB; ++i)
	{
		p[i] = i;
		g1[i] = (float)((rand() % (BB + BB)) - BB) / BB;

		for (j = 0; j < 2; ++j)
			g2[i][j] = (float)((rand() % (BB + BB)) - BB) / BB;
		M_NormalizeVec2(g2[i]);

		for (j = 0; j < 3; ++j)
			g3[i][j] = (float)((rand() % (BB + BB)) - BB) / BB;
		M_Normalize(g3[i]);
	}

	while (--i)
	{
		int k = p[i];
		p[i] = p[j = rand() % BB];
		p[j] = k;
	}

	for (i = 0; i < BB + 2; ++i)
	{
		p[BB + i] = p[i];
		g1[BB + i] = g1[i];
		for (j = 0; j < 2; ++j)
			g2[BB + i][j] = g2[i][j];
		for (j = 0; j < 3; ++j)
			g3[BB + i][j] = g3[i][j];
	}

	srand(K2System.GetRandomSeed32());
}


/*====================
  M_Noise1
  ====================*/
float	M_Noise1(float x)
{
   int bx0, bx1;
   float rx0, rx1, sx, t, u, v;
   float vec[1] = {x};

   NOISE_SETUP(0, bx0, bx1, rx0, rx1);

   sx = s_curve(rx0);
   u = rx0 * g1[p[bx0]];
   v = rx1 * g1[p[bx1]];

   return LERP(sx, u, v);
}


/*====================
  M_Noise2
  ====================*/
float	M_Noise2(float x, float y)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int i, j;
	float vec[2] = {x, y};

	NOISE_SETUP(0, bx0, bx1, rx0, rx1);
	NOISE_SETUP(1, by0, by1, ry0, ry1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

	q = g2[b00]; u = at2(rx0,ry0);
	q = g2[b10]; v = at2(rx1,ry0);
	a = LERP(sx, u, v);

	q = g2[b01]; u = at2(rx0,ry1);
	q = g2[b11]; v = at2(rx1,ry1);
	b = LERP(sx, u, v);

	return LERP(sy, a, b);
}


/*====================
  M_Noise3
  ====================*/
float	M_Noise3(float x, float y, float z)
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sx, sy, sz, a, b, c, d, t, u, v;
	int i, j;
	float vec[3] = {x, y, z};

	NOISE_SETUP(0, bx0, bx1, rx0, rx1);
	NOISE_SETUP(1, by0, by1, ry0, ry1);
	NOISE_SETUP(2, bz0, bz1, rz0, rz1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];

	sx = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

	q = g3[b00 + bz0]; u = at3(rx0, ry0, rz0);
	q = g3[b10 + bz0]; v = at3(rx1, ry0, rz0);
	a = LERP(sx, u, v);

	q = g3[b01 + bz0]; u = at3(rx0, ry1, rz0);
	q = g3[b11 + bz0]; v = at3(rx1, ry1, rz0);
	b = LERP(sx, u, v);

	c = LERP(sy, a, b);

	q = g3[b00 + bz1]; u = at3(rx0, ry0, rz1);
	q = g3[b10 + bz1]; v = at3(rx1, ry0, rz1);
	a = LERP(sx, u, v);

	q = g3[b01 + bz1]; u = at3(rx0, ry1, rz1);
	q = g3[b11 + bz1]; v = at3(rx1, ry1, rz1);
	b = LERP(sx, u, v);

	d = LERP(sy, a, b);

	return LERP(sz, c, d);
}


/*====================
  M_SmoothRandAngle1
  ====================*/
float	M_SmoothRandAngle1(float x)
{
	int bx0, bx1;
	float rx0, rx1, sx, t, u, v;
	float vec[1] = {x};

	NOISE_SETUP(0, bx0, bx1, rx0, rx1);

	sx = s_curve(rx0);
	u = g1[p[bx0]] * 180.0f;
	v = g1[p[bx1]] * 180.0f;

	return M_LerpAngle(sx, u, v);
}


/*====================
  M_SmoothRand1

  Like perlin noise but without the "bouncing" toward zero
  ====================*/
float	M_SmoothRand1(float x)
{
   int bx0, bx1;
   float rx0, rx1, sx, t, u, v;
   float vec[1] = {x};

   NOISE_SETUP(0, bx0, bx1, rx0, rx1);

   sx = s_curve(rx0);
   u = g1[p[bx0]];
   v = g1[p[bx1]];

   return LERP(sx, u, v);
}


/*====================
  M_InitSimpleNoise2
  ====================*/
void    M_InitSimpleNoise2()
{
	int x,y;

	for (y = 0; y < 128; ++y)
	{
		for (x = 0; x < 128; ++x)
		{
			lattice[y][x] = M_Randnum(-1.0f, 1.0f);
		}
	}
}


/*====================
  M_SimpleNoise2
  ====================*/
float   M_SimpleNoise2(uint x, uint y)
{   
	return lattice[y % 128][x % 128];
}

#define FP_BITS(fp) (*(int *)&(fp))
#define FP_ABS_BITS(fp) (FP_BITS(fp)&0x7FFFFFFF)
#define FP_SIGN_BIT(fp) (FP_BITS(fp)&0x80000000)
#define FP_ONE_BITS 0x3F800000

typedef union FastSqrtUnion
{
  float f;
  uint i;
}
FastSqrtUnion;


/*====================
  M_Normalize
  ====================*/
float	M_Normalize(vec3_t out)
{
	float length;

	length = sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
	if (length == 0)
	{
		out[0] = 0;
		out[1] = 0;
		out[2] = 0;
		return 0;
	}
	
	out[0] = out[0] / length;
	out[1] = out[1] / length;
	out[2] = out[2] / length;

	//clamp components between -1 and 1 (necessary because of precision issues with sqrt)
	for (int n = 0; n < 3; ++n)
	{
		if (out[n] > 1)
			out[n] = 1;
		else if (out[n] < -1)
			out[n] = -1;
	}
	return length;
}


/*====================
  M_NormalizeVec2
  ====================*/
float M_NormalizeVec2(vec2_t out)
{
	float length;

	length = sqrt(out[0] * out[0] + out[1] * out[1]);
	if (length == 0)
	{
		out[0] = 0;
		out[1] = 0;		
		return 0;
	}
	
	out[0] = out[0] / length;
	out[1] = out[1] / length;	

	//clamp components between -1 and 1 (necessary because of precision issues with sqrt)
	for (int n = 0; n < 2; ++n)
	{
		if (out[n] > 1)
			out[n] = 1;
		else if (out[n] < -1)
			out[n] = -1;
	}

	return length;
}


/*====================
  M_GetDistanceSqVec2
  ====================*/
float	M_GetDistanceSqVec2(const vec2_t pos1, const vec2_t pos2)
{
	vec2_t diff;

	M_SubVec2(pos1, pos2, diff);
	return M_DotProduct2(diff, diff);
}


/*====================
  M_GetDistanceSq
  ====================*/
float	M_GetDistanceSq(const CVec3f &v3A, const CVec3f &v3B)
{
	CVec3f v3Diff(v3A - v3B);
	return DotProduct(v3Diff, v3Diff);
}


/*====================
  M_AABBOnPlaneSide
  ====================*/
EPlaneTest	M_AABBOnPlaneSide(const CBBoxf &bbBox, const CPlane &p)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Neg, v3Pos; // negative and positive vertices
	
	if (p.v3Normal.x > 0.0f)
	{
		v3Pos.x = v3Max.x;
		v3Neg.x = v3Min.x;
	}
	else
	{
		v3Pos.x = v3Min.x;
		v3Neg.x = v3Max.x;
	}

	if (p.v3Normal.y > 0.0f)
	{
		v3Pos.y = v3Max.y;
		v3Neg.y = v3Min.y;
	}
	else
	{
		v3Pos.y = v3Min.y;
		v3Neg.y = v3Max.y;
	}

	if (p.v3Normal.z > 0.0f)
	{
		v3Pos.z = v3Max.z;
		v3Neg.z = v3Min.z;
	}
	else
	{
		v3Pos.z = v3Min.z;
		v3Neg.z = v3Max.z;
	}

	if (DotProduct(p.v3Normal, v3Pos) < p.fDist)
		return PLANE_NEGATIVE;
	else if (DotProduct(p.v3Normal, v3Neg) > p.fDist)
		return PLANE_POSITIVE;
	else
		return PLANE_INTERSECTS;
}


/*====================
  M_OBBOnPlaneSide
  ====================*/
EPlaneTest		M_OBBOnPlaneSide(const CBBoxf &bbBox, const CVec3f &v3Pos, const CAxis &aAxis, const CPlane &p)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Negative, v3Positive; // Negative and positive vertices
	CVec3f	v3Normal;

	// Transform the plane normal into the space of the box so we can work out the P and N vertices
	v3Normal.x = DotProduct(p.v3Normal, aAxis[0]);
	v3Normal.y = DotProduct(p.v3Normal, aAxis[1]);
	v3Normal.z = DotProduct(p.v3Normal, aAxis[2]);

	if (v3Normal.x > 0.0f)
	{
		v3Positive.x = v3Max.x;
		v3Negative.x = v3Min.x;
	}
	else
	{
		v3Positive.x = v3Min.x;
		v3Negative.x = v3Max.x;
	}

	if (v3Normal.y > 0.0f)
	{
		v3Positive.y = v3Max.y;
		v3Negative.y = v3Min.y;
	}
	else
	{
		v3Positive.y = v3Min.y;
		v3Negative.y = v3Max.y;
	}

	if (v3Normal.z > 0.0f)
	{
		v3Positive.z = v3Max.z;
		v3Negative.z = v3Min.z;
	}
	else
	{
		v3Positive.z = v3Min.z;
		v3Negative.z = v3Max.z;
	}
	// we need the N and P vertices in world space
	v3Positive = TransformPoint(v3Positive, aAxis, v3Pos);
	// dg - deferred the second xform

	if (M_DotProduct(p.v3Normal, v3Positive) < p.fDist)
	{
		return PLANE_NEGATIVE;
	}
	else
	{
		v3Negative = TransformPoint(v3Negative, aAxis, v3Pos);
		if (M_DotProduct(p.v3Normal, v3Negative) > p.fDist)
			return PLANE_POSITIVE;
		else
			return PLANE_INTERSECTS;
	}
}


/*====================
  M_BlendMatrix
  ====================*/
void	M_BlendMatrix(const matrix43_t *a, const matrix43_t *b, float fAmount, matrix43_t *out)
{
	out->axis[0][0] = a->axis[0][0] + b->axis[0][0] * fAmount;
	out->axis[0][1] = a->axis[0][1] + b->axis[0][1] * fAmount;
	out->axis[0][2] = a->axis[0][2] + b->axis[0][2] * fAmount;
	out->axis[1][0] = a->axis[1][0] + b->axis[1][0] * fAmount;
	out->axis[1][1] = a->axis[1][1] + b->axis[1][1] * fAmount;
	out->axis[1][2] = a->axis[1][2] + b->axis[1][2] * fAmount;
	out->axis[2][0] = a->axis[2][0] + b->axis[2][0] * fAmount;
	out->axis[2][1] = a->axis[2][1] + b->axis[2][1] * fAmount;
	out->axis[2][2] = a->axis[2][2] + b->axis[2][2] * fAmount;
	out->pos[0] = a->pos[0] + b->pos[0] * fAmount;
	out->pos[1] = a->pos[1] + b->pos[1] * fAmount;
	out->pos[2] = a->pos[2] + b->pos[2] * fAmount;
}


/*====================
  M_QuatToAxis
  ====================*/
void	M_QuatToAxis(const vec4_t quat, vec3_t m[3])
{
	// calculate coefficients
	float x2(quat[X] + quat[X]), y2(quat[Y] + quat[Y]), z2(quat[Z] + quat[Z]);
	float xx(quat[X] * x2), xy(quat[X] * y2), xz(quat[X] * z2);
	float yy(quat[Y] * y2), yz(quat[Y] * z2), zz(quat[Z] * z2);
	float wx(quat[W] * x2), wy(quat[W] * y2), wz(quat[W] * z2);

	m[0][0] = 1.0f - (yy + zz);
	m[1][0] = xy - wz;
	m[2][0] = xz + wy;		

	m[0][1] = xy + wz;
	m[1][1] = 1.0f - (xx + zz);
	m[2][1] = yz - wx;		

	m[0][2] = xz - wy;
	m[1][2] = yz + wx;
	m[2][2] = 1.0f - (xx + yy);
}

CAxis	M_QuatToAxis(const CVec4f &v4)
{
	// calculate coefficients
	float x2(v4[X] + v4[X]), y2(v4[Y] + v4[Y]), z2(v4[Z] + v4[Z]);
	float xx(v4[X] * x2), xy(v4[X] * y2), xz(v4[X] * z2);
	float yy(v4[Y] * y2), yz(v4[Y] * z2), zz(v4[Z] * z2);
	float wx(v4[W] * x2), wy(v4[W] * y2), wz(v4[W] * z2);

	return CAxis(
		CVec3f(1.0f - (yy + zz), xy + wz, xz - wy),
		CVec3f(xy - wz, 1.0f - (xx + zz), yz + wx),
		CVec3f(xz + wy, yz - wx, 1.0f - (xx + yy))
		);
}


/*====================
  M_AxisToQuat
  ====================*/
void	M_AxisToQuat(const vec3_t m[3], vec4_t quat)
{
	float	tr, s, q[4];
	int		i, j, k;
	int		nxt[3] = {1, 2, 0};
	
	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0)
	{
		s = sqrt (tr + 1.0f);
		quat[W] = s / 2.0f;
		s = 0.5f / s;
		quat[X] = (m[1][2] - m[2][1]) * s;
		quat[Y] = (m[2][0] - m[0][2]) * s;
		quat[Z] = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;
		if (m[1][1] > m[0][0])
			i = 1;
		if (m[2][2] > m[i][i])
			i = 2;

		j = nxt[i];
		k = nxt[j];
		s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0f)
			s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;

		quat[X] = q[0];
		quat[Y] = q[1];
		quat[Z] = q[2];
		quat[W] = q[3];
	}
}

CVec4f	M_AxisToQuat(const CAxis &m)
{
	CVec4f quat;
	float	tr, s, q[4];
	int		i, j, k;
	int		nxt[3] = {1, 2, 0};
	
	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0)
	{
		s = sqrt (tr + 1.0f);
		quat[W] = s / 2.0f;
		s = 0.5f / s;
		quat[X] = (m[1][2] - m[2][1]) * s;
		quat[Y] = (m[2][0] - m[0][2]) * s;
		quat[Z] = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;
		if (m[1][1] > m[0][0])
			i = 1;
		if (m[2][2] > m[i][i])
			i = 2;

		j = nxt[i];
		k = nxt[j];
		s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0f)
			s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;

		quat[X] = q[0];
		quat[Y] = q[1];
		quat[Z] = q[2];
		quat[W] = q[3];
	}

	return quat;
}


/*====================
  M_GetYawFromForwardVec2
  ====================*/
float	M_GetYawFromForwardVec2(const CVec2f &v2Forward)
{
	float fResult;

	if (v2Forward.x == 0.0f && v2Forward.y == 0.0f)
	{
		fResult = 0.0f;
	}
	else if (v2Forward.x == 0.0f)
	{
		if (v2Forward.y > 0.0f)
			fResult = 0.0f;
		else
			fResult = 180.0f;
	}
	else
	{
		fResult = RAD2DEG(atan2(-v2Forward.x, v2Forward.y));
	}

	if (fResult > 180.0f)
		fResult -= 360.0f;

	return fResult;
}


/*====================
  M_GetAnglesFromForwardVec
  ====================*/
void	M_GetAnglesFromForwardVec(const CVec3f &vForward, CVec3f &vAngles)
{
	if (vForward.x == 0.0f && vForward.y == 0.0f) // straight up
	{
		// +Z will go up, -z down
		if (vForward.z > 0.0f)
			vAngles[PITCH] = 90.0f;
		else
			vAngles[PITCH] = -90.0f;
		
		vAngles[YAW] = 0.0f;
	}
	else
	{
		if (vForward.x == 0.0f) // north - south
		{
			// +y will go north, -y south

			if (vForward.y > 0.0f)
				vAngles[YAW]  = 0.0f;
			else
				vAngles[YAW]  = 180.0f;
		}
		else
		{
			vAngles[YAW] = RAD2DEG(atan2(-vForward.x, vForward.y));
		}

		if (vForward.z == 0.0f)
		{
			vAngles[PITCH] = 0.0f;
		}
		else
		{
			vAngles[PITCH] = RAD2DEG(atan2(vForward.z, sqrt(vForward.x * vForward.x + vForward.y * vForward.y)));
		}
	}

	vAngles[ROLL] = 0.0f;
}


/*====================
  M_GetAnglesFromForwardVec
  ====================*/
CVec3f	M_GetAnglesFromForwardVec(const CVec3f &vForward)
{
	CVec3f v3Angles;

	if (vForward.x == 0.0f && vForward.y == 0.0f) // straight up
	{
		// +Z will go up, -z down
		if (vForward.z > 0.0f)
			v3Angles[PITCH] = 90.0f;
		else
			v3Angles[PITCH] = -90.0f;
		
		v3Angles[YAW] = 0.0f;
	}
	else
	{
		if (vForward.x == 0.0f) // north - south
		{
			// +y will go north, -y south
			if (vForward.y > 0.0f)
				v3Angles[YAW]  = 0.0f;
			else
				v3Angles[YAW]  = 180.0f;
		}
		else
		{
			v3Angles[YAW] = RAD2DEG(atan2(-vForward.x, vForward.y));
		}

		if (vForward.z == 0.0f)
		{
			v3Angles[PITCH] = 0.0f;
		}
		else
		{
			v3Angles[PITCH] = RAD2DEG(atan2(vForward.z, sqrt(vForward.x * vForward.x + vForward.y * vForward.y)));
		}
	}

	v3Angles[ROLL] = 0.0f;

	return v3Angles;
}


/*====================
  M_GetForwardVecFromAngles
  ====================*/
CVec3f	M_GetForwardVecFromAngles(const CVec3f &v3Angles)
{
	float sp = DEGSIN(v3Angles[PITCH]);
	float cp = DEGCOS(v3Angles[PITCH]);
	float sy = DEGSIN(v3Angles[YAW]);
	float cy = DEGCOS(v3Angles[YAW]);

	return CVec3f(cp * -sy, cp * cy, sp);
}


/*====================
  M_GetForwardVec2FromYaw
  ====================*/
CVec2f	M_GetForwardVec2FromYaw(float fYaw)
{
	float sy = DEGSIN(fYaw);
	float cy = DEGCOS(fYaw);

	return CVec2f(-sy, cy);
}


/*====================
  M_Randnum

  simple random number generator
  32,767 values are possible for any given range.
  ====================*/
float	M_Randnum(float fMin, float fMax)
{
	if (fMax <= fMin)
		return fMin;

	float x = (rand() & 0x7fff) / (float)0x7fff;
	return x * (fMax - fMin) + fMin;
}

int		M_Randnum(int iMin, int iMax)
{
	if (iMax <= iMin)
		return iMin;

	int x = rand() % (iMax - iMin + 1);
	return x + iMin;
}

uint	M_Randnum(uint uiMin, uint uiMax)
{
	if (uiMax <= uiMin)
		return uiMin;

	uint x = rand() % (uiMax - uiMin + 1);
	return x + uiMin;
}


/*====================
  M_TransformBounds

  rotates the bounding box points and creates a new axis-aligned
  bounding box out of it (new box may be larger than the original)
  ====================*/
void	M_TransformBounds(const CVec3f &bMin, const CVec3f &bMax, const CVec3f &pos, const CAxis &axis, CVec3f &bMin_out, 
						  CVec3f &bMax_out)
{
	CVec3f trans_bMin, trans_bMax;
	CVec3f p;

	M_ClearBounds(trans_bMin, trans_bMax);

	p = TransformPoint(CVec3f(bMin.x, bMin.y, bMin.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMin.x, bMin.y, bMax.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMin.x, bMax.y, bMin.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMin.x, bMax.y, bMax.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMax.x, bMin.y, bMin.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMax.x, bMin.y, bMax.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMax.x, bMax.y, bMin.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);
	p = TransformPoint(CVec3f(bMax.x, bMax.y, bMax.z), axis, pos);
	p.AddToBounds(trans_bMin, trans_bMax);

	bMin_out = trans_bMin;
	bMax_out = trans_bMax;
}


/*====================
  M_RayPlaneIntersect

  intersection test for ray with plane
  ====================*/
float	M_RayPlaneIntersect(const CVec3f &v3Origin, const CVec3f &v3Dir, const CPlane &plPlane, CVec3f &v3Result)
{
	v3Result.Clear();

	float fDenom(DotProduct(plPlane.v3Normal, v3Dir));

	float fTime((plPlane.fDist - DotProduct(plPlane.v3Normal, v3Origin)) / fDenom);
	if (fTime < 0.0f)
		return 0.0f;

	v3Result = v3Origin + (v3Dir * fTime);
	return fTime;
}


/*====================
  M_CompareVec3
  ====================*/
bool	M_CompareVec3(const vec3_t a, const vec3_t b, float fEpsilon)
{
	for (int i(0); i < 3; ++i)
	{
		if (fabs(a[i] - b[i]) > fEpsilon)
			return false;
	}
	return true;
}


/*====================
  M_CompareVec4
  ====================*/
bool	M_CompareVec4(const vec4_t a, const vec4_t b, float fEpsilon)
{
	for (int i(0); i < 4; ++i)
	{
		if (fabs(a[i] - b[i]) > fEpsilon)
			return false;
	}
	return true;
}


/*====================
  M_PointOnLine
  ====================*/
void	M_PointOnLine(const vec3_t origin, const vec3_t dir, float time, vec3_t out)
{
	out[0] = origin[0] + dir[0] * time;
	out[1] = origin[1] + dir[1] * time;
	out[2] = origin[2] + dir[2] * time;
}


/*====================
  M_LerpQuat

  From "Slerping Clock Cycles"
  ====================*/
void	M_LerpQuat(float fLerp, const vec4_t from, const vec4_t to, vec4_t result)
{
	float fScale0, fScale1;

	// calc cosine
	float fCosOmega(from[X] * to[X] + from[Y] * to[Y] + from[Z] * to[Z] + from[W] * to[W]);

	float fAbsCosOmega(fabs(fCosOmega));
	
	if ((1.0f - fAbsCosOmega) > QUAT_EPSILON)
	{
		float fSinSqr(1.0f - fAbsCosOmega * fAbsCosOmega);
		
		float fSinOmega(1.0f / sqrt(fSinSqr));
		float fOmega(atan2(fSinSqr * fSinOmega, fAbsCosOmega));

		fScale0 = sin((1.0f - fLerp) * fOmega) * fSinOmega;
		fScale1 = sin(fLerp * fOmega) * fSinOmega;
	}
	else
	{
		fScale0 = 1.0f - fLerp;
		fScale1 = fLerp;
	}
	
	// Adjust sign
	fScale1 = (fCosOmega >= 0.0f) ? fScale1 : -fScale1;
	
	result[X] = fScale0 * from[X] + fScale1 * to[X];
	result[Y] = fScale0 * from[Y] + fScale1 * to[Y];
	result[Z] = fScale0 * from[Z] + fScale1 * to[Z];
	result[W] = fScale0 * from[W] + fScale1 * to[W];
}

CVec4f	M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To)
{
	float fScale0, fScale1;

	// calc cosine
	float fCosOmega(v4From.x * v4To.x + v4From.y * v4To.y + v4From.z * v4To.z + v4From.w * v4To.w);

	float fAbsCosOmega(fabs(fCosOmega));
	
	if ((1.0f - fAbsCosOmega) > QUAT_EPSILON)
	{
		float fSinSqr(1.0f - fAbsCosOmega * fAbsCosOmega);
		float fSinOmega(1.0f / sqrt(fSinSqr));
		float fOmega(atan2(fSinSqr * fSinOmega, fAbsCosOmega));

		fScale0 = sin((1.0f - fLerp) * fOmega ) * fSinOmega;
		fScale1 = sin(fLerp * fOmega) * fSinOmega;
	}
	else
	{
		fScale0 = 1.0f - fLerp;
		fScale1 = fLerp;
	}
	
	// Adjust sign
	fScale1 = (fCosOmega >= 0.0f) ? fScale1 : -fScale1;

	return CVec4f
	(
		fScale0 * v4From.x + fScale1 * v4To.x,
		fScale0 * v4From.y + fScale1 * v4To.y,
		fScale0 * v4From.z + fScale1 * v4To.z,
		fScale0 * v4From.w + fScale1 * v4To.w
	);
}


/*====================
  M_LineBoxIntersect3d
  ====================*/
bool	M_LineBoxIntersect3d(const CVec3f &vecStart, const CVec3f &vecEnd, const CVec3f &vecBMin, const CVec3f &vecBMax, float &fFraction)
{
	CVec3f v;
	CVec3f t0(0.0f, 0.0f, 0.0f);
	CVec3f t1(1.0f, 1.0f, 1.0f);
	float enter, exit;
	int i;

	if (vecStart.InBounds(vecBMin, vecBMax))
	{
		fFraction = 0.0f;
		return true;
	}

	v = vecEnd - vecStart;

	if (v == V_ZERO)
	{
		//line is a point outside the box
		return false;
	}

	//get first times of overlapping axes	
	for (i = 0; i < 3; ++i)
	{
		bool cont = false;
		
		if (v[i] == 0.0f)
		{
			if (vecStart[i] > vecBMax[i] || vecStart[i] < vecBMin[i])
				return false;		//this axis is outside the box
			else
				continue;
		}

		if (v[i] < 0.0f)
		{
			t0[i] = (vecBMax[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMin[i]-vecStart[i]) / v[i];
			cont = true;
		}
		else
		{
			t0[i] = (vecBMin[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMax[i]-vecStart[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	enter = MAX(t0.x, MAX(t0.y, t0.z));
	exit = MIN(t1.x, MIN(t1.y, t1.z));

	if (enter <= exit && enter < 1)
	{
		fFraction = enter;
		return true;
	}

	return false;
}

bool	M_LineBoxIntersect3d(const CVec3f &vecStart, const CVec3f &vecEnd, const CBBoxf &bbBounds, float &fFraction)
{
	CVec3f v;
	CVec3f t0(0.0f, 0.0f, 0.0f);
	CVec3f t1(1.0f, 1.0f, 1.0f);
	float enter, exit;
	int i;

	const CVec3f &vecBMin(bbBounds.GetMin());
	const CVec3f &vecBMax(bbBounds.GetMax());

	if (vecStart.InBounds(vecBMin, vecBMax))
	{
		fFraction = 0.0f;
		return true;
	}

	v = vecEnd - vecStart;

	if (v == V_ZERO)
	{
		//line is a point outside the box
		return false;
	}

	//get first times of overlapping axes	
	for (i = 0; i < 3; ++i)
	{
		bool cont = false;
		
		if (v[i] == 0.0f)
		{
			if (vecStart[i] > vecBMax[i] || vecStart[i] < vecBMin[i])
				return false;		//this axis is outside the box
			else
				continue;
		}

		if (v[i] < 0.0f)
		{
			t0[i] = (vecBMax[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMin[i]-vecStart[i]) / v[i];
			cont = true;
		}
		else
		{
			t0[i] = (vecBMin[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMax[i]-vecStart[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	enter = MAX(t0.x, MAX(t0.y, t0.z));
	exit = MIN(t1.x, MIN(t1.y, t1.z));

	if (enter <= exit && enter < 1)
	{
		fFraction = enter;
		return true;
	}

	return false;
}


/*====================
  M_PointInBounds2d
  ====================*/
bool	M_PointInBounds2d(const vec2_t p, const vec2_t bMin, const vec2_t bMax)
{
	for (int i(0); i < 2; ++i)
	{
		if ((p[i] < bMin[i]) || (p[i] > bMax[i]))		
			return false;		
	}

	return true;
}


/*====================
  M_ClampLerp
  ====================*/
float	M_ClampLerp(float amount, float low, float high)
{
	amount = CLAMP(amount, 0.0f, 1.0f);
	return LERP(amount, low, high);
}


/*====================
  M_LerpAngle

  lerp across the smallest arc
  ====================*/
float	M_LerpAngle(float a, float low, float high) 
{
	float ret;

	if (high - low > 180.0f)
	{
		low += 360.0f;
	}
	if (high - low < -180.0f)
	{
		high += 360.0f;
	}

	ret = LERP(a, low, high);
	if (ret < 0.0f)
		ret += 360.0f;
	if (ret > 360.0f)
		ret -= 360.0f;

	return ret;
}


/*====================
  M_LerpAngles
  ====================*/
CVec3f	M_LerpAngles(float a, const CVec3f &v3Low, const CVec3f &v3High)
{
	return CVec3f
	(
		M_LerpAngle(a, v3Low.x, v3High.x),
		M_LerpAngle(a, v3Low.y, v3High.y),
		M_LerpAngle(a, v3Low.z, v3High.z)
	);
}


/*====================
  M_ChangeAngle
  ====================*/
float	M_ChangeAngle(float fAngleStep, float fStartAngle, float fEndAngle)
{
	float fResult(fEndAngle);
	
	if (fStartAngle - fEndAngle > 180.0f)
		fEndAngle += 360.0f;
	if (fStartAngle - fEndAngle < -180.0f)
		fStartAngle += 360.0f;

	if (fStartAngle > fEndAngle)
	{
		if (fStartAngle - fEndAngle <= fAngleStep)
			fResult = fEndAngle;
		else
			fResult = fStartAngle - fAngleStep;
	}
	else if (fEndAngle > fStartAngle)
	{
		if (fEndAngle - fStartAngle <= fAngleStep)
			fResult = fEndAngle;
		else
			fResult = fStartAngle + fAngleStep;
	}

	if (fResult >= 180.0f)
		fResult -= 360.0f;

	return fResult;
}


/*====================
  M_DiffAngle
  ====================*/
float	M_DiffAngle(float fAngle1, float fAngle2)
{
	if (fAngle1 - fAngle2 > 180.f)
		fAngle2 += 360.f;
	if (fAngle1 - fAngle2 < -180.f)
		fAngle1 += 360.f;

	if (fAngle1 > fAngle2)
		return fAngle1 - fAngle2;
	else
		return fAngle2 - fAngle1;
}


/*====================
  M_YawToPosition
  ====================*/
float	M_YawToPosition(const CVec2f &v2Pos0, const CVec2f &v2Pos1)
{
	return M_GetYawFromForwardVec2(v2Pos1 - v2Pos0);
}


/*====================
  M_YawToPosition
  ====================*/
float	M_YawToPosition(const CVec3f &v3Pos0, const CVec3f &v3Pos1)
{
	return M_GetYawFromForwardVec2(CVec2f(v3Pos1.x - v3Pos0.x, v3Pos1.y - v3Pos0.y));
}


/*====================
  M_SlerpDirection
  ====================*/
CVec3f	M_SlerpDirection(float fLerp, const CVec3f &v3A, const CVec3f &v3B)
{
	float fAngle(acos(DotProduct(v3A, v3B)));

	if (fAngle > 0.001f)
		return CVec3f(v3A * ((sin((1.0f - fLerp) * fAngle)) / sin(fAngle)) + v3B * ((sin(fLerp * fAngle)) / sin(fAngle)));
	else
		return v3A;
}


/*====================
  M_ArcLengthFactor

  returns the scale between the length of a chord at
  angle1 and angle2 and the arc-length
  ====================*/
float	M_ArcLengthFactor(float fAngle1, float fAngle2)
{
	float fCoordLength(sqrt(cos(fAngle1) * cos(fAngle1) + sin(fAngle2) * sin(fAngle2)));
	float fArcLength(2.0f * M_PI * fabs(fAngle1 - fAngle2));

	return fArcLength / fCoordLength;
}


/*====================
  M_RandomDirection
  ====================*/
CVec3f	M_RandomDirection()
{
	// Generate a random point in the unit square
	CVec2f	r(M_Randnum(0.0f, 1.0f), M_Randnum(0.0f, 1.0f));

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)), 2.0f * M_PI * r.y);

	float sx, cx, sy, cy;

	M_SinCos(s.x, sx, cx);
	M_SinCos(s.y, sy, cy);

	// Convert to directional vector
	return CVec3f(cy * sx, sy * sx, cx);
}

CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle)
{
	// Generate a random point in the unit square
	CVec2f	r(M_Randnum(0.0f, 1.0f), M_Randnum(0.0f, 1.0f));

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)) * (fAngle/180.0f), 2.0f * M_PI * r.y);

	float sx, cx, sy, cy;

	M_SinCos(s.x, sx, cx);
	M_SinCos(s.y, sy, cy);

	// Convert to directional vector
	CVec3f	v3Ret(cy * sx, sy * sx, cx);

	// Rotate toward v3Dir
	CAxis	aAxis(GetAxisFromUpVec(v3Dir));
	
	return TransformPoint(v3Ret, aAxis);
}

CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle0, float fAngle1)
{
	// Generate a random point in the unit square
	CVec2f	r(M_Randnum(0.0f, 1.0f), M_Randnum(0.0f, 1.0f));

	float fA0(fAngle0/180.0f), fA1(fAngle1/180.0f);

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)) * (fA1 - fA0) + DEG2RAD(fAngle0), 2.0f * M_PI * r.y);

	float sx, cx, sy, cy;

	M_SinCos(s.x, sx, cx);
	M_SinCos(s.y, sy, cy);

	// Convert to directional vector
	CVec3f	v3Ret(cy * sx, sy * sx, cx);

	// Rotate toward v3Dir
	CAxis	aAxis(GetAxisFromUpVec(v3Dir));
	
	return TransformPoint(v3Ret, aAxis);
}


/*====================
  M_RandomPointInSphere
  ====================*/
CVec3f	M_RandomPointInSphere()
{
	for (;;)
	{
		CVec3f p(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

		if (p.LengthSq() <= 1.0f)
			return p;
	}
}


/*====================
  M_RandomPointInCircle
  ====================*/
CVec2f	M_RandomPointInCircle()
{
	for (;;)
	{
		CVec2f p(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

		if (p.LengthSq() <= 1.0f)
			return p;
	}
}


/*====================
  M_AreaOfTriangle

  The area of a triangle in 3-space is one half the magnitude of the
  cross-product of two sides of the triangle.
  ====================*/
float	M_AreaOfTriangle(const CVec3f &v3A, const CVec3f &v3B, const CVec3f &v3C)
{
	return Length(CrossProduct(v3B - v3A, v3C - v3A)) / 2.0f;
}


const int NUM_BLUE_NOISE(8192);

SVec2f	s_av2BlueNoise[NUM_BLUE_NOISE] = 
{
	#include "blue_noise.h"
};


/*====================
  M_BlueNoise
  ====================*/
uint	M_BlueNoise(const CRectf &rec, int iDepth, vector<CVec2f> &v2Points)
{
	iDepth = MIN(iDepth, NUM_BLUE_NOISE);

	for (int i(0); i < iDepth; ++i)
	{
		if (rec.AltContains(CVec2f(s_av2BlueNoise[i])))
		{
			const CVec2f &v2A(s_av2BlueNoise[i]);

			v2Points.push_back(CVec2f(ILERP(v2A.x, rec.left, rec.right), ILERP(v2A.y, rec.top, rec.bottom)));
		}
	}

	return uint(v2Points.size());
}


/*====================
  M_GammaDistribution
  ====================*/
float	M_GammaDistribution(int x, int k, float fTheta)
{
	float fBetaX((1.0f / fTheta) * x);

	float fReturn(0.0f);
	LONGLONG llNFact(1);
	for (int n(0); n < k; ++n)
	{
		fReturn += pow(M_E, -fBetaX) * (pow(fBetaX, n) / llNFact);
		llNFact *= (n + 1);
	}

	return 1.0f - fReturn;
}


/*====================
  M_CalcAxisExtents
  ====================*/
void	M_CalcAxisExtents(const CVec3f &v3Axis, const vector<CVec3f> &v3Points, float &fMin, float &fMax)
{
	fMin = FAR_AWAY;
	fMax = -FAR_AWAY;

	for (vector<CVec3f>::const_iterator it(v3Points.begin()); it != v3Points.end(); ++it)
	{
		float fDist(DotProduct(v3Axis, *it));

		if (fDist < fMin)
			fMin = fDist;
		if (fDist > fMax)
			fMax = fDist;
	}
}


/*====================
  M_CalcAxisExtents
  ====================*/
void	M_CalcAxisExtents(const CVec3f &v3Axis, const CBBoxf &bbBox, float &fMin, float &fMax)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Neg, v3Pos; // negative and positive vertices
	
	if (v3Axis.x > 0)
	{
		v3Pos.x = v3Max.x;
		v3Neg.x = v3Min.x;
	}
	else
	{
		v3Pos.x = v3Min.x;
		v3Neg.x = v3Max.x;
	}

	if (v3Axis.y > 0)
	{
		v3Pos.y = v3Max.y;
		v3Neg.y = v3Min.y;
	}
	else
	{
		v3Pos.y = v3Min.y;
		v3Neg.y = v3Max.y;
	}

	if (v3Axis.z > 0)
	{
		v3Pos.z = v3Max.z;
		v3Neg.z = v3Min.z;
	}
	else
	{
		v3Pos.z = v3Min.z;
		v3Neg.z = v3Max.z;
	}

	fMin = DotProduct(v3Axis, v3Neg);
	fMax = DotProduct(v3Axis, v3Pos);
}


/*====================
  M_LinePointDistance

  The distance between a line and a point is the cross product of the
  difference between the origin of a ray on the line and the point and the direction
  of that ray.
  ====================*/
float	M_LinePointDistance(const CVec3f &v3Origin, const CVec3f &v3Dir, const CVec3f &v3Point)
{
	return Length(CrossProduct(v3Point - v3Origin, v3Dir));
}


/*====================
  M_ClosestPointToSegment2d
  ====================*/
CVec2f	M_ClosestPointToSegment2d(const CVec2f &v2A, const CVec2f &v2B, const CVec2f &v2Test)
{
	CVec2f v2Segment(v2B - v2A);
	CVec2f v2Dir(v2Test - v2A);
    float fDot(DotProduct(v2Dir, v2Segment));

	// "Behind" point A
    if (fDot <= 0.0f)
		return v2A;

	// "Ahead of" point B
	float fLengthSq(v2Segment.LengthSq());
    if (fDot >= fLengthSq)
		return v2B;

	// On the segment
	return v2A + (v2Segment * (fDot / fLengthSq));
}


/*====================
  M_RayIntersectsLineSeg2d
  ====================*/
bool	M_RayIntersectsLineSeg2d(const vec2_t src, const vec2_t dir, const vec2_t line_src, const vec2_t line_dest, float epsilon)
{
	const float distance = dir[0]*src[1] - dir[1]*src[0];

	const float distance_src = distance + dir[1]*line_src[0] - dir[0]*line_src[1];
	const float distance_dest   = distance + dir[1]*line_dest[0] - dir[0]*line_dest[1];

	return   ( distance_src > -epsilon && distance_dest <  epsilon )
		  || ( distance_src <  epsilon && distance_dest > -epsilon );
}


/*====================
  M_2dBoundsIntersect
  ====================*/
bool	M_2dBoundsIntersect(const vec2_t bmin_a, const vec2_t bmax_a, const vec2_t bmin_b, const vec2_t bmax_b)
{
	int n;

	for (n=0; n<2; n++)
	{
		if (bmin_a[n] > bmax_b[n] || bmin_b[n] > bmax_a[n])
			return false;
	}
	
	return true;
}


/*====================
  M_RotatePointAroundAxis
  ====================*/
CVec3f	M_RotatePointAroundAxis(const CVec3f &v3In, const CVec3f &v3Axis, float fAngle)
{
	CVec3f v3Fulcrum(NormalProject(v3Axis, v3In));

	CVec3f p0(v3In - v3Fulcrum);
	CVec3f p1(CrossProduct(v3Axis, p0));
 
	float fSin, fCos;
	M_SinCos(DEG2RAD(fAngle), fSin, fCos);

	return v3Fulcrum + p0 * fCos + p1 * fSin;
}


/*====================
  M_RotatePointAroundLine
  ====================*/
CVec3f	M_RotatePointAroundLine(const CVec3f &v3In, const CVec3f &v3Start, const CVec3f &v3End, float fAngle)
{
	CVec3f v3Axis(Normalize(v3End - v3Start));

	CVec3f v3Fulcrum(NormalProject(v3Axis, v3In - v3Start));

	CVec3f p0(v3In - v3Fulcrum - v3Start);
	CVec3f p1(CrossProduct(v3Axis, p0));

	float fSin, fCos;
	M_SinCos(DEG2RAD(fAngle), fSin, fCos);

	return v3Start + v3Fulcrum + p0 * fCos + p1 * fSin;
}


/*====================
  M_ClipLine

  Clip everything on the positive side of the plane
  ====================*/
bool	M_ClipLine(const CPlane &plPlane, CVec3f &p1, CVec3f &p2)
{
	if (plPlane.Distance(p1) > 0.0f)
	{
		if (plPlane.Distance(p2) > 0.0f)
		{
			// Both positive
			return false;
		}
		else
		{
			// Crossing positive into negative
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(p1, p2, plPlane, fFraction))
			{
				p1 = LERP(fFraction, p1, p2);
				return true;
			}

			return false;
		}
	}
	else
	{
		if (plPlane.Distance(p2) > 0.0f)
		{
			// Crossing negative into positive
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(p1, p2, plPlane, fFraction))
			{
				p2 = LERP(fFraction, p1, p2);
				return true;
			}

			return false;
		}
		else
		{
			// Both negative
			return true;
		}
	}
}


#ifdef _WIN32
/*====================
  M_GetCRC32
  ====================*/
uint	M_GetCRC32(const byte *pBuffer, uint uiSize)
{
	return ~crc32(0, pBuffer, uiSize);
}
#else
/*====================
  M_GetCRC32
  ====================*/
uint	M_GetCRC32(const byte *pBuffer, uint uiSize)
{
	return crc32(0, pBuffer, uiSize);
}
#endif


/*====================
  M_GetAngle8
  ====================*/
byte	M_GetAngle8(float fAngle)
{
	while (fAngle < 0.0f) fAngle += 360.0f;
	
	return byte(INT_ROUND(fAngle / 360.0f * 255.0f));
}


/*====================
  M_GetAngle
  ====================*/
float	M_GetAngle(byte yAngle8)
{
	return yAngle8 / 255.0f * 360.0f;
}


#if 0

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */


/*====================
  M_SeedRandom
  ====================*/
void	M_SeedRandom(unsigned long uiSeed)
{
    for (int i(0); i < N; ++i)
	{
         mt[i] = uiSeed & 0xffff0000;
         uiSeed = 69069 * uiSeed + 1;
         mt[i] |= (uiSeed & 0xffff0000) >> 16;
         uiSeed = 69069 * uiSeed + 1;
    }
    mti = N;
}


/*====================
  genrand

  Mersenne Twister
  ====================*/
static
uint	genrand()
{
    uint y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
            M_SeedRandom(uint(rand())); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y;
}


/*====================
  M_Random
  ====================*/
int		M_Random(int i)
{
	if (i <= 1)
		return 0;
	else
		return genrand() % i;
}


/*====================
  M_Random
  ====================*/
int		M_Random(int iLow, int iHigh)
{
	if (iHigh <= iLow)
	{
		return iLow;
	}
	else
	{
		uint uiRange(iHigh - iLow + 1);

		uint uiRand(genrand());
		int iOffset(uiRand % uiRange);

		return (iLow + iOffset);
	}
}


/*====================
  M_Random
  ====================*/
uint	M_Random(uint uiLow, uint uiHigh)
{
	if (uiHigh <= uiLow)
	{
		return uiLow;
	}
	else
	{
		uint uiRange(uiHigh - uiLow + 1);

		uint uiRand(genrand());
		uint uiOffset(uiRand % uiRange);

		return (uiLow + uiOffset);
	}
}


/*====================
  M_Random
  ====================*/
float	M_Random(float fLow, float fHigh)
{
	float fRange(fHigh - fLow);

	if (fRange <= 0.0f)
	{
		return fLow;
	}
	else
	{
		uint uiRand(genrand());
		float fOffset(static_cast<float>(uiRand) * 2.3283064370807974e-10f);

		return (fLow + fOffset * fRange);
	}
}


/*====================
  M_Random
  ====================*/
float	M_Random()
{
	unsigned int uiRand(genrand());
	return (static_cast<float>(uiRand) * 2.3283064370807974e-10f);
}
#endif


/*====================
  M_Init
  ====================*/
void    M_Init(LONGLONG rngseed)
{
	M_InitNoise();
	M_InitSimpleNoise2();

	K2_SRAND(rngseed);

#if 0
	M_SeedRandom(uint(rand()));
#endif
}
