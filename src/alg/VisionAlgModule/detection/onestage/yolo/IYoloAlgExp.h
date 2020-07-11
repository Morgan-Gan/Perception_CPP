#pragma once
#include <string>
#include <iostream>
#include "comm/Any.h"

#define IDETEC_TYOLOALG_API extern "C"

IDETEC_TYOLOALG_API bool InitModuleDll(common_template::Any&& anyObj);
