#include <list>
#include <map>
#include <tuple>
#include <memory>

#include "cp_socket.h"
#include "base_socket.h"
#include "interface.h"
#include "business.h"

/*
 *[0] : client address
 *[1] : recieved req_body_len
 *[2] : recieved buffer
 */

using client_context_t = std::tuple<ss_addr_t, data_len_t, iface::Request, iface::Response>;

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

business::BusinessState recv_begin(cp_socket_t sock, ss_addr_t addr)
{
	// reset
	contexts[sock] = client_context_t(ss_addr_t(), 0, iface::Request(), iface::Response());
	return business::OK;
}

data_len_t recv_expect_length(cp_socket_t sock)
{
	data_len_t recvd_len = std::get<1>(contexts[sock]);
	if (recvd_len < sizeof(iface::Request))
	{
		return sizeof(iface::Request);
	}
	else
	{
		data_len_t data_len = std::get<2>(contexts[sock]).header.length;
		return data_len - recvd_len;
	}
}

business::BusinessState recv_data(cp_socket_t sock, char *data, size_t data_len)
{
	auto tuple = contexts[sock];
	int32_t id = 0;
	data_len_t length = 0;

	// recvd_len is 0
	if (std::get<1>(tuple) == 0)
	{
		// not enough a header length
		if (data_len < sizeof(iface::Request))
		{
			set_resp_data(std::get<3>(tuple), -1, sizeof(int32_t), iface::DATA_ERROR);
			return business::FAILED;
		}

		id = std::get<2>(tuple).header.id = ntohl(((iface::Request *)data)->header.id);
		length = std::get<2>(tuple).header.length = ntohl(((iface::Request *)data)->header.length);

		// now jump over header
		data += sizeof(iface::Header);
		data_len -= sizeof(iface::Header);

		std::shared_ptr<iface::Request> buffer((iface::Request *)new char[length]);
		if (buffer.get() == nullptr)
		{
			set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::INTERNAL_ERROR);
			return business::FAILED;
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
	if (data_len + std::get<1>(tuple) > length + sizeof(iface::Request))
	{
		set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::DATA_ERROR);
		return business::FAILED;
	}

	// cache data, wait for arrival of remaining data
	memcpy(std::get<2>(tuple).data.get(), data, data_len);
	std::get<1>(tuple) += data_len;

	// recv all data
	if (std::get<1>(tuple) == length)
	{
		set_resp_data(std::get<3>(tuple), id, sizeof(int32_t), iface::OK);
		// TODO: request virus_database

		return business::END;
	}

	return business::OK;
}

int recv_end(cp_socket_t sock)
{
	contexts.erase(sock);
	return business::OK;
}

