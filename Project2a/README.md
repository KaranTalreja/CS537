
<html>

<head>
<title>Project 2a: Shell with Job Control</title> 
</head> 

<body text=black bgcolor=white link=#00aacc vlink=#00aacc alink=orange>

<center><table><tr><td width=700pt>

<center>
<font color=#00aacc>
<h1>Project 2a: Shell with Job Control</h1> 
</font> 
</center> 

<b> You must do the Shell part of the project BY YOURSELF.</b>

<h2>Contents</h2>
<ul>
<li> <a href="#object">     Objectives</a>
<li> <a href="#part1">      Overview</a>
<li> <a href="#spec1">      Program Specification</a>
<li> <a href="#C1">         C Hints</a>
<li> <a href="#grading">    Grading</a>
</ul>

<a name="object"> <h2> Objectives </h2> </a>
<p>

There are three objectives to this assignment: 

<ol>
<li> To familiarize yourself with the Linux programming environment.

<li> To learn how processes are handled (i.e., starting and waiting for their termination).

<li>  To gain exposure to the necessary functionality in shells.

</ol>

<a name="part1"> <h2> Overview </h2> </a>
<p>


In this assignment, you will implement a <b>command line
interpreter</b> or <b>shell</b>.  The shell should operate in this
basic way: when you type in a command (in response to its prompt), the
shell creates a child process that executes the command you entered
and then prompts for more user input when it has finished.

<p>
The shells you implement will be similar to, but much simpler, than
the one you run every day in Unix.  You can find out which shell you
are running by typing <b>echo $SHELL</b> at a prompt.  You may then
wish to look at the man pages for <b>sh</b> or the shell you are
running to learn more about all of the functionality that can be
present.  For this project, you do not need to implement much
functionality.  You will need
to be able to handle running multiple commands simultaneously.


<p> Your shell can be run in two modes: <b>interactive</b> and
<b>batch</b>.  In interactive mode, you will display a prompt (the
string "mysh> ", note the space AFTER the the ">" character) to
<b>stdout</b>) and the user of the shell will type in a command at
the prompt.  In batch mode, your shell is started by specifying a
batch file on its command line; the batch file contains the list of
commands (each on its own line) that should be executed.  In batch
mode, you should <b>not</b> display a prompt.  In batch mode you
should echo each line you read from the batch file back to the user
(stdout) before executing it; this will help you when you debug your
shells (and us when we test your programs). In both interactive and
batch mode, your shell terminates when it sees the <tt>exit</tt>
command on a line or reaches the end of the input stream (i.e., the
end of the batch file or the user types 'Ctrl-D').

<p> Jobs may be executed in either the <b>foreground</b> or the
<b>background</b>. When a job is run in the foreground, your shell
waits until that job completes before it proceeds and displays the
next prompt.  When a job is run in the background (as denoted with the
<tt>'&'</tt> character as the last non-whitespace character on the
line), your shell starts the job, but then immediately returns and
displays the next prompt with the background job still running.  

<p> Each job that is executed by your shell should be given its own
<b>unique job id</b> (<tt>jid</tt>).  Your shell must assign each new command
(whether it completes successfully or not) the next integer, starting
with jid 1.  Empty lines and built-in shell commands (such as
<tt>'j'</tt> and <tt>'myw'</tt> described below) should not advance the
jid count.   

<p>For example, given this sequence of input:

<p>
<pre>  
mysh> /bin/ls
mysh>
mysh> /bin/ls   -l
mysh>   output &
mysh> nonexistentjob 
mysh> output    -o 10 &
mysh>   output -o 20   &
</pre>


<p>The jobs are given jids as follows:
<p>
<table>
<tr><td>  /bin/ls <td>1
<tr><td>  /bin/ls -l <td> 2
<tr><td>  output & <td> 3
<tr><td>  nonexistentjob <td> 4
<tr><td>  output -o 10 & <td> 5 
<tr><td>  output -o 20 & <td> 6
</table>


<p> Users are able to perform very limited job control with your
shell.  First, users are able to find out which of their jobs are
still running by using the shell built-in command <tt>'j'</tt> .  When
your shell sees the <tt>'j'</tt> command, it is to print
to <b>standard output</b> the jid of each of the currently running
jobs (i.e., the background jobs that have not yet finished) followed
by a colon, <tt>:</tt>, and the job name and its arguments (without
the '&').  Use <tt>write()</tt> for this.  For your shell to find out
which jobs are still running, you may find the Unix system call
<tt>waitpid</tt> useful; more details are given below.  Be careful
that your shell returns the current information about which jobs are really
running and doesn't simply report the background jobs that the user
hasn't explicitly waited for.

<p> Given the previous sequence of input commands and assuming that
all of the background jobs are still running, then, when the user
types <tt>j</tt>, then your shell should write: 

<p>
<tt>
      3 : output 
<br>  5 : output -o 10 
<br>  6 : output -o 20 
</tt>

<p> Note for us to correctly test your code, <b>your output must match this
format exactly!  Remove all extra whitespace that may have appeared around command names and/or arguments.</b>

<p> Second, users are able to tell the shell to wait for a particular
job to terminate by using the built-in command <tt>'myw'</tt>.  When
your shell sees the <tt>'myw'</tt> command along with the jid of a
job, it waits for the specified job to complete before accepting more
input.  When the job completes, your shell should write the message,
"<waittime> : Job <jid> terminated" to STDOUT; if in interactive mode,
your shell should then display your prompt on the next line.

<p>Continuing our example from above, if the user types the command
<tt>'myw 3'</tt>, then your shell waits for jid 3 to terminate and then prints:

<p>
<tt> 
     WAITTIME : Job 3 terminated
<br> mysh>
</tt>

<p>
where <code>WAITTIME</code> is the amount of time in microseconds that
your shell had to <b>wait</b> for this job to terminate.  For example, if job
3 ran for 4 seconds, and then the user types <tt>myw 3</tt> and then job
3 runs for (exactly) 2 seconds more, then your shell will write:
<p>
<tt> 
     2000000 : Job 3 terminated
</tt>

<p>

<p> If the <tt>'myw'</tt> command is used with the jid of a job that has
already completed, then your shell should immediately return and write
the same message as above.  Note that this case can occur even when
the user was just informed that a given job is executing, if the job
terminates before the user enters the <tt>'myw'</tt> command.  In other
words, there is a race condition between these two events.

<p>Continuing from our example, if the user types the command
<tt>'myw 1'</tt>, then your shell returns immediately and prints:

<p>
<tt> 
     0 : Job 1 terminated
<br> mysh>
</tt>


<p> Finally, if the <tt>'myw'</tt> command is used with an invalid jid,
then your shell should immediately return and print the message,
"Invalid jid <jid>". 

<p>In our example, if the user types the command <tt>'myw 20'</tt>, then
your shell returns immediately and prints <font color="0f0fff">(to STDERR)</font>:

<p>
<tt> 
     Invalid jid 20
<br> mysh>
</tt>

<p>Note that <tt>exit</tt>, <tt>j</tt>, and <tt>myw</tt> are all
built-in shell commands.  They are not to be executed and cannot be
placed in the background (you should just ignore the '&' if it is
specified).  These commands are <b>not</b> given jids.

<p>This project is not as hard as it may seem at first reading; in fact,
the code you write will be much, much smaller than this specification.
Writing your shell in a simple manner is a matter of finding the
relevant library routines and calling them properly.  Your finished
programs will probably be under 200 lines, including comments.  If you
find that you are writing a lot of code, it probably means that you
are doing something wrong and should take a break from hacking and
instead think about what you are trying to do.

<a name="spec1"> <h3> Program Specifications </h3> </a>

Your C program must be invoked exactly as follows:

<pre><font color="0f0fff">
	mysh [batchFile]
</font></pre>

The command line arguments to your shell are to be interpreted as
follows.

<ul>
<li> <sample><font color="0f0fff">batchFile</sample></font>: an
<b>optional</b> argument.  If present, your shell will read each line of the
batchFile for commands to be executed.  If not present, your shell
will run in interactive mode by printing a prompt to the user at
stdout and reading the command from stdin.
</ul>

For example, if you run your program as 
<pre><font color="0f0fff">
	mysh /p/course/cs537-1/file1.txt
</font></pre>

then your program will read commands from
<b>/p/course/cs537-1/file1.txt</b> until it sees the <b>exit</b> command.  

<p>Defensive programming is an important concept in operating systems:
an OS can't simply fail when it encounters an error; it must check all
parameters before it trusts them.  In general, there should be no
circumstances in which your C program will core dump, hang
indefinately, or prematurely terminate.  Therefore, your program must
respond to all input in a reasonable manner; by "reasonable", we mean
print an understandable error message and either continue processing
or exit, depending upon the situation.  

<p> You should consider the following situations as errors; in each
case, your shell should print a message using <tt>write</tt>
to <b>STDERR_FILENO</b> and exit gracefully with a return code of 1:

<ul>
<li> An incorrect number of command line arguments to your shell program.  Print exactly <code>Usage: mysh [batchFile]</code>  (with no extra spaces)
<li> The batch file does not exist or cannot be opened.  Print exacty <code>Error:
Cannot open file foo</code> (assuming the file was named <code>foo</code>).
</ul>

<p>

For the following situation, you should print a message
(using <tt>write()</tt>) to the user (STDERR_FILENO)
and <b>continue</b> processing:
<ul>
<li> A command does not exist or cannot be executed.  Print exactly <code>job: Command not found</code> (assuming the command was named <code>job</code>).
</ul>

Optionally, to make coding your shell easier, you may print an error
message and continue processing in the following situation:
<ul>
<li> A very long command line (for this project, over 512 characters).
</ul>

Your shell should also be able to handle the following scenarios,
which are <b>not</b> errors:
<ul>
<li> An empty command line.
<li> Multiple white spaces on a command line.
<li> White space before or after the <tt>'&'</tt> character.
<li> Batch file ends without <b>exit</b> command or user types
'Ctrl-D' as command in interactive mode.
<li> Additional flags or arguments appear along with built-in command (e.g., <code>j hello</code>); do not treat as a built-in command and create a new process with the specified arguments
<li> The <tt>'&'</tt> character appears after a built-in command (e.g., <code>exit &</code>); ignore the <tt>'&'</tt> character and execute normally.
<li> If the <tt>'&'</tt> character appears in the middle of a line,
then the job should not be placed in the background; instead, the
<tt>'&'</tt> character is treated as one of the job arguments.

</ul>

All of these requirements will be tested extensively!  

<p>For simplicity, you can limit the number of background jobs that are
currently running to 32.  However, you must allow an unlimited number
of background jobs to be started over the lifetime of your shell.

<a name=C1> <h3> C Hints </h3> </a>

Your shell is basically a loop: it repeatedly prints a prompt (if in
interactive mode), parses the input, executes the command specified on
that line of input, and waits for the command to finish, if it is in
the foreground.  This is repeated until the user types "exit" or ends
their input.  

<p>You should structure your shell such that it creates a new process for
each new command.  There are two advantages of creating a new process.
First, it protects the main shell process from any errors that occur
in the new command.  Second, it allows easy concurrency; that is,
multiple commands can be started and allowed to execute
simultaneously.  

<p>For each running job, you will want to track some information in a
data structure.  It is up to you to determine what information you
need to keep and he list structure you want to use (e.g.,
an array or a linked list).  This information will allow you to wait
for the appropriate job to complete, as requested by the user.

<p>To simplify things for you in this first assignment, we will suggest a
few library routines you may want to use to make your coding easier.
(Do not expect this detailed of advice for future assignments!)  You
are free to use these routines if you want or to disregard our
suggestions.

<p> To find information on these library routines, look at the manual
pages (using the Unix command <b>man</b>).  You will also find man
pages useful for seeing which header files you should include.

<p>
Make sure you use the <tt>write()</tt> system call for all printing
(including prompts, error messages, and job status), whether to stdout
or to stderr.  <font color="0f0fff">Why should you
use <tt>write()</tt> instead of fprintf or printf?  The main
difference between the two is that <tt>write()</tt> performs its
output immediately whereas <tt>fprintf()</tt> buffers the output
temporarily in memory before flushing it.  As a result, if you
use <tt>fprintf()</tt> you will probably see output from your shell
intermingled in unexpected ways with output from the jobs
you <tt>fork()</tt>; you will fail our tests if your output is
intermingled.  If you decide to use <tt>fprintf()</tt> make sure you
ALWAYS call <tt>fflush()</tt> immediately after the call
to <tt>fprintf()</tt>.  </font>

<h3>Parsing</h3>

For reading lines of input, you may want to look at <b>fgets()</b>.
To open a file and get a handle with type <b>FILE *</b>, look into
<b>fopen()</b>.  Be sure to check the return code of these routines
for errors!  (If you see an error, the routine <b>perror()</b> is
useful for displaying the problem.)  You may find the
<b>strtok()</b> routine useful for parsing the command line (i.e., for
extracting the arguments within a command separated by ' ').

<h3>Executing Commands</h3>

Look into <tt>fork</tt>, <tt>execv</tt>, and <tt>waitpid</tt>.    

<p>
The <tt>fork</tt> system call creates a new process.  After this point,
two processes will be executing within your code.  You will be able to
differentiate the child from the parent by looking at the return value
of <tt>fork</tt>; the child sees a 0, the parent sees the <tt>pid</tt> of
the child.  Note that you will need to map between your <tt>jid</tt> and
this <tt>pid</tt>.

<p>
You will note that there are a variety of commands in the <tt>exec</tt>
family; <b>for this project, you must use <tt>execvp</tt></b>.  Remember that if
<tt>execvp</tt> is successful, it will not return; if it does return,
there was an error (e.g., the command does not exist).  The most
challenging part is getting the arguments correctly specified.  The
first argument specifies the program that should be executed, with the
full path specified; this is straight-forward.  The second argument,
<tt>char *argv[]</tt> matches those that the program sees in its
function prototype:

<p><tt>int main(int argc, char *argv[]);</tt>

<p> Note that this argument is an array of strings, or an array of
pointers to characters.  For example, if you invoke a program with:

<p><tt>/bin/foo 205 535</tt>

<p> then argv[0] = "/bin/foo", argv[1] = "205" and argv[2] = "535".  Note
the list of arguments must be terminated with a NULL pointer; that is,
argv[3] = NULL.  We strongly recommend that you carefully check that
you are constructing this array correctly!

<p> The <tt>waitpid</tt> system call allows the parent process to wait for
one of its children.  Note that it returns the pid of the completed child;
again, you will need to map between your <tt>jid</tt> and this
<tt>pid</tt>.  You will want to investigate the different options that
can be passed to <tt>waitpid</tt> so that your shell can query the OS
about which jobs are still running without having to wait for a job to
terminate.  

<p>To find out how long the shell waits for a particular job to
complete, you can use the <tt>gettimeofday()</tt> function.  Just
calculate the number of microseconds that elapse between
when the shell calls <tt>waitpid</tt> and when the call returns.  

<h3>Miscellaneous Hints</h3>

Remember to get the <b>basic functionality</b> of your shell working
before worrying about all of the error conditions and end cases.  For
example, first focus on interactive mode, and get a single command
running in the foreground working (probably first command with no
arguments, such as "ls").  Then, add in the functionality to work in
batch mode (most of our test cases will use batch mode, so make sure
this works!).  Next, handle starting up jobs in the background;
waiting for them and then listing their status should be next.
Finally, make sure that you are correctly handling all of the cases
where there is miscellaneous white space around commands or missing
commands.

<p> We strongly recommend that you check the return codes of all
system calls from the very beginning of your work.  This will often
catch errors in how you are invoking these new system calls.

<a name=grading> <h2> Grading </h2> </a>

<p> To ensure that we compile your C  correctly for the
demo, you will need to create a simple <b>makefile</b>; this way our
scripts can just run <tt>make</tt> to compile your code with the right
libraries and flags.  If you don't know how to write a makefile, you
might want to look at the man pages for <tt>make</tt>.  Otherwise, check
out this very simple <a href=Makefile>sample makefile</a>.

<p>
The name of your final executable should be <code>mysh</code> , i.e. your
C program must be invoked exactly as follows:</p> 

<pre><blockquote>
% ./mysh
% ./mysh inputTestFile
</blockquote> </pre> 

<p>Copy all of your .c source files into the appropriate subdirectory. Do <b>
not</b> submit any .o files. Make sure that your code runs correctly on the
linux machines in the 13XX labs.</p> 

<p>
The majority of your grade for this assignment will depend upon how
well your implementation works.  We will run your program on a suite
of about 20 test cases.  Be sure that you
thoroughly exercise your program's capabilities on a wide range of
test suites, so that you will not be unpleasantly surprised when we
run our tests.

<p>For testing your code, you will probably want to run commands that
take awhile to complete.  Try compiling and running this very simple C
<a href=output.c>program</a>; when multiple copies are run in the
background you should see the output from each process interleaved.
See the code for more details.

<p>We will again verify that your code passes lint and valgrind tests, as in Project P1a.

</td> </tr> </table> </center> </body> </html> 

