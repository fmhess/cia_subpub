/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_CIA_SUBPUB_SUBSCRIBE_CLIENT_H_
#define JSONRPC_CPP_STUB_CIA_SUBPUB_SUBSCRIBE_CLIENT_H_

#include <jsonrpccpp/client.h>

namespace cia_subpub {
    class subscribe_client : public jsonrpc::Client
    {
        public:
            subscribe_client(jsonrpc::IClientConnector &conn, jsonrpc::clientVersion_t type = jsonrpc::JSONRPC_CLIENT_V2) : jsonrpc::Client(conn, type) {}

            void subscribe(const std::string& channel, const std::string& socket_path) throw (jsonrpc::JsonRpcException)
            {
                Json::Value p;
                p["channel"] = channel;
                p["socket_path"] = socket_path;
                this->CallNotification("subscribe",p);
            }
    };

}
#endif //JSONRPC_CPP_STUB_CIA_SUBPUB_SUBSCRIBE_CLIENT_H_
