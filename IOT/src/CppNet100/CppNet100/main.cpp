#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_
//以C的方式导出
extern "C"
{
	//导出库函数给外部使用
	int _declspec(dllexport) Add(int a, int b)
	{
		return a + b;
	}
}

#endif // !_CPP_NET_100_DLL_H_
