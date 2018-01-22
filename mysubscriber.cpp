#include <iostream>

#include "subscribe_client.h"
#include "abstract_publish_server.h"
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>
#include <unistd.h>
#include <cstdio>

class receive_publication_server : public cia_subpub::abstract_publish_server
{
public:
	receive_publication_server(jsonrpc::AbstractServerConnector &connector);

	virtual void publish(const std::string& channel, int event_id);
};

receive_publication_server::receive_publication_server(jsonrpc::AbstractServerConnector &connector) :
    cia_subpub::abstract_publish_server(connector)
{
}

void receive_publication_server::publish(const std::string& channel, int event_id)
{
	std::cout << "Channel: \"" << channel << "\" event: " << event_id << std::endl;
}

int main(int argc, char* argv[])
{
	const char *channel = "test";
	if(argc > 1)
		channel = argv[1];

	jsonrpc::UnixDomainSocketClient client_connector("/tmp/mysubscribesocket");
	cia_subpub::subscribe_client sub_client(client_connector);
	
	const std::string receive_publication_socket_path = tmpnam(NULL); // FIXME: tmpnam is evil
	std::cout << "Receive publication socket path: \"" << receive_publication_socket_path << "\"" << std::endl;
	jsonrpc::UnixDomainSocketServer server_connector(receive_publication_socket_path.c_str());
	receive_publication_server pub_server(server_connector);

	pub_server.StartListening();
	sub_client.subscribe(channel , receive_publication_socket_path);
    getchar();
	pub_server.StopListening();

    return 0;
}
