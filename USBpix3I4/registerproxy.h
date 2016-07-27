#ifndef _REGISTERPROXY_H
#define _REGISTERPROXY_H

#include <functional>

struct Register {
	int addr;
	int size;
	int offset;

	enum {
		READ = 1 << 0,
		WRITE = 1 << 1,
		READ_WRITE = READ | WRITE
	} type;
};

struct FERegister {
	int addr;
	int size;
	int offset;

	enum {
		READ = 1 << 0,
		WRITE = 1 << 1,
		MSB_LAST = 1 << 2,
		READ_WRITE = READ | WRITE,
		READ_WRITE_MSB = READ_WRITE | MSB_LAST
	} type;
};

class RegisterProxy {
	public:
		RegisterProxy(const std::function<uint64_t(void)> &_read, const std::function<void(uint64_t)> &_write) : read(_read), write(_write) {}

		uint64_t operator=(uint64_t val);
		operator uint64_t () const;

	protected:
		std::function<uint64_t(void)> read;
		std::function<void(uint64_t)> write;
};

inline uint64_t RegisterProxy::operator=(uint64_t val) {
	write(val);
	return val;
}

inline RegisterProxy::operator uint64_t () const {
	return read();
}

#endif
