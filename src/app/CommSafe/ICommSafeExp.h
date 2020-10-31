#pragma once
#include <string>
#include <iostream>
#include "comm/Any.h"

extern "C" bool InitModuleDll(common_template::Any&& anyObj);
extern "C" bool InitModuleDllAlg(common_template::Any&& anyObj);