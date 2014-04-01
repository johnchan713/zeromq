//-----------------ZMQ_freelance1_SERVER---------------
#include <zmq.hpp>
#include <zmsg.hpp>
#include <stdio.h>
#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
	if (argc <2) {
	std::cout << "Error: syntax should be " << argv[0] << " <endpoint> \n";	
	return 0;
	}

    std::cout << "creating context and socket..\n";
    zmq::context_t ctx(1);
    zmq::socket_t socket(ctx, ZMQ_REP);

    std::cout << "binding to " << argv[1] << "... \n";
    socket.bind(argv[1]);

    std::cout << "Service is ready at " << argv[1] << std::endl;
    while (1) {
	zmsg zm (socket);

	std::cout << "Reply received - " << zm.body () << std::endl;
	zm.send(socket);
	std::cout << "response has been sent to client!\n";
    }
	
    ctx.close();
    return 0;
}
