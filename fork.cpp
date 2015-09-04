#include <iostream>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char* argv[])
{
	int pid;
	if((pid = fork()) < 0)
	{
		cerr << "Error forking\n";
	}
	else if (pid == 0)
	{
		for(int i = 0; i < 5; i++)
			cout << "Child\n";
		exit(0);
	}
	else
	{	
		
		for(int i = 0; i < 5; i++)
			cout << "Parent\n";
		exit(0);
	}
	return 0;
}
