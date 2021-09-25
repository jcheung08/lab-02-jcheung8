// -----------------------------------
// 	COMP 530: Operating Systems
//
//	Fall 2021 - Lab 2
// -----------------------------------

#define OK 0
#define FAIL -1
#define NONE -1
#define DEBUG 1
#define GET 0
#define POST 1
#define MAX_KEY_VALUE_PAIRS 100
#define MAX_KEY_CHARS 101
#define MAX_VALUE_CHARS 101
#define REQUEST_MAX_CHARS 512
#define RESPONSE_MAX_CHARS 512

// ------------------------------------
// Function prototype that initializes the signal 
// handler function
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments:	None
//
// Return:      0 (OK): no errors (see #define)
//			   -1 (FAIL): Error occurred (see #define)
//
int initialize_handler();

// ------------------------------------
// Function prototype that alloates memory for 
// the request_keys and request_values 2D arrays.
//
// ******************************************
// We'll assume the following limitations:
// ******************************************
//	1. Max number of key/value pairs is 100 (see #define MAX_KEY_VALUE_PAIRS)
//	2. Max number of characters in a key is 100 characters (see #define MAX_KEY_CHARS)
//	3. Max number of characters in a value is 100 characters (see #define MAX_VALUE_CHARS)
//
// You'll malloc these two 2D arrays (request_keys and request_values) 
// based on limitations above
//
// Arguments:	None
//
// Return:      None
//
void allocate_data_arrays();

// ------------------------------------
// Function prototype that unallocates request_keys
// and request_values 2D arrays memory.
//
// Arguments:	None
//
// Return:      None
//
void unallocate_data_arrays();

// ------------------------------------
// Function prototype that defines a signal handler 
// to process asynchronous SIGCHLD events (i.e., when 
// a child process terminates).
//
//
// Arguments:	signal type: integer value that defines 
//							 the type of signal. In this 
//							 lab we are only interested in
//						 	 signals that = SIGCHLD
//
// Return:      None
//
void sig_child_handler( int signal_type );

// ------------------------------------
// Function prototype that runs a loop that 
// continuely listens for client connections
//
//
// Arguments:	None
//
// Return:      0 (OK): No errors (see #define)
//			   -1 (FAIL): Error occurred (see #define)
//
int run_server();

// ------------------------------------
// Function prototype that creates a server socket 
// and then binds it to the specified port_number. Client
// socket connections use this port number to communicate
// with the server.
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments:	port_number = port number the server socket
//							  will listen for client
//							  connections.
//
// Return:      0 (OK): No errors (see #define)
//			   -1 (FAIL): Error occurred (see #define)
//
int bind_port( unsigned int port_number );

// ------------------------------------
// Function prototype that reads the
// request message from the socket, and 
// writes the response message to the socket.
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments:	client_socket_fd: file descriptor (fd) value
//								  positive integer number typically 
//								  greater than two (0=stdin, 1=stdout, 2=stderror)
//
// Return:      None
//
void accept_client( int client_socket_fd );

// ------------------------------------
// Function prototype that parses an HTTP 
// request message sent from a client. This
// function will populate the request_keys 
// and request_values 2D arrays.
//
//
// Arguments:	http_request: string that holds the 
//							  request message
//
// Return:      0 (GET):  Method field is GET (see #define)
//				1 (POST): Method field is POST (see #define)
//			   -1 (FAIL): Method field is not GET or POST (see #define)
//
int parse_request( char* http_request );

// ------------------------------------
// Function prototype that creates an HTTP 
// response message that is sent to the 
// client. This function uses the data in the 
// request_keys and request_values 2D arrays 
// (performed by the parse_request function)
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments:	http_response: string that holds the
//							   response message
//				request_method: GET=0, POST=1, FAIL=-1
//
// Return:      None
//
void create_response( int request_method, char* http_response );
