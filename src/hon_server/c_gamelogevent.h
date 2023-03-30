// (C)2008 S2 Games
// c_gamelogevent.h
//
//=============================================================================
#ifndef __C_GAMELOGEVENT_H__
#define __C_GAMELOGEVENT_H__

//=============================================================================
// CGameLogEvent
//=============================================================================
class CGameLogEvent
{
private:
    EGameLogEvent   m_eEvent;
    tsmapts         m_mapProperties;

    CGameLogEvent();

public:
    ~CGameLogEvent()    {}
    CGameLogEvent(const tstring &sLine) :
    m_eEvent(GAME_LOG_INVALID)
    {
        if (sLine.empty())
            return;

        // Get event
        size_t zStart(sLine.find_first_not_of(_T(" \n\r\t")));
        size_t zEnd(sLine.find_first_of(_T(" \n\r\t"), zStart));
        m_eEvent = GetGameLogEventFromString(sLine.substr(zStart, zEnd - zStart));
        if (m_eEvent == GAME_LOG_INVALID)
            return;

        tstring sProperty;
        while (zStart < sLine.length())
        {
            zStart = sLine.find_first_not_of(_T(" \n\r\t"), zEnd);
            if (zStart == tstring::npos)
                break;
            
            zEnd = sLine.find(_T(':'), zStart);
            if (zEnd == tstring::npos)
                break;

            sProperty = sLine.substr(zStart, zEnd - zStart);

            zStart = zEnd + 1;
            if (zStart >= sLine.length())
                break;

            if (sLine[zStart] == _T('\"'))
            {
                zStart += 1;
                zEnd = sLine.find(_T('\"'), zStart);
                if (zEnd == tstring::npos)
                    break;
                m_mapProperties[sProperty] = sLine.substr(zStart, zEnd - zStart);
                zEnd += 2;
                continue;
            }

            zEnd = sLine.find_first_of(_T(" \n\r\t"), zStart);
            if (zEnd != tstring::npos)
            {
                m_mapProperties[sProperty] = sLine.substr(zStart, zEnd - zStart);
                zEnd += 1;
            }
            else
            {
                m_mapProperties[sProperty] = sLine.substr(zStart);
                break;
            }
        }
    }

    EGameLogEvent   GetEventType() const    { return m_eEvent; }

    const tstring&  GetProperty(const tstring &sName, const tstring &sDefault = TSNULL) const
    {
        tsmapts_cit it(m_mapProperties.find(sName));
        if (it == m_mapProperties.end())
            return sDefault;
        return it->second;
    }

    int     GetPropertyInt(const tstring &sName, int iDefault = 0) const
    {
        tsmapts_cit it(m_mapProperties.find(sName));
        if (it == m_mapProperties.end())
            return iDefault;
        return AtoI(it->second);
    }

    float       GetPropertyFloat(const tstring &sName, float fDefault = 0.0f) const
    {
        tsmapts_cit it(m_mapProperties.find(sName));
        if (it == m_mapProperties.end())
            return fDefault;
        return AtoF(it->second);
    }
};
//=============================================================================

#endif //__C_GAMELOGEVENT_H__
