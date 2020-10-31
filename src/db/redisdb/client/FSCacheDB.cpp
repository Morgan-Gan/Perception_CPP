#include "FSCacheDB.h"  

using namespace REDIS_DB;

static FSCacheDB* s_sharedCache = NULL;
FSCacheDB::FSCacheDB()
{
	s_sharedCache = this;
}

FSCacheDB* FSCacheDB::sharedCacheDB()
{
	return s_sharedCache;
}