/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Responder.cpp
 *
 ****/

#include "include/Network/Mdns/Responder.h"
#include <Platform/System.h>
#include <Platform/Station.h>

namespace mDNS
{
DEFINE_FSTR(fstrServicesLocal, "_services._dns-sd._udp.local")

bool Responder::begin(const String& hostname)
{
	this->hostname = hostname;
	if(!server.begin()) {
		return false;
	}
	server.addHandler(*this);

	// TODO: Schedule initial announcement
	// System.queueCallback(InterruptCallback([]() { mdns_resp_announce(netif_default); }));

	debug_i("MDNS initialised for '%s'", hostname.c_str());
	return true;
}

bool Responder::addService(Service& svc)
{
	debug_i("[MDNS] addService '%s'", svc.getInstanceName().c_str());
	if(services.contains(svc)) {
		debug_e("[mDNS] Service already in use");
		return false;
	}

	services.add(&svc);
	return true;
}

bool Responder::removeService(Service& svc)
{
	// TODO: Announce removal ?
	return services.remove(&svc);
}

void Responder::end()
{
	server.removeHandler(*this);
	debug_i("[mDNS] Responder stopped");
}

bool Responder::onMessage(Message& message)
{
	if(message.getType() != Message::Type::query) {
		// Pass to other handlers
		return true;
	}

	debug_d("[mDNS] Query from %s:%u", message.getRemoteIp().toString().c_str(), message.getRemotePort());

	Service::Host host;
	host.name = hostname;
	host.nameWithDomain = hostname + fstrDotLocal;

	for(auto& question : message.questions) {
		String questionName = question.getName();

		auto send = [&](Reply& reply) {
			server.send(reply);
			debug_d("[mDNS]   >> Responded to %s %s", ::toString(question.getType()).c_str(), questionName.c_str());
		};

		auto noMatch = [&]() {
			debug_d("[mDNS]   No match for %s %s", ::toString(question.getType()).c_str(), questionName.c_str());
		};

		if(question.getName() == host.nameWithDomain) {
			switch(question.getType()) {
			case ResourceType::A: {
				Reply reply(question);
				reply.addAnswer<Resource::A>(host.nameWithDomain, WifiStation.getIP());
				send(reply);
				break;
			}
			case ResourceType::AAAA: {
				Reply reply(question);
				reply.addAnswer<Resource::AAAA>(host.nameWithDomain, Ip6Address{});
				send(reply);
				break;
			}
			default:
				noMatch();
			}
			continue;
		}

		if(fstrServicesLocal == questionName) {
			Reply reply(question);
			Answer* firstAnswer{nullptr};
			for(auto& svc : services) {
				if(firstAnswer == nullptr) {
					auto ptr = reply.addAnswer<Resource::PTR>(questionName, svc.getServiceName());
					firstAnswer = &ptr.answer;
				} else {
					reply.addAnswer<Resource::PTR>(firstAnswer->getName(), svc.getServiceName());
				}
			}
			send(reply);
			continue;
		}

		bool handled{false};
		for(auto& svc : services) {
			Reply reply(question);
			if(svc.handleQuestion(question, host, reply)) {
				send(reply);
				handled = true;
			}
		}
		if(!handled) {
			noMatch();
		}
	}

	// Pass message to other handlers
	return true;
}

} // namespace mDNS
