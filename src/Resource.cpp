#include "include/Network/Mdns/Resource.h"
#include "include/Network/Mdns/Answer.h"
#include "Packet.h"
#include <Data/HexString.h>

String toString(mDNS::ResourceType type)
{
	switch(type) {
#define XX(name, value, desc)                                                                                          \
	case mDNS::ResourceType::name:                                                                                     \
		return F(#name);
		MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
	default:
		return String(unsigned(type));
	}
}

namespace mDNS
{
namespace Resource
{
uint8_t* Record::recordPtr() const
{
	return answer.getRecordPtr();
}

uint16_t Record::recordSize() const
{
	return answer.getRecordSize();
}

String Record::toString() const
{
	return makeHexString(recordPtr(), recordSize(), ' ');
}

IpAddress A::getAddress() const
{
	// Keep bytes in network order
	uint32_t addr;
	memcpy(&addr, recordPtr(), sizeof(addr));
	return addr;
}

Name PTR::getName() const
{
	return Name(answer.getResponse(), recordPtr());
}

String TXT::toString(const String& sep) const
{
	String s;
	Packet pkt{recordPtr()};
	auto size = recordSize();
	while(pkt.pos < size) {
		auto len = pkt.read8();
		if(s) {
			s += sep;
		}
		s += pkt.readString(len);
	}
	return s;
}

uint8_t TXT::count() const
{
	if(mCount == 0) {
		Packet pkt{recordPtr()};
		auto size = recordSize();
		while(pkt.pos < size) {
			auto len = pkt.read8();
			pkt.skip(len);
			++mCount;
		}
	}
	return mCount;
}

String TXT::operator[](uint8_t index) const
{
	uint8_t len;
	auto p = get(index, len);
	return p ? String(p, len) : nullptr;
}

String TXT::getValue(const char* name, uint16_t namelen) const
{
	Packet pkt{recordPtr()};
	auto size = recordSize();
	while(pkt.pos < size) {
		auto len = pkt.read8();
		auto entry = reinterpret_cast<const char*>(pkt.ptr());
		if(len > namelen && entry[namelen] == '=' && memicmp(entry, name, namelen) == 0) {
			return String(entry + namelen + 1, len - namelen - 1);
		}
		pkt.skip(len);
	}
	return nullptr;
}

const char* TXT::get(uint8_t index, uint8_t& len) const
{
	Packet pkt{recordPtr()};
	auto size = recordSize();
	for(; pkt.pos < size; --index) {
		len = pkt.read8();
		if(index == 0) {
			return reinterpret_cast<const char*>(pkt.ptr());
		}
		pkt.skip(len);
	}
	return nullptr;
}

String AAAA::toString() const
{
	return makeHexString(recordPtr(), recordSize(), ':');
}

uint16_t SRV::getPriority() const
{
	return Packet{recordPtr()}.read16();
}

uint16_t SRV::getWeight() const
{
	return Packet{recordPtr(), 2}.read16();
}

uint16_t SRV::getPort() const
{
	return Packet{recordPtr(), 4}.read16();
}

Name SRV::getHost() const
{
	return Name(answer.getResponse(), recordPtr() + 6);
}

String SRV::toString(const String& sep) const
{
	String s;
	s.reserve(32);
	s += "p=";
	s += getPriority();
	s += sep;
	s += "w=";
	s += getWeight();
	s += sep;
	s += F("port=");
	s += getPort();
	s += sep;
	s += F("host=");
	s += getHost();
	return s;
}

} // namespace Resource
} // namespace mDNS
