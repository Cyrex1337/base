#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "module.hpp"

class Process
{
public:
	Process( ) = default;
	Process( const char* attachTo );

public:
	bool attachTo( const char* attachTo );
	HANDLE getHandle( ) const;
	Module getModule( const char* module );

public:
	static bool getStatus( const char* process );
	static uint32_t getID( const char* process );
	static uint32_t getThreadCount( const char* process );

	uint32_t getID( ) const;
	uint32_t getThreadCount( ) const;

public:
	template <typename T>
	T Read( const void* ptr );

	template <typename T>
	bool Read( const void* ptr, T& buffer );

	template <typename T>
	T Read( const DWORD ptr );

	template <typename T>
	bool Read( const DWORD ptr, T& buffer );

	template <typename T>
	bool Write( void* ptr, T value );

	template <typename T>
	bool Write( const DWORD ptr, T value );

	bool WriteArray( unsigned char* _array, const char* bytes, int length );

	std::string readString( const char* ptr, int length = -1 );
	std::string readStringV2( const char* ptr, int length = -1 );

	const Module* searchModuleList( const char* existingModuleName );

private:
	bool alreadyAttached = false;
	uint32_t id, threadCount;
	HANDLE handle;
	std::vector<Module*> modules;
};

template<typename T>
inline T Process::Read( const void* ptr )
{
	T buffer;
	ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), &buffer, sizeof( T ), nullptr );
	return buffer;
}

template<typename T>
inline bool Process::Read( const void* ptr, T& buffer )
{
	return !!ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), &buffer, sizeof( T ), nullptr );
}

template<typename T>
inline T Process::Read( const DWORD ptr )
{
	T buffer;
	ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), &buffer, sizeof( T ), nullptr );
	return buffer;
}

template<typename T>
inline bool Process::Read( const DWORD ptr, T & buffer )
{
	return !!ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), &buffer, sizeof( T ), nullptr );
}

template<typename T>
inline bool Process::Write( void* ptr, T value )
{
	return !!WriteProcessMemory( this->handle, reinterpret_cast< LPVOID >( ptr ), &value, sizeof( T ), nullptr );
}

template<typename T>
inline bool Process::Write( const DWORD ptr, T value )
{
	return !!WriteProcessMemory( this->handle, reinterpret_cast< LPVOID >( ptr ), &value, sizeof( T ), nullptr );
}

extern std::shared_ptr<Process> g_pProcess;