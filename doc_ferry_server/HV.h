#pragma once

#include "cache_part.h"
#include "interface.h"
#include <memory>

namespace vc
{
#define DEFAULT_MAX_MEMORY_CACHE	((data_len_t)(1024))

	class HV
	{
	public:
		HV();
		virtual ~HV();
		bool is_value_complete();
		bool is_head_complete();
		bool is_complete();
		int write(const char *p_data, int data_len);
		int write(iface::Header *header, char *value);

		void clear();
//	protected:
		void head_completed();
		void value_completed();
		int write_head(const char *p_data, int data_len);
		int write_value(const char *p_data, int data_len);

		storage_to to;

		iface::Header			header;
		cache_t					*value;

		int						hlen;
		bool					head_complete;

		data_len_t				vlen;
		bool					value_complete;
	};
}