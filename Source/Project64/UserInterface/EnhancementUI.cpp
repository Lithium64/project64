/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "stdafx.h"
#include <Project64\UserInterface\EnhancementUI.h>
#include <Project64-core\N64System\Enhancement\Enhancements.h>

class CEditEnhancement :
    public CDialogImpl < CEditEnhancement >
{
public:
    BEGIN_MSG_MAP_EX(CEditEnhancement)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER_EX(IDC_GAMESHARK, BN_CLICKED, OnGamesharkBtn)
        COMMAND_ID_HANDLER(IDC_BTN_GAMESHARK, OnEditGameshark)
        COMMAND_ID_HANDLER(IDOK, OnOkCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    enum { IDD = IDD_Enhancement_Edit };

    CEditEnhancement(CEnhancementUI & EnhancementUI, CEnhancement * Enhancement);

private:
    CEditEnhancement(void);
    CEditEnhancement(const CEditEnhancement&);
    CEditEnhancement& operator=(const CEditEnhancement&);

    LRESULT	OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGamesharkBtn(UINT Code, int id, HWND ctl);
    LRESULT OnEditGameshark(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOkCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    CEnhancementUI & m_EnhancementUI;
    CEnhancement::CodeEntries m_Entries;
    CEnhancement * m_EditEnhancement;
};

class CEditGS :
    public CDialogImpl < CEditGS >
{
public:
    BEGIN_MSG_MAP_EX(CEditGS)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_CHEAT_CODES, EN_CHANGE, OnCheatChanged)
        COMMAND_ID_HANDLER(IDOK, OnOkCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    enum { IDD = IDD_Enhancement_GS };

    CEditGS(CEnhancement::CodeEntries & Entries);

private:
    CEditGS();
    CEditGS(const CEditGS&);
    CEditGS& operator=(const CEditGS&);

    LRESULT	OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
    LRESULT OnOkCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
    LRESULT OnCheatChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

    bool CheatCode(CEnhancement::CodeEntries & Entries);

    CEnhancement::CodeEntries & m_Entries;
};

CEnhancementUI::CEnhancementUI(void) :
    m_hSelectedItem(NULL)
{
}

void CEnhancementUI::Display(HWND hParent, bool BlockExecution)
{
    if (g_BaseSystem)
    {
        g_BaseSystem->ExternalEvent(SysEvent_PauseCPU_Enhancement);
    }
    if (BlockExecution)
    {
        m_bModal = true;
        DoModal(hParent);
    }
    else if (m_hWnd != NULL)
    {
        SetFocus();
    }
    else
    {
        m_bModal = false;
        Create(hParent);
    }
}

LRESULT	CEnhancementUI::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowText(wGS(CHEAT_TITLE).c_str());

    m_TreeList.Attach(GetDlgItem(IDC_ENHANCEMENTLIST));
    LONG Style = m_TreeList.GetWindowLong(GWL_STYLE);
    m_TreeList.SetWindowLong(GWL_STYLE, TVS_CHECKBOXES | TVS_SHOWSELALWAYS | Style);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 40, 40);
    HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TRI_STATE));
    ImageList_AddMasked(hImageList, hBmp, RGB(255, 0, 255));
    DeleteObject(hBmp);
    m_TreeList.SetImageList(hImageList, TVSIL_STATE);

    m_Enhancements = g_Enhancements->Enhancements();
    RefreshList();
    return TRUE;
}

LRESULT CEnhancementUI::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return TRUE;
}

LRESULT CEnhancementUI::OnAddEnhancement(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CEditEnhancement(*this, nullptr).DoModal(m_hWnd);
    RefreshList();
    return TRUE;
}

LRESULT CEnhancementUI::OnEnhancementListRClicked(NMHDR* pNMHDR)
{
    TVHITTESTINFO ht = { 0 };
    uint32_t dwpos = GetMessagePos();
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1);

    TreeView_HitTest(pNMHDR->hwndFrom, &ht);
    m_hSelectedItem = ht.hItem;

    POINT Mouse;
    GetCursorPos(&Mouse);
    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ENHANCEMENT_MENU));
    HMENU hPopupMenu = GetSubMenu(hMenu, 0);

    if (m_hSelectedItem == NULL)
    {
        RemoveMenu(hPopupMenu, 3, MF_BYPOSITION);
        RemoveMenu(hPopupMenu, 2, MF_BYPOSITION);
        RemoveMenu(hPopupMenu, 1, MF_BYPOSITION);
    }

    TrackPopupMenu(hPopupMenu, 0, Mouse.x, Mouse.y, 0, m_hWnd, NULL);
    DestroyMenu(hMenu);
    return TRUE;
}

LRESULT CEnhancementUI::OnEnhancementListDClicked(NMHDR* pNMHDR)
{
    uint32_t dwpos = GetMessagePos();
    TVHITTESTINFO ht = { 0 };
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1);

    TreeView_HitTest(pNMHDR->hwndFrom, &ht);

    if ((ht.flags & TVHT_ONITEMLABEL) == 0 || ht.hItem == 0)
    {
        return 0;
    }
    TVITEM item = { 0 };
    item.mask = TVIF_PARAM;
    item.hItem = (HTREEITEM)ht.hItem;
    if (!m_TreeList.GetItem(&item) || item.lParam == NULL)
    {
        return 0;
    }

    CEditEnhancement(*this, (CEnhancement *)item.lParam).DoModal(m_hWnd);
    RefreshList();
    return 0;
}

void CEnhancementUI::AddCodeLayers(LPARAM Enhancement, const std::wstring & Name, HTREEITEM hParent, bool Active)
{
    TV_INSERTSTRUCT tv;

    wchar_t Text[500], Item[500];
    if (Name.length() > ((sizeof(Text) / sizeof(Text[0])) - 5)) { g_Notify->BreakPoint(__FILE__, __LINE__); }
    wcscpy(Text, Name.c_str());
    if (wcschr(Text, L'\\') > 0) { *wcschr(Text, L'\\') = 0; }

    tv.item.mask = TVIF_TEXT;
    tv.item.pszText = Item;
    tv.item.cchTextMax = sizeof(Item);
    tv.item.hItem = m_TreeList.GetChildItem(hParent);
    while (tv.item.hItem)
    {
        m_TreeList.GetItem(&tv.item);
        if (wcscmp(Text, Item) == 0)
        {
            TV_CHECK_STATE State = TV_GetCheckState(tv.item.hItem);
            if ((Active && State == TV_STATE_CLEAR) || (!Active && State == TV_STATE_CHECKED))
            {
                TV_SetCheckState(tv.item.hItem, TV_STATE_INDETERMINATE);
            }
            size_t StartPos = wcslen(Text) + 1;
            std::wstring TempCheatName;
            if (StartPos < Name.length())
            {
                TempCheatName = Name.substr(StartPos);
            }
            AddCodeLayers(Enhancement, TempCheatName, tv.item.hItem, Active);
            return;
        }
        tv.item.hItem = m_TreeList.GetNextSiblingItem(tv.item.hItem);
    }

    tv.hInsertAfter = TVI_SORT;
    tv.item.mask = TVIF_TEXT | TVIF_PARAM;
    tv.item.pszText = Text;
    tv.item.lParam = Enhancement;
    tv.hParent = hParent;
    hParent = m_TreeList.InsertItem(&tv);
    TV_SetCheckState(hParent, Active ? TV_STATE_CHECKED : TV_STATE_CLEAR);

    if (wcscmp(Text, Name.c_str()) == 0) { return; }
    AddCodeLayers(Enhancement, Name.substr(wcslen(Text) + 1), hParent, Active);
}

void CEnhancementUI::RefreshList()
{
    m_TreeList.DeleteAllItems();
    for (CEnhancementList::iterator itr = m_Enhancements.begin(); itr != m_Enhancements.end(); itr++)
    {
        std::string Name = itr->second.GetNameAndExtension();
        if (Name.length() == 0)
        {
            continue;
        }
        AddCodeLayers((LPARAM)&itr->second, stdstr(Name).ToUTF16(), TVI_ROOT, itr->second.Active());
    }
}

CEnhancementUI::TV_CHECK_STATE CEnhancementUI::TV_GetCheckState(HTREEITEM hItem)
{
    TVITEM tvItem = { 0 };
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    m_TreeList.GetItem(&tvItem);

    switch (tvItem.state >> 12)
    {
    case 1: return TV_STATE_CHECKED;
    case 2: return TV_STATE_CLEAR;
    case 3: return TV_STATE_INDETERMINATE;
    }
    return TV_STATE_UNKNOWN;
}

bool CEnhancementUI::TV_SetCheckState(HTREEITEM hItem, TV_CHECK_STATE state)
{
    TVITEM tvItem = { 0 };
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = (HTREEITEM)hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    switch (state)
    {
    case TV_STATE_CHECKED: tvItem.state = INDEXTOSTATEIMAGEMASK(1); break;
    case TV_STATE_CLEAR: tvItem.state = INDEXTOSTATEIMAGEMASK(2); break;
    case TV_STATE_INDETERMINATE: tvItem.state = INDEXTOSTATEIMAGEMASK(3); break;
    default: tvItem.state = INDEXTOSTATEIMAGEMASK(0); break;
    }
    return m_TreeList.SetItem(&tvItem) != 0;
}

CEditEnhancement::CEditEnhancement(CEnhancementUI & EnhancementUI, CEnhancement * Enhancement) :
    m_EnhancementUI(EnhancementUI),
    m_EditEnhancement(Enhancement)
{
}

LRESULT	CEditEnhancement::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_Entries = m_EditEnhancement != nullptr ? m_EditEnhancement->GetEntries() : CEnhancement::CodeEntries();
    GetDlgItem(IDC_OVERCLOCK).EnableWindow(false);
    GetDlgItem(IDC_OVER_CLOCK_MODIFIER).EnableWindow(false);
    GetDlgItem(IDC_CODE_NAME).SetWindowText(m_EditEnhancement != nullptr ? stdstr(m_EditEnhancement->GetName()).ToUTF16().c_str() : L"");
    GetDlgItem(IDC_NOTES).SetWindowText(m_EditEnhancement != nullptr ? stdstr(m_EditEnhancement->GetNote()).ToUTF16().c_str() : L"");
    CButton(GetDlgItem(IDC_AUTOON)).SetCheck(m_EditEnhancement != nullptr ? (m_EditEnhancement->GetOnByDefault() ? BST_CHECKED : BST_UNCHECKED) : BST_UNCHECKED);
    CButton(GetDlgItem(IDC_GAMESHARK)).SetCheck(m_Entries.size() > 0 ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
}

LRESULT CEditEnhancement::OnGamesharkBtn(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
{
    CButton(GetDlgItem(IDC_GAMESHARK)).SetCheck(m_Entries.size() > 0 ? BST_CHECKED : BST_UNCHECKED);
    CEditGS(m_Entries).DoModal(m_hWnd);
    CButton(GetDlgItem(IDC_GAMESHARK)).SetCheck(m_Entries.size() > 0 ? BST_CHECKED : BST_UNCHECKED);
    return 0;
}

LRESULT CEditEnhancement::OnEditGameshark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    OnGamesharkBtn(0, 0, 0);
    return TRUE;
}

LRESULT CEditEnhancement::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return TRUE;
}

LRESULT CEditEnhancement::OnOkCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CEnhancementList & Enhancements = m_EnhancementUI.m_Enhancements;
    if (m_EditEnhancement != nullptr)
    {
        m_EditEnhancement->SetName(GetCWindowText(GetDlgItem(IDC_CODE_NAME)).c_str());
        m_EditEnhancement->SetOnByDefault(CButton(GetDlgItem(IDC_AUTOON)).GetCheck() == BST_CHECKED);
        m_EditEnhancement->SetEntries(m_Entries);
        m_EditEnhancement->SetNote(GetCWindowText(GetDlgItem(IDC_NOTES)).c_str());
    }
    else
    {
        CEnhancement Enhancement("Enhancement");
        Enhancement.SetName(GetCWindowText(GetDlgItem(IDC_CODE_NAME)).c_str());
        Enhancement.SetOnByDefault(CButton(GetDlgItem(IDC_AUTOON)).GetCheck() == BST_CHECKED);
        Enhancement.SetEntries(m_Entries);
        Enhancement.SetNote(GetCWindowText(GetDlgItem(IDC_NOTES)).c_str());
        Enhancements.AddItem(Enhancement);
    }
    g_Enhancements->UpdateEnhancements(Enhancements);
    m_EnhancementUI.RefreshList();
    EndDialog(wID);
    return TRUE;
}


CEditGS::CEditGS(CEnhancement::CodeEntries & Entries) :
    m_Entries(Entries)
{
}

LRESULT	CEditGS::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    stdstr Buffer;
    for (size_t i = 0, n = m_Entries.size(); i < n; i++)
    {
        const CEnhancement::CodeEntry Entry = m_Entries[i];
        if (!Buffer.empty())
        {
            Buffer.append("\r\n");
        }
        Buffer += stdstr_f("%08X %s", Entry.Command, Entry.Value.c_str());
    }
    CEdit CheatCodes(GetDlgItem(IDC_CHEAT_CODES));
    CheatCodes.SetWindowText(Buffer.ToUTF16().c_str());
    return TRUE;
}

LRESULT CEditGS::OnCheatChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CEnhancement::CodeEntries Entries;
    GetDlgItem(IDOK).EnableWindow(CheatCode(Entries));
    return true;
}

LRESULT CEditGS::OnOkCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CEnhancement::CodeEntries Entries;
    if (CheatCode(Entries))
    {
        m_Entries = Entries;
        EndDialog(wID);
    }
    return TRUE;
}

LRESULT CEditGS::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return TRUE;
}

bool CEditGS::CheatCode(CEnhancement::CodeEntries & Entries)
{
    Entries.clear();

    bool ValidCode = true;
    CEdit CheatCodes(GetDlgItem(IDC_CHEAT_CODES));
    uint32_t numlines = CheatCodes.GetLineCount();

    for (uint32_t line = 0; line < numlines; line++)
    {
        const char * formatnormal = "XXXXXXXX XXXX";

        wchar_t wstr[128] = { 0 };
        *(LPWORD)wstr = sizeof(wstr) / sizeof(wstr[0]);
        uint32_t len = CheatCodes.GetLine(line, wstr);
        wstr[len] = 0;

        if (len <= 0) { continue; }

        std::string str = stdstr().FromUTF16(wstr);
        char tempformat[128] = { 0 };
        for (uint32_t i = 0; i < sizeof(tempformat); i++)
        {
            if (isxdigit(str[i]))
            {
                tempformat[i] = 'X';
            }
            if (str[i] == ' ')
            {
                tempformat[i] = str[i];
            }
            if (str[i] == 0) { break; }
        }

        if (strcmp(tempformat, formatnormal) == 0)
        {
            CEnhancement::CodeEntry Entry;
            Entry.Command = strtoul(str.c_str(), 0, 16);
            const char * ReadPos = strchr(str.c_str(), ' ');
            if (ReadPos != nullptr)
            {
                Entry.Value = ReadPos + 1;
                Entries.push_back(Entry);
            }
            else
            {
                ValidCode = false;
                break;
            }
        }
        else
        {
            ValidCode = false;
            break;
        }
    }

    if (ValidCode && Entries.size() > 0)
    {
        CEnhancement Item("");
        Item.SetName("Test");
        Item.SetEntries(Entries);
        if (!Item.Valid() || Item.CodeOptionSize() != 0)
        {
            return false;
        }
    }
    return ValidCode;
}
