#include <assert.h>

#include <list>
#include <map>
#include <tuple>
#include <memory>

#include "cp_socket.h"
#include "virus_check.h"
#include "header_value.h"


vc::VirusCheck::VirusCheck() :
	to(vc::VirusCheck::storage_to_t::TO_MEM)
{

}

vc::VirusCheck::~VirusCheck()
{

}

int vc::VirusCheck::get_n_resps()
{
	return this->resps.size();
}

bool vc::VirusCheck::get_response(vc::Response &resp)
{
	if (this->resps.size())
	{
		resp = this->resps.front();
		return true;
	}
	return false;
}

int vc::VirusCheck::on_packet(const char *p_data, int data_len)
{
	int wlen = 0;
	while (true)
	{
		switch (this->to)
		{
		case vc::VirusCheck::storage_to_t::TO_FILE:
			// TODO
		case vc::VirusCheck::storage_to_t::TO_SHM:
			// TODO
		case vc::VirusCheck::storage_to_t::TO_MEM:
			wlen = HV_write_memory(&this->req, p_data, data_len);
			break;
		}
		p_data += wlen;
		data_len -= wlen;

		/* a request is complete */
		if (HV_is_complete(&this->req))
		{
			/* prepare response */
			iface::Header header = { this->req.header.id, sizeof(int) };
			int errcode = iface::OK;
			vc::Response resp;
			HV_set(&resp, &header, (char *)&errcode);
			this->resps.push(resp);

			// TODO: call virus_database to determin

			// TODO: if necessary clear memory

			/* reset */
			this->req.hlen = 0;
			this->req.data_size = 0;
			this->req_head_valid = false;
		}

		if (data_len == 0)
		{
			break;
		}

	}
	return vc::VirusCheck::State::OK;
}