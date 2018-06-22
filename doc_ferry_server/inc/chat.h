#pragma once

#include "interface.h"
#include "HV.h"

namespace vc
{

	class Chat
	{
	public:
		virtual void request(vc::HV &req) = 0;
		virtual void response(vc::HV &req, iface::State state) = 0;
	};

}
