#include <iostream>

#include "stubclient.h"
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>

using namespace jsonrpc;
using namespace std;

int main()
{
    UnixDomainSocketClient client("/tmp/mytestsocket");
    StubClient c(client);
    try
    {
        cout << c.sayHello("Peter") << endl;
        c.notifyServer();
    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }
}
