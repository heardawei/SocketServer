#pragma once

#include "header_value.h"
#include "base_socket.h"
#include <queue>


namespace vc
{

	class VirusCheck
	{
	public:
		enum State
		{
			OK = 10,
			ERR,
		};

		enum storage_to_t {
			TO_MEM,
			TO_SHM,
			TO_FILE
		};

		VirusCheck();
		~VirusCheck();
		int on_packet(const char *p_data, int data_len);
		int get_n_resps();
		bool get_response(vc::Response &resp);
	protected:
		storage_to_t	to;

		bool req_head_valid;
		vc::Request	req;
		std::queue<vc::Response> resps;
	};

}
