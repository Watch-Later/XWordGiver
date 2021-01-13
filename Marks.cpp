//////////////////////////////////////////////////////////////////////////////
// Marks.cpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#include "XWordGiver.hpp"

//////////////////////////////////////////////////////////////////////////////
// global variables

// �}�[�N�B
std::vector<XG_Pos>      xg_vMarks;

// ��d�}�X�P����B
std::vector<std::wstring>     xg_vMarkedCands;

//////////////////////////////////////////////////////////////////////////////
// static variables

// �I�����Ă����d�}�X�P����̃C���f�b�N�X�B
static int  s_iMarkedCands = -1;

//////////////////////////////////////////////////////////////////////////////

// �}�[�N��������擾����B
void __fastcall XgGetStringOfMarks(std::wstring& str)
{
    WCHAR sz[64];
    str.clear();
    str += XgLoadStringDx1(IDS_HLINE);
    str += xg_pszNewLine;
    for (const auto& mark : xg_vMarks) {
        if (xg_bSolved)
            StringCbPrintf(sz, sizeof(sz), L"(%u, %u)%c\r\n",
                mark.m_i + 1, mark.m_j + 1,
                xg_solution.GetAt(mark.m_i, mark.m_j));
        else
            StringCbPrintf(sz, sizeof(sz), L"(%u, %u)%c\r\n",
                mark.m_i + 1, mark.m_j + 1,
                xg_xword.GetAt(mark.m_i, mark.m_j));
        str += sz;
    }
}

// �}�[�N����Ă��邩�i��d�}�X�j�H
int __fastcall XgGetMarked(const std::vector<XG_Pos>& vMarks, const XG_Pos& pos)
{
    const int size = static_cast<int>(vMarks.size());
    for (int i = 0; i < size; i++) {
        if (vMarks[i] == pos)
            return i;
    }
    return -1;
}

// �}�[�N����Ă��邩�i��d�}�X�j�H
int __fastcall XgGetMarked(const std::vector<XG_Pos>& vMarks, int i, int j)
{
    return XgGetMarked(vMarks, XG_Pos(i, j));
}

// �}�[�N����Ă��邩�i��d�}�X�j�H
int __fastcall XgGetMarked(int i, int j)
{
    return XgGetMarked(xg_vMarks, XG_Pos(i, j));
}

// ��d�}�X���X�V���ꂽ�B
void __fastcall XgMarkUpdate(void)
{
    WCHAR sz[64];
    std::wstring str;

    // ���łɉ������邩�ǂ����ɂ���Đ؂�ւ��B
    const XG_Board *xw = (xg_bSolved ? &xg_solution : &xg_xword);

    // �t�@�C���������邩�H
    if (xg_strFileName.size() || PathFileExistsW(xg_strFileName.c_str())) {
        // �}�[�N����Ă��邩�H ������\�����邩�H
        LPWSTR pchFileTitle = PathFindFileNameW(xg_strFileName.c_str());
        if (XgGetMarkWord(xw, str) && xg_bShowAnswer) {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_APPTITLE2), str.data(), pchFileTitle);
            ::SetWindowTextW(xg_hMainWnd, sz);
        } else {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_APPTITLE), pchFileTitle);
            ::SetWindowTextW(xg_hMainWnd, sz);
        }
    } else {
        // �}�[�N����Ă��邩�H ������\�����邩�H
        if (XgGetMarkWord(xw, str) && xg_bShowAnswer) {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_APPINFO2), str.data());
            ::SetWindowTextW(xg_hMainWnd, sz);
        } else {
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_APPINFO));
            ::SetWindowTextW(xg_hMainWnd, sz);
        }
    }
}

// �w��̃}�X�Ƀ}�[�N����i��d�}�X�j�B
void __fastcall XgSetMark(const XG_Pos& pos)
{
    for (const auto& markpos : xg_vMarks) {
        if (markpos == pos)
            return;
    }
    xg_vMarks.emplace_back(pos);

    // �}�[�N�̍X�V��ʒm����B
    XgMarkUpdate();
}

// �w��̃}�X�Ƀ}�[�N����i��d�}�X�j�B
void __fastcall XgSetMark(int i, int j)
{
    XgSetMark(XG_Pos(i, j));
}

// �w��̃}�X�̃}�[�N�i��d�}�X�j����������B
void __fastcall XgDeleteMark(int i, int j)
{
    // (i, j)���}�[�N����Ă��Ȃ���Ζ����B
    int nMarked = XgGetMarked(i, j);
    if (nMarked == -1)
        return;

    // ��d�}�X�������B
    xg_vMarks.erase(xg_vMarks.begin() + nMarked);

    // �}�[�N�̍X�V��ʒm����B
    XgMarkUpdate();
}

// �}�[�N�������ݒ肷��B
void __fastcall XgSetStringOfMarks(LPCWSTR psz)
{
    // ����������B
    xg_vMarkedCands.clear();
    xg_vMarks.clear();

    // ��؂�������邩�H
    LPCWSTR pch = XgLoadStringDx1(IDS_HLINE);
    psz = wcsstr(psz, pch);
    if (psz == nullptr) {
        // ��؂�����Ȃ���Γ�d�}�X�̏��͂Ȃ��B
        return;
    }
    psz += wcslen(pch);

    // �}�[�N���������͂���B
    int count = 0;
    while (*psz != L'\0' && *psz != ZEN_ULEFT) {
        // 1000�����ȏ�͂��肦�Ȃ��B
        if (count++ > 1000)
            break;

        // �ǂݔ�΂��B
        while (*psz == L' ' || *psz == L'\r' || *psz == L'\n' || *psz == L'(')
            psz++;
        if (*psz == L'\0' || *psz == ZEN_ULEFT)
            break;

        // �������Ȃ���ΏI���B
        if (!(L'0' <= *psz && *psz <= L'9'))
            break;

        // �s�̃C���f�b�N�X��ǂݍ��ށB
        int i = _wtoi(psz) - 1;
        while (L'0' <= *psz && *psz <= L'9')
            psz++;
        if (*psz == L'\0' || *psz == ZEN_ULEFT)
            break;

        // �ǂݔ�΂��B
        while (*psz == L' ' || *psz == L',')
            psz++;

        // �������Ȃ���ΏI���B
        if (!(L'0' <= *psz && *psz <= L'9'))
            break;

        // ��̃C���f�b�N�X��ǂݍ��ށB
        int j = _wtoi(psz) - 1;

        // �ǂݔ�΂��B
        while ((L'0' <= *psz && *psz <= L'9') || *psz == L')')
            psz++;
        while (XgIsCharKanaW(*psz) || XgIsCharHankakuAlphaW(*psz) ||
               XgIsCharZenkakuAlphaW(*psz) || XgIsCharKanjiW(*psz) ||
               XgIsCharHangulW(*psz))
        {
            psz++;
        }

        // �I���ł���ΏI���B
        if (*psz == L'\0' || *psz == ZEN_ULEFT)
            break;

        // �}�[�N��ݒ肷��B
        XgSetMark(i, j);
    }
}

// ��d�}�X�P����擾����B
bool __fastcall XgGetMarkWord(const XG_Board *xw, std::wstring& str)
{
    // ����������B
    str.clear();

    // ��d�}�X�P�ꂪ���邩�H
    if (xg_vMarks.size()) {
        bool bExists = true;

        // ��d�}�X�P����\�z����B
        for (const auto& mark : xg_vMarks) {
            const WCHAR ch = xw->GetAt(mark.m_i, mark.m_j);
            // �󔒃}�X�����}�X�ł���΁A��d�}�X�P��͂Ȃ��B
            if (ch == ZEN_SPACE || ch == ZEN_BLACK)
                bExists = false;
            str += ch;
        }
        return bExists;     // ���݂��邩�H
    }
    return false;   // ���s�B
}

// ��d�}�X�P������擾����B
bool __fastcall XgGetMarkedCandidates(void)
{
    // ����������B
    s_iMarkedCands = -1;
    xg_vMarkedCands.clear();

    // �����Ȃ���Ύ��s�B
    if (!xg_bSolved)
        return false;

    // �}�X�̕������}���`�Z�b�g�ɕϊ��B
    std::unordered_multiset<WCHAR> msCells;
    xg_vec_to_multiset(msCells, xg_solution.m_vCells);

    // xg_dict_1�ɓo�^����Ă���P��ɂ��ČJ��Ԃ��B
    for (const auto& data : xg_dict_1) {
        // �P������o���B2�����ȉ��͖����B
        const std::wstring& word = data.m_word;
        if (word.size() <= 2)
            continue;

        // �P����}���`�Z�b�g�֕ϊ��B
        std::unordered_multiset<WCHAR> ms;
        xg_str_to_multiset(ms, word);

        // �����}���`�Z�b�g�ɂȂ��Ă��邩�H
        if (xg_submultiseteq(ms, msCells)) {
            // �z�u�𒲂ׂ�B
            std::vector<XG_Pos> vPos;
            for (size_t k = 0; k < word.size(); k++) {
                for (int i = 0; i < xg_nRows; ++i) {
                    for (int j = 0; j < xg_nCols; ++j) {
                        if (word[k] == xg_solution.GetAt(i, j) &&
                            XgGetMarked(vPos, i, j) == -1)
                        {
                            vPos.emplace_back(i, j);
                            goto break2;
                        }
                    }
                }
break2:;
            }

            // �ׂ荇���z�u������Ύ��s�B
            const int size = static_cast<int>(vPos.size());
            for (int i = 0; i < size - 1; i++) {
                for (int j = i + 1; j < size; j++) {
                    if (vPos[i].m_i == vPos[j].m_i) {
                        if (vPos[i].m_j + 1 == vPos[j].m_j ||
                            vPos[i].m_j == vPos[j].m_j + 1)
                        {
                            goto failed;
                        }
                    }
                    if (vPos[i].m_j == vPos[j].m_j)
                    {
                        if (vPos[i].m_i + 1 == vPos[j].m_i ||
                            vPos[i].m_i == vPos[j].m_i + 1)
                        {
                            goto failed;
                        }
                    }
                }
            }
            xg_vMarkedCands.emplace_back(word);
failed:;
        }
    }

    // xg_dict_2�ɓo�^����Ă���P��ɂ��ČJ��Ԃ��B
    for (const auto& data : xg_dict_2) {
        // �P������o���B2�����ȉ��͖����B
        const std::wstring& word = data.m_word;
        if (word.size() <= 2)
            continue;

        // �P����}���`�Z�b�g�֕ϊ��B
        std::unordered_multiset<WCHAR> ms;
        xg_str_to_multiset(ms, word);

        // �����}���`�Z�b�g�ɂȂ��Ă��邩�H
        if (xg_submultiseteq(ms, msCells)) {
            // �z�u�𒲂ׂ�B
            std::vector<XG_Pos> vPos;
            for (size_t k = 0; k < word.size(); k++) {
                for (int i = 0; i < xg_nRows; ++i) {
                    for (int j = 0; j < xg_nCols; ++j) {
                        if (word[k] == xg_solution.GetAt(i, j) &&
                            XgGetMarked(vPos, i, j) == -1)
                        {
                            vPos.emplace_back(i, j);
                            goto break2_2;
                        }
                    }
                }
break2_2:;
            }

            // �ׂ荇���z�u������Ύ��s�B
            const int size = static_cast<int>(vPos.size());
            for (int i = 0; i < size - 1; i++) {
                for (int j = i + 1; j < size; j++) {
                    if (vPos[i].m_i == vPos[j].m_i) {
                        if (vPos[i].m_j + 1 == vPos[j].m_j ||
                            vPos[i].m_j == vPos[j].m_j + 1)
                        {
                            goto failed_2;
                        }
                    }
                    if (vPos[i].m_j == vPos[j].m_j)
                    {
                        if (vPos[i].m_i + 1 == vPos[j].m_i ||
                            vPos[i].m_i == vPos[j].m_i + 1)
                        {
                            goto failed_2;
                        }
                    }
                }
            }
            xg_vMarkedCands.emplace_back(word);
failed_2:;
        }
    }

    sort(xg_vMarkedCands.begin(), xg_vMarkedCands.end(), xg_wstring_size_greater());
    return !xg_vMarkedCands.empty();
}

// ��d�}�X�P���ݒ肷��B
void __fastcall XgSetMarkedWord(const std::wstring& str)
{
    // ����������B
    xg_vMarks.clear();

    // ��d�}�X�P��ƕ����}�X�̏��ɏ]���ē�d�}�X��ݒ肷��B
    for (const auto ch : str) {
        int m = rand() % xg_nRows;
        int n = rand() % xg_nCols;
        for (int i = 0; i < xg_nRows; i++) {
            for (int j = 0; j < xg_nCols; j++) {
                int i0 = (i + m) % xg_nRows;
                int j0 = (j + n) % xg_nCols;
                if (ch == xg_solution.GetAt(i0, j0) &&
                    XgGetMarked(i0, j0) == -1)
                {
                    XgSetMark(i0, j0);
                    goto break2;
                }
            }
        }
break2:;
    }

    // �C���[�W���X�V����B
    XgUpdateImage(xg_hMainWnd, 0, 0);
}

// ��d�}�X�P�����ɂ���B
void __fastcall XgSetMarkedWord(void)
{
    // �󕶎����ݒ肷��B
    std::wstring str;
    XgSetMarkedWord(str);
}

// ���̓�d�}�X�P����擾����B
void __fastcall XgGetNextMarkedWord(void)
{
    if (xg_vMarkedCands.empty()) {
        // ��d�}�X�P���₪�Ȃ��ꍇ�A�����擾����B
        if (!XgGetMarkedCandidates()) {
            // ��₪���������B
            XgCenterMessageBoxW(xg_hMainWnd, XgLoadStringDx1(IDS_NOMARKCANDIDATES), NULL,
                                MB_ICONERROR);
            return;     // ���s�B
        }

        // ��d�}�X�P��̌����擾�����B�ŏ��̌���ݒ肷��B
        s_iMarkedCands = 0;
    }
    else if (s_iMarkedCands + 1 < static_cast<int>(xg_vMarkedCands.size()))
    {
        // ���̌���ݒ肷��B
        s_iMarkedCands++;
    }

    // ��d�}�X�P���ݒ肷��B
    XgSetMarkedWord(xg_vMarkedCands[s_iMarkedCands]);
}

// �O�̓�d�}�X�P����擾����B
void __fastcall XgGetPrevMarkedWord(void)
{
    if (xg_vMarkedCands.empty()) {
        // ��d�}�X�P���₪�Ȃ��ꍇ�A�����擾����B
        if (!XgGetMarkedCandidates()) {
            // ��₪���������B
            XgCenterMessageBoxW(xg_hMainWnd, XgLoadStringDx1(IDS_NOMARKCANDIDATES), NULL,
                                MB_ICONERROR);
            return;
        }

        // ��d�}�X�P��̌����擾�����B�ŏ��̌���ݒ肷��B
        s_iMarkedCands = 0;
    } else if (s_iMarkedCands > 0) {
        // ��O�̌���ݒ肷��B
        s_iMarkedCands--;
    }

    // ��d�}�X�P���ݒ肷��B
    XgSetMarkedWord(xg_vMarkedCands[s_iMarkedCands]);
}

//////////////////////////////////////////////////////////////////////////////
