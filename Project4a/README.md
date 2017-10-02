
<html>

<head>
<title>Project 4a: Scalable Web Servers</title> 
</head> 

<body text=black bgcolor=white link=#00aacc vlink=#00aacc alink=orange>

<center><table><tr><td width=500pt>

<center>
<font color=#00aacc>
<h1>Project 4a: Scalable Web Server</h1> 
</font> 
</center> 

<h2>Note</h2> 

<p><b>Security:</b> It might be worthwhile to be a little careful with security
during this project. Probably not a big deal, but a good way to help with this
is to make sure to run the web server out of a special directory with only a
few files in it (e.g., a subdirectory of your build, or something you create
specially in /tmp), and further to disallow any path names that have a <b>..</b> 
in them (which would allow people to go up a level or more in the directory
hierarchy and thus explore any files you have access to). Minimally, don't
leave your web server running for a long time.</p> 

<h2>Background</h2> 

<p>In this assignment, you will be developing a real, working <b>web server.</b> 
To simplify this project, we are providing you with the code for a
very basic web server. This basic web server operates with only a single
thread; it will be your job to make the web server multi-threaded so that it
is more efficient.</p> 

<h2>HTTP Background</h2> 

<p>Before describing what you will be implementing in this project, we will
provide a very brief overview of how a simple web server works and the HTTP
protocol. Our goal in providing you with a basic web server is that you should
be shielded from all of the details of network connections and the HTTP
protocol. The code that we give you already handles everything that we
describe in this section. If you are really interested in the full details of
the HTTP protocol, you can read the
<a href=http://www.w3.org/Protocols/rfc2616/rfc2616.html>specification,</a> but
we do not recommend this for this project.</p> 

<p>Most web browsers and web servers interact using a text-based protocol
called HTTP (Hypertext Transfer Protocol). A web browser opens an Internet
connection to a web server and requests some content with HTTP. The web server
responds with the requested content and closes the connection. The browser
reads the content and displays it on the screen.</p> 

<p>Each piece of content on the server is associated with a file. If a client
requests a specific disk file, then this is referred to as static content. If
a client requests that a executable file be run and its output returned, then
this is dynamic content. Each file has a unique name known as a URL (Universal
Resource Locator). For example, the URL <code>www.cs.wisc.edu:80/index.html</code> 
identifies an HTML file called &ldquo;index.html&rdquo; on Internet host
&ldquo;www.cs.wisc.edu&rdquo; that is managed by a web server listening on port
80. The port number is optional and defaults to the well-known HTTP port of
80. URLs for executable files can include program arguments after the file
name. A &ldquo;?&rdquo; character separates the file name from the arguments and
each argument is separated by a &ldquo;&&rdquo; character. This string of
arguments will be passed to a CGI program as part of its
&ldquo;QUERY_STRING&rdquo; environment variable.</p> 

<p>An HTTP request (from the web browser to the server) consists of a request
line, followed by zero or more request headers, and finally an empty text
line. A request line has the form: <code>method uri version</code> . The <code>
method</code> is usually GET (but may be other things, such as POST, OPTIONS, or
PUT). The <code>URI</code> is the file name and any optional arguments (for dynamic
content). Finally, the <code>version</code> indicates the version of the HTTP
protocol that the web client is using (e.g., HTTP/1.0 or HTTP/1.1).</p> 

<p>An HTTP response (from the server to the browser) is similar; it consists
of a response line, zero or more response headers, an empty text line, and
finally the interesting part, the response body. A response line has the form
<code>version status message</code> . The <code>status</code> is a three-digit positive
integer that indicates the state of the request; some common states are 200
for <b>OK</b> , 403 for <b>Forbidden</b> , and 404 for <b>Not found</b> . Two important
lines in the header are <b>Content-Type</b> , which tells the client the MIME type
of the content in the response body (e.g., html or gif) and <b>
Content-Length</b> , which indicates its size in bytes.</p> 

<p>Again, you don't need to know this information about HTTP unless you want
to understand the details of the code we have given you. <b>You will not need
to modify any of the procedures in the web server that deal with the HTTP
protocol or network connections.</b> </p> 

<h2>Basic Web Server</h2> 

<p>The code for the web server is available from <code>~cs537-1/public/p4.</code> 
You should copy over all of the files there into your own working
directory. You should compile the files by simply typing <b>make</b> . Compile and
run this basic web server before making any changes to it! <b>make clean</b> 
removes .o files and lets you do a clean build.</p> 

<p>When you run this basic web server, you need to specify the port number
that it will listen on; you should specify port numbers that are greater than
about 2000 to avoid active ports. When you then connect your web browser to
this server, make sure that you specify this same port. For example, assume
that you are running on mumble21.cs and use port number 2003; copy your
favorite html file to the directory that you start the web server from. Then,
to view this file from a web browser (running on the same or a different
machine), use the url: <code>mumble21.cs.wisc.edu:2003/favorite.html</code>.  Note that your client (the browser) may need to be on the CS network to connect to your server.  </p> 

<p>The web server that we are providing you is only about 200 lines of C code,
plus some helper functions. To keep the code short and understandable, we are
providing you with the absolute minimum for a web server. For example, the web
server does not handle any HTTP requests other than GET, understands only a
few content types, and supports only the QUERY_STRING environment variable for
CGI programs. This web server is also not very robust; for example, if a web
client closes its connection to the server, it may crash. We do not expect you
to fix these problems!</p> 

<p>The helper functions are simply wrappers for system calls that check the
error codes of those system codes and immediately terminate if an error
occurs. One should <b>always check error codes!</b> However, many programmer
don't like to do it because they believe that it makes their code less
readable; the solution, as you know, is to use these wrapper functions. We
expect that you will write wrapper functions for the new system routines that
you call.</p> 

<h2>Overview: New Functionality</h2> 

<p>In this project, you will be adding one key piece of functionality
  to the basic web server: you will make it multi-threaded.  You will also be
modifying how the web server is invoked so that it can handle new input
parameters (e.g., the number of threads to create).</p> 

<p>The basic web server that we provided has a single thread of
control. Single-threaded web servers suffer from a fundamental performance
problem in that only a single HTTP request can be serviced at a time. Thus,
every other client that is accessing this web server must wait until the
current http request has finished; this is especially a problem if the current
http request is a long-running CGI program or is resident only on disk (i.e.,
is not in memory). Thus, the most important extension that you will be adding
is to make the basic web server multi-threaded.</p> 

<p>The simplest approach to building a multi-threaded server is to spawn a new
thread for every new http request. The OS will then schedule these threads
according to its own policy. The advantage of creating these threads is that
now short requests will not need to wait for a long request to complete;
further, when one thread is blocked (i.e., waiting for disk I/O to finish) the
other threads can continue to handle other requests. However, the drawback of
the one-thread-per-request approach is that the web server pays the overhead
of creating a new thread on every request.</p> 

<p>Therefore, the generally preferred approach for a multi-threaded server is
to create a <b>fixed-size pool</b> of worker threads when the web server is first
started. With the pool-of-threads approach, each thread is blocked until there
is an http request for it to handle. Therefore, if there are more worker
threads than active requests, then some of the threads will be blocked,
waiting for new http requests to arrive; if there are more requests than
worker threads, then those requests will need to be buffered until there is a
ready thread.</p> 

<p>In your implementation, you must have a master thread that begins by
creating a pool of worker threads, the number of which is specified on the
command line. Your master thread is then responsible for accepting new http
connections over the network and placing the descriptor for this connection
into a fixed-size buffer; in your basic implementation, the master thread
should not read from this connection. The number of elements in the buffer is
also specified on the command line. Note that the existing web server has a
single thread that accepts a connection and then immediately handles the
connection; in your web server, this thread should place the connection
descriptor into a fixed-size buffer and return to accepting more
connections.</p> 

<p>Each worker thread is able to handle both static and dynamic
requests. A worker thread wakes when there is an http request in the
queue. Once the worker thread wakes, it performs the read on the
network descriptor, obtains the specified content (by either reading
the static file or executing the CGI process), and then returns the
content to the client by writing to the descriptor. The worker thread
then waits for another http request.</p>

<p>Note that the master thread and the worker threads are in a
producer-consumer relationship and require that their accesses to the shared
buffer be synchronized. Specifically, the master thread must block and wait if
the buffer is full; a worker thread must wait if the buffer is empty. In this
project, you are required to use <code>condition variables.</code> Note: <b>if your
implementation performs any busy-waiting (or spin-waiting) instead, you will
be heavily penalized.</b> </p> 

<p>Side note: Do not be confused by the fact that the basic web server we
provide forks a new process for each CGI process that it runs. Although, in a
very limited sense, the web server does use multiple processes, it never
handles more than a single request at a time; the parent process in the web
server explicitly waits for the child CGI process to complete before
continuing and accepting more http requests. <b>When making your server
multi-threaded, you should not modify this section of the code.</b> </p> 

<h2>Program Specifications</h2> 

<p>Your C program must be invoked exactly as follows:

<pre>
prompt> server [portnum] [threads] [buffers] 
</pre> </p> 

<p>The command line arguments to your web server are to be interpreted as
follows.
<blockquote>
<ul>
<li><b>portnum:</b> the port number that the web server should listen on; the
basic web server already handles this argument.</li> 
<li><b>threads:</b> the number of worker threads that should be created within
the web server. Must be a positive integer.</li> 
<li><b>buffers:</b> the number of request connections that can be accepted at one
time. Must be a positive integer. Note that it is not an error for more or
less threads to be created than buffers.</li> 
</ul> </blockquote> </p> 

<p>For example, if you run your program as:

<blockquote>
<pre>
server 5003 8 16
</pre> </blockquote> </p> 

<p>then your web server will listen to port 5003, create 8 worker
  threads for handling http requests and allocate 16 buffers for connections that are currently in progress (or waiting).</p> 

<h2>Hints</h2> 

<p>We recommend understanding how the code that we gave you works. All of the
code is available from <code>~cs537-1/public/p4</code> . We provide the following
files:

<blockquote>
<ul>
<li><b>server.c:</b> Contains main() for the basic web server.</li> 
<li><b>request.c:</b> Performs most of the work for handling requests in the
basic web server. All procedures in this file begin with the string
&ldquo;request&rdquo;.</li> 
<li><b>cs537.c:</b> Contains wrapper functions for the system calls invoked by the
basic web server and client. The convention is to capitalize the first letter
of each routine. Feel free to add to this file as you use new libraries or
system calls. You will also find a corresponding header (.h) file that should
be included by all of your C files that use the routines defined here.</li> 
<li><b>client.c:</b> Contains main() and the support routines for the very simple
web client. To test your server, you may want to change this code so that it
can send simultaneous requests to your server. At a minimum, you will want to
run multiple copies of this client.</li> 
<li><b>output.c:</b> Code for a CGI program. Basically, it spins for a fixed
amount of time, which you may useful in testing various aspects of your server.</li> 
</ul> </blockquote> </p> 

<p>We also provide you with a sample Makefile that creates server, client, and
output.cgi. You can type <b>make</b> to create all of these programs. You can
type <b>make clean</b> to remove the object files and the executables. You can
type <b>make server</b> to create just the server program, etc. As you create new
files, you will need to add them to the Makefile.</p> 

<p>The best way to learn about the code is to compile it and run it. Run the
server we gave you with your preferred web browser. Run this server with the
client code we gave you. You can even have the client code we gave you contact
any other server that speaks HTTP. Make small changes to the server code
(e.g., have it print out more debugging information) to see if you understand
how it works. Note that your client (the browser) may need to be on the CS network to connect to your server.  </p> 

<p>We anticipate that you will find the following routines useful for creating
and synchronizing threads: <code>pthread_create, pthread_mutex_init,
pthread_mutex_lock, pthread_mutex_unlock, pthread_cond_init,
pthread_cond_wait, pthread_cond_signal.</code> To find information on these library
routines: <b>RTFM</b> , which stands for <b>Read The Manual</b> ; you should also feel
free to read the OS book ( <b>RTFB</b> ?), which contains a great amount of detail on
how to build producer-consumer relationships between threads.</p> 

</td> </tr> </table> </center> </body> </html> 





