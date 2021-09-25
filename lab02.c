// -----------------------------------
// 	COMP 530: Operating Systems
//
//	Fall 2021 - Lab 2
// -----------------------------------

#include <stdio.h>		// IO functions
#include <stdlib.h> 	// atof function
#include "server.h"		// our code :)


int main( int argc, char *argv[] ) {

	unsigned int port_number = 0;

	// -------------------------------------------------
	// Verify correct number of arguments are provided
	// when the server is executed.

	if ( argc == 2 ) { 

		port_number = (unsigned int) atoi( argv[1] );

		allocate_data_arrays();
		initialize_handler();
		run_server( port_number );
			

	} else {

		printf( "Port number is not specified!\n" );
		printf( "Example usage: %s %s\n", argv[0], "8080" );

	}

	return 0;
     
} // end main() function
