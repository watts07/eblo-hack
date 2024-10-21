#pragma once
#include <Windows.h>
#include <vector>

namespace helper {
	std::uint8_t* ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset);
	std::uint8_t* GetAbsoluteAddress(std::uint8_t* pRelativeAddress, int nPreOffset, int nPostOffset);
	uintptr_t FindPattern(uintptr_t pModuleBaseAddress, const char* szSignature, size_t nSelectResultIndex = 0);
}