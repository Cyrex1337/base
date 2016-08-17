#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

//learn_more
#define InRange(x, a, b)	(x >= a && x <= b)
#define getBits(x)			(InRange(x, '0', '9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xA))
#define getByte(x)			(getBits(x[0]) << 4 | getBits(x[1]))

class Module
{
public:
	Module( ) = default;
	Module( BYTE* baseAddress, DWORD size, HANDLE baseHandle, const char* name );

public:
	uint32_t getBaseAddress( ) const;
	uint32_t getSize( ) const;
	const char* getName( ) const;
	uint32_t findPattern( const char* pattern ) const;
	uint32_t findUnformattedPattern( const char* pattern, const char* hitMask ) const;

private:
	uint32_t baseAddress = 0;
	uint32_t size = 0;
	HANDLE baseHandle = 0;
	std::string moduleName;
};