# EECE5811-HW2

Lock Analysis (2pt)
Consider “Figure 28.9: Lock With Queues, Test-and-set, Yield, And Wakeup.” What happens if you remove the code about guard 
(removing Line 3, Line 9, Line 14, Line 15, Line 18, Line 21, Line 27, Line 28, Line 34)? Is the new algorithm still correct? 

You can assume that there is no issue of wakeup/waiting race.

If you believe the new lock algorithm is still correct, briefly explain why.

If you think it’s incorrect, present an execution (by the malicious scheduler), and explain what property is violated.

Your execution should clearly show the relevant interleaving between threads.

Key concepts: lock, algorithm design and analysis

ANSWER: running file hwf.c
THE ALGORITHM renders the guard is missing and multiple threads were able to get to the critical section thus mutual exclusiveness is violated.

the output is as follows:

./a.out
Thread 139751953311488 ENTER critical section

Thread 139751944918784 ENTER critical section

Thread 139751953311488 EXIT critical section

Thread 139751944918784 EXIT critical section



===============================================================================================================================

QLock Analysis II (1pt)
Consider “Figure 28.6: Using LL/SC To Build A Lock” What happens if you replace Line 12 by 
		lock->flag = lock->flag - 1;

At first glance, this may seem correct: a thread holding the lock should observe flag == 1, so subtracting 1 should release the lock.


Is the modified lock implementation correct?

If you believe it is correct, briefly explain why.

If you believe it is incorrect, give an execution that demonstrates the problem and explain which correctness property is violated.

Hint: Consider all possible executions, including cases in which application code uses the lock incorrectly.


Key concepts: lock, algorithm design and analysis



Answer: this modified lock is incorrect due to multiple things, 
any thread can change the flag even if it doesn't own it, second : the flag might go from 1 to 0 and -1 and if that happened the lock is permenantly broken.
if multiple threads were able to access critical section the mutual exclusion is violated.


==================================================================================================


Lock Analysis III (1pt)
Consider “Figure 28.9: Lock With Queues, Test-and-set, Yield, And Wakeup” 
Explain why adding setpark() right before “m->guard = 0;” solves the “Wakeup/Waiting Race” issue that we discussed in lecture.


Your explanation should describe an execution in which the race occurs without setpark(), and then explain what changes when setpark() is added.


Key concepts: lock, algorithm design and analysis
Answer:
  risk of deadlock or thread goes to sleep forever== lock is broken
let's assume we have two threads A and B and A had the lock.
1st senario: without setpark():

B acquired the guard, B sees the flag ==1 , B added to the Queue queue_add(q,B) 
then B releases guard , m->guard=0, B intendes to sleep but not yet called park()
context switch to A 
A acquired guard, A de queued B, A calls unpark(b) but what? B didn't sleep that wakeup is lost now
then B resumes and calls park() then B goes to sleep FOREVERRRRRR, 

Senario B: setpark() usage before m->guard=0;


B acquired the guard, B sees the flag ==1 , B added to the Queue queue_add(q,B) , setpark(B), B is about to sleep,
B release the guard m->guard=0, kernel knows B is about to sleep,
A acquired guard, dequeues B, calls unpark B, b doesn't miss the wakeup call because it was setparked , b calls park but returns immediately and doesnot sleep and b acquired the lock now.

================================================================================================================================================
Q2-4

Comparing Lock algorithms (2 pt)
Implement Ticket lock (Figure 28.7) and compare-and-swap spin lock (Ch. 28.9) 


Do not use a built-in mutex to implement either lock. Instead, implement the lock algorithms using the atomic operations provided by your language.
You may only use Go, C, or C++.


Design an experiment to compare the lock acquisition waiting time of the two lock implementations under different levels of contention.


For one lock acquisition, define waiting time as: the time between a thread/goroutine starting its attempt to acquire the lock and successfully acquiring it.

Your experiment should vary the amount of contention. Use the same workload and experimental setup when comparing the two locks. Run enough lock acquisitions and trials to obtain meaningful results.

In your submission, include: 
A description of your implementation.
Instructions for compiling and running your program.
A description of your benchmark design.
Your experimental results.
A brief analysis of the results, including what you observed as contention increased.


Answer:







