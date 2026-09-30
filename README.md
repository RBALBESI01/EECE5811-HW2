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

Lock Analysis II (1pt)
Consider “Figure 28.6: Using LL/SC To Build A Lock” What happens if you replace Line 12 by 
		lock->flag = lock->flag - 1;

At first glance, this may seem correct: a thread holding the lock should observe flag == 1, so subtracting 1 should release the lock.


Is the modified lock implementation correct?

If you believe it is correct, briefly explain why.

If you believe it is incorrect, give an execution that demonstrates the problem and explain which correctness property is violated.

Hint: Consider all possible executions, including cases in which application code uses the lock incorrectly.


Key concepts: lock, algorithm design and analysis


