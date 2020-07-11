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
const int nMaxRecCache = 102400;
const int nMaxPkgLen = 4096;
const int nMaxOneLoadPkgLen = 1024;
const int nReadFileBuffer = 1024;
const int nSmsNumLen = 32;
const int nSmsContentLen = 2048;
const int nWcbBuffLen = 1024;
const int nMaxConfigContentLen = 512;

using Json = nlohmann::json;
using DllShrPtr = std::shared_ptr<common_template::DllParser>;