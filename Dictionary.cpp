//////////////////////////////////////////////////////////////////////////////
// Dictionary.cpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#include "XWordGiver.hpp"
#include "Auto.hpp"

// �����f�[�^�B
std::vector<XG_WordData>     xg_dict_data;

//////////////////////////////////////////////////////////////////////////////
// �����f�[�^�̃t�@�C�������B

// Unicode����s�ǂݍ��ށB
void XgReadUnicodeLine(LPWSTR pchLine)
{
    XG_WordData entry;
    WCHAR szWord[64];

    // �R�����g�s�𖳎�����B
    if (*pchLine == L'#') {
        return;
    }

    // ��s�𖳎�����B
    if (*pchLine == L'\0') {
        return;
    }

    // �q���g���������B
    LPWSTR pchHint = wcschr(pchLine, L'\t');
    if (pchHint) {
        *pchHint = 0;
        pchHint++;
    } else {
        pchHint = nullptr;
    }

    // ��R�t�B�[���h�ȍ~�𖳎�����B
    if (pchHint) {
        if (LPWSTR pch = wcschr(pchHint, L'\t'))
            *pch = 0;
    }

    // �P�ꕶ�����S�p�E�J�^�J�i�E�啶���ɂ���B
    LCMapStringW(JPN_LOCALE,
        LCMAP_FULLWIDTH | LCMAP_KATAKANA | LCMAP_UPPERCASE,
        pchLine, static_cast<int>(wcslen(pchLine) + 1), szWord, 64);

    // ������̑O��̋󔒂���菜���B
    entry.m_word = szWord;
    xg_str_trim(entry.m_word);

    // �����Ȏ���傫�Ȏ��ɂ���B
    for (size_t i = 0; i < ARRAYSIZE(xg_large); i++)
        xg_str_replace_all(entry.m_word,
            std::wstring(xg_small[i]), std::wstring(xg_large[i]));

    // �P��ƃq���g��o�^����B�ꎚ�ȉ��̒P��͓o�^���Ȃ��B
    if (entry.m_word.size() > 1) {
        if (pchHint) {
            entry.m_hint = pchHint;
            xg_str_trim(entry.m_hint);
        }
        else
            entry.m_hint.clear();

        xg_dict_data.emplace_back(std::move(entry));
    }
}

// Unicode�̃t�@�C���̒��g��ǂݍ��ށB
bool XgReadUnicodeFile(LPWSTR pszData, DWORD /*cchData*/)
{
    // �ŏ��̈�s�����o���B
    LPWSTR pchLine = wcstok(pszData, xg_pszNewLine);
    if (pchLine == nullptr)
        return false;

    // ��s����������B
    do {
        XgReadUnicodeLine(pchLine);
        pchLine = wcstok(nullptr, xg_pszNewLine);
    } while (pchLine);
    return true;
}

// ANSI (Shift_JIS) �̃t�@�C���̒��g��ǂݍ��ށB
bool __fastcall XgReadAnsiFile(LPCSTR pszData, DWORD /*cchData*/)
{
    // Unicode�ɕϊ��ł��Ȃ��Ƃ��͎��s�B
    int cchWide = MultiByteToWideChar(SJIS_CODEPAGE, 0, pszData, -1, nullptr, 0);
    if (cchWide == 0)
        return false;

    // Unicode�ɕϊ����ď�������B
    std::wstring strWide(cchWide - 1, 0);
    MultiByteToWideChar(SJIS_CODEPAGE, 0, pszData, -1, &strWide[0], cchWide);
    return XgReadUnicodeFile(&strWide[0], cchWide - 1);
}

// UTF-8�̃t�@�C���̒��g��ǂݍ��ށB
bool __fastcall XgReadUtf8File(LPCSTR pszData, DWORD /*cchData*/)
{
    // Unicode�ɕϊ��ł��Ȃ��Ƃ��͎��s�B
    int cchWide = MultiByteToWideChar(CP_UTF8, 0, pszData, -1, nullptr, 0);
    if (cchWide == 0)
        return false;

    // Unicode�ɕϊ����ď�������B
    std::wstring strWide(cchWide - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, pszData, -1, &strWide[0], cchWide);
    return XgReadUnicodeFile(&strWide[0], cchWide - 1);
}

// �����t�@�C����ǂݍ��ށB
bool __fastcall XgLoadDictFile(LPCWSTR pszFile)
{
    DWORD cbRead, i;

    // ����������B
    xg_dict_data.clear();

    // �t�@�C�����J���B
    AutoCloseHandle hFile(CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                      OPEN_EXISTING, 0, nullptr));
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    // �t�@�C���T�C�Y���擾����B
    DWORD cbFile = ::GetFileSize(hFile, nullptr);
    if (cbFile == 0xFFFFFFFF)
        return false;

    try {
        // ���������m�ۂ��ăt�@�C������ǂݍ��ށB
        std::vector<BYTE> pbFile(cbFile + 4, 0);
        i = cbFile;
        if (!ReadFile(hFile, &pbFile[0], cbFile, &cbRead, nullptr))
            return false;

        // BOM�`�F�b�N�B
        if (pbFile[0] == 0xFF && pbFile[1] == 0xFE) {
            // Unicode
            std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[2]);
            if (!XgReadUnicodeFile(&str[0], static_cast<DWORD>(str.size())))
                return false;
            i = 0;
        } else if (pbFile[0] == 0xFE && pbFile[1] == 0xFF) {
            // Unicode BE
            XgSwab(&pbFile[0], cbFile);
            std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[2]);
            if (!XgReadUnicodeFile(&str[0], static_cast<DWORD>(str.size())))
                return false;
            i = 0;
        } else if (pbFile[0] == 0xEF && pbFile[1] == 0xBB && pbFile[2] == 0xBF) {
            // UTF-8
            std::wstring str = XgUtf8ToUnicode(reinterpret_cast<LPCSTR>(&pbFile[3]));
            if (!XgReadUnicodeFile(&str[0], static_cast<DWORD>(str.size())))
                return false;
            i = 0;
        } else {
            for (i = 0; i < cbFile; i++) {
                // �i�������������Unicode�Ɣ��f����B
                if (pbFile[i] == 0) {
                    if (i & 1) {
                        // Unicode
                        if (!XgReadUnicodeFile(reinterpret_cast<LPWSTR>(&pbFile[0]), cbFile / 2))
                            return false;
                    } else {
                        // Unicode BE
                        XgSwab(&pbFile[0], cbFile);
                        if (!XgReadUnicodeFile(reinterpret_cast<LPWSTR>(&pbFile[0]), cbFile / 2))
                            return false;
                    }
                    i = 0;
                    break;
                }
            }
        }

        if (i == cbFile) {
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                    reinterpret_cast<LPCSTR>(pbFile[0]),
                                    static_cast<int>(cbFile), nullptr, 0))
            {
                // UTF-8
                if (!XgReadUtf8File(reinterpret_cast<LPSTR>(&pbFile[0]), cbFile))
                    return false;
            } else {
                // ANSI
                if (!XgReadAnsiFile(reinterpret_cast<LPSTR>(&pbFile[0]), cbFile))
                    return false;
            }
        }

        // �񕪒T���̂��߂ɁA���ёւ��Ă����B
        std::sort(xg_dict_data.begin(), xg_dict_data.end(), xg_word_less());
        return true; // �����B
    } catch (...) {
        // ��O�����������B
    }

    return false; // ���s�B
}

// �����f�[�^���\�[�g���A��ӓI�ɂ���B
void __fastcall XgSortAndUniqueDictData(void)
{
    sort(xg_dict_data.begin(), xg_dict_data.end(), xg_word_less());
    auto last = unique(xg_dict_data.begin(), xg_dict_data.end(),
                       XG_WordData_Equal());
    std::vector<XG_WordData> dict_data;
    for (auto it = xg_dict_data.begin(); it != last; ++it) {
        dict_data.emplace_back(*it);
    }
    xg_dict_data = std::move(dict_data);
}

// �~�j�������쐬����B
std::vector<XG_WordData> XgCreateMiniDict(void)
{
    std::vector<XG_WordData> ret;
    for (const auto& hint : xg_vecTateHints)
    {
        ret.emplace_back(hint.m_strWord, hint.m_strHint);
    }
    for (const auto& hint : xg_vecYokoHints)
    {
        ret.emplace_back(hint.m_strWord, hint.m_strHint);
    }
    std::sort(ret.begin(), ret.end(),
        [](const XG_WordData& a, const XG_WordData& b) {
            return a.m_word < b.m_word;
        }
    );
    return ret;
}
