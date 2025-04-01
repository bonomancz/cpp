
TCP KEEP-ALIVE server
---------------------

About:
---------------------
Tcp keep-alive server for Windows written in the C++ programming language.
Tcp keep-alive server is a command line tool.
Keep-alive server is used together with TCP Keep-alive client in situations when you need to verify quality of TCP traffic transmission.
Server receives PING message from tcp keep-alive client and sends back PONG reply and both sides logs their activity into log file with timestamp in milliseconds.
TCP server uses its own thread pool for thread organization and management.  
It also uses the winsock2.h library in its own class to manage network connections in separate threads.  

Command line usage:
---------------------
tcpkeepaliveserver2.exe [--config=tcpkeepaliveserver2.conf(string)]<br /><br />

Arguments:<br />
--config=:      path to main configuration file (included with source code)<br /><br />

Example usage: <br />
tcpkeepaliveclient2.exe --config=tcpkeepaliveserver2.conf