#ifndef __AGGROLIST_GUI_H__
#define __AGGROLIST_GUI_H__

// core
#include "ceventhandler.h"

// presentation
#include "NtlSLDef.h"
#include "NtlPLGui.h"
#include "NtlSLEvent.h"

class CAggroListGui :public CNtlPLGui, public RWS::CEventHandler
{
public:
	CAggroListGui(const RwChar* pName);
	virtual ~CAggroListGui();

	RwBool		Create();
	VOID		Destroy();

	// HandleEvents
	VOID		HandleEvents(RWS::CMsg& msg);
	RwInt32		SwitchDialog(bool bOpen);

	void		InitList();
	VOID		DrawAggroList();

protected:
	VOID				ClickedCloseButton(gui::CComponent* pComponent);

protected:

	gui::CStaticBox*	m_pTitle;
	gui::CButton*		m_pExitButton;
	gui::CPanel*		m_pAggroBack[NTL_PARTY_MAX_AGGRO];
	gui::CProgressBar*	m_pPlayAggro[NTL_PARTY_MAX_AGGRO];
	gui::CStaticBox*	m_pPlayInfo[NTL_PARTY_MAX_AGGRO];
	gui::CStaticBox*	m_pPlayPoint[NTL_PARTY_MAX_AGGRO];

	gui::CSlot			m_slotCloseButton;

protected:

	SERIAL_HANDLE		m_hTargetId;
	SERIAL_HANDLE		m_hUpdataId;
};

#endif
