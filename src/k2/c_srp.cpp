// Apr 6 2023
// c_srp.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_srp.h"

#include "srp.h"
#include "md5.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// https://github.com/theli-ua/pyHoNBot/blob/master/hon/masterserver.py#L23
SRP_HashAlgorithm alg     = SRP_SHA256;
SRP_NGType        ng_type = SRP_NG_CUSTOM;
const char* n_hex = "DA950C6C97918CAE89E4F5ECB32461032A217D740064BC12FC0723CD204BD02A7AE29B53F3310C13BA998B7910F8B6A14112CBC67BDD2427EDF494CB8BCA68510C0AAEE5346BD320845981546873069B337C073B9A9369D500873D647D261CCED571826E54C6089E7D5085DC2AF01FD861AE44C8E64BCA3EA4DCE942C5F5B89E5496C2741A9E7E9F509C261D104D11DD4494577038B33016E28D118AE4FD2E85D9C3557A2346FAECED3EDBE0F4D694411686BA6E65FEE43A772DC84D394ADAE5A14AF33817351D29DE074740AA263187AB18E3A25665EACAA8267C16CDE064B1D5AF0588893C89C1556D6AEF644A3BA6BA3F7DEC2F3D6FDC30AE43FBD6D144BB";
const char* g_hex = "2";
//=============================================================================


/*====================
  SHA256String
  ====================*/
tstring SHA256String(const void* data, size_t len)
{
    size_t uiHashLen(srp_hash_length(SRP_SHA256));
    void* pMem = MemManager.Allocate(uiHashLen);
    const byte* pHash(srp_hash(SRP_SHA256, data, len, pMem));
    tstring sReturn(BytesToHexString(pHash, uiHashLen));
    MemManager.Deallocate(pMem);
    return sReturn;
}
tstring SHA256String(const string& sData)
{
    return SHA256String(sData.c_str(), sData.size());
}
tstring SHA256String(const wstring& sData)
{
    return SHA256String(WideToSingle(sData));
}

/*====================
  HashPassword
  ====================*/
tstring HashPassword(const tstring& sPassword, const tstring& sSalt2)
{
    // https://github.com/theli-ua/pyHoNBot/blob/master/hon/masterserver.py#L37
    // usr.password = sha256(md5(md5(password).hexdigest() + salt2 + '[!~esTo0}').hexdigest() + 'taquzaph_?98phab&junaj=z=kuChusu').hexdigest()
    auto sResult = SHA256String(MD5String(MD5String(sPassword) + sSalt2 + _T("[!~esTo0}")) + _T("taquzaph_?98phab&junaj=z=kuChusu"));
    return sResult;
}

/*--------------------
  HashPassword
  --------------------*/
CMD(HashPassword)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: HashPassword <password> <salt2>") << newl;
        return false;
    }

    Console << HashPassword(vArgList[0], vArgList[1]) << newl;
    return true;
}

/*--------------------
  ProcessChallenge
  --------------------*/
CMD(ProcessChallenge)
{
    if (vArgList.size() < 5)
    {
        Console << _T("syntax: ProcessChallenge <username> <password> <salt> <salt2> <B>") << newl;
        return false;
    }

    const auto& sUsername (vArgList[0]);
    const auto& sPassword (vArgList[1]);
    const auto& sSalt (vArgList[2]);
    const auto& sSalt2 (vArgList[3]);
    const auto& sB (vArgList[4]);

    CSRP cSRP;
    tstring sA = cSRP.Start(sUsername, sPassword);
    tstring sM = cSRP.ProcessChallenge(sSalt, sSalt2, sB);
    tstring sKey = cSRP.GetKey();
    Console << _T("username: ") << sUsername << newl;
    Console << _T("salt: ") << sSalt << newl;
    Console << _T("salt2: ") << sSalt2 << newl;
    Console << _T("B: ") << sB << newl;
    Console << _T("M: ") << sM << newl;
    Console << _T("key: ") << sKey << newl;
    Console << _T("A: ") << sA << newl;
    return true;
}



/*====================
  CSRP::CSRP
  ====================*/
CSRP::CSRP()
: m_pUser(nullptr)
, m_pVerifier(nullptr)
{
}


/*====================
  CSRP::~CSRP
  ====================*/
CSRP::~CSRP()
{
    Cleanup();
}


/*====================
  CSRP::Start
  ====================*/
tstring
CSRP::Start(const tstring &sUsername, const tstring &sPassword)
{
    Cleanup();

    m_sUsername = sUsername;
    m_sPassword = sPassword;

//    srp_create_salted_verification_key( alg, ng_type, username,
//                (const unsigned char *)password,
//                strlen(password),
//                &bytes_s, &len_s, &bytes_v, &len_v, n_hex, g_hex );

    m_pUser =  srp_user_new( alg, ng_type,
                             n_hex, g_hex);

    const unsigned char * bytes_A = nullptr;
    size_t len_A   = 0;
    srp_user_start_authentication( m_pUser, &bytes_A, &len_A );

    return BytesToHexString(bytes_A, len_A);
}


/*====================
  CSRP::ProcessChallenge
  ====================*/
tstring
CSRP::ProcessChallenge(const tstring &sSalt, const tstring &sSalt2, const tstring &sB)
{
    if (m_pUser == nullptr)
    {
        Console.Err << _T("CSRP::ProcessChallenge(): m_pUser is nullptr. Did you forget to call Start()?") << newl;
        Cleanup();
        return TSNULL;
    }

    auto sUser(TStringToUTF8(m_sUsername));
    auto sPass(TStringToUTF8(HashPassword(m_sPassword, sSalt2)));
    const unsigned char * bytes_pass    = (const unsigned char *)sPass.c_str();
    size_t len_pass = sPass.size();

    auto sSaltBytes(HexStringToBytes(sSalt));
    const unsigned char * bytes_s    = sSaltBytes.data();
    size_t len_s = sSaltBytes.size();

    auto sBBytes(HexStringToBytes(sB));
    const unsigned char * bytes_B    = sBBytes.data();
    size_t len_B = sBBytes.size();

    const unsigned char * bytes_M    = nullptr;
    size_t len_M = 0;

    /* Host -> User: (bytes_s, bytes_B) */
    srp_user_process_challenge( m_pUser, sUser.c_str(), bytes_pass, len_pass, bytes_s, len_s, bytes_B, len_B, &bytes_M, &len_M );

    if ( !bytes_M )
    {
        Console.Err << _T("CSRP: User SRP-6a safety check violation!") << newl;
        Cleanup();
        return TSNULL;
    }

    return BytesToHexString(bytes_M, len_M);

//    srp_user_get_a(m_pUser, &bytes_A, &len_A);
//
//    srp_create_verification_key_from_salt(alg, ng_type, srp_user_get_username(m_pUser),
//                (const unsigned char *)sPass.c_str(), sPass.size(),
//                bytes_s, len_s,
//                &bytes_v, &len_v);
//
//    srp_verifier_delete( m_pVerifier );
//    m_pVerifier = srp_verifier_new(alg, ng_type,
//                                   srp_user_get_username(m_pUser),
//                                   bytes_s, len_s,
//                                   bytes_v, len_v,
//                                   bytes_A, len_A,
//                                   &bytes_B2, &len_B2);
//
//    srp_verifier_verify_session(m_pVerifier, bytes_M, &bytes_HAMK, &len_HAMK);
//    if ( !srp_verifier_is_authenticated(m_pVerifier))
//    {
//        Console.Err << _T("CSRP: Not authenticated") << newl;
//        return TSNULL;
//    }


//    srp_user_get_proof(m_pUser, &bytes_HAMK, &len_HAMK);
//    return BytesToHexString(bytes_HAMK, len_HAMK);

//    /* User -> Host: (username, bytes_A) */
//    m_pVerifier =  srp_verifier_new( alg, ng_type, username, bytes_s, len_s, bytes_v, len_v,
//                                     bytes_A, len_A, & bytes_B, &len_B, n_hex, g_hex );
//
//    /* User -> Host: (bytes_M) */
//    srp_verifier_verify_session( ver, bytes_M, &bytes_HAMK );

//    /* User -> Host: (bytes_M) */
//    srp_verifier_verify_session( m_pVerifier, bytes_M, &bytes_HAMK, &len_HAMK );
//
//    if ( !bytes_HAMK )
//    {
//        Console.Err << _T("CSRP: User authentication failed!") << newl;
//        Cleanup();
//        return TSNULL;
//    }
//
//    return BytesToHexString(bytes_HAMK, len_HAMK);

#if 0
    /* User -> Host: (username, bytes_A) */
    m_pVerifier =  srp_verifier_new( alg, ng_type, username, bytes_s, len_s, bytes_v, len_v,
                                     bytes_A, len_A, & bytes_B, &len_B, n_hex, g_hex );

    if ( !bytes_B )
    {
        Console.Err << _T("CSRP: Verifier SRP-6a safety check violated!") << newl;
        Cleanup();
        return false;
    }

    /* Host -> User: (bytes_s, bytes_B) */
    srp_user_process_challenge( m_pUser, bytes_s, len_s, bytes_B, len_B, &bytes_M, &len_M );

    if ( !bytes_M )
    {
        Console.Err << _T("CSRP: User SRP-6a safety check violation!") << newl;
        Cleanup();
        return false;
    }

    /* User -> Host: (bytes_M) */
    srp_verifier_verify_session( m_pVerifier, bytes_M, &bytes_HAMK );

    if ( !bytes_HAMK )
    {
        Console.Err << _T("CSRP: User authentication failed!") << newl;
        Cleanup();
        return false;
    }

    /* Host -> User: (HAMK) */
    srp_user_verify_session( m_pUser, bytes_HAMK );

    if ( !srp_user_is_authenticated(m_pUser) )
    {
        printf("Server authentication failed!\n");
    }
#endif
}

/*====================
  CSRP::GetKey
  ====================*/
tstring
CSRP::GetKey()
{
    if (!m_pUser)
        return TSNULL;
    const unsigned char * bytes_a = nullptr;
    size_t len_a  = 0;
    srp_user_get_a(m_pUser, &bytes_a, &len_a);
    if (!bytes_a || !len_a)
        return TSNULL;
    return BytesToHexString(bytes_a, len_a);
}

/*====================
  CSRP::GetUsername
  ====================*/
tstring
CSRP::GetUsername()
{
    return m_sUsername;
}

/*====================
  CSRP::Cleanup
  ====================*/
void
CSRP::Cleanup()
{
    srp_verifier_delete( m_pVerifier );
    m_pVerifier = nullptr;

    srp_user_delete( m_pUser );
    m_pUser = nullptr;
}

