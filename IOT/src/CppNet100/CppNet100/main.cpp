#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_
//��C�ķ�ʽ����
extern "C"
{
	//�����⺯�����ⲿʹ��
	int _declspec(dllexport) Add(int a, int b)
	{
		return a + b;
	}
}

#endif // !_CPP_NET_100_DLL_H_
