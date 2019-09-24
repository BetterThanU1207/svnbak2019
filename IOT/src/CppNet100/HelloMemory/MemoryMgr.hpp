#ifndef _MEMORYMGR_HPP_
#define _MEMORYMGR_HPP_

#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)//宏代替函数层级、参数一定要注意
#else
	#define xPrintf(...)
#endif // _DEBUG


#define MAX_MEMORY_SIZE 1024
class MemoryAlloc;

//内存块 最小单元
class MemoryBlock
{
public:
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//所属大内存块（池）
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};

//内存池
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
		//释放内存池
		if (_pBuf)
		{
			free(_pBuf);
		}
	}

	//申请内存
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
			if (NULL != pReturn)//malloc的内存需要做判空处理否则警告C6011 申请失败则为NULL
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
		return (char*)pReturn + sizeof(MemoryBlock);//蓝色区域才是用户实际可用内存
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)//共享内存的情况
		{
			return;
		}
		if (pBlock->bPool)//在内存池中
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else//池外 向系统直接申请的 直接释放
		{
			free(pBlock);//::优先查找系统库函数
		}		
	}

	//初始化
	void initMemory()
	{
		//断言 不满足条件抛出错误
		assert(nullptr == _pBuf);
		if (_pBuf)
		{
			return;
		}
		//计算内存池的大小
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSize * _nBlockSize;
		//向系统申请池内存
		_pBuf = (char*)malloc(bufSize);
		assert(NULL != _pBuf);
		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//遍历内存块进行初始化
		MemoryBlock* pTemp1 = _pHeader;
		
		for (size_t i = 1; i < _nBlockSize; i++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*) (_pBuf + (i* realSize) );
			pTemp2->bPool = true;
			pTemp2->nID = i;//内存块和数组下标顺序对应
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSize;
	//内存单元的数量
	size_t _nBlockSize;
};

//便于在声明类成员变量时初始化MemoryAlloc的成员数据
template<size_t nSize, size_t nBlockSize>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//不同位数操作系统下的指针字节数 64-8,32-4
		const size_t n = sizeof(void*);
		//内存对齐 61/8=7    61%8=5     7*8+8
		_nSize = (nSize/n)*n + (nSize % n ? n : 0);
		_nBlockSize = nBlockSize;
	}
private:
};

//内存管理工具 单例模式
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
	{//单例模式 静态
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else//超出内存池内存块大小，直接向系统申请
		{
			MemoryBlock*  pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			if (NULL != pReturn)//malloc的内存需要做判空处理否则警告C6011 申请失败则为NULL
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

	//释放内存
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem:%llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)//在内存池中
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//池外 向系统直接申请的 直接释放
		{
			if (--pBlock->nRef == 0)
			{
				//释放实际的
				free(pBlock);//::优先查找系统库函数
			}
		}		
	}

	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
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
	//内存池映射数组 多申请一个数组元素
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};


#endif // !_MEMORYMGR_HPP_
