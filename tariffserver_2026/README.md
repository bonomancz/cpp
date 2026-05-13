
Tariffserver server 2026, for Avaya PBX
---------------------------------------

About:
---------------------------------------
C++ Linux network billing server for Avaya PBX.<br />
Written in pure C++ using modern techniques with<br />
TCP server using Epollet in multithreading enviroment with<br /> 
their own circular buffer queues for the highest efficiency.<br />
It uses its own network stack with thread pool.<br />
It stores billing sentences in mysql database.<br />
It can be easily adapted for any other database type like postgresql and others.<br />
Extra header files used.<br />
<thread><br />
<mutex><br />
<socket><br />
<sstream>
<deque><br />
<mariadb/mysql.h><br /><br />

Command line usage:
---------------------------------------
tariffserver [--config=tariffserver.conf(string)]<br /><br />

Arguments:<br />
--config=       path to main configuration file (included with source code)<br /><br />

Example usage: <br />
tariffserver --config=tariffserver.conf