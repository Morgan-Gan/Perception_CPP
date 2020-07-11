#pragma once
#include <string>
#include <iostream>
#include "comm/Any.h"

#define IDETEC_SSDALG_API extern "C"

IDETEC_SSDALG_API bool InitModuleDll(common_template::Any&& anyObj);
