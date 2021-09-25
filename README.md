# Lab 02

## Introduction

In this lab, we'll build on the knowledge learned about a process (program running in memory, or a task). In particular, you'll gain familiarity with concepts that are pertinent to application development and how operating systems (OS) provide service points through which programs request services from the kernel. To be concise, you will make use of the following concepts:
* Process control
* Signals
* System calls
* Client and server model 
* HTTP protocol (GET and POST requests)

In particular, this lab has three goals:
1. Become familiar with how the OS provides **process control** services that can be invoked by a user application. In general, process control includes the creation of new processes, program execution, and process termination. Specifically, there are three primary **system calls**: [fork](https://man7.org/linux/man-pages/man2/fork.2.html), [exec](https://man7.org/linux/man-pages/man3/exec.3.html), and [waitpid](https://linux.die.net/man/2/waitpid). Here, we'll only use two of the three system calls. The `fork` system call (used to create a new process) and the `waidpid` system call (used to detect when a process has been terminated).   
2. Become familiar with a system **signal**. In short, a signal is a technique used to notify a running process that some condition has occurred. In general, the process receiving the signal has three choices: (1) ignore the signal, (2) let the default action occur, or (3) provide a function, or **signal handler**, that is called when the signal occurs (called *catching* the signal). Here, you'll implement a signal handler that catches a [SIGCHLD](https://man7.org/linux/man-pages/man7/signal.7.html) signal that is sent to the parent process when a child process has terminated.
3. Write a simple **server** application that implements process control (`fork` and `waitpid`) and a `SIGCHLD` signal handler. In general, your server will listen for and echo data sent in **HTTP GET** and **POST** requests sent from a **client** (such as a web browser). More specifically, the server will **fork** a new process when a client connects to it, and when the client process terminates the SIGCHLD signal will be sent to the server and the signal handler will execute **waitpid** and release system resources.


### Reading
* Process control:
  * [fork](https://man7.org/linux/man-pages/man2/fork.2.html) man page
  * [waitpid](https://linux.die.net/man/2/waitpid) man page
  * [Process API](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf) OSTEP textbook
* String processing in C:
	* [string.h](https://www.tutorialspoint.com/c_standard_library/string_h.htm) API
	* [Appendix B3](https://sakai.unc.edu/access/content/group/a472e70f-8d81-405f-9d03-6798b4211bf8/Textbooks/CProgramming.pdf) C Programming Language textbook
* HTTP Protocol:
	* [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) protocol specification
	* [Web server](https://developer.mozilla.org/en-US/docs/Learn/Server-side/First_steps/Client-Server_overview) primer by Mozilla

## Terminal Multiplexor (TMUX)
Because the server program executes a loop that runs continuously (listening for client connections), you'll need access to two command line terminals. Specifically,
1. one terminal to write, compile, and test your coding solution, and
2. a second terminal to start/stop the server.

Normally, in a typical Linux desktop environment (i.e., one not running in a Docker environment), opening more than one command line terminal is common practice and very simple to do. However, since the Docker environment only creates a single command line terminal, the TMUX program allows you to run multiple command line terminals inside a single command line terminal. I know ... sounds very recursive :)

A more comprehensive explaination of the TMUX program can be found in this [article](https://linuxize.com/post/getting-started-with-tmux/), but the commands listed below are the most important ones. **Note:** The Linux OS running in your docker environment already has TMUX installed. That is, you don't need to install TMUX again.

#### Starting a new TMUX session
In a terminal, simply execute the tmux program
```
> tmux
```

#### Detaching from a TMUX session
In the TMUX session, simply type the following
```
> Ctrl+b d
```

#### List active TMUX sessions
In a terminal or TMUX session, simply execute the tmux command
```
> tmux ls
```

#### Attaching to a TMUX session
In a terminal or TMUX session, simply execute the tmux command
```
> tmux attach-session -t <id>
```
where **\<id\>** is an active TMUX session identifier. For unnamed sessions, it will be an positive integer number (e.g., 0). See **List active TMUX sessions** to view all the active TMUX sessions including their ids.

## Background

### HTTP Request Message Format

An http request message is really just a string that has the format shown below.

<img src="pics/http_request_message_format.png" width="75%" height="75%"/>
<!-- ![http_request](pics/http_request_message_format.png) -->

In particular, the http request string is delimited by three different characters:
1. `sp` : character that represents a single space (e.g., ' '), 
2. `cr` : character that represents a carriage return (e.g., '\r'), and 
3. `lf` : character that represents a line feed (e.g., '\n').

The http request fields used in this lab:
- `method`: string that can be one of two values (*GET* or *POST*)
- `url` : string that includes key/value pairs (when method=GET)
- `entity body` : string that includes key/value pairs (when method=POST)

#### Method = GET

As outlined in the testing section (step 3), when a client (in our case curl) sends a GET http request to the server, the `url` field has the following format  
```
/?key1=value1&key2=value2&...
```
The characters after `/?` define the key/value pairs. Specifically, the `=` character delimits the key from its value, and the `&` character delimits multiple key/value pairs. For example, when the following curl request is performed,
```
./request get "first=feng&last=cheng"
```
the http request string sent to the server is shown below.

<img src="pics/example_get_request.png" width="40%" height="40%"/>
<!-- ![http_request](pics/example_get_request.png) -->

#### Method = POST

Similarly, when a client sends a POST http request to the server, the `entity body` field has the following format  
```
key1=value1&key2=value2&...
```
Note: in a POST request characters `/?` are not included in the `entity body`. For example, when the following curl request is performed,
```
./request post "first=feng&last=cheng&email=feng@cs.unc.edu"
```
the http request string sent to the server is shown below.

<img src="pics/example_post_request.png" width="50%" height="50%"/>
<!-- ![http_request](pics/example_post_request.png) -->

### 2D Data Arrays

The `server.c` file defines two global variables that store the key/value pairs that are parsed in the http request string. Specifically, a 2D array named `request_keys` and a 2D array named `request_values` (both are double pointers). Additionally, there is a third global variable named `number_key_value_pairs` that stores how many key/value pairs are parsed in the http request string. 

In short, the data in these three global variables are set by the `parse_request` function, and then the `create_response` function uses the data to create an http response string.

For example, if the key/value pairs in a request string are
```
first=feng&last=cheng&email=feng@cs.unc.edu
```
then `number_key_value_pairs` is equal to three, and the data in the two arrays would be:
* `request_keys[0]`, `request_keys[1]`, and `request_keys[2]` would be `first`, `last`, and `email`, respectively.
* `request_values[0]`, `request_values[1]`, and `request_valus[2]` would be `feng`, `cheng`, and `feng@cs.unc.edu`, respectively.

## Structure
<!-- #### Lab 02 Structure -->
Similar to Lab 01, you will implement a set of functions to be called by an external program. The difference between Lab 02 and Lab 01, you **may not modify** the `main` function provided in the `lab02.c` source file. Instead, the `request` executable is used to test the correctness of your coding solution (see Step-3 in [Testing](#testing) section).

There are two starter files for this lab: 
* `server.h` header file and 
* `server.c` source file. 

In both of these files (`server.h` and `server.c`) you **may not** 
* remove or add functions,
* remove or add global variables, or
* remove or add additional header files 

If you do, then you'll recieve **a zero grade** for this lab assignment. No exceptions. If you have any questions regarding this stipulation, you must ask one of the Comp 530 instructors.

In summary, here is a description of each file and what you are expected to do with each one:
* `Makefile`- contains information used to compile your program with one command. **Do not modify.**
* `server.h`- contains proto-type declarations of each function that will be included in the final program. **Do not modify.**
* `server.c`- contains implementations of each function proto-type. You will fully implement, or modify, **only** the following functions listed below:
	* `allocate_data_arrays` : Malloc memory for two [2D arrays](#2d-data-arrays).
	* `unallocate_data_arrays` : Free the memory malloc'd in the allocate data array function.
	* `sig_child_handler` : Signal handler function that reaps the child process.
	* `run_server` : Fork a child process that will call the `accept_client` function.
	* `parse_request` : Parse the GET or POST [http request](#http-request-message-format) and populate the [2D arrays](#2d-data-arrays) with the key/value data.

Lastly, there are **copious** amounts of comments in the `server.h` header and `server.c` source files. Read the comments carefully, they provide information needed to complete this assignment.

#### Server.c Source File
In the server.c please be sure to format your PID as a single 9-digit number with no spaces nor
dashes. Additionally, be sure your capitalization and punctuation of the honor code pledge are correct.
```
// PID: 9DigitPidNoSpacesOrDashes
// I pledge the COMP530 honor code.
```

## Testing

Once again, `lab02.c` will be the driver program that starts the server application and begins to listen for requests on your localhost. Please follow the **Step-by-Step Example** approach to compile, run, and test your coding solution.

**Step 0**. Install libcurl inside the docker:
```
bash update.sh
```
*Note*: You only need to run the `update.sh` bash script once. If you close and restart your docker, re-run this script as needed.

**Step 1**. Compile your code using make:
```
make
```

**Step 2**. Starting the server on port `<port>`
```
./lab02 <port>
```
For example, if you want your server to listen on port 8080:
```
./lab02 8080
```

**Step 3**. Using curl client to send a GET or POST [http request](#http-request-message-format) to the server.

* General format to send a GET http request.
```
./request get "key1=value1&key2=value2&..."
```
An example that defines two key/value pairs.
```
./request get "first=feng&last=cheng"
```

* General format to send a POST http request.
```
./request post "key1=value1&key2=value2&..."
```
An example that defines three key/value pairs.
```
./request post "first=feng&last=cheng&email=feng@cs.unc.edu"
```

**Step 4**. When the server recieves the http request from the curl client, it will process the key/value data and then create an http response. The http response is then sent back to the curl client and the data will be displayed in the following format.

```
<html>
<body>
<h2>GET (or POST) Operation</h2>
<table>
<tr><th>Key</th><th>Value</th></tr>
<tr><td>key1</td><td>value1</td></tr>
<tr><td>key2</td><td>value2</td></tr>
            .
            .
            .
</table>
</body>
</html>
```
**NOTE: you need not worry about formatting the response correctly, we take care of that for you in the `create_response()` function.**

#### Using Test Cases:
We provide several simple test cases in `test_data`. You can use them to test the correctness of your implementation. Example of Usage:
After starting your server application in a tmux session, detach from the session and run the following commands corresponding to each of the test cases we provide:
```
./request $(<path/to/input)
```
* E.g. Case 0: **NOTE: the `$(<file)` syntax is used here so that the contents of `input` are passed in as arguments to the `./request` command.**
```
./request $(<test_data/case0/input)
```
will yield the following output:
```
<html>
<body>
<h2>GET Operation</h2>
<table>
<tr><th>Key</th><th>Value</th></tr>
<tr><td>first</td><td>Omar</td></tr>
<tr><td>last</td><td>Shaban</td></tr>
<tr><td>email</td><td>ohshaban@cs.unc.edu</td></tr>
</table>
</body>
</html>
```
The correct output is in `test_data/case0/output`. You can compare the differences line by line with your eyes. Another approch is to use command `diff` as you have seen in Lab01. The following is an example command:
```
./request $(<test_data/case0/input) > test_data/case0/my_output
diff test_data/case0/output test_data/case0/my_output
```
You should see the differences in the terminal. If there is no output (i.e., the two files are identical), then congrats and your coding solution passes the test case!

You can evaluate other test cases in `test_data`. You can also write your own test cases following the format to test the edge cases which are not covered by the provided ones.

**Think about edge cases for the request parameters, they will be tested by the autograder.**

## Submit your assignment
Assignment submissions will be made through [Gradescope](https://www.gradescope.com).

To submit your assignment, you must commit and push your work to this repository using git. The steps are the same as previous labs.

1. Navigate to the base folder of the repository within Docker.
2. Type `git status`. You should see a list of changes that have been made to the repository.
3. Type `git add *`. This signals that you want to place all modified / new files on the "stage" so that their changes can take effect.
4. Type `git commit -m "Your Message Here"`. This shows that you are "committing" to the changes you put on the stage. Instead of Your Message Here, you should write a meaningful message about what changes you have made.
5. Type `git push origin main`. This takes the commit that was made locally on your machine and "pushes" it to GitHub. Now, when you view this repository on GitHub you should be able to see the changes you've made, namely the addition of your source files!
6. Go to the COMP 530 course in GradeScope and click on the assignment called **lab-02**.
7. Click on the option to **Submit Assignment** and choose GitHub as the submission method.
8. You should see a list of your public repositories. Select the one named **lab-02-yourname** and submit it.
9. Your assignment should be autograded within a few seconds and you will receive feedback for the autograded portion.
10. If you receive all the points, then you have completed this lab! Otherwise, you are free to keep pushing commits to your GitHub repository and submit for regrading up until the deadline of the lab.

## Grading

Autograder (10pts): Check correctness of the algorithms. Submit as many times as you like before the due date to get full credit.
