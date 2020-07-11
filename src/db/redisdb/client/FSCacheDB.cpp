#include "FSCacheDB.h"  

using namespace db_redis;

static FSCacheDB* s_sharedCache = NULL;
FSCacheDB::FSCacheDB()
{
	s_sharedCache = this;
}

FSCacheDB* FSCacheDB::sharedCacheDB()
{
	return s_sharedCache;
}