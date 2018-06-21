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


		VirusCheck();
		~VirusCheck();
		int on_packet(const char *p_data, int data_len);
	protected:

		bool req_head_valid;
		vc::Request	req;
		std::queue<vc::Response> resps;
	};

}
