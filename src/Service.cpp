/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Service.cpp
 *
 ****/

#include "include/Network/Mdns/Service.h"
#include <Platform/Station.h>

String toString(mDNS::Service::Protocol protocol)
{
	return (protocol == mDNS::Service::Protocol::Udp) ? F("_udp") : F("_tcp");
}

namespace mDNS
{
DEFINE_FSTR(fstrDotLocal, ".local")

String Service::getInstanceName()
{
	String s;
	s.reserve(48);
	s += getInstance();
	s += '.';
	getServiceName(s);
	return s;
}

void Service::getServiceName(String& s)
{
	s += '_';
	s += getName();
	s += '.';
	s += toString(getProtocol());
	s += fstrDotLocal;
}

String Service::getServiceName()
{
	String s;
	s.reserve(20);
	getServiceName(s);
	return s;
}

bool Service::handleQuestion(Question& question, const Host& host, Request& reply)
{
	switch(question.getType()) {
	case ResourceType::ANY:
	case ResourceType::PTR: {
		auto svc = getServiceName();
		if(question.getName() != svc) {
			return false;
		}

		auto ptr = reply.addAnswer<Resource::PTR>(svc, getInstance() + ".");
		auto serviceName = ptr.answer.getName(); // e.g. "_http._tcp.local"
		auto instance = ptr.getName();
		instance.fixup(serviceName);

		reply.nextSection();
		reply.nextSection();

		auto txt = reply.addAnswer<Resource::TXT>(instance);
		addText(txt);

		auto srv = reply.addAnswer<Resource::SRV>(instance, 0, 0, getPort(), host.name + ".");
		srv.getHost().fixup(serviceName.getDomain());
		reply.addAnswer<Resource::A>(srv.getHost(), WifiStation.getIP());
		reply.addAnswer<Resource::AAAA>(srv.getHost(), Ip6Address{});
		break;
	}

	case ResourceType::SRV: {
		auto inst = getInstanceName();
		if(question.getName() != inst) {
			return false;
		}

		auto srv = reply.addAnswer<Resource::SRV>(inst, 0, 0, getPort(), host.nameWithDomain);

		reply.nextSection();
		reply.nextSection();

		reply.addAnswer<Resource::A>(srv.getHost(), WifiStation.getIP());
		reply.addAnswer<Resource::AAAA>(srv.getHost(), Ip6Address{});
		break;
	}

	case ResourceType::TXT: {
		auto inst = getInstanceName();
		if(question.getName() != inst) {
			return false;
		}

		auto txt = reply.addAnswer<Resource::TXT>(inst);
		addText(txt);
		break;
	}

	case ResourceType::A:
		if(question.getName() != host.nameWithDomain) {
			return false;
		}

		reply.addAnswer<Resource::A>(host.nameWithDomain, WifiStation.getIP());
		break;

	case ResourceType::AAAA:
		if(question.getName() != host.nameWithDomain) {
			return false;
		}

		reply.addAnswer<Resource::AAAA>(host.nameWithDomain, Ip6Address{});
		break;

	default:
		return false;
	}

	return true;
}

} // namespace mDNS
