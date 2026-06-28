#ifndef _NAVIA_INPUT_MEMORY_HPP_
#define _NAVIA_INPUT_MEMORY_HPP_

#include "../include/navia/input.h"

namespace navia::input::memory
{

struct Memory
{
	uint8_t *data;
	size_t size, pos;
};

void populate(navia_input *input, void *buf, size_t size);

}

#endif /* _NAVIA_INPUT_MEMORY_HPP_ */
