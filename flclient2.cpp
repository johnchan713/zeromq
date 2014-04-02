//---------------ZMQ_CPP_FREELANCE2_CLIENT------------
#include <zmq.hpp>
#include <zmsg.hpp>
#include <zhelpers.hpp>
#include <iostream>
//#include <czmq.h>

#define GLOBAL_TIMEOUT 2500

typedef struct _flclient_t flclient_t;
flclient_t *flclient_new(void);
void flclient_destroy(flclient_t ** self_p);
void flclient_connect(flclient_t * self, char *endpoint);
zmsg *flclient_request(flclient_t * self, zmsg ** request_p);

struct _flclient_t {
    zmq::context_t * ctx;
    zmq::socket_t * socket;
    size_t servers;
    uint sequence;
};

flclient_t *flclient_new(void)
{
    flclient_t *self;
    self = (flclient_t *) calloc(1, sizeof(flclient_t));
    self->ctx = new zmq::context_t(1);
    self->socket = new zmq::socket_t(*(self->ctx), ZMQ_DEALER);
    return self;
}

void flclient_destroy(flclient_t ** self_p)
{
    assert(self_p);
    if (*self_p) {
	flclient_t *self = *self_p;
	(self->ctx)->close();
	free(self);
	*self_p = NULL;
    }
}

void flclient_connect(flclient_t * self, char *endpoint)
{
    assert(self);
    (*(self->socket)).connect(endpoint);
    self->servers++;
}

zmsg *flclient_request(flclient_t * self, zmsg ** request_p)
{
    int debug = 0;
    assert(self);
    assert(*request_p);
    zmsg *request = *request_p;

    char sequence_text[10];
    sprintf(sequence_text, "%u", ++self->sequence);
    request->push_front(sequence_text);
    request->push_front("");

    int server;
    for (server = 0; server < self->servers; server++) {
	zmsg *msg = new zmsg(*request);
	msg->send(*(self->socket));
    }

    zmsg *reply = NULL;
    std::string part1, part2, part3;
    part1.clear();
    part2.clear();
    part3.clear();
    uint64_t endtime = s_clock() + GLOBAL_TIMEOUT;
    while (s_clock() < endtime) {
	std::cout.clear();
	zmq::pollitem_t items[] = {
	    {
	    *(self->socket), 0, ZMQ_POLLIN, 0}
	};
	int rc = zmq::poll(items, 1, (endtime - s_clock()) * 1000);
	if (rc == -1)
	    break;
	std::cout.clear();
	if (items[0].revents & ZMQ_POLLIN) {
	    reply = new zmsg(*(self->socket));
	    assert(reply->parts() == 3);

	    part1 = (char *) reply->pop_front().c_str();
	    part2 = (char *) reply->pop_front().c_str();
	    part3 = (char *) reply->pop_front().c_str();

	    //debug purpose
	    //std::cout << "First frame : " << part1 << std::endl;
	    //std::cout << "second frame : " << part2 << std::endl;
	    //std::cout << "third frame : " << part3 << std::endl;

	    int sequence_nbr = atoi(part3.c_str());
	    //std::cout << "received sequence no. from third frame : " << sequence_nbr << std::endl;
	    //std::cout << "Our sequence number: " << self->sequence << std::endl;

	    if (sequence_nbr == self->sequence)
		break;
	    reply->clear();
	}
    }

    (**request_p).clear();
    return reply;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
	std::
	    cout << "Error: syntax should be " << argv[0] <<
	    " <endpoint>... \n";
	return 0;
    }

    flclient_t *client = flclient_new();
    int argn;
    for (argn = 1; argn < argc; argn++)
	flclient_connect(client, argv[argn]);

    int requests = 10000;
    uint64_t start = s_clock();
    while (requests--) {
	zmsg *request = new zmsg();
	request->push_front("random name");
	zmsg *reply = flclient_request(client, &request);
	if (!reply) {
	    std::cout << "name service not availbale, aborting...\n";
	    break;
	}
	reply->clear();
    }
    std::cout << "Average round trip cost: " << (int) (s_clock() -
						       start) /
	10 << " usec \n";

    flclient_destroy(&client);
    return 0;
}
