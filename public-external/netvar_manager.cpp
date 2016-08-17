#include "netvar_manager.hpp"

const char* ClientClass::getNetworkName( )
{
	char pszNetworkName[ 128 ];
	char* ptr;
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ( DWORD )this + 0x8 ), ptr, sizeof( char* ), nullptr );
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ptr ), pszNetworkName, sizeof( pszNetworkName ), nullptr );
	return pszNetworkName;
}

RecvTable* ClientClass::getRecvTable( )
{
	return g_pProcess->Read<RecvTable*>( ( DWORD )this + 0xC );
}

ClientClass* ClientClass::getNext( )
{
	return g_pProcess->Read<ClientClass*>( ( DWORD )this + 0x10 );
}

int ClientClass::getClassID( )
{
	return g_pProcess->Read<int>( ( DWORD )this + 0x14 );
}

const char* RecvTable::getTableName( )
{
	char pszTableName[ 128 ];
	char* ptr;
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ( DWORD )this + 0xC ), ptr, sizeof( char* ), nullptr );
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ptr ), pszTableName, sizeof( pszTableName ), nullptr );
	return pszTableName;
}

int RecvTable::getPropCount( )
{
	return g_pProcess->Read<int>( ( DWORD )this + 0x4 );
}

RecvProp* RecvTable::getPropAt( int i )
{
	return ( RecvProp* )( g_pProcess->Read<DWORD>( ( DWORD )this ) + 0x3C * i );
}

SendPropType RecvProp::getType( )
{
	return g_pProcess->Read<SendPropType>( ( DWORD )this + 0x4 );
}

const char* RecvProp::getPropName( )
{
	char pszPropName[ 128 ];
	char* ptr;
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ( DWORD )this ), ptr, sizeof( char* ), nullptr );
	ReadProcessMemory( g_pProcess->getHandle( ), reinterpret_cast< LPCVOID >( ptr ), pszPropName, sizeof( pszPropName ), nullptr );
	return pszPropName;
}

int RecvProp::getPropTableOffset( )
{
	return g_pProcess->Read<int>( ( DWORD )this + 0x2C );
}

RecvTable* RecvProp::fetchTable( )
{
	return g_pProcess->Read<RecvTable*>( ( DWORD )this + 0x28 );
}

netvar_tree::netvar_tree( )
{
	ClientClass* pClass = ClientClass::Singleton( );
	while ( pClass != nullptr )
	{
		const auto class_info = std::make_shared<node>( 0 );
		auto* recv_table = pClass->getRecvTable( );
		populate_nodes( recv_table, &class_info->nodes );
		nodes.emplace( recv_table->getTableName( ), class_info );
		pClass = pClass->getNext( );
	}
}

void netvar_tree::populate_nodes( RecvTable* recv_table, map_type* map )
{
	for ( auto i = 0; i < recv_table->getPropCount( ); i++ )
	{
		auto* prop = recv_table->getPropAt( i );
		const auto prop_info = std::make_shared<node>( prop->getPropTableOffset( ) );

		if ( prop->getType( ) == DPT_DataTable )
			populate_nodes( prop->fetchTable( ), &prop_info->nodes );

		map->emplace( prop->getPropName( ), prop_info );
	}
}