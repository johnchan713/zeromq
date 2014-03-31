//---------------ZMQ_CPP_FREELANCE1_CLIENT------------
#include <zmq.hpp>
#include <zmsg.hpp>
#include <iostream>

#define REQUEST_TIMEOUT 1000
#define MAX_RETRIES	3

static zmsg *retry_request(zmq::context_t * ctx, char *endpoint,
			   zmsg * request)
{
    std::cout << "Typing echo service at " << endpoint << std::endl;
    zmq::socket_t * client = new zmq::socket_t(*ctx, ZMQ_REQ);
    client->connect(endpoint);
    zmsg *msg = new zmsg(*request);
    msg->send(*client);

    zmq::pollitem_t items[] = { {
    *client, 0, ZMQ_POLLIN, 0}};
    zmq::poll(items, 1, REQUEST_TIMEOUT * 1000);

    zmsg *reply = NULL;

    if (items[0].revents & ZMQ_POLLIN)
	reply = new zmsg(*client);

    client->close();
    return reply;
}

int main(int argc, char *argv[])
{
    zmq::context_t * ctx = new zmq::context_t(1);
    zmsg *request = new zmsg();
    request->body_set("Hello");
    zmsg *reply = NULL;

    int endpoints = argc - 1;
    if (endpoints == 0)
	std::
	    cout << "Error: syntax should be " << argv[0] <<
	    " <endpoint> ... \n";
    else if (endpoints == 1) {
	int retries;
	for (retries = 0; retries < MAX_RETRIES; retries++) {
	    char *endpoint = argv[1];
	    reply = retry_request(ctx, endpoint, request);
	    if (reply) {
		std::cout << "received reply from server 1\n";
		break;

	    }
	    std::
		cout << "No response from " << endpoint <<
		", retrying...\n";
	}
    } else {

	int i;
	for (i = 0; i < endpoints; i++) {

	    char *endpoint = argv[i + 1];
	    reply = retry_request(ctx, endpoint, request);
	    if (reply) {
		std::cout << "received reply from server " << i +
		    1 << "...\n";
		break;
	    }
	    std::cout << "No response from " << endpoint << std::endl;
	}
    }
    if (reply)
	std::cout << "Service is running OK\n";

    request->clear();
    reply->clear();
    ctx->close();

    return 0;
}
