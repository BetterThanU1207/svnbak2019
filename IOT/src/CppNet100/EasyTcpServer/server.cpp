#include "EasyTcpServer.hpp"

int main()
{
	EasyTcpServer server;
	server.BindPort(nullptr, 4567);
	server.ListenPort(5);
	while (server.isRun())
	{
		server.OnRun();
	}
	printf("���˳������������\n");
	getchar();
	return 0;
}