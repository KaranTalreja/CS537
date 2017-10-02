
<html>

<head>
<title>Project 3b: Shared Memory in xv6</title> 
</head> 

<body text=black bgcolor=white link=#00aacc vlink=#00aacc alink=orange>

<center><table><tr><td width=500pt>

<center>
<font color=#00aacc>
<h1>Project 3b: Shared Memory in xv6</h1> 
</font> 
</center> 

<h2>Objectives</h2> 

<p>There are two objectives to this assignment:</p> 

<ul>
<li>To familiarize you with the xv6 virtual memory system.</li> 

<li>To add shared memory segments.
</ul> 

<h2>Overview</h2> 

<!-- 
<p>In this project, you'll be changing xv6 to support a feature
virtually every modern OS does: causing an exception to occur when
your program dereferences a null pointer (i.e., a pointer to address
0x00). Sound simple? Well, it mostly is. But there are a few
details.</p>
-->

<p>In this project, you'll be adding a new simple facility to allow different processes to
share memory pages. Sound simple? Good! At least you think things sound
simple.</p> 

<h2>Details</h2> 

<!-- 
<h3>Part A: Null-pointer Dereference</h3> 

<p>In xv6, the VM system uses a simple two-level page table as
discussed in class. As it currently is structured, user code is loaded
into the very first part of the address space. Thus, if you
dereference a null pointer (i.e., a pointer to address 0x00), you will
not see an exception (as you might expect); rather, you will see
whatever code is the first bit of code in the program that is
running. Try it and see!</p>

<p>Thus, the first thing you might do is create a program that dereferences a
null pointer. It is simple! See if you can do it. Then run it on Linux as well
as xv6, to see the difference.</p> 

<p>Your job here will be to figure out how xv6 sets up a page table. Thus,
once again, this project is mostly about understanding the code, and not
writing very much. Look at how <code>exec()</code> works to better understand how
address spaces get filled with code and in general initialized. That will get
you most of the way.</p> 

<p>You should also look at <code>fork()</code> , in particular the part where the
address space of the child is created by copying the address space of the
parent. What needs to change in there?</p> 

<p>The rest of your task will be completed by looking through the code to
figure out where there are checks or assumptions made about the address
space. Think about what happens when you pass a parameter into the kernel, for
example; if passing a pointer, the kernel needs to be very careful with it, to
ensure you haven't passed it a bad pointer. How does it do this now? Does this
code need to change in order to work in your new version of xv6?</p> 

<p>One last hint: you'll have to look at the xv6 makefile as well. In there
user programs are compiled so as to set their entry point (where the first
instruction is) to 0. If you change xv6 to make the first page invalid,
clearly the entry point will have to be somewhere else (e.g., the next page,
or 0x1000). Thus, something in the makefile will need to change to reflect
this as well.</p> 

<p>You should be able to demonstrate what happens when user code tries to
access a null pointer. When this happens, xv6 should trap and kill the
process. The good news: this will happen without too much trouble on your
part, if you do the project in a sensible way, because xv6 already catches
illegal memory accesses.</p> 

<h3>Part B: Shared Pages</h3> 
-->


<p>In most operating systems, there are some different ways for processes to
communicate with one another. In this part of the project, you'll explore how
to add shared-memory pages to processes that are interested in communicating
through memory.</p> 

<p>
You will need to implement the following new systems calls:

<p><b>void *shmgetat(int key, int num_pages):</b> this is a simplified
combination of the two Linux system calls: <code>shmget()</code>
followed by <code>shmat()</code>.  The idea is that if processes
call <b>shmgetat()</b> with the same <b>key</b> for the first
argument, then they will share the specified number of physical pages.  Using
different keys in different calls to <b>shmgetat()</b> corresponds to
different physical pages.  

<p>On success, <b>shmgetat()</b> returns the virtual address of the
shared pages to the caller, so the process can read/write them.  In
all cases, <b>shmgetat</b> should map the shared phyiscal pages to the
next available virtual pages, starting at the high end of that
process' address space.  Note that different processes can have the
same physical pages mapped to different locations in their virtual
address space.

<p>For example, when a process calls <b>shmgetat(0, 1),</b> the OS
should map 1 physical page into the virtual address space of the
caller, starting at the very high end of the address space; these pages
should be zero-filled initially.  The system call returns the
virtual address where these pages are mapped into the caller's address
space. If another process then calls <b> shmgetat(0, ANY_VALUE),</b>
then this process should also get that same 1 page mapped into its virtual
address space (possibly at a different virtual address).  The two
processes can then each read and write to this page and thus
communicate.  Note that the second argument to <b>shmgetat</b> is
ignored if a key that has already been used is passed as the first
argument; it maps the same number of pages that were specified in the first call.

<p> Note that if a third or fourth process calls <b>shmgetat(0,
ANY_VALUE)</b> then they would have that same page mapped into their address spaces as well.

<p>However, if another key is used, then this corresponds to a new
shared region.  So, if any process then calls <b>shmgetat(1, 3)</b>,
the OS will map 3 (new) physical pages into the address space of the
calling process and associate these 3 pages with key value 1.  Subsequent calls
that use <b>key=1</b> will map these three pages into the calling process' address space.

<p>In all cases, the number of pages that can be specified is small:
just up to 4 pages to a single call to <b>shmgetat()</b>.  Keys 
can range from only 0 to 7.

<p>If any type of error ever occurs, <b>shmgetat</b> should return -1.

<p>Another system call is needed: <b>int shm_refcount(int key)</b>.
This call returns, for a particular key, how many processes currently
are sharing the associated pages. Note that if a process exits, you
need to decrement this reference count.  If the reference count for a
key goes to 0, then all state associated with those pages should be
freed.  Thus, if the reference count for a key goes to 0, and then a
subsequent call is made to <b>shmgetat(key, SOME_VALUE)</b>, this
subsequent call will be treated as a new shared segment with a new
number of shared physical pages (and the pages appropriately
initialized to zero).

<p> Of course, to use shared memory carefully, one has to think about
synchronization, but that is not your worry (for this project).</p>

<p>Some things to think about:
<ul>
<li>Failure cases: Bad argument to system call, address space already fully in
use (large heap).</li> 
<li>How to handle fork(): Upon fork, must also make sure child process has
access to shared page, and that reference counts are updated appropriately.</li> 
<li>How to track reference counts to each page so as to be able to implement
<b>shm_refcount()</b>.</li> 
</ul> </p> 

<font color=red> <p> Some clarifications (posted to class on Oct 21):
<ol>
<li> same process *can* call shmgetat more than once.
<ol>
    <li> even with the *same* key. when it is called with the *same* key, you just return the virtual address that it is already mapped at. and the reference count does not increase since it has been already mapped to this process by the very first call to shmgetat with this key.
    <li> a process can definitely call shmgetat multiple times with different keys. it will then have access to multiple shared regions.
</ol>

<li> keys are *not* per process. they are global to the system. if process A does shmgetat(3, 1), and then process B does shmgetat(3, ANY_VALUE), they will both have access to the *same* *one* physical page.

<li> when a fork is called, every shared region has to be accessible to the new process *without* it needing to call shmgetat().

<li> shared regions have to be mapped starting from the *very end* of calling process's address space.
    example:

<pre>
        //assume no shared regions exist yet in the system

        process A: shmgetat(3, 1); 

        //maps one physical page to the last page in calling process's
        virtual address space, if the address space is not already
        full.

        process A: shmgetat(2, 2); 

        //maps two physical pages to the second to and third to last
        page in calling process's virtual address space. note that the
        order of shared regions in a process's virtual address space
        is not correlated to value of the key.
</pre>


</ol>
</font>


<h2>The Code</h2> 

<p>The source code for xv6 (and associated README) can be found in <b>
~cs537-1/ta/xv6/</b> . Everything you need to build and run and even debug the
kernel is in there.</p> 

<p>Might be good to read the xv6 book a bit: <a href=xv6book.pdf>Here</a> .

<p><b>Particularly useful for this project: Chapter 1 .</b> </p> 



</td> </tr> </table> </center> </body> 








