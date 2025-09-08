#include "precomp_dboclient.h"
#include "AggroListGui.h"

// core
#include "NtlDebug.h"

// simulation layer
#include "NtlSLEvent.h"
#include "NtlSLEventFunc.h"
#include "NtlSLLogic.h"

// presentation
#include "NtlPLGuiManager.h"

// Dbo
#include "DboGlobal.h"
#include "DialogManager.h"
#include "DialogPriority.h"
#include "LobbyManager.h"

#include <sstream>

CAggroListGui::CAggroListGui(const RwChar* pName)
: CNtlPLGui(pName)
{
	m_hTargetId = INVALID_HOBJECT;
	m_hUpdataId = INVALID_HOBJECT;
}

CAggroListGui::~CAggroListGui()
{
}

RwBool CAggroListGui::Create() {

	NTL_FUNCTION("CAggroListGui::Create");

	if (!CNtlPLGui::Create("", "gui\\Aggro.srf", "gui\\AggroStatus.frm"))
		NTL_RETURN(FALSE);

	CNtlPLGui::CreateComponents(CNtlPLGuiManager::GetInstance()->GetGuiManager());

	m_pThis = (gui::CDialog*)GetComponent("dlgAggroStatus"); // 背景框 
	//m_pThis->SetPriority(dDIALOGPRIORITY_FLASH_PROLOG); // 设置成最顶级窗口

	m_pThis->SetPosition(150,150);

	m_pTitle = (gui::CStaticBox*)GetComponent("sttTitle");
	m_pExitButton = (gui::CButton*)GetComponent("btnClose");


	std::ostringstream osStr;
	for (int i = 0; i < NTL_PARTY_MAX_AGGRO; i++) {
		osStr.str("");
		osStr << "pnlAggroBack" << i + 1;
		m_pAggroBack[i] = (gui::CPanel*)GetComponent(osStr.str().c_str());
		osStr.str("");
		osStr << "pgbPlayAggro" << i + 1;
		m_pPlayAggro[i] = (gui::CProgressBar*)GetComponent(osStr.str().c_str());
		m_pPlayAggro[i]->SetPos(0);
		osStr.str("");
		osStr << "sttPlayInfo" << i + 1;
		m_pPlayInfo[i] = (gui::CStaticBox*)GetComponent(osStr.str().c_str());
		osStr.str("");
		osStr << "sttPlayPoint" << i + 1;
		m_pPlayPoint[i] = (gui::CStaticBox*)GetComponent(osStr.str().c_str());
	}
	
	m_pTitle->SetText(GetDisplayStringManager()->GetString("DST_AGGROLIST_TITLE"));
	m_slotCloseButton = m_pExitButton->SigClicked().Connect(this, &CAggroListGui::ClickedCloseButton);

	Show(false);

	LinkMsg(g_EventAggroListNfy, 0);
	LinkMsg(g_EventAggroUpdateNfy, 0);
	LinkMsg(g_EventAggroResetNfy, 0);
	LinkMsg(g_EventSobTargetSelect, 0);
	LinkMsg(g_EventPartyLeave, 0);
	NTL_RETURN(TRUE);
}

VOID CAggroListGui::Destroy()
{
	NTL_FUNCTION("CAggroListGui::Destroy");

	UnLinkMsg(g_EventAggroListNfy);
	UnLinkMsg(g_EventAggroUpdateNfy);
	UnLinkMsg(g_EventAggroResetNfy);
	UnLinkMsg(g_EventSobTargetSelect);
	UnLinkMsg(g_EventPartyLeave);
	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
}



VOID CAggroListGui::HandleEvents(RWS::CMsg& msg)
{
	// A team is required for the list to be displayed
	if (msg.Id == g_EventAggroListNfy) {
		// Replace here with the pet list of the other selected pet
		SDboEventAggroListNfy* pNotify = reinterpret_cast<SDboEventAggroListNfy*>(msg.pData);

		InitList();

		m_hUpdataId = pNotify->hTarget;
		if (m_hTargetId == pNotify->hTarget) 
		{

			if (pNotify->byCount == 0)
			{
				Show(false);
				return;
			}

			// Please select the current script
			for (int n = 0; n < NTL_PARTY_MAX_AGGRO-1;n++) {
				for (int j = 0; j < NTL_PARTY_MAX_AGGRO - n-1; j++) {
					if (pNotify->aAggroInfo[j].dwAggoPoint < pNotify->aAggroInfo[j+1].dwAggoPoint) {
						DWORD tmp = pNotify->aAggroInfo[j + 1].dwAggoPoint;
						pNotify->aAggroInfo[j + 1].dwAggoPoint = pNotify->aAggroInfo[j].dwAggoPoint;
						pNotify->aAggroInfo[j].dwAggoPoint = tmp;

						SERIAL_HANDLE tmp2 = pNotify->aAggroInfo[j + 1].handle;
						pNotify->aAggroInfo[j + 1].handle = pNotify->aAggroInfo[j].handle;
						pNotify->aAggroInfo[j].handle = tmp2;
					}
				}
			}

			std::wostringstream osStr;
			int total = (int)pNotify->dwTotalAggroPoint;

			for (int i = 0; i < NTL_PARTY_MAX_AGGRO; i++) {
				int point = (int)pNotify->aAggroInfo[i].dwAggoPoint;
				CNtlSob* pObj = GetNtlSobManager()->GetSobObject(pNotify->aAggroInfo[i].handle);
				if (pObj)
				{
					if (point > 0) {
						osStr.str(L"");
						osStr << Logic_GetName(pObj);
						m_pPlayInfo[i]->SetText(osStr.str().c_str());

						if (i == 0) {
							m_pPlayAggro[i]->SetPos(1000);

							osStr.str(L"");
							osStr << 100 << L"%";
							m_pPlayPoint[i]->SetText(osStr.str().c_str());
						}
						else {
							m_pPlayAggro[i]->SetPos(point * 1000 / total);

							osStr.str(L"");
							osStr << point * 100 / total << L"%";
							m_pPlayPoint[i]->SetText(osStr.str().c_str());
						}
					}
				}
				
			}
			Show(true);
		}
		else 
		{
			Show(false);
		}
	}
	else if (msg.Id == g_EventAggroUpdateNfy) {
		// Update the attribute list of the currently selected pet
		SDboEventAggroUpdateNfy* pNotify = reinterpret_cast<SDboEventAggroUpdateNfy*>(msg.pData);

		InitList();// Initialize Pet List

		m_hUpdataId = pNotify->hTarget; // Currently Updated Pet ID

		if (m_hTargetId == pNotify->hTarget) { // If the selected pet is the updated pet


			if (pNotify->byCount == 0)
			{
				Show(false);
				return;
			}

			// Please select the current script
			for (int n = 0; n < NTL_PARTY_MAX_AGGRO-1; n++) {
				for (int j = 0; j < NTL_PARTY_MAX_AGGRO - n-1; j++) {
					if (pNotify->aAggroInfo[j].dwAggoPoint < pNotify->aAggroInfo[j + 1].dwAggoPoint) {
						DWORD tmp = pNotify->aAggroInfo[j + 1].dwAggoPoint;
						pNotify->aAggroInfo[j + 1].dwAggoPoint = pNotify->aAggroInfo[j].dwAggoPoint;
						pNotify->aAggroInfo[j].dwAggoPoint = tmp;

						SERIAL_HANDLE tmp2 = pNotify->aAggroInfo[j + 1].handle;
						pNotify->aAggroInfo[j + 1].handle = pNotify->aAggroInfo[j].handle;
						pNotify->aAggroInfo[j].handle = tmp2;
					}
				}
			}

			std::wostringstream osStr;
			int total = (int)pNotify->dwTotalAggroPoint;
			for (int i = 0; i < NTL_PARTY_MAX_AGGRO; i++) {
				m_pAggroBack[i]->Show(true);
				int point = (int)pNotify->aAggroInfo[i].dwAggoPoint;

				CNtlSob* pObj = GetNtlSobManager()->GetSobObject(pNotify->aAggroInfo[i].handle);
				if (pObj)
				{
					if (point > 0) {
						osStr.str(L"");
						osStr << Logic_GetName(pObj);
						m_pPlayInfo[i]->SetText(osStr.str().c_str());

						if (i == 0) {
							m_pPlayAggro[i]->SetPos(1000);

							osStr.str(L"");
							osStr << 100 << L"%";
							m_pPlayPoint[i]->SetText(osStr.str().c_str());
						}
						else {
							m_pPlayAggro[i]->SetPos(point * 1000 / total);

							osStr.str(L"");
							osStr << point * 100 / total << L"%";
							m_pPlayPoint[i]->SetText(osStr.str().c_str());
						}
					}
				}
			}
			Show(true);
		}
		else 
		{
			Show(false);
		}
	}
	else if (msg.Id == g_EventAggroResetNfy) 
	{
		// Get the attribute value list of this pet
		//sGU_AGGRO_RESET_NFY* pNotify = reinterpret_cast<sGU_AGGRO_RESET_NFY*>(msg.pData);
		m_hUpdataId = INVALID_HOBJECT;
		m_hTargetId = INVALID_HOBJECT;
		InitList();
		Show(false);
	}
	else if (msg.Id == g_EventSobTargetSelect) {
		// When the current record is selected
		SNtlEventSobTargetSelect* pData = reinterpret_cast<SNtlEventSobTargetSelect*>(msg.pData);
		m_hTargetId = pData->hSerialId;
		if (m_hUpdataId != pData->hSerialId)
		{
			Show(false);
		}
	}
	else if (msg.Id == g_EventPartyLeave)
	{
		// Stop the pet list when the record panel refreshes
		m_hUpdataId = INVALID_HOBJECT;
		m_hTargetId = INVALID_HOBJECT;
		InitList();
		Show(false);
	}
}

RwInt32 CAggroListGui::SwitchDialog(bool bOpen)
{
	Show(bOpen);

	return TRUE;
}


VOID CAggroListGui::ClickedCloseButton(gui::CComponent* pComponent)
{
	Show(false);
}

void CAggroListGui::InitList() 
{
	for (int i = 0; i < NTL_PARTY_MAX_AGGRO; i++) 
	{
		m_pPlayAggro[i]->SetPos(0);
		m_pPlayInfo[i]->Clear();
		m_pPlayPoint[i]->Clear();
	}
}

VOID CAggroListGui::DrawAggroList()
{

}