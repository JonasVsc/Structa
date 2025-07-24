#ifndef STRUCTA_UTILS_H_
#define STRUCTA_UTILS_H_ 1

static inline clamp(uint32_t val, uint32_t min, uint32_t max)
{
	return val < min ? min : (val > max ? max : val);
}

#endif // STRUCTA_UTILS_H_