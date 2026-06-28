#ifndef _NAVIA_INPUT_FILE_HPP_
#define _NAVIA_INPUT_FILE_HPP_

#include <string>

#include "../include/navia/input.h"

namespace navia::input::file
{

bool populate(navia_input *input, const std::string &filename);

}

#endif /* _NAVIA_INPUT_FILE_HPP_ */
