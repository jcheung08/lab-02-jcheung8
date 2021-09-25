// PID: 730171743
// I pledge the COMP530 honor code.

// -----------------------------------
// 	COMP 530: Operating Systems
//
//	Fall 2021 - Lab 2
// -----------------------------------

#include "server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// -------------------------------------------------
// Global variables

int server_socket_fd = 0;         // server socket file descriptor
char** request_keys;              // two dimensional array that stores request keys
char** request_values;            // two dimensional array that stores the value for each key
unsigned int number_key_value_pairs = 0;  // total number of key/value pairs (i.e., total number of rows in the request_keys and values arrays)

// ------------------------------------
// Function implemenation that initializes the signal
// handler function.
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments: None
//
// Return:      0 (OK): no errors (see #define)
//         -1 (FAIL): Error occurred (see #define)
//
int initialize_handler() {
  if ( signal( SIGCHLD, sig_child_handler ) == SIG_ERR ) {
      perror( "Unable to create SIGCHLD handler!");
      return FAIL;
  }

  return OK;

} // end initialize_handler() function

// ------------------------------------
// Function implemenation that alloates memory for
// the request_keys and request_values 2D arrays.
//
// ******************************************
// We'll assume the following limitations:
// ******************************************
//  1. Max number of key/value pairs is 100 (see #define MAX_KEY_VALUE_PAIRS)
//  2. Max number of characters in a key is 100 characters (see #define MAX_KEY_CHARS)
//  3. Max number of characters in a value is 100 characters (see #define MAX_VALUE_CHARS)
//
// You'll malloc these two 2D arrays (request_keys and request_values)
// based on limitations above
//
// Arguments: None
//
// Return:    None
//
void allocate_data_arrays() {
request_keys = (char **) malloc(sizeof(char *) * MAX_KEY_VALUE_PAIRS);
request_values = (char **) (malloc(sizeof(char *) * MAX_KEY_VALUE_PAIRS));

for (int i = 0; i < MAX_KEY_VALUE_PAIRS; i++) {
  request_keys[i] = (char *) malloc(sizeof(char) * MAX_KEY_CHARS);
  request_values[i] = (char *) malloc(sizeof(char) * MAX_VALUE_CHARS);
}




} // end allocate_data_arrays() function

// ------------------------------------
// Function prototype that unallocates request_keys
// and request_values 2D arrays memory.
//
// Arguments:   None
//
// Return:      None
//
void unallocate_data_arrays() {
   for (int i = 0; i < MAX_KEY_VALUE_PAIRS; i++) {
     free(request_keys[i]);
     free(request_values[i]);
   }
   free(request_keys);
   free(request_values);

} // end unallocate_data_arrays() function

// ------------------------------------
// Function implementation that defines a signal handler
// to process asynchronous SIGCHLD events (i.e., when
// a child process terminates).
//
//
// Arguments: signal type: integer value that defines
//               the type of signal. In this
//               lab we are only interested in
//               signals that = SIGCHLD
//
// Return:      None
//
void sig_child_handler( int signal_type ) {
  // -------------------------------------
  // TODO:
  // -------------------------------------
  // You complete child signal handler code
  // to remove child process from process
  // table (i.e. reap the child)
  // -------------------------------------
  if (signal_type == SIGCHLD) {
    waitpid(-1, NULL, 0);
  }

} // end sig_child_handler() function

// ------------------------------------
// Function implemenation that runs a loop that
// continuely listens for client connections
//
//
// Arguments: None
//
// Return:      0 (OK): No errors (see #define)
//             -1 (FAIL): Error occurred (see #define)
//
int run_server( unsigned int port_number ) {
  int client_socket_fd = -1;

  struct sockaddr_in client_address;
  socklen_t client_length = sizeof( client_address );

  if ( bind_port( port_number ) == FAIL ) {

    printf("Failed to bind socket to port %d\n", port_number );
    printf("Stopping http server!\n" );

    return FAIL;

  } else {

    printf("server_socket_fd = %d\n", server_socket_fd );

    while ( listen( server_socket_fd, 0 ) == 0 ) {

      printf("Listening and accepting connections on port %d\n", port_number );

      client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_length);

      // -------------------------------------
      // TODO:
      // -------------------------------------
      // Modify code to fork a child process
      // in which accept_client() gets called
      // -------------------------------------
      int childProcess = fork();
      if (childProcess < 0) {
        //creation of child process unsuccessful
        return 1;
      } else if (childProcess == 0 && client_socket_fd >= 0) {
        //child process
        accept_client(client_socket_fd);
        return 0;;
      }
    }
  }

  return OK;

} // end run_server() function

// ------------------------------------
// Function implemenation that creates a server socket
// and then binds it to the specified port_number. Client
// socket connections use this port number to communicate
// with the server.
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments: port_number = port number the server socket
//                will listen for client
//                connections.
//
// Return:      0 (OK): No errors (see #define)
//         -1 (FAIL): Error occurred (see #define)
//
int bind_port(unsigned int port_number) {
  int set_option = 1;
  struct sockaddr_in server_address;

  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&set_option, sizeof(set_option));

  if (server_socket_fd < 0) return FAIL;

  bzero( (char *)&server_address, sizeof( server_address ) );

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port_number);

  if (bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0) {
    return OK;
  } else return FAIL;

} // end bind_port() function

// ------------------------------------
// Function implemenation that reads the
// request message from the socket, and
// writes the response message to the socket.
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments: client_socket_fd: file descriptor (fd) value
//                  positive integer number typically
//                  greater than two (0=stdin, 1=stdout, 2=stderror)
//
// Return:      None
//
void accept_client( int client_socket_fd ) {
  char request[ REQUEST_MAX_CHARS ];
  char response[ RESPONSE_MAX_CHARS ];

  close( server_socket_fd );
  bzero( request, REQUEST_MAX_CHARS );
  bzero( response, RESPONSE_MAX_CHARS );

  read( client_socket_fd, request, REQUEST_MAX_CHARS - 1 );

  int request_method = parse_request( request );

  if ( request_method != FAIL ) {
    create_response( request_method, response );
  } else {
    char msg[] = "Shame on you, bad request";
    sprintf(response, "HTTP/1.1 401 Bad Request\r\nContent-Length: %d\r\n\r\n%s", (int)strlen( msg ), msg );
  }

  write( client_socket_fd, response, strlen(response) );
  close( client_socket_fd );

  unallocate_data_arrays();

  exit( 0 );

} // end accept_client() function

// ------------------------------------
// Function implemenation that parses an HTTP
// request message sent from a client. This
// function will populate the request_keys
// and request_values 2D arrays.
//
//
// Arguments: http_request: string that holds the
//                request message
//
// Return:      0 (GET):  Method field is GET (see #define)
//        1 (POST): Method field is POST (see #define)
//         -1 (FAIL): Method field is not GET or POST (see #define)
//
//int main() {
  //parse_request("GET /?first=joseph&last=cheung&email=jcheung8@live.unc.edu HTTP/1.1\nHost: localhost:8080\nAccept: */*");
  //parse_request("POST / HTTP/1.1\nHost: localhost:8080\nAccept: */*\nContent-Length: 43\nContent-Type: application/x-www-form-urlencoded\n\nfirst=joseph&last=cheung&email=jcheung8@live.unc.edu");
  //return 0;
//}
int parse_request( char* http_request ) {
  allocate_data_arrays();
  int return_value = -1;
  for (int i = 0; i < strlen(http_request); i++) {
    if (http_request[i] == '&') {
      number_key_value_pairs++;
    }
  }
  number_key_value_pairs++;

  char character;
  if (http_request[0] == 'G' || http_request[0] == 'g') {
    return_value = 0;
  } else if (http_request[0] == 'P' || http_request[0] =='p') {
    return_value = 1;
  } else {
    return return_value;
  }
  int r1 = 0;
  int c1 = 0;
  int r2 = 0;
  int c2 = 0;
  int boo = 1;

  // GET Request
  if (return_value == 0) {
    if (strlen(http_request) == 0) {
      request_keys = NULL;
      request_values = NULL;
      return return_value;
    }
    for (int i = 0; i < strlen(http_request) - 6; i++) {
        character = http_request[i + 6];
        if (character == ' ') {
          break;
        } else if (character == '\n') {
          break;
        } else if (character == '\r') {
          break;
        }
        if (boo) {
          if (character != '=' && character != '&')  {
            request_keys[r1][c1] = character;
            c1++;
          } else if (character == '=' || character == '&') {
            r1++;
            c1 = 0;
            boo = 0;
          }
        } else {
          if (character != '=' && character != '&') {
            request_values[r2][c2] = character;
            c2++;
          } else if (character == '=' || character == '&') {
            r2++;
            c2 = 0;
            boo = 1;
          }
        }
    }
  }

  //POST Request
  int index = 0;
  if (return_value == 1) {
    if (strlen(http_request) == 0) {
      //request_keys = NULL;
      //request_values = NULL;
      return return_value;
    }
    for (int p = 0; p < strlen(http_request); p++) {
      if (http_request[p] == '\n' && http_request[p+1] == '\n') {
        index = p + 2;
      }
    }
    for (int i = 0; i < strlen(http_request) - index; i++) {
        character = http_request[i + index];
        if (character == ' ') {
          break;
        } else if (character == '\n') {
          break;
        } else if (character == '\r') {
          break;
        }
        if (boo) {
          if (character != '=' && character != '&')  {
            request_keys[r1][c1] = character;
            c1++;
          } else if (character == '=' || character == '&') {
            r1++;
            c1 = 0;
            boo = 0;
          }
        } else {
          if (character != '=' && character != '&') {
            request_values[r2][c2] = character;
            c2++;
          } else if (character == '=' || character == '&') {
            r2++;
            c2 = 0;
            boo = 1;
          }
        }
    }
  }
  /*printf("request keys:\n");
  for (int i = 0; i < number_key_value_pairs; i++) {
    printf("%s\n", request_keys[i]);
  }
  printf("request values:\n");
  for (int i = 0; i < number_key_value_pairs; i++) {
    printf("%s\n", request_values[i]);
  }*/

  return return_value;

} // end parse_request() function

// ------------------------------------
// Function implemenation that creates an HTTP
// response message that is sent to the
// client. This function uses the data in the
// request_keys and request_values 2D arrays
// (performed by the parse_request function)
//
// ****************************************
// DO NOT MODIFY THIS FUNCTION
// ****************************************
//
// Arguments: http_response: string that holds the
//                 response message
//        request_method: GET=0, POST=1, FAIL=-1
//
// Return:      None
//
void create_response( int request_method, char* http_response ) {
  char row[200] = "\0";
  char table_rows[512] = "\0";
  char entity_body[1024] = "\0";
  char *end_html = "</table>\n</body>\n</html>";


  if ( request_method == POST ) {
    strcpy( entity_body, "<html>\n<body>\n<h2>POST Operation</h2>\n<table>\n<tr><th>Key</th><th>Value</th></tr>\n");
  } else {
    strcpy( entity_body, "<html>\n<body>\n<h2>GET Operation</h2>\n<table>\n<tr><th>Key</th><th>Value</th></tr>\n" );
  }

  for ( unsigned int i=0; i<number_key_value_pairs; i++ ) {
      sprintf(row, "<tr><td>%s</td><td>%s</td></tr>\n", request_keys[i], request_values[i] );
      strcat( table_rows, row );
  }

  sprintf( http_response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s%s%s",
              ((int)strlen(entity_body) + (int)strlen(table_rows) +
              (int)strlen(end_html)),
              entity_body, table_rows, end_html);

} // end create_response() function
