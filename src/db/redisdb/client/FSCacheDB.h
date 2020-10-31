/**
*	cache tool
*	Created on : 2016-11-24
*		Author : yuanmj
*/

#ifndef FSCACHEDB_H_
#define FSCACHEDB_H_

#include <stdio.h> /* for size_t */
#include <stdarg.h> /* for va_list */

namespace REDIS_DB
{
	class FSCacheConfig
	{
	};

	class FSCacheData
	{
	public:
		virtual bool isNil(int index = 0) = 0;
		virtual int getInt(int index = 0) = 0;
		virtual float getFloat(int index = 0) = 0;
		virtual const char* getString(int index = 0) = 0;
		virtual bool getBool(int index = 0) = 0;
		virtual char* getBinary(int index = 0) = 0;
		virtual size_t getLength(int index = 0) = 0;
		virtual size_t getArrayLength() = 0;
		virtual bool isArray() = 0;
	};

	class FSCacheDB
	{
	public:
		FSCacheDB();

	public:

		/**
		* set string to key
		* @key		:  key
		* @value	:  value
		* @return £º bool
		*/
		virtual bool setString(const char* key, const char* value, unsigned int expire = 0) = 0;

		/**
		*  set binary data to key
		* @key		:  key
		* @value	:  value
		* @return £º bool
		*/
		virtual bool setBinary(const char* key, unsigned char* data, size_t len) = 0;

		/**
		* hash set a string to hash
		* @hash_name : hash name
		* @key		  : hash_key
		* @value	  : value
		* @return : bool
		*/
		virtual bool setHashString(const char* hash_name, const char* key, const char* value) = 0;

		/**
		*  set a binary data to hash
		* @hash_name : hash name
		* @key		  : hash_key
		* @value	  : value
		* @len		  : len
		* @return £º bool
		*/
		virtual bool setHashBinary(const char* hash_name, const char* key, unsigned char* data, size_t len) = 0;


		/**
		* add a string to list tail
		* @list_name : list name
		* @value	  : data
		* @return    : return list len
		*/
		virtual int  pushStringToList(const char* list_name, const char* value) = 0;


		/**
		* insert a binary data to list
		* @list_name : list name
		* @value	  : data
		* @return    : return list len
		*/
		virtual bool  setBinaryToList(const char* list_name, int index, char* data, size_t len) = 0;


		/**
		* insert a string to list
		* @list_name : list name
		* @value	  : data
		* @return    : return list len
		*/
		virtual bool  setStringToList(const char* list_name, int index, const char* value) = 0;

		/**
		* add a binary data to the tail of list
		* @list_name : list name
		* @value	  : data
		* @return    : return the list len
		*/
		virtual int  pushBinaryToList(const char* list_name, char* binary, size_t len) = 0;

		/**
		* add a data to the sorted set
		* @set_name : set name
		* @score	 : the sorted value
		* @value	 : the sorted data
		*/
		virtual void addSortedSet(const char* set_name, int score, const char* value) = 0;

		/**
		* get the ranking by the data
		* @set_name : set name
		* @value	 : get the data ranking
		* @return £º return the ranking
		*/
		virtual int  getSortByString(const char* set_name, const char* value) = 0;

	public:
		/**
		* get one value by key
		* @key 	: key_name
		* @outdata : put out data
		* @return : bool
		*/
		virtual bool getDataFromKey(const char* key, FSCacheData* outdata) = 0;

		/**
		*  get the value of hash memery
		* @hash_name : hash_name
		* @key		  : key
		* @outdata : put out data
		* @return £º bool
		*/
		virtual bool getDataFromHash(const char* hash_name, const char* key, FSCacheData* outdata) = 0;

		/**
		* get the data of ordered set
		* @set_name : set_name
		* @starr	  : start
		* @end		  : end
		* @outdata : put out data
		* @return £º bool
		*/
		virtual bool  getDataFromSoctedSet(const char* set_name, size_t start, size_t end, FSCacheData* outdata) = 0;

		/**
		* get the data of the list
		* @list_name : list_name
		* @starr	 : start
		* @end		  : end
		* @outdata : put out data
		* @return : bool
		*/
		virtual bool getDataFromList(const char* list_name, size_t start, size_t end, FSCacheData* outdata) = 0;

		/**
		* get the size of the list
		* @list_name : list_name
		* @return: list size
		*/
		virtual int getListLen(const char* list_name) = 0;

		/**
		* pop the head of list
		* @list_name : list_name
		* @outdata : putout data
		* @return : bool
		*/
		virtual bool shiftDataFromList(const char* list_name, FSCacheData* outdata) = 0;

		/**
		* remove the tail of the list
		* @list_name : list_name
		* @outdata : putout data
		* @return : bool
		*/
		virtual bool popDataFromList(const char* list_name, FSCacheData* outdata) = 0;

	public:
		/**
		* get one increate value
		* @key:incr key
		* @return : return value that after key incr
		*/
		virtual int incrKey(const char* key) = 0;

		/**
		* del a key
		* @key		:  key
		* @value	:  value
		* @return : bool
		*/
		virtual bool delKey(const char* key) = 0;

	public:
		/**
		* select DB
		* @index:  index	0~16
		*/
		virtual bool selectDB(int index) = 0;

		/**
		* get current using DB
		* @return: using DB numeber
		*/
		virtual int getDBNumber() = 0;

	public:
		/**
		* init connect
		*/
		virtual bool connect(FSCacheConfig* conf) = 0;

		/**
		* close connect
		*/
		virtual bool close() = 0;

		static FSCacheDB* sharedCacheDB();
	};
}
#endif /* FSCACHEDB_H_ */

