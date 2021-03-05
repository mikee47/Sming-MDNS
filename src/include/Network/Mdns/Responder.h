/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Responder.h
 *
 ****/

#pragma once

#include "Service.h"
#include "Server.h"

namespace mDNS
{
/**
 * @brief Special name for querying list of services
 */
DECLARE_FSTR(fstrServicesLocal)

class Responder : public Handler
{
public:
	/**
	 * @brief Initialise the responder
	 * @param hostname
	 * @retval bool true on success
	 */
	bool begin(const String& hostname);

	/**
	 * @brief Stop the responder
	 *
	 * Must reinitialise the stack again to restart
	 */
	void end();

	/**
	 * @brief Add a service object
	 *
	 * Responder doesn't own the object, must remain in scope
	 *
	 * @param svc Service object
	 * @retval bool true on success
	 */
	bool addService(Service& svc);

	bool removeService(Service& svc);

	/**
	 * @brief Used internally to process received query, but also handy for testing
	 */
	bool onMessage(Message& message) override;

private:
	String hostname;
	Service::List services;
};

} // namespace mDNS
