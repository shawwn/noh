// (C)2007 S2 Games
// c_accountmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_accountmanager.h"
#include "c_clientlogin.h"
#include "c_uicmd.h"
#include "c_eventmanager.h"
#include "c_httpmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CAccountManager)

UI_TRIGGER(CreateAccountError);
UI_TRIGGER(CreateAccountSuccess);

UI_TRIGGER(PayAccountError);
UI_TRIGGER(PayAccountSuccess);
//=============================================================================

/*====================
  CAccountManager::CAccountManager
  ====================*/
CAccountManager::CAccountManager() :
m_pRequest(NULL),
m_sShipName(_T("")),
m_sFirst(_T("")),
m_sLast(_T("")),
m_sEmail(_T("")),
m_sEmailConfirm(_T("")),
m_sNick(_T("")),
m_sPass(_T("")),
m_sPassConfirm(_T("")),
m_sAddress(_T("")),
m_sCity(_T("")),
m_sState(_T("")),
m_sCountry(_T("")),
m_sBirthMonth(_T("")),
m_sBirthDay(_T("")),
m_sBirthYear(_T("")),
m_sPostal(_T("")),
m_sPhone(_T("")),
m_sReferrer(_T("")),
m_sCardName(_T("")),
m_sCardVerification(_T("")),
m_sCardNumber(_T("")),
m_sCardMonth(_T("")),
m_sCardYear(_T("")),
m_iAccountID(-1)
{
}


/*====================
  CAccountManager::~CAccountManager
  ====================*/
CAccountManager::~CAccountManager()
{
    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
}


/*====================
  CAccountManager::MakePayment
  ====================*/
void CAccountManager::MakePayment(uint uiPackage)
{
// FIXME: DISABLED
#if 0
    int iAccountID;

    Vid.BeginFrame();
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();

    if (m_iAccountID != -1)
        iAccountID = m_iAccountID;
    else if (ClientLogin.GetAccountID() != -1)
        iAccountID = ClientLogin.GetAccountID();
    else
    {
        PayAccountError.Trigger(_T("You are not logged in to an account."));
        return;
    }
    
    SAFE_DELETE(m_pDBManager);
    m_pDBManager = K2_NEW(ctx_Net,  CHTTPPostRequest);
    m_pDBManager->SetHost(_T("savage2.s2games.com"));
    m_pDBManager->SetTargetURI(_T("/auth_net.php"));

#define CHECK_VAR(x,y) if (x.empty()) { PayAccountError.Trigger(_T("You must enter the ") y _T(" to ship to!")); return; }

    if (uiPackage == 5)
    {
        CHECK_VAR(m_sShipName, _T("name"))
        CHECK_VAR(m_sAddress, _T("address"))
        CHECK_VAR(m_sCity, _T("city"))
        CHECK_VAR(m_sCountry, _T("country"))
        CHECK_VAR(m_sPostal, _T("zip/postal code"))
        CHECK_VAR(m_sState, _T("state"))

        if (m_sState == _T("--"))
            CHECK_VAR(m_sProvince, _T("province"))
    }

#undef CHECK_VAR

    m_pDBManager->AddVariable(_T("ship_name"), m_sShipName);
    m_pDBManager->AddVariable(_T("address"), m_sAddress);
    m_pDBManager->AddVariable(_T("city"), m_sCity);
    m_pDBManager->AddVariable(_T("country"), m_sCountry);
    m_pDBManager->AddVariable(_T("zip"), m_sPostal);

    if (m_sState != _T("--"))
        m_pDBManager->AddVariable(_T("state"), m_sState);
    else
        m_pDBManager->AddVariable(_T("province"), m_sProvince);

    m_pDBManager->AddVariable(_T("c_zip"), m_sCardPostal);
    m_pDBManager->AddVariable(_T("card_name"), m_sCardName);
    m_pDBManager->AddVariable(_T("cvv"), m_sCardVerification);
    m_pDBManager->AddVariable(_T("card_number"), m_sCardNumber);
    m_pDBManager->AddVariable(_T("month"), m_sCardMonth);
    m_pDBManager->AddVariable(_T("year"), m_sCardYear);

    m_pDBManager->AddVariable(_T("s2ref"), _T("client"));

    m_pDBManager->AddVariable(_T("aid"), XtoA(iAccountID));
    m_pDBManager->AddVariable(_T("package"), XtoA(uiPackage));

    m_pDBManager->SendSecureRequest();
    ProcessPaymentResponse(m_pDBManager->GetResponse());
#endif
}


/*====================
  CAccountManager::ProcessPaymentResponse
  ====================*/
void    CAccountManager::ProcessPaymentResponse(const tstring &sResponse)
{
// FIXME: DISABLED
#if 0
    try
    {
        if (pResponse == NULL)
            EX_ERROR(_T("No response recieved, please try again later."));

        if (!pResponse->GetVarString("error").empty())
            EX_ERROR(StringToTString(pResponse->GetVarString("error")));

        if (pResponse->GetVarString("thanks") != "ok")
            EX_ERROR(_T("Could not determine if payment went though, contact technical support."));

        Console << _T("Account Payment - Success! Thank you!") << newl;
        PayAccountSuccess.Trigger(SNULL);

        if (!ClientLogin.Connected())
            ClientLogin.Connect();
    }
    catch (CException &ex)
    {
        ex.Process(_T("Account Payment - "), NO_THROW);
        PayAccountError.Trigger(ex.GetMsg());
    }
#endif
}


/*====================
  CAccountManager::CreateAccount
  ====================*/
void CAccountManager::CreateAccount()
{
#if 0
    SAFE_DELETE(m_pDBManager);
    CHTTPPostRequest httpRequest;
    httpRequest.SetHost(_T("savage2.s2games.com"));
    httpRequest.SetTargetURI(_T("/game_account_create.php"));

    httpRequest.AddVariable(_T("first_name"), m_sFirst);
    httpRequest.AddVariable(_T("last_name"), m_sLast);
    httpRequest.AddVariable(_T("email"), m_sEmail);
    httpRequest.AddVariable(_T("email2"), m_sEmailConfirm);
    httpRequest.AddVariable(_T("nickname"), m_sNick);
    httpRequest.AddVariable(_T("password"), m_sPass);
    httpRequest.AddVariable(_T("password2"), m_sPassConfirm);
    httpRequest.AddVariable(_T("address"), m_sAddress);
    httpRequest.AddVariable(_T("city"), m_sCity);
    httpRequest.AddVariable(_T("country"), m_sCountry);
    httpRequest.AddVariable(_T("birth_month"), m_sBirthMonth);
    httpRequest.AddVariable(_T("birth_day"), m_sBirthDay);
    httpRequest.AddVariable(_T("birth_year"), m_sBirthYear);
    httpRequest.AddVariable(_T("zip"), m_sPostal);
    httpRequest.AddVariable(_T("billing_choice"), _T(""));
    httpRequest.AddVariable(_T("referrer"), m_sReferrer);
    httpRequest.AddVariable(_T("state"), m_sState);

    if (!m_sProvince.empty())
        httpRequest.AddVariable(_T("province"), m_sProvince);

    httpRequest.SendSecureRequest();
    ProcessCreateResponse(httpRequest.GetResponse());
#endif
}


/*====================
  CAccountManager::ProcessCreateResponse
  ====================*/
void CAccountManager::ProcessCreateResponse(const tstring &sResponse)
{
// FIXME: DISABLED
#if 0
    try
    {
        if (pResponse == NULL)
            EX_ERROR(_T("No response received, please try again later."));

        if (!pResponse->GetVarString("error").empty())
            EX_ERROR(StringToTString(pResponse->GetVarString("error")));

#define CheckResponse(x, y) \
if (!pResponse->GetVarString(x).empty()) \
    EX_ERROR(_T(y) _T(" ") + StringToTString(pResponse->GetVarString(x)));

        CheckResponse("first_name", "First name");
        CheckResponse("last_name", "Last name");
        CheckResponse("email", "Email");
        CheckResponse("email2", "Email confirmation");
        CheckResponse("nickname", "Nickname");
        CheckResponse("password", "Password");
        CheckResponse("password2", "Password confirmation");
        CheckResponse("address", "Address");
        CheckResponse("city", "City");
        CheckResponse("state", "State");
        CheckResponse("province", "Province");
        CheckResponse("country", "Country");
        CheckResponse("birth_month", "Birth month");
        CheckResponse("birth_day", "Birth day");
        CheckResponse("birth_year", "Birth year");
        CheckResponse("zip", "Postal code");
        CheckResponse("phone_number", "Phone number");
        CheckResponse("billing_choice", "Payment type");
        CheckResponse("referrer", "Referrer");

#undef CheckResponse

        if (pResponse->GetVarString("account_id").empty())
            EX_ERROR(_T("No account ID returned."));

        m_iAccountID = AtoI(pResponse->GetVarString("account_id"));

        Console << _T("Account Creation - Success! Account ID: ") << pResponse->GetVarString("account_id") << _T(".") << newl;
        CreateAccountSuccess.Trigger(SNULL);
    }
    catch (CException &ex)
    {
        ex.Process(_T("Account Creation - "), NO_THROW);
        CreateAccountError.Trigger(ex.GetMsg());
    }
#endif
}


#if 0
/*--------------------
  CreateAccount
  --------------------*/
UI_VOID_CMD(CreateAccount, 0)
{
    AccountManager.CreateAccount();
}


/*--------------------
  MakePayment
  --------------------*/
UI_VOID_CMD(MakePayment, 1)
{
    AccountManager.MakePayment(AtoI(vArgList[0]->Evaluate()));
}

#define AcctDetailCmd(x) UI_VOID_CMD(AcctSet##x, 1)\
{                                       \
    AccountManager.Set##x(vArgList[0]->Evaluate()); \
}

AcctDetailCmd(CardName);
AcctDetailCmd(ShipName);
AcctDetailCmd(FirstName);
AcctDetailCmd(LastName);
AcctDetailCmd(Email);
AcctDetailCmd(EmailConfirm);
AcctDetailCmd(Nick);
AcctDetailCmd(Pass);
AcctDetailCmd(PassConfirm);
AcctDetailCmd(Address);
AcctDetailCmd(City);
AcctDetailCmd(Province);
AcctDetailCmd(State);
AcctDetailCmd(Country);
AcctDetailCmd(BirthMonth);
AcctDetailCmd(BirthDay);
AcctDetailCmd(BirthYear);
AcctDetailCmd(Postal);
AcctDetailCmd(Phone);
AcctDetailCmd(Referrer);

AcctDetailCmd(CardVerification);
AcctDetailCmd(CardPostal);
AcctDetailCmd(CardNumber);
AcctDetailCmd(CardMonth);
AcctDetailCmd(CardYear);
#endif

#undef AcctDetailCmd

