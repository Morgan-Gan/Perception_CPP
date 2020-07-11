#ifndef DATASVR_CSMARTBUFFER_H_
#define DATASVR_CSMARTBUFFER_H_

#include <string.h>

namespace common_cmmobj
{
	class CSmartBuffer
	{
#pragma pack(push, 1)
		struct
		{
			int step_size;
			int allocated_size;
			int data_size;
			char* buffer;
		} m_Data;
#pragma pack( pop )

	public:
		enum { Error = (int)-1 };

	public:
		CSmartBuffer(int step_size = 1024);
		CSmartBuffer(int allocated_size, int step_size = 1024);
		CSmartBuffer(const CSmartBuffer& copy);
		virtual ~CSmartBuffer();

	public:
		const char* append(const char* buf, int len);
		const char* append(const char single);
		const char* append(const CSmartBuffer& buffer);
		int length() const;
		const char* data() const;
		const char* data(const char* buf, int len);
		void empty();
		bool isEmpty() const;

		// += operator. Maps to append
		CSmartBuffer& operator += (const char * suffix);

		// += operator. Maps to append
		CSmartBuffer& operator += (char single);

		// += operator. Maps to append
		CSmartBuffer& operator += (const CSmartBuffer& src);

		// = operator. Map to copy construct
		CSmartBuffer& operator = (const CSmartBuffer& copy);

		// = operator. Map to copy construct
		CSmartBuffer& operator = (const char single);

		int find(const void* data, int length, int from_index = 0);
		int find(char& value);
		void truncate(int length);
		void substr(int nStartIndex, int nEndIndex);
		void ends();
		void endl();
		bool at(char& value, int index);
	};
}
#endif // CSMARTBUFFER_H
