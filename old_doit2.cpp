#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctime>
#include <iomanip>
#include <string>

using namespace std;

struct ProcessInfo
{
	string name;
	int pid;
	int shell_id;
};

int timevalToMS(struct timeval tv)
{
	int ms = tv.tv_sec * 1000;
	ms += tv.tv_usec / 1000;
	return ms;
}

void showResourceUsage()
{
	struct rusage res;
	getrusage(RUSAGE_CHILDREN, &res);

	int userTime = timevalToMS(res.ru_utime);
	int systemTime = timevalToMS(res.ru_stime);

	cout << "User CPU Time: " << userTime << endl;
	cout << "System CPU Time: " << systemTime << endl;
	cout << "Time Slice Expired: " << res.ru_nivcsw << endl;
	cout << "Waiting for a resource: " << res.ru_nvcsw << endl;
	cout << "Major page faults: " << res.ru_majflt << endl;
	cout << "Minor page faults: " << res.ru_minflt << endl << endl;
	
}

struct ProcessInfo runCommand(char* cmd[], bool background, int shell_id = 1)
{
	int pid;
	int wallTime;
	struct timeval tv[2];
	struct rusage res;
	struct ProcessInfo pInfo;
	pInfo.name = cmd[0];
	pInfo.shell_id = shell_id;

	gettimeofday(&tv[0], NULL);

	if((pid = fork()) < 0)
	{
		cerr << "Error forking process" << endl;
		exit(-1);
	}
	else if(pid == 0)
	{
		execvp(cmd[0], cmd);
		exit(0);
	}
	else
	{	
		pInfo.pid = pid;
		cout << '[' << shell_id << "] "  << pid << endl;

		if(!background)
		{
			waitpid(pid, NULL, 0);
			/* Couldn't figure out how to calculate wall time
			   without waiting for the process to end. 
			   Skipping wallTime for background processea.s */
			gettimeofday(&tv[1], NULL);
			wallTime = timevalToMS(tv[1]) - timevalToMS(tv[0]);
			cout << "Wall Time: " << wallTime << endl;
			showResourceUsage();
		}
	}
	return pInfo;
}

int main(int argc, char* argv[])
{	
	if(argc > 1) // has arguements
	{
		for(int i = 0; i < argc; i++)
			argv[i] = argv[i + 1];
		runCommand(argv, false);
	}
	else	// use our shell!
	{
		char *cmd[32]; // assuming no more than 32 arguements
		char *commandLine = new char[128]; // assuming line is no more than 128 chars
		vector<struct ProcessInfo> processes; // list of processes
		do
		{
			char cwd[1024];
			getcwd(cwd, 1024);
			cout << cwd << " ==>"; // I like seeing my pwd
			cin.getline(commandLine, 128);
			cmd[0] = strtok(commandLine, " ");
			int i = 1;
			while(cmd[i] = strtok(NULL, " "))
				i++;
			if(strcmp(cmd[0], "cd") == 0)
				chdir(cmd[1]);
			else if(strcmp(commandLine, "exit") != 0 && !cin.eof())
			processes.push_back(runCommand(cmd, false));

			else
				break;	// TODO wait for all processes
		}
		while(true);

	}

	return 0;
}
