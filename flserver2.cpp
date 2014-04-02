//-----------------ZMQ_CPP_FREELANCE2_SERVER---------------
#include <zmq.hpp>
#include <zmsg.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2) {
	std::
	    cout << "Error: syntax should be " << argv[0] <<
	    " <endpoint> \n";
	return 0;
    }

    zmq::context_t * ctx = new zmq::context_t(1);
    zmq::socket_t * server = new zmq::socket_t(*ctx, ZMQ_REP);
    server->bind(argv[1]);

    std::cout << "Service is ready at " << argv[1] << std::endl;
    while (1) {
	zmsg *msg = new zmsg(*server);
	if (!msg)
	    break;

	assert(msg->parts() == 2);

	std::string identity = (char *) (*msg).pop_front().c_str();
	std::string sequence = (char *) (*msg).pop_front().c_str();

	zmsg *reply = new zmsg();
	reply->push_front(const_cast<char*>(identity.c_str()));
	reply->push_front(const_cast<char*>(sequence.c_str()));
	reply->send(*server);
    }

    if (s_interrupted)
	std::cout << "Interrupted \n";

    ctx->close();
    return 0;
}
