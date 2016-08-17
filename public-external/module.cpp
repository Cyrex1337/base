#include "module.hpp"
#include "process.hpp"

Module::Module( BYTE* baseAddress, DWORD size, HANDLE baseHandle, const char* name )
{
	this->baseAddress = reinterpret_cast< uint32_t >( baseAddress );
	this->size = size;
	this->baseHandle = baseHandle;
	this->moduleName = std::string( name );
}

uint32_t Module::getBaseAddress( ) const
{
	return this->baseAddress;
}

uint32_t Module::getSize( ) const
{
	return this->size;
}

const char* Module::getName( ) const
{
	return this->moduleName.c_str( );
}

//learn_more
// "A1 ? ? ? ?"
uint32_t Module::findPattern( const char* pattern ) const
{
	unsigned char* memoryRegion = static_cast< unsigned char* >( malloc( this->size ) );
	ReadProcessMemory( this->baseHandle, reinterpret_cast< LPCVOID >( this->baseAddress ), memoryRegion, this->size, nullptr );

	PBYTE firstMatch = 0;
	const unsigned char* pat = reinterpret_cast< const unsigned char* >( pattern );
	for ( PBYTE pCur = memoryRegion; pCur < memoryRegion + this->size; pCur++ )
	{
		if ( *pCur == (BYTE)'\?' || *pCur == getByte( pat ) )
		{
			if ( !firstMatch )
				firstMatch = pCur;

			pat += ( *pat == ( BYTE )'\?' ) ? 2 : 3;

			if ( !*pat )
			{
				free( memoryRegion );
				return ( uint32_t )( firstMatch - memoryRegion + this->baseAddress );
			}
		}
		else if ( firstMatch )
		{
			pCur = firstMatch;
			pat = reinterpret_cast< const unsigned char* >( pattern );
			firstMatch = 0;
		}
	}

	free( memoryRegion );
	return 0;
}

// "\xA1\x00\x00\x00\x00"
// "x???"
uint32_t Module::findUnformattedPattern( const char* pattern, const char* hitMask ) const
{
	unsigned char* memoryRegion = static_cast< unsigned char* >( malloc( this->size ) );
	ReadProcessMemory( this->baseHandle, reinterpret_cast< LPCVOID >( this->baseAddress ), memoryRegion, this->size, nullptr );

	for ( PBYTE pCur = memoryRegion; pCur < memoryRegion + this->size; pCur++ )
	{
		PBYTE data = pCur;
		PBYTE pat = ( PBYTE )pattern;
		const char* mask = hitMask;

		for ( ; pat < ( PBYTE )pattern + strlen( hitMask ); pat++, data++, ++mask )
		{
			if ( !*mask )
			{
				free( memoryRegion );
				return pCur - memoryRegion + this->baseAddress;
			}

			if ( *pat != *data && *mask == 'x' )
				break;
		}
	}

	free( memoryRegion );
	return 0;
}
