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
		if (HV_full(&this->req))
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



#if 0

/*
*[0] : client address
*[1] : recieved req_body_len
*[2] : recieved buffer
*/

using client_context_t = std::tuple<ss_addr_t, data_len_t, vc::Request, iface::Response>;

using buffer_t = std::tuple<vc::Request, iface::Response>;

/*
 *[0]: correspond socket -> client_context_t
 *[1]: client context
 */
using clients_context_t = std::map<cp_socket_t, client_context_t>;


static clients_context_t contexts;

void set_resp_data(iface::Response &resp, int32_t id, data_len_t data_len, int32_t errcode)
{
	resp.header.id		= htonl(id);
	resp.header.length	= htonl(data_len);
	resp.errcode		= htonl(errcode);
}

vc::VirusCheckState recv_begin(ss_addr_t addr)
{
	// reset
	contexts[sock] = client_context_t(ss_addr_t(), 0, vc::Request(), iface::Response());
	return vc::VirusCheck::OK;
}

char *get_buffer(int *len)
{
	data_len_t recvd_len = std::get<1>(contexts[sock]);
	if (recvd_len == 0)
	{
		*len = sizeof(vc::Request);
		return 
	}
	else
	{
		data_len_t data_len = std::get<2>(contexts[sock]).header.length;
		return data_len - recvd_len;
	}
}

vc::VirusCheckState recv_data(char *data, size_t data_len)
{
	auto tuple = contexts[sock];
	int32_t id = 0;
	data_len_t length = 0;

	// recvd_len is 0
	if (std::get<1>(tuple) == 0)
	{
		// not enough a header length
		if (data_len < sizeof(vc::Request))
		{
			set_resp_data(std::get<3>(tuple), -1, sizeof(int32_t), iface::DATA_ERROR);
			return vc::VirusCheck::ERR;
		}

		id = std::get<2>(tuple).header.id = ntohl(((vc::Request *)data)->header.id);
		length = std::get<2>(tuple).header.length = ntohl(((vc::Request *)data)->header.length);

		// now jump over header
		data += sizeof(iface::Header);
		data_len -= sizeof(iface::Header);

		std::shared_ptr<vc::Request> buffer((vc::Request *)new char[length]);
		if (buffer.get() == nullptr)
		{
			set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::INTERNAL_ERROR);
			return vc::VirusCheck::ERR;
		}
		std::get<2>(tuple).data = buffer;
	}
	else
	{
		id = std::get<2>(tuple).header.id;
		length = std::get<2>(tuple).header.length;
	}

	/* here cache buffer is alloced */

	// actual data_len is larger than agreement
	if (data_len + std::get<1>(tuple) > length + sizeof(vc::Request))
	{
		set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::DATA_ERROR);
		return vc::VirusCheck::ERR;
	}

	// cache data, wait for arrival of remaining data
	memcpy(std::get<2>(tuple).data.get(), data, data_len);
	std::get<1>(tuple) += data_len;

	// recv all data
	if (std::get<1>(tuple) == length)
	{
		set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::OK);
		// TODO: request virus_database

		return vc::VirusCheck::END;
	}

	return vc::VirusCheck::OK;
}

int recv_end()
{
	contexts.erase(sock);
	return vc::VirusCheck::OK;
}

#endif