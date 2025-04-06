
NTP client
---------------------

About:
---------------------
Windows NTP client written in the C++ programming language.
NTP client is a command line tool.
NTP Client sends udp request a convert NTP server response into time info with milliseconds.
TCP client uses its own thread pool for thread organization and management.  
It also uses the winsock2.h library in its own class to manage network connections in separate threads.  

Command line usage:
---------------------
ntpClient [server host/ip (string)] [port (integer)] [threads count (integer)] [sleep ms (integer)]<br /><br />

Arguments:<br />
server host:    remote server host/ip<br />
port:           remote tcp port<br />
threads count:  threads used for communication<br />
sleep:          loop interval<br /><br />

Example usage:<br />
ntpClient 192.168.1.1 123 1 1000