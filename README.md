# multi_threading

1. Each train specified in the input file determines a thread. Threads have same functionalities including 'loading section', 'determine the next' and 'crossing section'.

2. The threads are working independently. There is no overall “controller” thread to control the start and stop of a thread, that is, a thread starts, stops, resumes based on the implementation of mutex and conditional variable.
 
3. There are three mutexes: main track to ensure the main track statues can only modified by one thread, function request() that lock when a thread calls the function and unlock when the main track is available again, and global variable 'lastDirection' which records the direction of the last train.

4. The main will firstly be initialized to available/idle. Once a train gets on the track, the statues is changed to be unavailable.

5. An integer array will be used to keep track all the ID of trains that have finished loading.

6. The function request() will be locked when a train finishes loading. Only the one with the highest priority (if there are multiple trains waiting) could get on the main track, that is, only the current train can modify the data structure.

7. One convar will be used to represent the dispatcher. The convar dispatch is associate the the main track mutex. The main track mutex should be locked once the pthread_con_wait() has been unblocked.

8.
1) for all the trains (lines) in the input file, create a thread for each of them
2) in each thread, load the train using usleep() and print out the statues once a train finished loading
3) if a train is loaded, get the mutex1, get mutex2 and put the train in the waitlist array
4) call findNext() to determine which train has the highest priority in the waitlist array
5) send the highest priority train on the main track and release mutex2 then release mutex1
6) if it is the only train then release mutex1
7) if it is not the highest priority train and the main track is not available then wait
8) when a train is gone get mutex3, assign the direction to global variable lastDirection
9) delete the gone train in the waitlist and move the train in second place to be the next train
10) call findNext() again and release the mutex2
11) when the track is available again release mutex1
