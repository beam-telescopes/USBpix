#ifndef _UTIL_H
#define _UTIL_H

template<typename T>
T selectBits(T val, int offset, int length) {
	return (val >> offset) & ((1ull << length) - 1);
}

template<typename T>
T clearBits(T val, int offset, int length) {
	return val & ~(((1ull << length) - 1) << offset);
}

template<typename T>
T reverseBits(T val, int length) {
	T res = 0;

	for(int i = 0; i < length; i++, val >>= 1) {
		res = (res << 1) | (val & 0x01);
	}

	return res;
}

template <class T, std::size_t ROW, std::size_t COL>
using Matrix = std::array<std::array<T, COL>, ROW>;

#endif
