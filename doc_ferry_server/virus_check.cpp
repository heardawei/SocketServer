#include <assert.h>

#include <list>
#include <map>
#include <tuple>
#include <memory>

#include "cp_socket.h"
#include "virus_check.h"
#include "header_value.h"
#include "debug_print.h"

vc::VirusCheck::VirusCheck()
{

}

vc::VirusCheck::~VirusCheck()
{

}

int vc::VirusCheck::on_packet(const char *p_data, int data_len)
{
	debug_print("%s %dB\n", __func__, data_len);
	int wlen = 0;
	while (true)
	{
		wlen = HV_write(&this->req, p_data, data_len);
		if (wlen == -1)
		{
			/* prepare response */
			iface::Header header = { this->req.header.id, sizeof(int) };
			int errcode = iface::INTERNAL_ERROR;
			vc::Response resp;
			HV_set(&resp, &header, (char *)&errcode);
			this->resps.push(resp);

			// TODO: call virus_database to determin

			// TODO: if necessary clear memory

			/* reset */
			this->req.hlen = 0;
			this->req.data_size = 0;
			this->req_head_valid = false;
			return vc::VirusCheck::State::ERR;
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