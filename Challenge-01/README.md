# Driver Explaination
This driver handles one IOCTL for changing the thread priority of a given thread id in the system process.<br>
The driver takes a struct which contains the thread id and the priority value as you can see in the code.

You can load the driver and use the client program to test it!<br>
Oh no, you got a BSOD :(<br>
Good luck! (Don't change much of the logic, the fix is very simple once you understand it).