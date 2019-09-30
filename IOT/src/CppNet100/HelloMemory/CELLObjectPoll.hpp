#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_

#include <stdlib.h>
#include <mutex>
#include <assert.h>

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)//宏代替函数层级、参数一定要注意
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
#endif // _DEBUG

template<class Type, size_t nPoolSize>
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		initPool();
	}
	~CELLObjectPool()
	{
		if (_pBuf)
		{
			delete[] _pBuf;
		}
		
	}
private:
	class NodeHeader
	{
	public:
		//下一块位置
		NodeHeader* pNext;
		//内存块编号
		int nID;
		//引用次数
		char nRef;
		//是否在内存池中
		bool bPool;
	private:
		//预留
		char c1;
		char c2;
	};
public:
	//释放对象内存
	void freeObjMemory(void* pMem)
	{
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		assert(1 == pBlock->nRef);
		xPrintf("freeObjMemory:%llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}

	//申请对象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			if (NULL != pReturn)//malloc的内存需要做判空处理否则警告C6011 申请失败则为NULL
			{
				pReturn->bPool = false;
				pReturn->nID = -1;
				pReturn->nRef = 1;
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
		xPrintf("allocObjMemory:%llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return (char*)pReturn + sizeof(NodeHeader);//蓝色区域才是用户实际可用内存
	}

private:
	//初始化对象池
	void initPool()
	{
		//断言
		assert(nullptr == _pBuf);
		if (_pBuf)
		{
			return;
		}
		//计算对象池大小
		size_t realSize = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSize * realSize;
		//申请池的内存
		_pBuf = new char[n];//优先向内存池中申请
		//初始化内存池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//遍历内存块进行初始化
		NodeHeader* pTemp1 = _pHeader;

		for (size_t i = 1; i < nPoolSize; i++)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (i * realSize));
			pTemp2->bPool = true;
			pTemp2->nID = i;//内存块和数组下标顺序对应
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

private:
	//头部内存单元
	NodeHeader* _pHeader;
	//对象池内存地址
	char* _pBuf;
	//
	std::mutex _mutex;
};

template<class Type, size_t nPoolSize>
class ObjectPollBase
{
public:
	void* operator new (size_t nSize)
	{
		return objectPool().allocObjMemory(nSize);
	}

	void operator delete (void* p)
	{
		objectPool().freeObjMemory(p);
	}
	template<typename ...Args>//不定参数 可变参数
	static Type* createObject(Args ... args)
	{
		Type* obj = new Type(args...);
		return obj;
	}

	static void destroyObject(Type* obj)
	{
		delete obj;
	}
private:
	typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;
	static ClassTypePool& objectPool()
	{
		//静态对象池CELLObjectPool对象
		static ClassTypePool sPool;
		return sPool;
	}
};


#endif // !_CELLObjectPool_hpp_
