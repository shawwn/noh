// (C)2007 S2 Games
// c_accountmanager.h
//
//=============================================================================
#ifndef __C_ACCOUNTMANAGER_H__
#define __C_ACCOUNTMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_uitrigger.h"
#include "c_cmd.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define AccountManager (*CAccountManager::GetInstance())
//=============================================================================

//=============================================================================
// CAccountManager
//=============================================================================
class CAccountManager
{
    SINGLETON_DEF(CAccountManager)

private:
    CHTTPRequest*   m_pRequest;

    tstring     m_sFirst;
    tstring     m_sLast;
    tstring     m_sShipName;
    tstring     m_sEmail;
    tstring     m_sEmailConfirm;
    tstring     m_sNick;
    tstring     m_sPass;
    tstring     m_sPassConfirm;
    tstring     m_sAddress;
    tstring     m_sCity;
    tstring     m_sState;
    tstring     m_sProvince;
    tstring     m_sCountry;
    tstring     m_sBirthMonth;
    tstring     m_sBirthDay;
    tstring     m_sBirthYear;
    tstring     m_sPostal;
    tstring     m_sPhone;
    tstring     m_sReferrer;

    tstring     m_sCardName;
    tstring     m_sCardPostal;
    tstring     m_sCardVerification;
    tstring     m_sCardNumber;
    tstring     m_sCardMonth;
    tstring     m_sCardYear;

    int         m_iAccountID;

public:
    ~CAccountManager();

    void            CreateAccount();
    void            MakePayment(uint uiPackage);
    void            ProcessCreateResponse(const tstring &sResponse);
    void            ProcessPaymentResponse(const tstring &sResponse);

    void            SetShipName(const tstring &sData)       { m_sShipName = sData; }
    void            SetFirstName(const tstring &sData)      { m_sFirst = sData; }
    void            SetLastName(const tstring &sData)       { m_sLast = sData; }
    void            SetEmail(const tstring &sData)  { m_sEmail = sData; }
    void            SetEmailConfirm(const tstring &sData)   { m_sEmailConfirm = sData; }
    void            SetNick(const tstring &sData)       { m_sNick = sData; }
    void            SetPass(const tstring &sData)       { m_sPass = sData; }
    void            SetPassConfirm(const tstring &sData){ m_sPassConfirm = sData; }
    void            SetAddress(const tstring &sData)    { m_sAddress = sData; }
    void            SetCity(const tstring &sData)       { m_sCity = sData; }
    void            SetProvince(const tstring &sData)   { m_sProvince = sData; }
    void            SetState(const tstring &sData)      { m_sState = sData; }
    void            SetCountry(const tstring &sData)    { m_sCountry = sData; }
    void            SetBirthMonth(const tstring &sData) { m_sBirthMonth = sData; }
    void            SetBirthDay(const tstring &sData)   { m_sBirthDay = sData; }
    void            SetBirthYear(const tstring &sData)  { m_sBirthYear = sData; }
    void            SetPostal(const tstring &sData)     { m_sPostal = sData; }
    void            SetPhone(const tstring &sData)      { m_sPhone = sData; }
    void            SetReferrer(const tstring &sData)   { m_sReferrer = sData; }

    void            SetCardName(const tstring &sData)   { m_sCardName = sData; }
    void            SetCardVerification(const tstring &sData)   { m_sCardVerification = sData; }
    void            SetCardPostal(const tstring &sData) { m_sCardPostal = sData; }
    void            SetCardNumber(const tstring &sData) { m_sCardNumber = sData; }
    void            SetCardMonth(const tstring &sData)  { m_sCardMonth = sData; }
    void            SetCardYear(const tstring &sData)   { m_sCardYear = sData; }

    void            ClearAccountID()                    { m_iAccountID = -1; }
};
//=============================================================================

#endif //__C_CLIENTLOGIN_H__
