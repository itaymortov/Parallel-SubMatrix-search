# Parallel SubMatrix search

I used MPI and CUDA, MPI is modular and can be used with how many cores you have and how many computers you have. when combined with cuda you can use diffrent computer's GPU to make this program run much faster with big pictures and objects.

I used the master process to get the input from the input file and then the master send the work to the slaves dynamicly, meaning that after the slave finished a job he will recieve another until there is no more jobs left. (I implemented it for 1 master and 1 slave but it can be expanded for more slaves fairly easily), the slaves use their GPU with CUDA to do the calculations and send the result back to the master. a job is to find if 1 object have a match in 1 pictures. in our input there are 5 pictures and 8 objects so there is a maximum of 40 jobs (can be less if the slave find 3 matches for a pictures before getting to the 8th object).
after the work is done the master print the results to a file called output.txt.
 
 ### To make and run the project use the command: 'make' than 'make run'.

