#include <iostream>

#include "publish_client.h"
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	const char *channel = "test";
	int event_id = 1;
	long publish_period = 1;
	if(argc > 1)
		channel = argv[1];
	if(argc > 2)
		event_id = strtol(argv[2], 0, 0);
	if(argc > 3)
		publish_period = strtol(argv[3], 0, 0);
	
	jsonrpc::UnixDomainSocketClient connector("/tmp/mypublishsocket");
	cia_subpub::publish_client client(connector);
	while (true)
	{
		try
		{
			sleep(publish_period);
			std::cout << "publishing event " << event_id << " on channel \"" << channel << "\"." << std::endl;
			client.publish(channel, event_id);
		}
		catch (jsonrpc::JsonRpcException e)
		{
			std::cerr << e.what() << std::endl;
			break;
		}
	}
}
