#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <ctime>
#include <iomanip>

using namespace std;


int main(int argc, char* argv[])
{	
	int pid;
	clock_t cpu_time = clock();
	double wall_time;
	struct timeval tv[2];
	gettimeofday(&tv[0], NULL);
	for(int i = 0; i < argc; i++)
		argv[i] = argv[i + 1];

	if((pid = fork()) < 0)
	{
		cerr << "Error forking process" << endl;
	}
	else if(pid == 0)
	{
		execvp(argv[0], argv);
	}
	else
	{
		waitpid(pid, NULL, 0);
	}
	
	cpu_time = (clock() - cpu_time);
	gettimeofday(&tv[1], NULL);
	wall_time = (tv[1].tv_sec - tv[0].tv_sec) * 1000;
	wall_time += (tv[1].tv_usec - tv[0].tv_usec) / 1000;
	cout << "CPU Time: "  << ((double)cpu_time / CLOCKS_PER_SEC * 1000) << endl;
	cout << "Wall Time: " << wall_time << endl;
	return 0;
}
