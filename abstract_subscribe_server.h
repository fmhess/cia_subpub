/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_CIA_SUBPUB_ABSTRACT_SUBSCRIBE_SERVER_H_
#define JSONRPC_CPP_STUB_CIA_SUBPUB_ABSTRACT_SUBSCRIBE_SERVER_H_

#include <jsonrpccpp/server.h>

namespace cia_subpub {
    class abstract_subscribe_server : public jsonrpc::AbstractServer<abstract_subscribe_server>
    {
        public:
            abstract_subscribe_server(jsonrpc::AbstractServerConnector &conn, jsonrpc::serverVersion_t type = jsonrpc::JSONRPC_SERVER_V2) : jsonrpc::AbstractServer<abstract_subscribe_server>(conn, type)
            {
                this->bindAndAddNotification(jsonrpc::Procedure("subscribe", jsonrpc::PARAMS_BY_NAME, "channel",jsonrpc::JSON_STRING,"socket_path",jsonrpc::JSON_STRING, NULL), &cia_subpub::abstract_subscribe_server::subscribeI);
                this->bindAndAddNotification(jsonrpc::Procedure("unsubscribe", jsonrpc::PARAMS_BY_NAME, "channel",jsonrpc::JSON_STRING,"socket_path",jsonrpc::JSON_STRING, NULL), &cia_subpub::abstract_subscribe_server::unsubscribeI);
            }

            inline virtual void subscribeI(const Json::Value &request)
            {
                this->subscribe(request["channel"].asString(), request["socket_path"].asString());
            }
            inline virtual void unsubscribeI(const Json::Value &request)
            {
                this->unsubscribe(request["channel"].asString(), request["socket_path"].asString());
            }
            virtual void subscribe(const std::string& channel, const std::string& socket_path) = 0;
            virtual void unsubscribe(const std::string& channel, const std::string& socket_path) = 0;
    };

}
#endif //JSONRPC_CPP_STUB_CIA_SUBPUB_ABSTRACT_SUBSCRIBE_SERVER_H_
