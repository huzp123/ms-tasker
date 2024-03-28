# What is MS-Tasker

A lightweight millisecond-level scheduled task executor developed in C++ language, currently only supports Linux systems

# Building

**Compiling requirements:**
* Linux
* GCC 4.8 or later

To build compile at the source folder:

    make    

# Running

To run MS-Tasker with the default configuration, just type (add the & symbol to run in the background):

    ./ms-tasker &
 
If you want to provide your config.ini, you have to run it using an additional parameter (the path of the configuration file):

    ./ms-tasker /path/to/config.ini &

# configuration

Edit to the config.ini file

    [demo]
    command=ls -l /var/
    time_interval=100
    output_file=./ms-task-demo.log

**explain**
* [demo]: Section name is require and unique
* command: Commands to be executed, is require
* time_interval: The interval between repeated command executions default 1 millisecond, is optional
* output_file: The command executed result output to file, is optional

