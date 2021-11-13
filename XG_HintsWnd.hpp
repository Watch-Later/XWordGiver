#pragma once

#include "XG_Window.hpp"

class XG_HintsWnd : public XG_Window
{
public:
    // �q���g�E�B���h�E�̃X�N���[���r���[�B
    inline static MScrollView         xg_svHintsScrollView;

    // �q���g�E�B���h�E��UI�t�H���g�B
    inline static HFONT               xg_hHintsUIFont = NULL;

    // �c�̃J�M�̃R���g���[���Q�B
    inline static HWND                xg_hwndTateCaptionStatic = NULL;
    inline static std::vector<HWND>   xg_ahwndTateStatics;
    inline static std::vector<HWND>   xg_ahwndTateEdits;

    // ���̃J�M�̃R���g���[���Q�B
    inline static HWND                xg_hwndYokoCaptionStatic = NULL;
    inline static std::vector<HWND>   xg_ahwndYokoStatics;
    inline static std::vector<HWND>   xg_ahwndYokoEdits;

    // �q���g�E�B���h�E�̈ʒu�ƃT�C�Y�B
    inline static int s_nHintsWndX = CW_USEDEFAULT, s_nHintsWndY = CW_USEDEFAULT;
    inline static int s_nHintsWndCX = CW_USEDEFAULT, s_nHintsWndCY = CW_USEDEFAULT;

    // �q���g���ύX���ꂽ���H
    static bool AreHintsModified(void)
    {
        if (xg_bHintsAdded) {
            return true;
        }

        if (::IsWindow(xg_hHintsWnd)) {
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                if (::SendMessageW(xg_ahwndTateEdits[i], EM_GETMODIFY, 0, 0)) {
                    return true;
                }
            }
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                if (::SendMessageW(xg_ahwndYokoEdits[i], EM_GETMODIFY, 0, 0)) {
                    return true;
                }
            }
        }
        return false;
    }

    // �q���g�f�[�^��ݒ肷��B
    static void SetHintsData(void)
    {
        for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
            ::SetWindowTextW(xg_ahwndTateEdits[i], xg_vecTateHints[i].m_strHint.data());
        }
        for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
            ::SetWindowTextW(xg_ahwndYokoEdits[i], xg_vecYokoHints[i].m_strHint.data());
        }
    }

    // �q���g�f�[�^���X�V����B
    static bool UpdateHintData(void)
    {
        bool updated = false;
        if (::IsWindow(xg_hHintsWnd)) {
            WCHAR sz[512];
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                if (::SendMessageW(xg_ahwndTateEdits[i], EM_GETMODIFY, 0, 0)) {
                    updated = true;
                    ::GetWindowTextW(xg_ahwndTateEdits[i], sz, 
                                     static_cast<int>(ARRAYSIZE(sz)));
                    xg_vecTateHints[i].m_strHint = sz;
                    ::SendMessageW(xg_ahwndTateEdits[i], EM_SETMODIFY, FALSE, 0);
                }
            }
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                if (::SendMessageW(xg_ahwndYokoEdits[i], EM_GETMODIFY, 0, 0)) {
                    updated = true;
                    ::GetWindowTextW(xg_ahwndYokoEdits[i], sz, 
                                     static_cast<int>(ARRAYSIZE(sz)));
                    xg_vecYokoHints[i].m_strHint = sz;
                    ::SendMessageW(xg_ahwndTateEdits[i], EM_SETMODIFY, FALSE, 0);
                }
            }
        }
        return updated;
    }

    XG_HintsWnd()
    {
    }

    virtual LPCTSTR GetWndClassName() const
    {
        return TEXT("XG_HintsWnd");
    }


    virtual void ModifyWndClassDx(WNDCLASSEX& wcx)
    {
        // No change
    }

    // �q���g�E�B���h�E�̃T�C�Y���ς�����B
    void OnSize(HWND hwnd, UINT /*state*/, int /*cx*/, int /*cy*/)
    {
        if (xg_hwndTateCaptionStatic == NULL)
            return;

        xg_svHintsScrollView.clear();

        MRect rcClient;
        ::GetClientRect(hwnd, &rcClient);

        MSize size1, size2;
        {
            HDC hdc = ::CreateCompatibleDC(NULL);
            WCHAR label[64];
            StringCbPrintf(label, sizeof(label), XgLoadStringDx1(IDS_DOWNNUMBER), 100);
            std::wstring strLabel = label;
            ::SelectObject(hdc, ::GetStockObject(SYSTEM_FIXED_FONT));
            ::GetTextExtentPoint32W(hdc, strLabel.data(), int(strLabel.size()), &size1);
            ::SelectObject(hdc, xg_hHintsUIFont);
            ::GetTextExtentPoint32W(hdc, strLabel.data(), int(strLabel.size()), &size2);
            ::DeleteDC(hdc);
        }

        WCHAR szText[512];
        HDC hdc = ::CreateCompatibleDC(NULL);
        HGDIOBJ hFontOld = ::SelectObject(hdc, xg_hHintsUIFont);
        int y = 0;

        // �^�e�̃J�M�B
        {
            MRect rcCtrl(MPoint(0, y + 4), 
                         MSize(rcClient.Width(), size1.cy + 4));
            xg_svHintsScrollView.AddCtrlInfo(xg_hwndTateCaptionStatic, rcCtrl);
            y += size1.cy + 8;
        }
        if (rcClient.Width() - size2.cx - 8 > 0) {
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                ::GetWindowTextW(xg_ahwndTateEdits[i], szText,
                                 ARRAYSIZE(szText));
                MRect rcCtrl(MPoint(size2.cx, y),
                             MSize(rcClient.Width() - size2.cx - 8, 0));
                if (szText[0] == 0) {
                    szText[0] = L' ';
                    szText[1] = 0;
                }
                ::DrawTextW(hdc, szText, -1, &rcCtrl,
                    DT_LEFT | DT_EDITCONTROL | DT_CALCRECT | DT_WORDBREAK);
                rcCtrl.right = rcClient.right;
                rcCtrl.bottom += 8;
                xg_svHintsScrollView.AddCtrlInfo(xg_ahwndTateEdits[i], rcCtrl);
                rcCtrl.left = 0;
                rcCtrl.right = size2.cx;
                xg_svHintsScrollView.AddCtrlInfo(xg_ahwndTateStatics[i], rcCtrl);
                y += rcCtrl.Height();
            }
        }
        // ���R�̃J�M�B
        {
            MRect rcCtrl(MPoint(0, y + 4),
                         MSize(rcClient.Width(), size1.cy + 4));
            xg_svHintsScrollView.AddCtrlInfo(xg_hwndYokoCaptionStatic, rcCtrl);
            y += size1.cy + 8;
        }
        if (rcClient.Width() - size2.cx - 8 > 0) {
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                ::GetWindowTextW(xg_ahwndYokoEdits[i], szText, ARRAYSIZE(szText));
                MRect rcCtrl(MPoint(size2.cx, y),
                             MSize(rcClient.Width() - size2.cx - 8, 0));
                if (szText[0] == 0) {
                    szText[0] = L' ';
                    szText[1] = 0;
                }
                ::DrawTextW(hdc, szText, -1, &rcCtrl,
                    DT_LEFT | DT_EDITCONTROL | DT_CALCRECT | DT_WORDBREAK);
                rcCtrl.right = rcClient.right;
                rcCtrl.bottom += 8;
                xg_svHintsScrollView.AddCtrlInfo(xg_ahwndYokoEdits[i], rcCtrl);
                rcCtrl.left = 0;
                rcCtrl.right = size2.cx;
                xg_svHintsScrollView.AddCtrlInfo(xg_ahwndYokoStatics[i], rcCtrl);
                y += rcCtrl.Height();
            }
        }

        ::SelectObject(hdc, hFontOld);
        ::DeleteDC(hdc);

        xg_svHintsScrollView.SetExtentForAllCtrls();
        xg_svHintsScrollView.UpdateAll();
    }

    struct XG_HintEditData
    {
        WNDPROC m_fnOldWndProc;
        bool    m_fTate;
    };

    static LRESULT CALLBACK XgHintEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WNDPROC fn;
        XG_HintEditData *data =
            reinterpret_cast<XG_HintEditData *>(
                ::GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_CHAR:
            if (wParam == L'\r' || wParam == L'\n') {
                // ���s�������ꂽ�B�K�v�Ȃ�΃f�[�^���X�V����B
                if (AreHintsModified()) {
                    auto hu1 = std::make_shared<XG_UndoData_HintsUpdated>();
                    auto hu2 = std::make_shared<XG_UndoData_HintsUpdated>();
                    hu1->Get();
                    {
                        // �q���g���X�V����B
                        UpdateHintData();
                    }
                    hu2->Get();
                    xg_ubUndoBuffer.Commit(UC_HINTS_UPDATED, hu1, hu2);
                }
            }
            return ::CallWindowProc(data->m_fnOldWndProc,
                hwnd, uMsg, wParam, lParam);

        case WM_SETFOCUS: // �t�H�[�J�X�𓾂��B
            if (wParam) {
                // �t�H�[�J�X�������R���g���[���̑I������������B
                HWND hwndLoseFocus = reinterpret_cast<HWND>(wParam);
                ::SendMessageW(hwndLoseFocus, EM_SETSEL, 0, 0);
            }
            // �t�H�[�J�X�̂���R���g���[����������悤�Ɉړ�����B
            xg_svHintsScrollView.EnsureCtrlVisible(hwnd);
            return ::CallWindowProc(data->m_fnOldWndProc,
                hwnd, uMsg, wParam, lParam);

        case WM_KILLFOCUS:  // �t�H�[�J�X���������B
            // �q���g�ɍX�V������΁A�f�[�^���X�V����B
            if (AreHintsModified()) {
                auto hu1 = std::make_shared<XG_UndoData_HintsUpdated>();
                auto hu2 = std::make_shared<XG_UndoData_HintsUpdated>();
                hu1->Get();
                {
                    // �q���g���X�V����B
                    UpdateHintData();
                }
                hu2->Get();
                xg_ubUndoBuffer.Commit(UC_HINTS_UPDATED, hu1, hu2);
            }
            // ���C�A�E�g�𒲐�����B
            ::PostMessageW(xg_hHintsWnd, WM_SIZE, 0, 0);
            return ::CallWindowProc(data->m_fnOldWndProc,
                hwnd, uMsg, wParam, lParam);

        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                ::PostMessageW(xg_hHintsWnd, WM_SIZE, 0, 0);
                ::SetFocus(NULL);
                break;
            }

            if (wParam == VK_TAB) {
                HWND hwndNext;
                if (::GetAsyncKeyState(VK_SHIFT) < 0)
                    hwndNext = ::GetNextDlgTabItem(xg_hHintsWnd, hwnd, TRUE);
                else
                    hwndNext = ::GetNextDlgTabItem(xg_hHintsWnd, hwnd, FALSE);
                ::SendMessageW(hwnd, EM_SETSEL, 0, 0);
                ::SendMessageW(hwndNext, EM_SETSEL, 0, -1);
                ::SetFocus(hwndNext);
                break;
            }

            if (wParam == VK_PRIOR || wParam == VK_NEXT) {
                HWND hwndParent = ::GetParent(hwnd);
                ::SendMessageW(hwndParent, uMsg, wParam, lParam);
                break;
            }

            if (wParam == VK_F6) {
                if (::GetAsyncKeyState(VK_SHIFT) < 0)
                    ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
                else
                    ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
                break;
            }

            return ::CallWindowProc(data->m_fnOldWndProc,
                hwnd, uMsg, wParam, lParam);

        case WM_NCDESTROY:
            fn = data->m_fnOldWndProc;
            ::LocalFree(data);
            return ::CallWindowProc(fn, hwnd, uMsg, wParam, lParam);

        case WM_MOUSEWHEEL:
            {
                HWND hwndParent = ::GetParent(hwnd);
                ::SendMessageW(hwndParent, uMsg, wParam, lParam);
            }
            break;

        default:
            return ::CallWindowProc(data->m_fnOldWndProc,
                hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }

    // �q���g�E�B���h�E���쐬���ꂽ�B
    BOOL OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
    {
        xg_hHintsWnd = hwnd;

        // �������B
        xg_ahwndTateStatics.clear();
        xg_ahwndTateEdits.clear();
        xg_ahwndYokoStatics.clear();
        xg_ahwndYokoEdits.clear();

        xg_svHintsScrollView.SetParent(hwnd);
        xg_svHintsScrollView.ShowScrollBars(FALSE, TRUE);

        if (xg_hHintsUIFont) {
            ::DeleteObject(xg_hHintsUIFont);
        }
        xg_hHintsUIFont = ::CreateFontIndirectW(XgGetUIFont());
        if (xg_hHintsUIFont == NULL) {
            xg_hHintsUIFont =
                reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
        }

        HWND hwndCtrl;

        hwndCtrl = ::CreateWindowW(
            TEXT("STATIC"), XgLoadStringDx1(IDS_DOWN),
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_NOTIFY |
            SS_CENTER | SS_CENTERIMAGE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        if (hwndCtrl == NULL)
            return FALSE;
        xg_hwndTateCaptionStatic = hwndCtrl;
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(::GetStockObject(SYSTEM_FIXED_FONT)),
            TRUE);

        hwndCtrl = ::CreateWindowW(
            TEXT("STATIC"), XgLoadStringDx1(IDS_ACROSS),
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_NOTIFY |
            SS_CENTER | SS_CENTERIMAGE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        if (hwndCtrl == NULL)
            return FALSE;
        xg_hwndYokoCaptionStatic = hwndCtrl;
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(::GetStockObject(SYSTEM_FIXED_FONT)),
            TRUE);

        XG_HintEditData *data;
        WCHAR sz[256];
        for (const auto& hint : xg_vecTateHints) {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_DOWNNUMBER), hint.m_number);
            hwndCtrl = ::CreateWindowW(TEXT("STATIC"), sz,
                WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX | SS_NOTIFY | SS_CENTERIMAGE,
                0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
            assert(hwndCtrl);
            ::SendMessageW(hwndCtrl, WM_SETFONT,
                reinterpret_cast<WPARAM>(xg_hHintsUIFont),
                TRUE);
            if (hwndCtrl == NULL)
                return FALSE;

            xg_ahwndTateStatics.emplace_back(hwndCtrl);

            hwndCtrl = ::CreateWindowExW(
                WS_EX_CLIENTEDGE,
                TEXT("EDIT"), hint.m_strHint.data(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
                0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
            assert(hwndCtrl);
            ::SendMessageW(hwndCtrl, WM_SETFONT,
                reinterpret_cast<WPARAM>(xg_hHintsUIFont),
                TRUE);
            if (hwndCtrl == NULL)
                return FALSE;

            data = reinterpret_cast<XG_HintEditData *>(
                ::LocalAlloc(LMEM_FIXED, sizeof(XG_HintEditData)));
            data->m_fTate = true;
            data->m_fnOldWndProc = reinterpret_cast<WNDPROC>(
                ::SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(XgHintEdit_WndProc)));
            ::SetWindowLongPtr(hwndCtrl, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(data));
            xg_ahwndTateEdits.emplace_back(hwndCtrl);
        }
        for (const auto& hint : xg_vecYokoHints) {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_ACROSSNUMBER), hint.m_number);
            hwndCtrl = ::CreateWindowW(TEXT("STATIC"), sz,
                WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX | SS_NOTIFY | SS_CENTERIMAGE,
                0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
            assert(hwndCtrl);
            ::SendMessageW(hwndCtrl, WM_SETFONT,
                reinterpret_cast<WPARAM>(xg_hHintsUIFont),
                TRUE);
            if (hwndCtrl == NULL)
                return FALSE;

            xg_ahwndYokoStatics.emplace_back(hwndCtrl);

            hwndCtrl = ::CreateWindowExW(
                WS_EX_CLIENTEDGE,
                TEXT("EDIT"), hint.m_strHint.data(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
                0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
            assert(hwndCtrl);
            ::SendMessageW(hwndCtrl, WM_SETFONT,
                reinterpret_cast<WPARAM>(xg_hHintsUIFont),
                TRUE);
            if (hwndCtrl == NULL)
                return FALSE;

            data = reinterpret_cast<XG_HintEditData *>(
                ::LocalAlloc(LMEM_FIXED, sizeof(XG_HintEditData)));
            data->m_fTate = false;
            data->m_fnOldWndProc = reinterpret_cast<WNDPROC>(
                ::SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(XgHintEdit_WndProc)));
            ::SetWindowLongPtr(hwndCtrl, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(data));
            xg_ahwndYokoEdits.emplace_back(hwndCtrl);
        }

        if (xg_ahwndTateEdits.size())
            ::SetFocus(xg_ahwndTateEdits[0]);

        ::PostMessageW(hwnd, WM_SIZE, 0, 0);

        return TRUE;
    }

    // �q���g�E�B���h�E���c�ɃX�N���[�����ꂽ�B
    void OnVScroll(HWND /*hwnd*/, HWND /*hwndCtl*/, UINT code, int pos)
    {
        xg_svHintsScrollView.Scroll(SB_VERT, code, pos);
    }

    // �q���g�E�B���h�E���j�����ꂽ�B
    void OnDestroy(HWND hwnd)
    {
        if (xg_hHintsWnd) {
            // �q���g�f�[�^���X�V����B
            UpdateHintData();
        }

        // ���݂̈ʒu�ƃT�C�Y��ۑ�����B
        MRect rc;
        ::GetWindowRect(hwnd, &rc);
        XG_HintsWnd::s_nHintsWndX = rc.left;
        XG_HintsWnd::s_nHintsWndY = rc.top;
        XG_HintsWnd::s_nHintsWndCX = rc.Width();
        XG_HintsWnd::s_nHintsWndCY = rc.Height();

        xg_hHintsWnd = NULL;
        xg_hwndTateCaptionStatic = NULL;
        xg_hwndYokoCaptionStatic = NULL;
        xg_ahwndTateStatics.clear();
        xg_ahwndTateEdits.clear();
        xg_ahwndYokoStatics.clear();
        xg_ahwndYokoEdits.clear();
        xg_svHintsScrollView.clear();

        ::DeleteObject(xg_hHintsUIFont);
        xg_hHintsUIFont = NULL;
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        if (codeNotify == STN_CLICKED) {
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                if (xg_ahwndTateStatics[i] == hwndCtl) {
                    ::SendMessageW(xg_ahwndTateEdits[i], EM_SETSEL, 0, -1);
                    ::SetFocus(xg_ahwndTateEdits[i]);
                    return;
                }
            }
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                if (xg_ahwndYokoStatics[i] == hwndCtl) {
                    ::SendMessageW(xg_ahwndYokoEdits[i], EM_SETSEL, 0, -1);
                    ::SetFocus(xg_ahwndYokoEdits[i]);
                    return;
                }
            }
        }
    }

    // �L�[�������ꂽ�B
    void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
    {
        if (!fDown)
            return;

        switch (vk) {
        case VK_PRIOR:
            ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEUP, 0), 0);
            break;

        case VK_NEXT:
            ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEDOWN, 0), 0);
            break;

        case VK_TAB:
            if (xg_ahwndTateEdits.size())
                ::SetFocus(xg_ahwndTateEdits[0]);
            break;

        case VK_F6:
            if (::GetAsyncKeyState(VK_SHIFT) < 0)
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
            else
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
            break;
        }
    }


    // �}�E�X�z�C�[������]�����B
    void __fastcall
    OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
    {
        if (::GetAsyncKeyState(VK_SHIFT) < 0) {
            if (zDelta < 0)
                ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
            else if (zDelta > 0)
                ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
        } else {
            if (zDelta < 0)
                ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
            else if (zDelta > 0)
                ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
        }
    }

    // �q���g �E�B���h�E�̃T�C�Y�𐧌�����B
    void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
    {
        lpMinMaxInfo->ptMinTrackSize.x = 256;
        lpMinMaxInfo->ptMinTrackSize.y = 128;
    }

    // �u�q���g�v�E�B���h�E�̃R���e�L�X�g���j���[�B
    void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
    {
        WCHAR szClass[8];
        szClass[0] = 0;
        GetClassNameW(hwndContext, szClass, ARRAYSIZE(szClass));
        if (lstrcmpiW(szClass, L"EDIT") == 0)
            return;

        HMENU hMenu = LoadMenuW(xg_hInstance, MAKEINTRESOURCEW(2));
        HMENU hSubMenu = GetSubMenu(hMenu, 1);

        // �E�N���b�N���j���[��\������B
        ::SetForegroundWindow(hwnd);
        INT nCmd = ::TrackPopupMenu(
            hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD,
            xPos, yPos, 0, hwnd, NULL);
        ::PostMessageW(hwnd, WM_NULL, 0, 0);
        if (nCmd)
            ::PostMessageW(xg_hMainWnd, WM_COMMAND, nCmd, 0);

        ::DestroyMenu(hMenu);
    }

    // �q���g �E�B���h�E�̃E�B���h�E �v���V�[�W���[�B
    virtual LRESULT CALLBACK
    WindowProcDx(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_SIZE, OnSize);
        HANDLE_MSG(hWnd, WM_VSCROLL, OnVScroll);
        HANDLE_MSG(hWnd, WM_KEYDOWN, OnKey);
        HANDLE_MSG(hWnd, WM_KEYUP, OnKey);
        HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_MOUSEWHEEL, OnMouseWheel);
        HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
        HANDLE_MSG(hWnd, WM_CONTEXTMENU, OnContextMenu);
        default:
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
        return 0;
    }
};
