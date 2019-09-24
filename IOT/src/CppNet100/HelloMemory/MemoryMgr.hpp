#ifndef _MEMORYMGR_HPP_
#define _MEMORYMGR_HPP_

#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)//����溯���㼶������һ��Ҫע��
#else
	#define xPrintf(...)
#endif // _DEBUG


#define MAX_MEMORY_SIZE 1024
class MemoryAlloc;

//�ڴ�� ��С��Ԫ
class MemoryBlock
{
public:
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�������ڴ�飨�أ�
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char c1;
	char c2;
	char c3;
};

//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;
	}
	~MemoryAlloc()
	{
		//�ͷ��ڴ��
		if (_pBuf)
		{
			free(_pBuf);
		}
	}

	//�����ڴ�
	void* allocMemory(size_t nSize)
	{
		if (!_pBuf)
		{ 
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			if (NULL != pReturn)//malloc���ڴ���Ҫ���пմ�����򾯸�C6011 ����ʧ����ΪNULL
			{
				pReturn->bPool = false;
				pReturn->nID = -1;
				pReturn->nRef = 1;
				pReturn->pAlloc = nullptr;
				pReturn->pNext = nullptr;
			}			
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMemory:%llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return (char*)pReturn + sizeof(MemoryBlock);//��ɫ��������û�ʵ�ʿ����ڴ�
	}
	//�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)//�����ڴ�����
		{
			return;
		}
		if (pBlock->bPool)//���ڴ����
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else//���� ��ϵͳֱ������� ֱ���ͷ�
		{
			free(pBlock);//::���Ȳ���ϵͳ�⺯��
		}		
	}

	//��ʼ��
	void initMemory()
	{
		//���� �����������׳�����
		assert(nullptr == _pBuf);
		if (_pBuf)
		{
			return;
		}
		//�����ڴ�صĴ�С
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSize * _nBlockSize;
		//��ϵͳ������ڴ�
		_pBuf = (char*)malloc(bufSize);
		assert(NULL != _pBuf);
		//��ʼ���ڴ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//�����ڴ����г�ʼ��
		MemoryBlock* pTemp1 = _pHeader;
		
		for (size_t i = 1; i < _nBlockSize; i++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*) (_pBuf + (i* realSize) );
			pTemp2->bPool = true;
			pTemp2->nID = i;//�ڴ��������±�˳���Ӧ
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

protected:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С
	size_t _nSize;
	//�ڴ浥Ԫ������
	size_t _nBlockSize;
};

//�������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
template<size_t nSize, size_t nBlockSize>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//��ͬλ������ϵͳ�µ�ָ���ֽ��� 64-8,32-4
		const size_t n = sizeof(void*);
		//�ڴ���� 61/8=7    61%8=5     7*8+8
		_nSize = (nSize/n)*n + (nSize % n ? n : 0);
		_nBlockSize = nBlockSize;
	}
private:
};

//�ڴ������ ����ģʽ
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}
	~MemoryMgr()
	{

	}

public:
	static MemoryMgr& Instance()
	{//����ģʽ ��̬
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else//�����ڴ���ڴ���С��ֱ����ϵͳ����
		{
			MemoryBlock*  pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			if (NULL != pReturn)//malloc���ڴ���Ҫ���пմ�����򾯸�C6011 ����ʧ����ΪNULL
			{
				pReturn->bPool = false;
				pReturn->nID = -1;
				pReturn->nRef = 1;
				pReturn->pAlloc = nullptr;
				pReturn->pNext = nullptr;
				xPrintf("allocMem:%llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
				return (char*)pReturn + sizeof(MemoryBlock);
			}
		}
	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem:%llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)//���ڴ����
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//���� ��ϵͳֱ������� ֱ���ͷ�
		{
			if (--pBlock->nRef == 0)
			{
				//�ͷ�ʵ�ʵ�
				free(pBlock);//::���Ȳ���ϵͳ�⺯��
			}
		}		
	}

	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (size_t i = nBegin; i <= nEnd; i++)
		{
			_szAlloc[i] = pMemA;
		}
	}
private:
	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloctor<128, 10> _mem128;
	MemoryAlloctor<256, 10> _mem256;
	MemoryAlloctor<512, 10> _mem512;
	MemoryAlloctor<1024, 10> _mem1024;
	//�ڴ��ӳ������ ������һ������Ԫ��
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};


#endif // !_MEMORYMGR_HPP_
