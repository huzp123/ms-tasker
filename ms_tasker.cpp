#include <iostream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

using namespace std;

const char* pid_file = "./ms-tasker.pid";
int fd = -1;
pid_t pid = -1, ppid = -1;

map<string, map<string, string> > ini_parser(const string &filename)
{
    map<string, map<string, string> > ini;
    string line;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Open INI file failed!" << endl;
        return ini;
    }

    string section_name = "";
    map<string, string> section_value;

    while(getline(file, line))
    {
        if(line.empty()) {continue;}
        if(line[0] == '#' || line[0] == ';') {continue;}
        if(line[0] == '['){
            if(section_name.length() > 0) {
                ini[section_name] = section_value;
            }
            section_name = "";
            section_value.clear();

            section_name = line.substr(1, line.length() - 2);
            continue;
        }

        int pos = line.find("=");
        if(pos == -1) {continue;}

        string key = line.substr(0, pos);
        string val = line.substr(pos + 1, line.length());

        section_value[key] = val;
    }
    
    if(section_name.length() > 0) {
        ini[section_name] = section_value;
    }

    file.close();

    return ini;
}

void execute_thread(const string& cmd, unsigned long sleep_milli_sec, const string& output_file = "")
{
    while(true)
    {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe == nullptr)
        {
            cerr << "Open pipe failed!" << endl;
            return;
        }
        if(output_file.length() > 0) {
            ofstream ofile(output_file, ios::app | ios::out);
            if(!ofile.is_open()) {
                pclose(pipe);
                continue;
            }
            ofile << "\r\n";
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                ofile << buffer;
            }
            ofile.close();
        }
        pclose(pipe);
        
        this_thread::sleep_for(chrono::milliseconds(sleep_milli_sec));
    }
}

void work_process(map<string, map<string, string> > &settings)
{
    vector<thread> task_threads;
    for(const auto &section : settings) {
        string command = "", time_interval = "", output_file = "";
        for(const auto &pair : section.second) {
            if("command" == pair.first) {
                command = pair.second;
            }
            if("time_interval" == pair.first) {
                time_interval = pair.second;
            }
            if("output_file" == pair.first) {
                output_file = pair.second;
            }
        }
        if(command.length() > 0) {
            if(time_interval == "") {time_interval = "1";} 
            task_threads.push_back(thread(execute_thread, command, stoul(time_interval), output_file));
        }
    }

    for(auto iter = task_threads.begin(); iter != task_threads.end(); iter++) {
        iter->join();
    }
}

void exit_handler(int signum)
{
    if(getpid() != ppid) exit(0);

    kill(pid, SIGKILL);
    close(fd);
    remove(pid_file);
    exit(0);
}

int main(int argc, char* argv[])
{
    if (ppid < 0) {
        fd = open(pid_file, O_CREAT | O_EXCL | O_RDWR, 0666);
        if (fd < 0) {
            cerr << "ms-tasker process is running" << endl;
            return 0;
        }
        ofstream ofs(pid_file);
        ofs << getpid();
        ofs.close();
        ppid = getpid();
    }
    string ini_file;
    if(argc < 2) {
        ini_file = "./config.ini";
    }else {
        ini_file = argv[1];
    }
    
    signal(SIGTERM, exit_handler);
    signal(SIGKILL, exit_handler);
    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
    
    map<string, map<string, string> > settings = ini_parser(ini_file);
    if(settings.size() < 1){return 0;}
    
    while(true) {
        pid = fork();
        if(pid == -1) {
            cerr << "worker process start failed." << endl;
            return 0;
        }else if(pid > 0) {
            wait(nullptr);
        }else {
            work_process(settings);
        }
    }
    
    return 0;
}
