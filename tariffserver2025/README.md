
Tariffserver server 2025, Avaya PBX
-----------------------------------

About:
-----------------------------------
C++ Linux network billing server for Avaya PBX. It uses its own network stack with thread pool.<br />
It stores billing sentences in mysql database.<br />
It can be easily adapted for any other database type like postgresql and others.<br />
Extra header files used.<br />
<thread>
<mutex>
<socket>
<sstream>
<deque>
<mariadb/mysql.h>

Command line usage:
-----------------------------------
tariffserver2025 [--config=tariffserver.conf(string)]<br /><br />

Arguments:<br />
--config=       path to main configuration file (included with source code)<br /><br />

Example usage: <br />
tariffserver2025 --config=tariffserver.conf