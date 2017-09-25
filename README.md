# Master-Worker-e-x-calculation

This Project describes the functionality of select and epoll in event driven programming as well as how file descriptors,pipes,fork,exec and processes work.

The implementation takes an x and n and a polling mechanism like select and epoll and calculates e^x. This particular endpoint is calculated by spawning n number of processes which calculates x^i/i! where i ranges from 0 to n. The child processes communicate with parent process through pipes and parent has event driven functionality which is implemented via select and epoll mechanisms.

To compile the program:
make

To run the program:
make run
