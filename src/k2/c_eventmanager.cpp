// (C)2007 S2 Games
// c_eventmanager.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_eventmanager.h"
#include "c_vid.h"
#include "c_uimanager.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
ELoadingDisplay IModalDialog::s_eDisplay(LOADING_DISPLAY_NONE);

CUITrigger  IModalDialog::s_triggerTitle(_T("LoadingTitle"));
CUITrigger  IModalDialog::s_triggerProgress(_T("LoadingProgress"));
CUITrigger  IModalDialog::s_triggerLoadingImage(_T("LoadingImage"));
CUITrigger  IModalDialog::s_triggerVisible(_T("LoadingVisible"));
uint        IModalDialog::s_uiLastUpdate(INVALID_TIME);
uint        IModalDialog::s_uiNumLoadingJobs(1);
uint        IModalDialog::s_uiLoadingJob(0);
float       IModalDialog::s_fProgress(0.0f);
tstring     IModalDialog::s_sLoadingInterface;
//=============================================================================

/*--------------------
  HideModalDialog
  --------------------*/
CMD(HideModalDialog)
{
    IModalDialog::Hide();

    return true;
}

UI_VOID_CMD(HideModalDialog, 0)
{
    IModalDialog::Hide();
}
