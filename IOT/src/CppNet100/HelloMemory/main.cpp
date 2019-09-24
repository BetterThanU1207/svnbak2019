#include <stdlib.h>
#include "Alloctor.h"

int main()
{
	char* data[12];
	for (size_t i = 0; i < 12; i++)
	{
		data[i]  = new char[60];
		
	}
	for (size_t i = 0; i < 12; i++)
	{
		delete[] data[i];
	}
	return 0;
}