#pragma once
#include <inttypes.h>


namespace iface
{

#define data_len_t uint32_t

#pragma pack(push)
#pragma pack(1)
typedef struct 
{
	int32_t		id;
	data_len_t	length;
}Header;

typedef struct
{
	Header	header;
	std::shared_ptr<void> data;
}Request;

typedef struct
{
	Header	header;
	int	errcode;
}Response;
#pragma pack(pop)


enum ServerState
{
	OK = 0,
	DATA_ERROR,
	CONNECTION_ERROR,
	INTERNAL_ERROR,
	// virus_database_error ...
};

} /* namespace iface */
