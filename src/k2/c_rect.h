// (C)2005 S2 Games
// c_rect.h
//
//=============================================================================
#ifndef __C_RECT_H__
#define __C_RECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec2.h"
//=============================================================================

template <class T> class CRect;

typedef CRect<float> CRectf;
typedef CRect<double> CRectd;

typedef CRect<int> CRecti;

typedef CRect<uint> CRectui;

enum ERectCoontact
{
    RECT_NONE = 0,  // No overlap
    RECT_INTERSECT, // Rectangles touch
    RECT_CONTAINED, // This rectangle is completely inside the other
    RECT_CONTAINS   // Other rectangle is completely inside this one
};

//=============================================================================
// CRect
//=============================================================================
template <class T>
class CRect
{
public:
    T   left, top, right, bottom;

    // Ctors / Dtor
    ~CRect() {}

    CRect() : left(0), top(0), right(0), bottom(0) {}

    CRect(T _Left, T _Top, T _Right, T _Bottom) :
    left(_Left), top(_Top), right(_Right), bottom(_Bottom)
    {
    }

    CRect(CVec2<T> bmin, CVec2<T> bmax) :
    left(bmin.x), top(bmin.y), right(bmax.x), bottom(bmax.y)
    {
    }

    // Accessors
    T       GetWidth() const        { return right - left; }
    T       GetHeight() const       { return bottom - top; }
    T       GetArea() const         { return (right - left) * (bottom - top); }
    bool    IsNormalized() const    { return right > left && bottom > top; }

    // Conversions
    operator CRecti()   { return CRecti(int(left), int(top), int(right), int(bottom)); }

    operator CRectui()  { return CRectui(uint(left), uint(top), uint(right), uint(bottom)); }

    operator CRectf()   { return CRectf(float(left), float(top), float(right), float(bottom)); }
    operator CRectd()   { return CRectd(double(left), double(top), double(right), double(bottom)); }

    void    Normalize()
    {
        if (left > right)
            SWAP(left, right);
    
        if (top > bottom)
            SWAP(top, bottom);
    }

    // Set
    void    Set(T _Left, T _Top, T _Right, T _Bottom)
    {
        left = _Left;   top = _Top;
        right = _Right; bottom = _Bottom;
    }

    // Crop - constrains this rect to the dimensions of the input rect
    void    Crop(const CRect<T> &recCrop)
    {
        left = (left > recCrop.left) ? left : recCrop.left;
        top = (top > recCrop.top) ? top : recCrop.top;
        right = (right < recCrop.right) ? right : recCrop.right;
        bottom = (bottom < recCrop.bottom) ? bottom : recCrop.bottom;
    }

    void    Crop(T _left, T _top, T _right, T _bottom)
    {
        Crop(CRect<T>(_left, _top, _right, _bottom));
    }

    // Shift - relative positioning
    void    ShiftX(T _x)        { left += _x; right += _x; }
    void    ShiftY(T _y)        { top += _y; bottom += _y; }
    void    Shift(T _x, T _y)   { left += _x; right += _x; top += _y; bottom += _y; }

    // Move - absolute positioning
    void    MoveToX(T _x)       { T width(right - left); left = _x; right = _x + width; }
    void    MoveToY(T _y)       { T height(bottom - top); top = _y; bottom = _y + height; }
    void    MoveTo(T _x, T _y)  { T width(right - left); T height(bottom - top); left = _x; right = _x + width; top = _y; bottom = _y + height; }
    void    MoveTo(const CVec2<T> &vec) { T width(right - left); T height(bottom - top); left = vec.x; right = vec.x + width; top = vec.y; bottom = vec.y + height; }

    // Stretch - relative scaling, top/left is anchor point
    void    StretchX(T _x)      { right += _x; }
    void    StretchY(T _y)      { bottom += _y; }
    void    Stretch(T _x, T _y) { right += _x; bottom += _y; }

    // Size - absolute scaling, top/left is anchor point
    void    SetSizeX(T _x)      { right = left + _x; }
    void    SetSizeY(T _y)      { bottom = top + _y; }
    void    SetSize(T _x, T _y) { right = left + _x; bottom = top + _y; }

    // Center - Align rect's center at this point
    void    Center(T _x, T _y)
    {
        T   width(right - left);
        T   height(bottom - top);

        left = _x - (width / 2);
        right = left + width;
        top = _y - (height / 2);
        bottom = top + height;
    }

    // Assignment
    CRect<T>&   operator=(const CRect<T> &rec)
    {
        left = rec.left;
        top = rec.top;
        right = rec.right;
        bottom = rec.bottom;

        return *this;
    }

    CRect<T>    operator-(const CRect<T> &rec)  { return CRect(left - rec.left, top - rec.top, right - rec.right, bottom - rec.bottom); }

    CRect<T>&   operator-=(const CRect<T> &rec)
    {
        left -= rec.left;
        top -= rec.top;
        right -= rec.right;
        bottom -= rec.bottom;

        return *this;
    }

    CRect<T>    operator+(const CRect<T> &rec)  { return CRect(left + rec.left, top + rec.top, right + rec.right, bottom + rec.bottom); }

    CRect<T>&   operator+=(const CRect<T> &rec)
    {
        left += rec.left;
        top += rec.top;
        right += rec.right;
        bottom += rec.bottom;

        return *this;
    }

    CRect<T>&   operator|=(const CRect<T> &rec)
    {
        left = MIN(left, rec.left);
        top = MIN(top, rec.top);
        right = MAX(right, rec.right);
        bottom = MAX(bottom, rec.bottom);

        return *this;
    }

    // Returns a CRect of the intersecting area
    CRect<T>    operator&(const CRect<T> &rec)
    {
        return CRect(MAX(left, rec.left), MAX(top, rec.top),
                MIN(right, rec.right), MIN(bottom, rec.bottom));
    }

    // Returns a CRect that bounds both rectangles
    CRect<T>    operator|(const CRect<T> &rec)
    {
        return CRect(MIN(left, rec.left), MIN(top, rec.top),
                MAX(right, rec.right), MAX(bottom, rec.bottom));
    }

    // Clip - compares two rectangles, returns other rect's relation to this rect
    ERectCoontact   Clip(const CRect<T> &other) const
    {
        // Test for a no hit
        if (left > other.right ||
            right < other.left ||
            top > other.bottom ||
            bottom < other.top)
            return RECT_NONE;

        // Test for containment
        if (left <= other.left &&
            right >= other.right &&
            top <= other.top &&
            bottom >= other.bottom)
            return RECT_CONTAINED;

        if (other.left <= left &&
            other.right >= right &&
            other.top <= top &&
            other.bottom >= bottom)
            return RECT_CONTAINS;

        return RECT_INTERSECT;
    }

    // Point operations
    template <class P>
    bool    Contains(const CVec2<P> &point) const
    {
        if (point.x >= left &&
            point.x <= right &&
            point.y >= top &&
            point.y <= bottom)
            return true;
        else
            return false;
    }

    template <class P>
    bool    AltContains(const CVec2<P> &point) const
    {
        if (point.x >= left &&
            point.x < right &&
            point.y >= top &&
            point.y < bottom)
            return true;
        else
            return false;
    }

    const CVec2<T>& lt() const          { return *reinterpret_cast<const CVec2<T> *>(&left); }
    CVec2<T>&   lt()                    { return *reinterpret_cast<CVec2<T> *>(&left); }

    const CVec2<T>& rb() const          { return *reinterpret_cast<const CVec2<T> *>(&right); }
    CVec2<T>&   rb()                    { return *reinterpret_cast<CVec2<T> *>(&right); }

    CVec2<T>    GetMid() const
    {
        return (lt() + rb()) / 2.0f;
    }

    bool    operator==(const CRect<T> &b) const     { return (left == b.left && top == b.top && right == b.right && bottom == b.bottom); }
    bool    operator!=(const CRect<T> &b) const     { return (left != b.left || top != b.top || right != b.right || bottom != b.bottom); }

    void    AddPoint(const CVec2<T> &v2Point)
    {
        left = MIN(left, v2Point.x);
        top = MIN(top, v2Point.y);

        right = MAX(right, v2Point.x);
        bottom = MAX(bottom, v2Point.y);
    }
};
//=============================================================================

#endif // __C_RECT_H__
