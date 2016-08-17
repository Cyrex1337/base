#include "process.hpp"

std::shared_ptr<Process> g_pProcess;

Process::Process( const char* attachTo )
{
	this->alreadyAttached = true;

	while ( !Process::getStatus( attachTo ) )
		Sleep( 10 );

	this->id = Process::getID( attachTo );
	this->threadCount = Process::getThreadCount( attachTo );

	this->handle = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, this->id );
}

bool Process::attachTo( const char* attachTo )
{
	if ( this->alreadyAttached )
		return false;

	while ( !Process::getStatus( attachTo ) )
		Sleep( 10 );

	this->id = Process::getID( attachTo );
	this->threadCount = Process::getThreadCount( attachTo );

	this->handle = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, this->id );
	if ( this->handle == INVALID_HANDLE_VALUE )
		return false;

	return true;
}

HANDLE Process::getHandle( ) const
{
	return this->handle;
}

Module Process::getModule( const char * module )
{
	HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->id );
	if ( snapshot == INVALID_HANDLE_VALUE )
		return Module( 0, 0, 0, 0 );

	MODULEENTRY32 moduleEntry;
	if ( !Module32First( snapshot, &moduleEntry ) )
	{
		CloseHandle( snapshot );
		return Module( 0, 0, 0, 0 );
	}

	do
	{
		if ( _stricmp( module, moduleEntry.szModule ) == 0 )
		{
			CloseHandle( snapshot );
			return Module( moduleEntry.modBaseAddr, moduleEntry.modBaseSize, this->handle, module );
		}
	} while ( Module32Next( snapshot, &moduleEntry ) );

	CloseHandle( snapshot );
	return Module( 0, 0, 0, 0 );
}

bool Process::getStatus( const char* process )
{
	HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( snapshot == INVALID_HANDLE_VALUE )
		return false;

	PROCESSENTRY32 processEntry;
	if ( !Process32First( snapshot, &processEntry ) )
	{
		CloseHandle( snapshot );
		return false;
	}

	do
	{
		if ( _stricmp( process, processEntry.szExeFile ) == 0 )
		{
			CloseHandle( snapshot );
			return true;
		}
	} while ( Process32Next( snapshot, &processEntry ) );

	CloseHandle( snapshot );
	return false;
}

uint32_t Process::getID( const char* process )
{
	HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( snapshot == INVALID_HANDLE_VALUE )
		return false;

	PROCESSENTRY32 processEntry;
	if ( !Process32First( snapshot, &processEntry ) )
	{
		CloseHandle( snapshot );
		return false;
	}

	do
	{
		if ( _stricmp( process, processEntry.szExeFile ) == 0 )
		{
			CloseHandle( snapshot );
			return processEntry.th32ProcessID;
		}
	} while ( Process32Next( snapshot, &processEntry ) );

	CloseHandle( snapshot );
	return false;
}


uint32_t Process::getThreadCount( const char* process )
{
	HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( snapshot == INVALID_HANDLE_VALUE )
		return false;

	PROCESSENTRY32 processEntry;
	if ( !Process32First( snapshot, &processEntry ) )
	{
		CloseHandle( snapshot );
		return false;
	}

	do
	{
		if ( _stricmp( process, processEntry.szExeFile ) == 0 )
		{
			CloseHandle( snapshot );
			return processEntry.cntThreads;
		}
	} while ( Process32Next( snapshot, &processEntry ) );

	CloseHandle( snapshot );
	return false;
}

uint32_t Process::getID( ) const
{
	return this->id;
}

uint32_t Process::getThreadCount( ) const
{
	return this->threadCount;
}

bool Process::WriteArray( unsigned char* _array, const char* bytes, int length )
{
	PBYTE _bytes = ( PBYTE )bytes;
	return !!WriteProcessMemory( this->handle, reinterpret_cast< LPVOID >( _array ), _bytes, length, nullptr );
}

std::string Process::readString( const char* ptr, int length )
{
	if ( length != -1 )
	{
		char* buffer = static_cast< char* >( malloc( length ) );
		ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), buffer, length, nullptr );
		std::string ret( buffer );
		free( buffer );
		return ret;
	}

	char max[ MAX_PATH ];
	int pos;
	ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), max, MAX_PATH, nullptr );
	std::string tmp( max );
	if ( pos = tmp.find( '\0' ) != std::string::npos )
		return tmp.substr( 0, pos );
}

std::string Process::readStringV2( const char* ptr, int length )
{
	if ( length != -1 )
	{
		char* buffer = static_cast< char* >( malloc( length ) );
		ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), buffer, length, nullptr );
		std::string ret( buffer );
		free( buffer );
		return ret;
	}

	const char* tmp = ptr;
	int counter = 0;
	while ( true )
	{
		char curChar;
		ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( tmp ), &curChar, sizeof( char ), nullptr );

		if ( curChar == '\0' )
		{
			counter++;
			break;
		}
		else
			counter++;

		tmp++;
	}

	char* buffer = static_cast< char* >( malloc( counter ) );
	ReadProcessMemory( this->handle, reinterpret_cast< LPCVOID >( ptr ), buffer, counter, nullptr );
	std::string ret( buffer );
	free( buffer );
	return ret;
}

const Module * Process::searchModuleList( const char * existingModuleName )
{
	for ( auto module : modules )
	{
		if ( module->getName( ) == existingModuleName )
			return module;
	}

	return nullptr;
}
