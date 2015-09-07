#ifndef MATHTOOLS_H
#define MATHTOOLS_H

inline void setBit(int & res, int i) {
	res |= 1 << i;
}

inline void clearBit(int & res, int i) {
	res &= ~(1 << i);
}

inline int checkBit(const int & res, int i) {
	return (res>>i) & 1;
}

inline void toggingBit(int & res, int i) {
	res ^= (1 << i);
}

#endif
