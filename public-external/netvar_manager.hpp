#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "process.hpp"

class RecvProp;
class RecvTable
{ 
public:
	const char* getTableName( );
	int getPropCount( );
	RecvProp* getPropAt( int i );
};

typedef enum
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_String,
	DPT_Array,	// An array of the base types (can't be of datatables).
	DPT_DataTable,
#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
	DPT_Quaternion,
#endif
	DPT_NUMSendPropTypes
} SendPropType;

class RecvProp
{
public:
	SendPropType getType( );
	const char* getPropName( );
	int getPropTableOffset( );
	RecvTable* fetchTable( );
};

//y3t1y3t
class ClientClass
{
public:
	const char* getNetworkName( );
	RecvTable* getRecvTable( );
	ClientClass* getNext( );
	int getClassID( );

public:
	static ClientClass* Singleton( )
	{
		auto client = g_pProcess->searchModuleList( "client.dll" );
		// get pointer to "DT_TEWorldDecal" string
		uint32_t world = client->findUnformattedPattern( "DT_TEWorldDecal", "xxxxxxxxxxxxxxx" );
		// search for first x-ref, add 0x2B and dereference to get first ClientClass* (or clientclass array ptr)
		uint32_t classes = g_pProcess->Read<uint32_t>( ( const void* )( client->findUnformattedPattern( ( const char* )world, "xxxx" ) + 0x2B ) );
	}
};

// altimor
class netvar_tree
{
	struct node;
	using map_type = std::unordered_map<std::string, std::shared_ptr<node>>;

	struct node
	{
		node( int offset ) : offset( offset ) { }
		map_type nodes;
		int offset;
	};

	map_type nodes;

public:
	netvar_tree( );

private:
	void populate_nodes( class RecvTable *recv_table, map_type *map );

	/**
	* get_offset_recursive - Return the offset of the final node
	* @map:	Node map to scan
	* @acc:	Offset accumulator
	* @name:	Netvar name to search for
	*
	* Get the offset of the last netvar from map and return the sum of it and accum
	*/
	int get_offset_recursive( map_type &map, int acc, const char *name )
	{
		return acc + map[ name ]->offset;
	}

	/**
	* get_offset_recursive - Recursively grab an offset from the tree
	* @map:	Node map to scan
	* @acc:	Offset accumulator
	* @name:	Netvar name to search for
	* @args:	Remaining netvar names
	*
	* Perform tail recursion with the nodes of the specified branch of the tree passed for map
	* and the offset of that branch added to acc
	*/
	template<typename ...args_t>
	int get_offset_recursive( map_type &map, int acc, const char *name, args_t ...args )
	{
		const auto &node = map[ name ];
		return get_offset_recursive( node->nodes, acc + node->offset, args... );
	}

public:
	/**
	* get_offset - Get the offset of a netvar given a list of branch names
	* @name:	Top level datatable name
	* @args:	Remaining netvar names
	*
	* Initiate a recursive search down the branch corresponding to the specified datable name
	*/
	template<typename ...args_t>
	int get_offset( const char *name, args_t ...args )
	{
		const auto &node = nodes[ name ];
		return get_offset_recursive( node->nodes, node->offset, args... );
	}
};