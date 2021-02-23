#pragma once
#include <iostream>

#define LOGGER_LOG(x) std::cout << __FUNCTION__  << ": " << x << std::endl
#define LOGGER_DEBUG(x) do { std::cout << "Debug: "; LOGGER_LOG(x);  } while(0)
#define LOGGER_ERROR(x) do { std::cout << "Error: "; LOGGER_LOG(x);  } while(0)