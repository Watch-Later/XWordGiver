﻿#pragma once

#include "XG_Dialog.hpp"

// [ルール プリセット]ダイアログ。
class XG_RulePresetDialog : public XG_Dialog
{
public:
    XG_RulePresetDialog()
    {
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        // ダイアログを中央へ移動する。
        XgCenterDialog(hwnd);

        WCHAR szText[64];
        StringCbPrintfW(szText, sizeof(szText), L"%u", xg_nRules);

        HWND hCmb1 = GetDlgItem(hwnd, cmb1);
        ComboBox_RealSetText(hCmb1, szText);
        return TRUE;
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case IDOK:
            {
                WCHAR szText[64];
                HWND hCmb1 = GetDlgItem(hwnd, cmb1);
                ComboBox_RealGetText(hCmb1, szText, _countof(szText));

                xg_nRules = (wcstoul(szText, NULL, 0) & VALID_RULES);
                xg_nRules |= RULE_DONTDIVIDE;

                // ダイアログを閉じる。
                ::EndDialog(hwnd, IDOK);
            }
            break;

        case IDCANCEL:
            // ダイアログを閉じる。
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        }
        return 0;
    }

    INT_PTR DoModal(HWND hwnd)
    {
        return DialogBoxDx(hwnd, IDD_RULEPRESET);
    }
};
