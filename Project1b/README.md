<html>

<head>
<title>Project 1b: xv6 Intro</title> 
</head> 

<body text=black bgcolor=white link=#00aacc vlink=#00aacc alink=orange>

<center><table><tr><td width=800pt>

<center>
<font color=#00aacc>
<h1>Project 1b: xv6 Intro</h1> 
</font> 
</center> 

<p>We'll be doing kernel hacking projects in <b>xv6</b>, a port of a classic
version of unix to a modern processor, Intel's x86. It is a clean and
beautiful little kernel.

<p>This first project is just a warmup, and thus relatively light.

<p> The goal of the project is simple: to add one system call to xv6
and create one user-level application that calls it.

<p> The system call is:
<ul>
<li> <b>int getnumsyscallp(void)</b> returns the total number of
  system calls that have been issued by the calling process, not
  including calls to <code>getnumsyscallp()</code> itself.  The count
  should be incremented <b>before</b> a system call is issued, not
  after.  The system call will simply return the value of a counter
  that is associated with the calling process.  
</ul>

The user-level application should behave as follows:
<ul>
<li> <b>syscallptest N</b>.  This program takes one
argument, <b>N</b>, which is the number of system calls (excluding getnumsyscallp()) it makes between calls to <code>getnumsyscallp()</code>.  
Before it calls <code>exit()</code>, it should print out two values: the value returned
by <code>getnumsyscallp()</code> when it is called first
within <code>main()</code> and the value returned
by <code>getnumsyscallp()</code> after the <b>N</b> system calls have
been made.
</ul>

You must use the names of the system call and the application exactly as specified!

<h2>The Code</h2> 

<p>The source code for xv6 (and associated README) can be found in <b>
~cs537-1/ta/xv6/</b> . Everything you need to build, run, and even debug the
kernel is in there; start by reading the README.</p> 

<p>After you have un-tarred the <code>xv6.tar.gz</code> file, you can
run <code>make qemu-nox</code> to compile all the code and run it
using the QEMU emulator.  Test out the unmodified code by running a
few of the existing user-level applications, like <code>ls</code>
and <code>forktest</code>.  To quit the emulator, type <code>Ctl-a x</code>.  

<p>Using gdb (the debugger) may be helpful in understanding code.
Look at the Makefile to see how to start up the debugger.  Get
familiar with this fine tool!</p>

<p>You will not write many lines of code for this project.  Instead, a
lot of your time will be spent learning where different routines are
located in the existing source code.  You will end up modifying files
that are mostly in the <b>kernel</b> subdirectory.  The primary files
you will want to examine in detail
include <code>syscall.c</code>, <code>sysproc.c</code>, <code>proc.h</code>,
and <code>proc.c</code>.

<p>You may also find the following book about xv6 useful, written by the
same team that ported xv6 to x86:
<a href=http://pdos.csail.mit.edu/6.828/2012/xv6/book-rev7.pdf>book</a> .
<b>Particularly useful for this project: Chapters 0 and 3 (and maybe 4).</b> Note
that our version of xv6 is slightly older than the book's, so you may
encounter a difference here and there.</p> 

<h2>Tips</h2> 

<p>To add a system call, find some other very simple system call,
like <code>getpid()</code>, copy it in all the ways you think are
needed, and modify it to havethe name <code>getnumsyscallp()</code>.
Compile the code to see if you found everything you need to copy and
change.

<p>Then think about the changes that you will need to make
so <code>getnumsyscallp()</code> acts like itself instead
of <code>getpid()</code>.
<ul>
<li> You need a counter <b>per process</b>.  What is the data structure that is associated with each process?  Try adding a new field to this structure.
<li> You need to <b>initialize</b> the counter when the process is first created.  Where is a good place to initialize a per-process counter?  In xv6, a process is created using the <code>fork()</code> routine.
<li> You need to <b>increment</b> the counter in the right place.  As the video from discussion section described in detail, the <code>syscall()</code> procedure is where you want to look.  Be sure you don't increment the counter if the system call number corresponds to <code>getnumsyscallp()</code>!  
</ul>

<p>
For this project, you
  do not need to worry about concurrency or locking.

<p>You also need to create a user-level application <code>syscallptest</code> that calls <code>getnumsyscallp()</code> exactly two times.  Again, we suggest copying one of the straight-forward utilities that exist in the <code>user</code> subdirectory.  

<p> Some things to watch out for:
<ul>
<li> Calling variants of <cdoe>printf()</code> in your application involves making system calls!  Therefore, make sure you save the values returned by <code>getnumsyscallp()</code> before your program prints out any values!
<li> You will see that the initial value returned by <code>getnumsyscallp()</code> is not zero (big hint: it should be TWO).  Creating a new process from the shell involves making system calls.  If you are curious, you can determine what these system calls are by looking through <code>sh.c</code>.  You will see that one is <code>exec()</code> and one is <code>sbrk()</code>, which allocates memory to this new process.
<li> For invoking a number of system calls equal to the argument <b>N</b> passed to this application, we recommend invoking a simple system call like <code>getpid()</code>.
</ul>

Good luck!  While the xv6 code base might seem intimidating at first, you only need to understand very small portions of it for this project.  This project is very doable!

<!-- We will provide a testing program: --
  -- systest totalcalls totalsuccessful --> 
</td> </tr> </table> </center> </body> </html> 
