#include "helper.h"
#include <Windows.h>
#include <vector>

uintptr_t helper::FindPattern(uintptr_t pModuleBaseAddress, const char* szSignature, size_t nSelectResultIndex) {
    auto PatternToBytes = [](const char* szpattern) {
        auto       m_iBytes = std::vector<int>{};
        const auto szStartAddr = const_cast<char*>(szpattern);
        const auto szEndAddr = const_cast<char*>(szpattern) + strlen(szpattern);

        for (auto szCurrentAddr = szStartAddr; szCurrentAddr < szEndAddr; ++szCurrentAddr) {
            if (*szCurrentAddr == '?') {
                ++szCurrentAddr;
                if (*szCurrentAddr == '?') ++szCurrentAddr;
                m_iBytes.push_back(-1);
            }
            else m_iBytes.push_back(strtoul(szCurrentAddr, &szCurrentAddr, 16));
        }
        return m_iBytes;
        };

    const auto pDosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
    const auto pNTHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + pDosHeader->e_lfanew);
    const auto dwSizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
    auto       m_iPatternBytes = PatternToBytes(szSignature);
    const auto pScanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);
    const auto m_iPatternBytesSize = m_iPatternBytes.size();
    const auto m_iPatternBytesData = m_iPatternBytes.data();
    size_t nFoundResults = 0;

    for (auto i = 0ul; i < dwSizeOfImage - m_iPatternBytesSize; ++i) {
        bool bFound = true;

        for (auto j = 0ul; j < m_iPatternBytesSize; ++j) {
            if (pScanBytes[i + j] != m_iPatternBytesData[j] && m_iPatternBytesData[j] != -1) {
                bFound = false;
                break;
            }
        }

        if (bFound) {
            if (nSelectResultIndex != 0) {
                if (nFoundResults < nSelectResultIndex) {
                    nFoundResults++;
                    bFound = false;
                }
                else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
            }
            else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
        }
    }
    return NULL;
}
std::uint8_t* helper:: ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset)
{
    std::uint32_t nRVA = *reinterpret_cast<std::uint32_t*>(nAddressBytes + nRVAOffset);
    std::uint64_t nRIP = reinterpret_cast<std::uint64_t>(nAddressBytes) + nRIPOffset;

    return reinterpret_cast<std::uint8_t*>(nRVA + nRIP);
}
std::uint8_t* helper::GetAbsoluteAddress(std::uint8_t* pRelativeAddress, int nPreOffset, int nPostOffset)
{
    pRelativeAddress += nPreOffset;
    pRelativeAddress += sizeof(std::int32_t) + *reinterpret_cast<std::int32_t*>(pRelativeAddress);
    pRelativeAddress += nPostOffset;
    return pRelativeAddress;
}
