
TCP KEEP-ALIVE client
---------------------

About:
---------------------
Tcp keep-alive client for Windows written in the C++ programming language.
Tcp keep-alive client is a command line tool.
Keep-alive client is used together with TCP Keep-alive server in situations when you need to verify quality of TCP traffic transmission.
Client sends PING message and waits for PONG reply from server and both sides logs their activity into log file with timestamp in milliseconds.
TCP client uses its own thread pool for thread organization and management.  
It also uses the winsock2.h library in its own class to manage network connections in separate threads.  

Command line usage:
---------------------
tcpkeepaliveclient2 [server host/ip (string)] [port (integer)] [threads count (integer)] [sleep ms (integer)]<br /><br />

Arguments:<br />
server host:    remote server host/ip<br />
port:           remote tcp port<br />
threads count:  threads used for communication<br />
sleep:          loop interval<br /><br />

Example usage:<br />
tcpkeepaliveclient2 192.168.1.1 9999 10 200