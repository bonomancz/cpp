
About:
-----------
Wallboard screens service written in C++ programming language.  
Wallboard screens periodically creates screenshot .png images from defined URL.
Look for configuration in main configuration file wallboardScreens.conf.
It uses MS EDGE as default webbrowser.
It uses the thread library for launching all configured screenshots simultaneously at the same moment.
I am sharing this software Wallboard screens freely available for download and for further possible modifications and extensions.  


Starting wallboardScreens:
--------------------------
.\wallboardScreens --config=.\wallboardScreens.conf


Configuration options:
----------------------

; main loop interval ( > 0 )
service_loop_timeout = 10000

; wallboard thread terminate interval ( > 0 )
service_thread_timeout = 8000

; default browser location path
service_browser_path = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"
