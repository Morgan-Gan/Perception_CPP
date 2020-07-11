#include "SmartBuffer.h"

using namespace common_cmmobj;

CSmartBuffer::CSmartBuffer( int step_size /*1024*/ )
{
    memset( &m_Data, 0, sizeof(m_Data) );
    m_Data.step_size = step_size;
}

CSmartBuffer::CSmartBuffer( int allocated_size, int step_size /*1024*/ )
{
    memset( &m_Data, 0, sizeof(m_Data) );

    m_Data.step_size = step_size;
    m_Data.allocated_size = allocated_size;
    m_Data.buffer = new char[ allocated_size ];
    memset( m_Data.buffer, 0, m_Data.allocated_size );
}

CSmartBuffer::CSmartBuffer( const CSmartBuffer& copy )
{
    memcpy( &m_Data, &copy.m_Data, sizeof(m_Data) );
    m_Data.buffer = new char[ m_Data.allocated_size ];
    memcpy( m_Data.buffer, copy.m_Data.buffer, m_Data.allocated_size );
}

CSmartBuffer::~CSmartBuffer()
{
    if ( m_Data.buffer != NULL )
    {
        delete[] m_Data.buffer;
		m_Data.buffer = NULL;
    }
}

const char* CSmartBuffer::append( const char* buf, int len )
{
	if ( m_Data.data_size + len > m_Data.allocated_size )
	{
		int new_allocated_size = m_Data.data_size + len + m_Data.step_size;
		char* temp_buffer = new char[ new_allocated_size ];
		memset( temp_buffer, 0, new_allocated_size );
		if ( m_Data.data_size > 0 )
		{
			memcpy( temp_buffer, m_Data.buffer, m_Data.data_size );
		}
		if ( m_Data.buffer != NULL )
		{
			delete[] m_Data.buffer;
		}
		m_Data.buffer = temp_buffer;
		m_Data.allocated_size = new_allocated_size;
	}

	memcpy( m_Data.buffer + m_Data.data_size, buf, len );
	m_Data.data_size += len;

	return m_Data.buffer;
}

const char* CSmartBuffer::append( const char single )
{
    return append( &single, 1 );
}

const char* CSmartBuffer::append( const CSmartBuffer& buffer )
{
    return append( buffer.data(), buffer.length() );
}

int CSmartBuffer::length() const
{
    return m_Data.data_size;
}

const char* CSmartBuffer::data() const
{
    return m_Data.buffer;
}

const char* CSmartBuffer::data( const char* buf, int len )
{
    empty();
    return append( buf, len );
}

void CSmartBuffer::empty()
{
	if(NULL != m_Data.buffer)
	{
		delete []m_Data.buffer;
		m_Data.buffer = NULL;
	}
	m_Data.allocated_size = 0;
    m_Data.data_size = 0;
}

bool CSmartBuffer::isEmpty() const
{
    return m_Data.data_size == 0;
}

// += operator. Maps to append
CSmartBuffer& CSmartBuffer::operator += (const char * suffix)
{
    append( suffix, strlen(suffix) );
    return *this;
}

// += operator. Maps to append
CSmartBuffer& CSmartBuffer::operator += (char single)
{
    append( &single, 1 );
    return *this;
}

// += operator. Maps to append
CSmartBuffer& CSmartBuffer::operator += (const CSmartBuffer& src)
{
    append( src.data(), src.length() );
    return *this;
}

// = operator. Map to copy construct
CSmartBuffer& CSmartBuffer::operator = (const CSmartBuffer& copy )
{
    data( copy.data(), copy.length() );
    return *this;
}

// = operator. Map to copy construct
CSmartBuffer& CSmartBuffer::operator = (const char single )
{
    data( &single, 1 );
    return *this;
}

int CSmartBuffer::find ( const void* data, int length, int from_index /* = 0 */  )
{
	if ( (length <= 0) || (from_index >= m_Data.data_size) || (length > m_Data.data_size) )
	{
		return Error;
	}

	int search_depth = m_Data.data_size - length;
	const char* search_for = (const char*) data;
	for ( int i = from_index; i <= search_depth; i++ )
	{
		if ( search_for[ 0 ] != m_Data.buffer[ i ] )
		{
			continue;
		}

		if ( memcmp(search_for, m_Data.buffer + i, length ) == 0 )
		{
			return i;
		}
	}

	return Error;
}

int CSmartBuffer::find (char& value)
{
	for (int i = 0; i < m_Data.data_size; i++)
	{
		if (value == m_Data.buffer[i])
		{
			return i;
		}
	}

	return -1;
}

void CSmartBuffer::truncate( int length )
{
	if ( m_Data.data_size < length )
	{
		empty();
		return;
	}

	int left = m_Data.data_size - length;
	memcpy( m_Data.buffer, m_Data.buffer + length, left );
	m_Data.data_size = left;
}

void CSmartBuffer::substr(int nStartIndex,int nEndIndex)
{
	if(0 > nStartIndex || 0 > nEndIndex || nEndIndex < nStartIndex)
	{
		return;
	}

	int nLen = nEndIndex - nStartIndex + 1;
	if(m_Data.data_size > nLen)
	{
		memcpy(m_Data.buffer,m_Data.buffer + nStartIndex,nLen);

		int right = m_Data.data_size - nLen;

		if(0 < right)
		{
			memset(m_Data.buffer + nLen,0,right);
		}

		m_Data.data_size = nLen;
	}
}

void CSmartBuffer::ends()
{
    char string_end = '\0';
    append( &string_end, 1 );
}

void CSmartBuffer::endl()
{
    char string_end_line = '\n';
    append( &string_end_line, 1 );
}

bool CSmartBuffer::at( char& value, int index )
{
	if ( (index < 0) || (m_Data.data_size <= index) )
	{
		return false;
	}

	value = m_Data.buffer[ index ];
	return true;
}
