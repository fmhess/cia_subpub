CXXFLAGS = -Wall -g
EXECUTABLES = cia_subpub_server mypublisher mysubscriber

.PHONY: all
all: $(EXECUTABLES)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(EXECUTABLES)
	
publish_client.h: publish_spec.json
	jsonrpcstub --cpp-client=cia_subpub::publish_client $<
	
abstract_publish_server.h: publish_spec.json
	jsonrpcstub --cpp-server=cia_subpub::abstract_publish_server $<

subscribe_client.h: subscribe_spec.json
	jsonrpcstub --cpp-client=cia_subpub::subscribe_client $<
	
abstract_subscribe_server.h: subscribe_spec.json
	jsonrpcstub --cpp-server=cia_subpub::abstract_subscribe_server $<

cia_subpub_server: CXXFLAGS += -pthread
cia_subpub_server: LDFLAGS += -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-server -ljsonrpccpp-client
cia_subpub_server: cia_subpub_server.cpp abstract_publish_server.h publish_client.h abstract_subscribe_server.h

mypublisher: LDFLAGS += -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client
mypublisher: mypublisher.cpp publish_client.h

mysubscriber: LDFLAGS += -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client -ljsonrpccpp-server
mysubscriber: mysubscriber.cpp subscribe_client.h abstract_publish_server.h
