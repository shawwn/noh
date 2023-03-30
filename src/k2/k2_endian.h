// (C)2005 S2 Games
// k2_endian.h
//
//=============================================================================
#ifndef __K2_ENDIAN_H__
#define __K2_ENDIAN_H__

/*====================
  SwapIntEndian
 ====================*/
inline int SwapIntEndian(int in)
{
    return ((in >> 24) & 255) +
           (((in >> 16) & 255) << 8) +
           (((in >> 8) & 255) << 16) +
           ((in & 255) << 24);
}


/*====================
  SwapFloatEndian
 ====================*/
inline float SwapFloatEndian(float in)
{
    union
    {
        float f;
        byte b[4];
    }
    u1, u2;

    u1.f = in;
    u2.b[0] = u1.b[3];
    u2.b[1] = u1.b[2];
    u2.b[2] = u1.b[1];
    u2.b[3] = u1.b[0];
    return u2.f;
}


/*====================
  SwapFloatEndian
 ====================*/
inline double SwapDoubleEndian(double in)
{
    union
    {
        double d;
        byte b[8];
    }
    u1, u2;

    u1.d = in;
    u2.b[0] = u1.b[7];
    u2.b[1] = u1.b[6];
    u2.b[2] = u1.b[5];
    u2.b[3] = u1.b[4];
    u2.b[4] = u1.b[3];
    u2.b[5] = u1.b[2];
    u2.b[6] = u1.b[1];
    u2.b[7] = u1.b[0];
    return u2.d;
}





/*====================
  SwapShortEndian
 ====================*/
inline int SwapShortEndian(int in)
{
    return ((in >> 8) & 255) + ((in & 255) << 8);
}


/*====================
  SwapInt64Endian
 ====================*/
inline LONGLONG SwapInt64Endian(LONGLONG in)
{
    return ((in >> 56) & 255) +
           (((in >> 48) & 255) << 8) +
           (((in >> 40) & 255) << 16) +
           (((in >> 32) & 255) << 24) +
           (((in >> 24) & 255) << 32) +
           (((in >> 16) & 255) << 40) +
           (((in >> 8) & 255) << 48) +
           ((in & 255) << 56);
}


//=============================================================================
#if BYTE_ORDER == LITTLE_ENDIAN
inline int LittleInt(int in)                { return in; }
inline short LittleShort(short in)          { return in; }
inline float LittleFloat(float in)          { return in; }
inline double LittleDouble(double in)       { return in; }
inline LONGLONG LittleInt64(LONGLONG in)    { return in; }
#define ToLittle(x) // no op
#else //BYTE_ORDER != LITTLE_ENDIAN
inline int  LittleInt(int in)       { return SwapIntEndian(in); }
inline short LittleShort(short in)  { return SwapShortEndian(in); }
inline float LittleFloat(float in)  { return SwapFloatEndian(in); }
inline double LittleDouble(double in)       { return SwapDoubleEndian(in); }
inline LONGLONG LittleInt64(LONGLONG in)    { return SwapInt64Endian(in); }
inline void ToLittle(int &i) { i = SwapIntEndian(i); }
inline void ToLittle(uint &ui) { ui = SwapIntEndian(ui); }
inline void ToLittle(short &n) { n = SwapShortEndian(n); }
inline void ToLittle(float &f) { f = SwapFloatEndian(f); }
inline void ToLittle(double &d) { d = SwapDoubleEndian(d); }
inline void ToLittle(LONGLONG &ll) { ll = SwapInt64Endian(ll); } 
#endif //BYTE_ORDER
//=============================================================================
#endif //__K2_ENDIAN_H__
