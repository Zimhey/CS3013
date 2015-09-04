/*
	Corey Dixon
	CS3013: Proj1
*/
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctime>
#include <iomanip>
#include <string.h>
#include <vector>
#include <sstream>

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

ProcessInfo runCommand(char* cmd[], bool background, int shell_id = 1)
{
	int pid;
	int wallTime;
	struct timeval tv[2];
	struct rusage res;
	ProcessInfo pInfo;
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
			   Skipping wallTime for background a process */
			gettimeofday(&tv[1], NULL);
			wallTime = timevalToMS(tv[1]) - timevalToMS(tv[0]);
			cout << "Wall Time: " << wallTime << endl;
			showResourceUsage();
		}
	}
	return pInfo;
}

void pollChildren(vector<ProcessInfo> &procs, bool wait)
{
	vector<vector<ProcessInfo>::iterator> toRemove;
	
	for(vector<ProcessInfo>::iterator it = procs.begin();
		it != procs.end(); )
	{
		int status;
		if(!wait)
			waitpid(it->pid, &status, WNOHANG);
		else
			waitpid(it->pid, &status, 0);
		if(WIFEXITED(status))
		{
			cout << '[' << it->shell_id << "] "
			<< it->pid << " Complted" << endl;
			it = procs.erase(it);
			showResourceUsage();
		}
		else
			it++;
		
	}
}

void showJobs(vector<ProcessInfo> &procs)
{
	for(vector<ProcessInfo>::iterator it = procs.begin();
		it != procs.end(); it++ )
	{	cout << '[' << it->shell_id << "] "
			<< it->pid << " " << it->name << endl;

	}
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
		char commandLine[128]; // assuming line is no more than 128 chars
		char *oneCommand; // single command
		vector<ProcessInfo> processes; // list of processes
		int shell_id = 1;
		do
		{
			bool background;
			char cwd[1024];
			getcwd(cwd, 1024);
			cout << cwd << " ==>"; // I like seeing my pwd
			cin.getline(commandLine, 128);
			pollChildren(processes, false);
			stringstream buffer;
			buffer << commandLine;
			/* this got messy while getting it to work
			would clean up code more however i'm running out
			of time and need to turn this in */
			while(!buffer.eof() && buffer.str() != "")
			{
				string str = buffer.str();
				background = false;
				strcpy(commandLine, buffer.str().c_str());
				int nextCmd;
				if(strstr(commandLine, "&")) // has a & in line TODO handle multiple commands
				{
					oneCommand = strtok(commandLine, "&"); // get a single command
					background = true;
					nextCmd = strlen(oneCommand) + 2;
					nextCmd = min((int)strlen(buffer.str().c_str()), nextCmd);
					buffer.str(str.substr(nextCmd));
				}
				else
				{
					oneCommand = commandLine;
					nextCmd = strlen(oneCommand);
				}
				buffer.str(str.substr(nextCmd)); // get rid of the last command
			
				cmd[0] = strtok(oneCommand, " ");
				int i = 1;
				while(cmd[i] = strtok(NULL, " ")) // convert to an array of cstrings for exec
					i++;		
				if(strcmp(cmd[0], "cd") == 0)
				{
					chdir(cmd[1]);
					getcwd(cwd, 1024);
					cout << "Directory change to " << cwd << endl;
				}
				else if(strcmp(oneCommand, "jobs") == 0)
					showJobs(processes);
				else if(strcmp(oneCommand, "exit") != 0 && !cin.eof())
				{
					if(processes.empty()) // if we have no processes the shell id is 1
						shell_id = 1;
					if(background) // only add to list of processes if its in the background
						processes.push_back(runCommand(cmd, background,
								shell_id++));
					else
					runCommand(cmd, false, shell_id);			
				}
				else
				{
					while(!processes.empty()) // wait for all processes to finish
					{
						pollChildren(processes, true);
					}
					return 0;  // exit
				
				}
			}
		}
		while(true);
	}

	return 0;
}
