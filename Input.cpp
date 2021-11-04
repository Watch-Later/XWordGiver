//////////////////////////////////////////////////////////////////////////////
// Input.cpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#include "XWordGiver.hpp"

// ���̓p���b�g��\�����邩�H
bool xg_bShowInputPalette = false;

// ���͕�����؂�ւ���B
void __fastcall XgInputDirection(HWND hwnd, INT nDirection)
{
    switch (nDirection) {
    case 1:
        xg_bTateInput = TRUE;
        break;
    case 0:
        xg_bTateInput = FALSE;
        break;
    case -1:
        xg_bTateInput = !xg_bTateInput;
        break;
    }

    if (xg_hwndInputPalette) {
        if (xg_bTateOki) {
            if (xg_bTateInput) {
                SetDlgItemTextW(xg_hwndInputPalette, 20052, XgLoadStringDx1(IDS_VINPUT));
            } else {
                SetDlgItemTextW(xg_hwndInputPalette, 20052, XgLoadStringDx1(IDS_HINPUT));
            }
        } else {
            if (xg_bTateInput) {
                SetDlgItemTextW(xg_hwndInputPalette, 20052, XgLoadStringDx1(IDS_VINPUT2));
            } else {
                SetDlgItemTextW(xg_hwndInputPalette, 20052, XgLoadStringDx1(IDS_HINPUT2));
            }
        }
    }

    XgUpdateStatusBar(hwnd);
    xg_prev_vk = 0;
}

// ���������؂�ւ���B
void __fastcall XgSetCharFeed(HWND hwnd, INT nMode)
{
    switch (nMode) {
    case 1:
        xg_bCharFeed = true;
        break;
    case 0:
        xg_bCharFeed = false;
        break;
    case -1:
        xg_bCharFeed = !xg_bCharFeed;
        break;
    }

    if (xg_hwndInputPalette) {
        if (xg_bCharFeed) {
            CheckDlgButton(xg_hwndInputPalette, 20050, BST_CHECKED);
        } else {
            CheckDlgButton(xg_hwndInputPalette, 20050, BST_UNCHECKED);
        }
    }

    xg_prev_vk = 0;
}

// ���s����B
void __fastcall XgReturn(HWND hwnd)
{
    if (xg_bTateInput) {
        ++xg_caret_pos.m_j;
        if (xg_caret_pos.m_j >= xg_nCols) {
            xg_caret_pos.m_j = 0;
        }
        xg_caret_pos.m_i = 0;
    } else {
        ++xg_caret_pos.m_i;
        if (xg_caret_pos.m_i >= xg_nRows) {
            xg_caret_pos.m_i = 0;
        }
        xg_caret_pos.m_j = 0;
    }

    XgUpdateStatusBar(hwnd);
    XgEnsureCaretVisible(hwnd);
    XgUpdateImage(hwnd);

    xg_prev_vk = 0;
}

// ��d�}�X�؂�ւ��B
void __fastcall XgToggleMark(HWND hwnd)
{
    INT i = xg_caret_pos.m_i;
    INT j = xg_caret_pos.m_j;

    // �}�[�N����Ă��Ȃ����H
    if (XgGetMarked(i, j) == -1) {
        // �}�[�N����Ă��Ȃ��}�X�B�}�[�N���Z�b�g����B
        XG_Board *pxw;

        if (xg_bSolved && xg_bShowAnswer)
            pxw = &xg_solution;
        else
            pxw = &xg_xword;

        if (pxw->GetAt(i, j) != ZEN_BLACK)
            XgSetMark(i, j);
        else
            ::MessageBeep(0xFFFFFFFF);
    } else {
        // �}�[�N���Z�b�g����Ă���}�X�B�}�[�N����������B
        XgDeleteMark(i, j);
    }

    // �C���[�W���X�V����B
    INT x = XgGetHScrollPos();
    INT y = XgGetVScrollPos();
    XgMarkUpdate();
    XgUpdateImage(hwnd, x, y);
}

// �������N���A�B
void __fastcall XgClearNonBlocks(HWND hwnd)
{
    xg_caret_pos.clear();

    auto sa1 = std::make_shared<XG_UndoData_SetAll>();
    auto sa2 = std::make_shared<XG_UndoData_SetAll>();
    sa1->Get();

    if (xg_bSolved) {
        xg_bShowAnswer = false;
    }
    for (INT i = 0; i < xg_nRows; ++i) {
        for (INT j = 0; j < xg_nCols; ++j) {
            WCHAR oldch = xg_xword.GetAt(i, j);
            if (oldch != ZEN_BLACK && oldch != ZEN_SPACE) {
                xg_xword.SetAt(i, j, ZEN_SPACE);
            }
        }
    }

    sa2->Get();
    xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);

    XgUpdateImage(hwnd);

    xg_prev_vk = 0;
}

// ����������s����B
void __fastcall XgCharFeed(HWND hwnd)
{
    if (!xg_bCharFeed)
        return;

    if (xg_bTateInput) {
        ++xg_caret_pos.m_i;
        if (xg_caret_pos.m_i >= xg_nRows) {
            xg_caret_pos.m_i = 0;
            ++xg_caret_pos.m_j;
            if (xg_caret_pos.m_j >= xg_nCols) {
                xg_caret_pos.m_j = 0;
            }
        }
    } else {
        ++xg_caret_pos.m_j;
        if (xg_caret_pos.m_j >= xg_nCols) {
            xg_caret_pos.m_j = 0;
            ++xg_caret_pos.m_i;
            if (xg_caret_pos.m_i >= xg_nRows) {
                xg_caret_pos.m_i = 0;
            }
        }
    }

    XgUpdateStatusBar(hwnd);
    xg_prev_vk = 0;
}

// BackSpace�����s����B
void __fastcall XgCharBack(HWND hwnd)
{
    if (!xg_bCharFeed) {
        MainWnd_OnChar(hwnd, L'_', 1);
        return;
    }

    if (xg_bTateInput) {
        if (xg_caret_pos.m_i == 0) {
            if (xg_caret_pos.m_j == 0) {
                xg_caret_pos.m_i = xg_nRows - 1;
                xg_caret_pos.m_j = xg_nCols - 1;
            } else {
                xg_caret_pos.m_i = xg_nRows - 1;
                --xg_caret_pos.m_j;
            }
        } else {
            --xg_caret_pos.m_i;
        }
    } else {
        if (xg_caret_pos.m_j == 0) {
            if (xg_caret_pos.m_i == 0) {
                xg_caret_pos.m_j = xg_nCols - 1;
                xg_caret_pos.m_i = xg_nRows - 1;
            } else {
                xg_caret_pos.m_j = xg_nCols - 1;
                --xg_caret_pos.m_i;
            }
        } else {
            --xg_caret_pos.m_j;
        }
    }

    xg_bCharFeed = false;
    MainWnd_OnChar(hwnd, L'_', 1);
    xg_bCharFeed = true;

    XgUpdateStatusBar(hwnd);
    xg_prev_vk = 0;
}

// ���������͂��ꂽ�B
void __fastcall MainWnd_OnChar(HWND hwnd, TCHAR ch, int cRepeat)
{
    WCHAR oldch = xg_xword.GetAt(xg_caret_pos);
    if (oldch == ZEN_BLACK && xg_bSolved)
        return;

    auto sa1 = std::make_shared<XG_UndoData_SetAt>();
    auto sa2 = std::make_shared<XG_UndoData_SetAt>();
    sa1->pos = sa2->pos = xg_caret_pos;
    sa1->ch = oldch;

    if (ch == L'_') {
        // ���E�B���h�E��j������B
        XgDestroyCandsWnd();
        if (!(xg_bSolved && oldch == ZEN_BLACK)) {
            sa2->ch = ZEN_SPACE;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ZEN_SPACE);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
        xg_prev_vk = 0;
    } else if (ch == L' ') {
        // ���E�B���h�E��j������B
        XgDestroyCandsWnd();
        {
            if (oldch == ZEN_SPACE && !xg_bSolved) {
                sa2->ch = ZEN_BLACK;
                xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
                xg_xword.SetAt(xg_caret_pos, ZEN_BLACK);

                XgEnsureCaretVisible(hwnd);
                XgUpdateImage(hwnd);
            } else if (oldch == ZEN_BLACK && !xg_bSolved) {
                sa2->ch = ZEN_SPACE;
                xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
                xg_xword.SetAt(xg_caret_pos, ZEN_SPACE);

                XgEnsureCaretVisible(hwnd);
                XgUpdateImage(hwnd);
            } else if (!xg_bSolved || !xg_bShowAnswer) {
                if (oldch != ZEN_BLACK) {
                    sa2->ch = ZEN_SPACE;
                    xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
                    xg_xword.SetAt(xg_caret_pos, ZEN_SPACE);

                    XgEnsureCaretVisible(hwnd);
                    XgUpdateImage(hwnd);
                }
            }
        }
        xg_prev_vk = 0;
    } else if (ch == L'#') {
        // ���E�B���h�E��j������B
        XgDestroyCandsWnd();

        if (!xg_bSolved) {
            sa2->ch = ZEN_BLACK;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ZEN_BLACK);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_ABC) {
        // �p�����͂̏ꍇ�B
        if (::GetAsyncKeyState(VK_CONTROL) < 0) {
            // [Ctrl]�L�[��������Ă���B
        } else if (XgIsCharHankakuUpperW(ch) || XgIsCharHankakuLowerW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();

            // �p����������啶���ɕϊ��B
            if (XgIsCharHankakuLowerW(ch)) {
                ch = ch + L'A' - L'a';
            }

            // ���p�p����S�p�p���ɕϊ��B
            ch = ZEN_LARGE_A + (ch - L'A');

            sa2->ch = ch;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        } else if (XgIsCharZenkakuUpperW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �S�p�啶���p�������͂��ꂽ�B
            sa2->ch = ch;

            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        } else if (XgIsCharZenkakuLowerW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �S�p�������p�������͂��ꂽ�B
            ch = ZEN_LARGE_A + (ch - ZEN_SMALL_A);

            sa2->ch = ch;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_KANA) {
        WCHAR newch = 0;
        // �J�i���͂̏ꍇ�B
        if (::GetAsyncKeyState(VK_CONTROL) < 0) {
            // [Ctrl]�L�[��������Ă���B
        }
        else if (XgIsCharHankakuUpperW(ch) || XgIsCharHankakuLowerW(ch))
        {
            ch = (WCHAR)(UINT_PTR)CharUpperW((LPWSTR)(UINT_PTR)ch);

            // ���[�}�����́B
            if (ch == L'A' || ch == L'I' || ch == L'U' ||
                ch == L'E' || ch == L'O' || ch == L'N')
            {
                switch (xg_prev_vk) {
                case 0:
                    switch (ch) {
                    case L'A': newch = ZEN_A; break;
                    case L'I': newch = ZEN_I; break;
                    case L'U': newch = ZEN_U; break;
                    case L'E': newch = ZEN_E; break;
                    case L'O': newch = ZEN_O; break;
                    case L'N': xg_prev_vk = ch; break;
                    }
                    break;
                case L'K':
                    switch (ch) {
                    case L'A': newch = ZEN_KA; break;
                    case L'I': newch = ZEN_KI; break;
                    case L'U': newch = ZEN_KU; break;
                    case L'E': newch = ZEN_KE; break;
                    case L'O': newch = ZEN_KO; break;
                    }
                    break;
                case L'S':
                    switch (ch) {
                    case L'A': newch = ZEN_SA; break;
                    case L'I': newch = ZEN_SI; break;
                    case L'U': newch = ZEN_SU; break;
                    case L'E': newch = ZEN_SE; break;
                    case L'O': newch = ZEN_SO; break;
                    }
                    break;
                case L'T':
                    switch (ch) {
                    case L'A': newch = ZEN_TA; break;
                    case L'I': newch = ZEN_CHI; break;
                    case L'U': newch = ZEN_TSU; break;
                    case L'E': newch = ZEN_TE; break;
                    case L'O': newch = ZEN_TO; break;
                    }
                    break;
                case L'N':
                    switch (ch) {
                    case L'A': newch = ZEN_NA; break;
                    case L'I': newch = ZEN_NI; break;
                    case L'U': newch = ZEN_NU; break;
                    case L'E': newch = ZEN_NE; break;
                    case L'O': newch = ZEN_NO; break;
                    case L'N': newch = ZEN_NN; break;
                    }
                    break;
                case L'H':
                    switch (ch) {
                    case L'A': newch = ZEN_HA; break;
                    case L'I': newch = ZEN_HI; break;
                    case L'U': newch = ZEN_FU; break;
                    case L'E': newch = ZEN_HE; break;
                    case L'O': newch = ZEN_HO; break;
                    }
                    break;
                case L'F':
                    switch (ch) {
                    case L'U': newch = ZEN_FU; break;
                    case L'O': newch = ZEN_HO; break;
                    }
                    break;
                case L'M':
                    switch (ch) {
                    case L'A': newch = ZEN_MA; break;
                    case L'I': newch = ZEN_MI; break;
                    case L'U': newch = ZEN_MU; break;
                    case L'E': newch = ZEN_ME; break;
                    case L'O': newch = ZEN_MO; break;
                    }
                    break;
                case L'Y':
                    switch (ch) {
                    case L'A': newch = ZEN_YA; break;
                    case L'I': newch = ZEN_I; break;
                    case L'U': newch = ZEN_YU; break;
                    case L'E': newch = ZEN_E; break;
                    case L'O': newch = ZEN_YO; break;
                    }
                    break;
                case L'R':
                    switch (ch) {
                    case L'A': newch = ZEN_RA; break;
                    case L'I': newch = ZEN_RI; break;
                    case L'U': newch = ZEN_RU; break;
                    case L'E': newch = ZEN_RE; break;
                    case L'O': newch = ZEN_RO; break;
                    }
                    break;
                case L'W':
                    switch (ch) {
                    case L'A': newch = ZEN_WA; break;
                    case L'I': newch = ZEN_WI; break;
                    case L'U': newch = ZEN_U; break;
                    case L'E': newch = ZEN_WE; break;
                    case L'O': newch = ZEN_WO; break;
                    }
                    break;
                case L'G':
                    switch (ch) {
                    case L'A': newch = ZEN_GA; break;
                    case L'I': newch = ZEN_GI; break;
                    case L'U': newch = ZEN_GU; break;
                    case L'E': newch = ZEN_GE; break;
                    case L'O': newch = ZEN_GO; break;
                    }
                    break;
                case L'Z':
                    switch (ch) {
                    case L'A': newch = ZEN_ZA; break;
                    case L'I': newch = ZEN_JI; break;
                    case L'U': newch = ZEN_ZU; break;
                    case L'E': newch = ZEN_ZE; break;
                    case L'O': newch = ZEN_ZO; break;
                    }
                    break;
                case L'J':
                    switch (ch) {
                    case L'I': newch = ZEN_JI; break;
                    case L'E': newch = ZEN_ZE; break;
                    }
                    break;
                case L'D':
                    switch (ch) {
                    case L'A': newch = ZEN_DA; break;
                    case L'I': newch = ZEN_DI; break;
                    case L'U': newch = ZEN_DU; break;
                    case L'E': newch = ZEN_DE; break;
                    case L'O': newch = ZEN_DO; break;
                    }
                    break;
                case L'B':
                    switch (ch) {
                    case L'A': newch = ZEN_BA; break;
                    case L'I': newch = ZEN_BI; break;
                    case L'U': newch = ZEN_BU; break;
                    case L'E': newch = ZEN_BE; break;
                    case L'O': newch = ZEN_BO; break;
                    }
                    break;
                case L'P':
                    switch (ch) {
                    case L'A': newch = ZEN_PA; break;
                    case L'I': newch = ZEN_PI; break;
                    case L'U': newch = ZEN_PU; break;
                    case L'E': newch = ZEN_PE; break;
                    case L'O': newch = ZEN_PO; break;
                    }
                    break;
                }
                if (newch)
                    xg_prev_vk = 0;
            } else if (xg_prev_vk == L'C' && ch == 'H') {
                xg_prev_vk = L'T';
            } else {
                xg_prev_vk = ch;
            }
        } else if (XgIsCharHiraganaW(ch)) {
            // �Ђ炪�Ȓ��ړ��́B
            WCHAR sz[2];
            ::LCMapStringW(JPN_LOCALE,
                LCMAP_FULLWIDTH | LCMAP_KATAKANA | LCMAP_UPPERCASE,
                &ch, 1, sz, ARRAYSIZE(sz));
            newch = sz[0];
            goto katakana;
        } else if (XgIsCharKatakanaW(ch)) {
katakana:;
            // �J�^�J�i���ړ��́B
            // �����Ȏ���傫�Ȏ��ɂ���B
            for (size_t i = 0; i < ARRAYSIZE(xg_small); i++) {
                if (static_cast<WCHAR>(ch) == xg_small[i][0]) {
                    newch = xg_large[i][0];
                    break;
                }
            }
            if (newch) {
                xg_prev_vk = 0;
            }
        } else if (ch == '-' || ch == ZEN_PROLONG) {
            newch = ZEN_PROLONG;
            xg_prev_vk = 0;
        } else {
            xg_prev_vk = 0;
        }

        if (newch) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();

            sa2->ch = newch;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, newch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_RUSSIA) {
        // ���V�A���͂̏ꍇ�B
        if (XgIsCharZenkakuCyrillicW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �L�����������ړ��́B
            sa2->ch = ch;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_DIGITS) {
        // �������͂̏ꍇ�B
        if (XgIsCharHankakuNumericW(ch))
            ch += 0xFF10 - L'0'; // �S�p�����ɂ���B
        if (XgIsCharZenkakuNumericW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �������ړ��́B
            sa2->ch = ch;
            xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
            xg_xword.SetAt(xg_caret_pos, ch);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgEnsureCaretVisible(hwnd);
            XgUpdateImage(hwnd);
        }
    }
}

// �L�[�������ꂽ�B
void __fastcall MainWnd_OnKey(HWND hwnd, UINT vk, bool fDown, int /*cRepeat*/, UINT /*flags*/)
{
    WCHAR ch;

    // ����̏����ɂ����āA�L�[���͂����ۂ���B
    if (!fDown)
        return;

    // ���z�L�[�R�[�h�ɉ����āA��������B
    switch (vk) {
    case VK_APPS:   // �A�v�� �L�[�B
        // �A�v�����j���[��\������B
        {
            HMENU hMenu = XgLoadPopupMenu(hwnd, 0);
            HMENU hSubMenu = GetSubMenu(hMenu, 0);

            INT nCellSize = xg_nCellSize * xg_nZoomRate / 100;

            // ���݂̃L�����b�g�ʒu�B
            POINT pt;
            pt.x = xg_nMargin + xg_caret_pos.m_j * nCellSize;
            pt.y = xg_nMargin + xg_caret_pos.m_i * nCellSize;
            pt.x += nCellSize / 2;
            pt.y += nCellSize / 2;
            pt.x -= XgGetHScrollPos();
            pt.y -= XgGetVScrollPos();

            // �c�[���o�[�����Ȃ�΁A�ʒu��␳����B
            RECT rc;
            if (::IsWindowVisible(xg_hToolBar)) {
                ::GetWindowRect(xg_hToolBar, &rc);
                pt.y += (rc.bottom - rc.top);
            }

            // �X�N���[�����W�ɕϊ�����B
            ::ClientToScreen(hwnd, &pt);

            // �E�N���b�N���j���[��\������B
            ::SetForegroundWindow(hwnd);
            ::TrackPopupMenu(
                hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                pt.x, pt.y, 0, hwnd, NULL);
            ::PostMessageW(hwnd, WM_NULL, 0, 0);

            ::DestroyMenu(hMenu);
        }
        xg_prev_vk = 0;
        break;

    case VK_PRIOR:  // PgUp�L�[�B
        ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
        xg_prev_vk = 0;
        break;

    case VK_NEXT:   // PgDn�L�[�B
        ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
        xg_prev_vk = 0;
        break;

    case VK_DELETE:
        // ���E�B���h�E��j������B
        XgDestroyCandsWnd();
        // ���݂̃L�����b�g�ʒu�̃}�X�̒��g����������B
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAt>();
            auto sa2 = std::make_shared<XG_UndoData_SetAt>();
            sa1->pos = sa2->pos = xg_caret_pos;
            sa1->ch = ch = xg_xword.GetAt(xg_caret_pos);

            if (ch != ZEN_SPACE && !xg_bSolved) {
                sa1->ch = ZEN_SPACE;
                xg_ubUndoBuffer.Commit(UC_SETAT, sa1, sa2);
                xg_xword.SetAt(xg_caret_pos, ZEN_SPACE);

                XgEnsureCaretVisible(hwnd);
                XgUpdateImage(hwnd);
            }
        }
        xg_prev_vk = 0;
        break;

    default:
        break;
    }
}

// IME���當�������͂��ꂽ�B
void __fastcall MainWnd_OnImeChar(HWND hwnd, WCHAR ch, LPARAM /*lKeyData*/)
{
    if (ch == ZEN_BLACK) {
        SendMessageW(hwnd, WM_CHAR, L'#', 1);
        return;
    }

    if (ch == ZEN_SPACE) {
        SendMessageW(hwnd, WM_CHAR, L' ', 1);
        return;
    }

    if (ch == L'_') {
        SendMessageW(hwnd, WM_CHAR, L'_', 1);
        return;
    }

    // ��������Ƃ��A���͏㏑���ł��Ȃ��B
    WCHAR oldch = xg_xword.GetAt(xg_caret_pos);
    if (xg_bSolved && oldch == ZEN_BLACK) {
        return;
    }

    if (xg_imode == xg_im_RUSSIA) {
        // ���V�A���̓��[�h�̏ꍇ�B
        // �L�����������H
        if (XgIsCharZenkakuCyrillicW(ch)) {
            CharUpperBuffW(&ch, 1);

            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            xg_xword.SetAt(xg_caret_pos, ch);
            XgEnsureCaretVisible(hwnd);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);
            
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_KANJI) {
        // �������̓��[�h�̏ꍇ�B
        // �������H
        if (XgIsCharKanjiW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            xg_xword.SetAt(xg_caret_pos, ch);
            XgEnsureCaretVisible(hwnd);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);
            
            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_KANA) {
        // �J�i���̓��[�h�̏ꍇ�B
        if (XgIsCharHiraganaW(ch)) {
            // �Ђ炪�ȓ��́B
            WCHAR sz[2];
            LCMapStringW(JPN_LOCALE,
                LCMAP_FULLWIDTH | LCMAP_KATAKANA | LCMAP_UPPERCASE,
                &ch, 1, sz, ARRAYSIZE(sz));
            ch = sz[0];
            goto katakana;
        } else if (XgIsCharKatakanaW(ch)) {
katakana:;
            // �J�^�J�i���́B
            // �����Ȏ���傫�Ȏ��ɂ���B
            for (size_t i = 0; i < ARRAYSIZE(xg_small); i++) {
                if (ch == xg_small[i][0]) {
                    ch = xg_large[i][0];
                    break;
                }
            }
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // ������ݒ肷��B
            xg_xword.SetAt(xg_caret_pos, ch);
            XgEnsureCaretVisible(hwnd);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_ABC) {
        if (XgIsCharHankakuUpperW(ch) || XgIsCharHankakuLowerW(ch) ||
            XgIsCharZenkakuUpperW(ch) || XgIsCharZenkakuLowerW(ch))
        {
            WCHAR sz[2];
            LCMapStringW(JPN_LOCALE,
                LCMAP_FULLWIDTH | LCMAP_KATAKANA | LCMAP_UPPERCASE,
                &ch, 1, sz, ARRAYSIZE(sz));
            ch = sz[0];

            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // ������ݒ肷��B
            xg_xword.SetAt(xg_caret_pos, ch);
            XgEnsureCaretVisible(hwnd);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgUpdateImage(hwnd);
        }
    } else if (xg_imode == xg_im_DIGITS) {
        // �������͂̏ꍇ�B
        if (XgIsCharHankakuNumericW(ch)) {
            ch += 0xFF10 - L'0'; // �S�p�����ɂ���B
        }
        if (XgIsCharZenkakuNumericW(ch)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // ������ݒ肷��B
            xg_xword.SetAt(xg_caret_pos, ch);
            XgEnsureCaretVisible(hwnd);

            if (xg_bCharFeed)
                XgCharFeed(hwnd);

            XgUpdateImage(hwnd);
        }
    }
}

// ���̓p���b�g�̏������B
BOOL InputPal_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    xg_hwndInputPalette = hwnd;
    XgInputDirection(hwnd, xg_bTateInput);
    XgSetCharFeed(hwnd, xg_bCharFeed);
    return FALSE;
}

// ���̓p���b�g�̃R�}���h�������B
void InputPal_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {
    case IDOK:
    case IDCANCEL:
    case IDCLOSE:
        xg_bShowInputPalette = false;
        DestroyWindow(hwnd);
        return;
    }

    HWND hButton = GetDlgItem(hwnd, id);

    WCHAR sz[10];
    GetWindowTextW(hButton, sz, 10);

    if (sz[0] && sz[1] == 0) {
        switch (sz[0]) {
        case ZEN_SPACE:
            MainWnd_OnImeChar(xg_hMainWnd, L'_', 0);
            break;
        case ZEN_UP:
            if (GetAsyncKeyState(VK_CONTROL) < 0) {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_MOSTUPPER, 0);
            } else {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_UP, 0);
            }
            break;
        case ZEN_DOWN:
            if (GetAsyncKeyState(VK_CONTROL) < 0) {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_MOSTLOWER, 0);
            } else {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_DOWN, 0);
            }
            break;
        case ZEN_LEFT:
            if (GetAsyncKeyState(VK_CONTROL) < 0) {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_MOSTLEFT, 0);
            } else {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_LEFT, 0);
            }
            break;
        case ZEN_RIGHT:
            if (GetAsyncKeyState(VK_CONTROL) < 0) {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_MOSTRIGHT, 0);
            } else {
                SendMessageW(xg_hMainWnd, WM_COMMAND, ID_RIGHT, 0);
            }
            break;
        default:
            MainWnd_OnImeChar(xg_hMainWnd, sz[0], 0);
            break;
        }
    } else {
        switch (id) {
        case 10093: // ����
            SendMessageW(xg_hMainWnd, WM_COMMAND, ID_ONLINEDICT, 0);
            break;
        case 20050: // ��������
            SendMessageW(xg_hMainWnd, WM_COMMAND, ID_CHARFEED, 0);
            break;
        case 20052: // ��/ֺ����
            SendMessageW(xg_hMainWnd, WM_COMMAND, ID_INPUTHV, 0);
            break;
        case 20054: // BS
            XgCharBack(xg_hMainWnd);
            break;
        case 20064: // NL
            XgReturn(xg_hMainWnd);
            break;
        case 20070: // �J�i��
            XgSetInputMode(xg_hMainWnd, xg_im_KANA);
            break;
        case 20071: // �c�u��/���u��
            if (xg_imode == xg_im_KANA) {
                xg_bTateOki = !xg_bTateOki;
                XgCreateInputPalette(xg_hMainWnd);
            }
            break;
        case 20072: // �p����
            XgSetInputMode(xg_hMainWnd, xg_im_ABC);
            break;
        case 20073: // ���V�A��
            XgSetInputMode(xg_hMainWnd, xg_im_RUSSIA);
            break;
        }
    }

    ::SetForegroundWindow(xg_hMainWnd);
}

void InputPal_OnDestroy(HWND hwnd)
{
    xg_hwndInputPalette = NULL;
}

// �L�[�������ꂽ�B
void InputPal_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (!fDown)
        return;

    switch (vk) {
    case VK_F6:
        if (::GetAsyncKeyState(VK_SHIFT) < 0)
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
        else
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
        break;

    case VK_ESCAPE:
        DestroyWindow(hwnd);
        break;
    }
}

void InputPal_OnMove(HWND hwnd, int x, int y)
{
    MRect rc;
    ::GetWindowRect(hwnd, &rc);
    xg_nInputPaletteWndX = rc.left;
    xg_nInputPaletteWndY = rc.top;
}

// ���̓p���b�g�̃_�C�A���O�v���V�W���[�B
INT_PTR CALLBACK
XgInputPaletteDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, InputPal_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, InputPal_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, InputPal_OnDestroy);
        HANDLE_MSG(hwnd, WM_KEYDOWN, InputPal_OnKey);
        HANDLE_MSG(hwnd, WM_MOVE, InputPal_OnMove);
    }
    return 0;
}

// ���̓p���b�g��j������B
BOOL XgDestroyInputPalette(void)
{
    xg_bShowInputPalette = false;
    if (xg_hwndInputPalette) {
        ::DestroyWindow(xg_hwndInputPalette);
        xg_hwndInputPalette = NULL;
        return TRUE;
    }
    return FALSE;
}

// ���̓p���b�g���쐬����B
BOOL XgCreateInputPalette(HWND hwndOwner)
{
    XgDestroyInputPalette();

    switch (xg_imode) {
    case xg_im_ABC:
        if (xg_bLowercase) {
            CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_ABCLOWER), hwndOwner,
                          XgInputPaletteDlgProc);
        } else {
            CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_ABC), hwndOwner,
                          XgInputPaletteDlgProc);
        }
        break;
    case xg_im_KANA:
        if (xg_bTateOki) {
            if (xg_bHiragana) {
                CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_HIRATATE), hwndOwner,
                              XgInputPaletteDlgProc);
            } else {
                CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_KATATATE), hwndOwner,
                              XgInputPaletteDlgProc);
            }
        } else {
            if (xg_bHiragana) {
                CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_HIRAYOKO), hwndOwner,
                              XgInputPaletteDlgProc);
            } else {
                CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_KATAYOKO), hwndOwner,
                              XgInputPaletteDlgProc);
            }
        }
        break;
    case xg_im_RUSSIA:
        if (xg_bLowercase) {
            CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_RUSSIALOWER), hwndOwner,
                          XgInputPaletteDlgProc);
        } else {
            CreateDialogW(xg_hInstance, MAKEINTRESOURCEW(IDD_RUSSIA), hwndOwner,
                          XgInputPaletteDlgProc);
        }
        break;
    case xg_im_DIGITS:
        // TODO: �������̓p���b�g��ǉ�����B
    default:
        return FALSE;
    }

    MRect rc;
    GetWindowRect(xg_hwndInputPalette, &rc);
    if (xg_nInputPaletteWndX != CW_USEDEFAULT) {
        MoveWindow(xg_hwndInputPalette,
            xg_nInputPaletteWndX, xg_nInputPaletteWndY,
            rc.Width(), rc.Height(), TRUE);
        SendMessageW(xg_hwndInputPalette, DM_REPOSITION, 0, 0);
    }

    ShowWindow(xg_hwndInputPalette, SW_SHOWNOACTIVATE);
    UpdateWindow(xg_hwndInputPalette);
    xg_bShowInputPalette = true;

    return xg_hwndInputPalette != NULL;
}

// ���̓��[�h��؂�ւ���B
void __fastcall XgSetInputMode(HWND hwnd, XG_InputMode mode)
{
    bool flag = (xg_imode != mode);

    xg_imode = mode;

    if (flag && xg_hwndInputPalette) {
        XgCreateInputPalette(hwnd);
    }

    XgUpdateStatusBar(hwnd);
    XgUpdateImage(hwnd, 0, 0);
}

void __fastcall XgSetInputModeFromDict(HWND hwnd)
{
    WCHAR ch;
    if (xg_dict_1.size()) {
        ch = xg_dict_1[0].m_word[0];
    } else if (xg_dict_2.size()) {
        ch = xg_dict_2[0].m_word[0];
    } else {
        return;
    }

    if (XgIsCharHiraganaW(ch) || XgIsCharKatakanaW(ch)) {
        XgSetInputMode(hwnd, xg_im_KANA);
    } else if (XgIsCharKanjiW(ch)) {
        XgSetInputMode(hwnd, xg_im_KANJI);
    } else if (XgIsCharZenkakuCyrillicW(ch)) {
        XgSetInputMode(hwnd, xg_im_RUSSIA);
    } else if (XgIsCharZenkakuNumericW(ch) || XgIsCharHankakuNumericW(ch)) {
        XgSetInputMode(hwnd, xg_im_DIGITS);
    } else if (XgIsCharZenkakuUpperW(ch) || XgIsCharZenkakuLowerW(ch) ||
               XgIsCharHankakuUpperW(ch) || XgIsCharHankakuLowerW(ch))
    {
        XgSetInputMode(hwnd, xg_im_ABC);
    }
}
