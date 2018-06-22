#include "virus_handler.h"
#include "debug_print.h"

VirusHandler::VirusHandler(cp_socket_t sock, ss_map_t *p_handlers) :
	SocketHandler(sock, p_handlers)
{
	debug_print("new client: %s:%d\n", this->addr.first.data(), this->addr.second);
}

VirusHandler::~VirusHandler()
{
	debug_print("del client: %s:%d\n", this->addr.first.data(), this->addr.second);
}

bool VirusHandler::readable()
{
	return cp_socket_valid(this->socket());
}

bool VirusHandler::writable()
{
	return this->resp.size();
}

void VirusHandler::request(vc::HV &req)
{
	// TODO: call virus_database to determin
	req.clear();
}

void VirusHandler::response(vc::HV &req, iface::State state)
{
	iface::Header header = { req.header.id, sizeof(iface::State) };
	vc::HV resp;
	resp.write(&header, (char *)&state);

	this->resp.push(resp);
}

void VirusHandler::handle_read()
{
	char buf[65535];

	int nread = this->recv(buf, sizeof(buf));
	if (nread == 0)
	{
		return;
	}
	else if (nread == SOCKET_ERROR)
	{
		return;
	}

	debug_print("read from [%s:%d] [%dB]\n", this->addr.first.data(), this->addr.second, nread);

	char *p_buf = buf;
	while (true)
	{
		int wlen = this->req.write(p_buf, nread);
		if (wlen == -1)
		{
			/* prepare response */
			this->req.clear();
			this->state = iface::INTERNAL_ERROR;
			this->response(this->req, iface::INTERNAL_ERROR);
			return;
		}

		p_buf += wlen;
		nread -= wlen;

		/* a request is complete */
		if (this->req.is_complete())
		{
			/* prepare response */
			this->request(this->req);
			this->state = iface::OK;
			this->response(this->req, iface::OK);
		}

		if (nread == 0)
		{
			break;
		}
	}
}

void VirusHandler::handle_write()
{
	while (this->resp.size())
	{
		vc::HV resp = this->resp.front();
		this->resp.pop();

		if (resp.value)
		{
			char buf[4096];

			this->send((const char *)&resp.header, sizeof(resp.header));

			for (size_t nread; (nread = cache_pop(resp.value, buf, sizeof(buf))) > 0;)
			{
				this->send(buf, nread);
			}

			// TODO: dangerous read errno
			debug_print("%s send id:%d, vlen:%d, errno:%d\n",
				__func__, ntohl(resp.header.id), ntohl(resp.header.length),
				ntohl(*(iface::State *)buf));

			delete_cache(resp.value);
			leave_cache(resp.value);
		}
	}
}

void VirusHandler::handle_expt()
{
	debug_print("warning: an exception occured, errcode(%d)\n", errno);
	this->handle_close();
}

void VirusHandler::handle_error()
{
	debug_print("warning: an error occured\n");
	this->handle_close();
}

void VirusHandler::handle_close()
{
	debug_print("debug: handle_close normally\n");
	this->close();
}