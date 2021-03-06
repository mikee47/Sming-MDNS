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

namespace mDNS
{
class Responder
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

	/**
	 * @brief Restart a running stack
	 *
	 * Call when IP address changes, for example.
	 */
	bool restart();

private:
#if LWIP_VERSION_MAJOR == 1
	String hostname;
	Service* service = nullptr;
#endif
};

} // namespace mDNS
