#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include "tuple/TpIndexs.h"
#include "tuple/TpApply.h"
#include "comm/Singleton.h"
#include "comm/Any.h"
#include "comm/DllParser.h"
#include "nlohmann/json.hpp"
#include "msgbus/MessageBus.h"

const int nCharLen = sizeof(char);
const int nIntLen = sizeof(int);
const int nUnsignedIntLen = sizeof(unsigned int);
const int nShortLen = sizeof(short);
const int nFloatLen = sizeof(float);
const int nDoubleLen = sizeof(double);
const int nMaxRecCache = 3000 * 1024;
const int nMaxPkgLen = 3000 * 1024;


using Json = nlohmann::json;
using DllShrPtr = std::shared_ptr<common_template::DllParser>;