#pragma once

static int IsLittleEndian()
{
	const static union
	{
		unsigned int i;
		unsigned char c[4];
	} u = { 0x00000001 };
	return u.c[0];
}

static int MirrorCopy(void* _Dst, const void* _Src, int Size)
{
	int i = 0;
	unsigned char* Dst = (unsigned char*)_Dst;
	unsigned char* Src = (unsigned char*)_Src;
	for (i = 0; i < Size; ++i)
	{
		Dst[i] = Src[Size - i -1];
	}
	return i;
}
// ����ƽ̨���ֽ�ת��Ϊ�����ʽ���ֽ�
static unsigned short WordToBigEndian(unsigned short _Src)
{
	unsigned short tmp = _Src;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned short));
	}
	return tmp;
}
// ����ƽ̨���ֽ�ת��ΪС����ʽ���ֽ�
static unsigned short WordToLittleEndian(unsigned short _Src)
{
	unsigned short tmp = _Src;
	if (!IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned short));
	}
	return tmp;
}
// �������ʽ�洢���ֽڣ�ת��Ϊ��ƽ̨���ֽ�
static unsigned short WordFromBigEndian(unsigned short _Src)
{
	unsigned short tmp = _Src;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned short));
	}
	return tmp;
}
// ��С����ʽ�洢���ֽڣ�ת��Ϊ��ƽ̨���ֽ�
static unsigned short WordFromLittleEndian(unsigned short _Src)
{
	unsigned short tmp = _Src;
	if (!IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned short));
	}
	return tmp;
}

// ����ƽ̨��˫�ֽ�ת��Ϊ�����ʽ��˫�ֽ�
static unsigned int DWordToBigEndian(unsigned int _Src)
{
	unsigned int tmp = _Src;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned int));
	}
	return tmp;
}
// ����ƽ̨��˫�ֽ�ת��ΪС����ʽ��˫�ֽ�
static unsigned int DWordToLittleEndian(unsigned int _Src)
{
	unsigned int tmp = _Src;
	if (!IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned int));
	}
	return tmp;
}
// �����ֽڴ洢��˫�ֽڣ�ת��Ϊ��ƽ̨��˫�ֽ�
static unsigned int DWordFromBigEndian(unsigned int _Src)
{
	unsigned int tmp = _Src;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned int));
	}
	return tmp;
}
// ��С����ʽ�洢��˫�ֽڣ�ת��Ϊ��ƽ̨��˫�ֽ�
static unsigned int DWordFromLittleEndian(unsigned int _Src)
{
	unsigned int tmp = _Src;
	if (!IsLittleEndian())
	{
		MirrorCopy(&tmp, &_Src, sizeof(unsigned int));
	}
	return tmp;
}

//��˱�С��
static float SwapFloat(float fValue)
{
	float tmp = fValue;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &fValue, sizeof(float));
	}
	return tmp;
}

static short SwapWord(short wValue)
{
	short tmp = wValue;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &wValue, sizeof(short));
	}
	return tmp;
}

static unsigned int SwapDword(unsigned int dwValue)
{
    unsigned int tmp = dwValue;
	if (IsLittleEndian())
	{
		MirrorCopy(&tmp, &dwValue, sizeof(unsigned int));
	}
	return tmp;
}