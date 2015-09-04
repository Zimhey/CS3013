#include <iostream>
#include <unistd.h>
#include <sys/types.h>

using namespace std;


int main(int argc, char* argv[])
{
	char *c[5];
	c[0] = "ls";
	c[1] = "-la";
	c[2] = 0;
	execvp(c[0], c);
	cout << "Test" << endl;

	return 0;
}
