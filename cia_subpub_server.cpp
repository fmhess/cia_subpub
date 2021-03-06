#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <iostream>
#include <cassert>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include "abstract_publish_server.h"
#include "publish_client.h"
#include "abstract_subscribe_server.h"

namespace cia_subpub
{
	class subscriber
	{
	public:
		subscriber(const std::string &channel, const std::string &socket_path) : 
			m_connector(socket_path), m_client(m_connector), m_channel(channel), 
			m_thread(&subscriber::thread_function, this), m_alive(true)
		{
		}
		
		~subscriber()
		{
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_alive = false;
			}
			m_condition_variable.notify_all();
			m_thread.join();
		}
		
		void publish(int event_id)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_event_queue.push_back(event_id);
			m_condition_variable.notify_all();
		}
		
		bool alive() const
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			return m_alive;
		}
	private:
		void thread_function()
		{
			while(true)
			{
				int event_id;
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_condition_variable.wait(lock, std::bind(&subscriber::thread_ready, this));
					if(m_alive == false) break;
					else if(m_event_queue.empty())
					{
						continue;
					}
					event_id = m_event_queue.front();
					m_event_queue.pop_front();
				}
				try
				{
					m_client.publish(m_channel, event_id);
				} catch(const jsonrpc::JsonRpcException &exception)
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					std::cerr << "Subscriber error: " << exception.what() << std::endl;
					m_alive = false;
					break;
				}
			}
		}
		
		bool thread_ready() const
		{
			return m_alive == false || m_event_queue.empty() == false;
		}
		
		mutable std::mutex m_mutex;
		jsonrpc::UnixDomainSocketClient m_connector;
		publish_client m_client;
		std::string m_channel;
		std::deque<int> m_event_queue;
		std::thread m_thread;
		std::condition_variable m_condition_variable;
		bool m_alive;
	};
	
	class subscriber_map
	{
	public:
		void publish(const std::string& channel, int event_id)
		{
			std::map<std::string, std::map<std::string, std::shared_ptr<subscriber> > > local_map;
			// make a local copy of the map while holding the mutex, so we can drop the mutex
			// before publishing (avoiding potential deadlocks if publish causes subscription changes).
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				
				// clean up map while we hold mutex 
				auto map_it = m_map.find(channel);
				if (map_it == m_map.end())
				{
					return;
				}
				auto &socket_map = (*map_it).second;
				for(auto socket_map_it = socket_map.begin(); socket_map_it != socket_map.end(); ++socket_map_it)
				{
					while(socket_map_it != socket_map.end() && (*socket_map_it).second->alive() == false)
					{
						std::cerr << "Removing dead subscriber from channel \"" << channel << "\"." << std::endl;
						auto dead_it = socket_map_it;
						++socket_map_it;
						socket_map.erase(dead_it);
					}
					if(socket_map_it == socket_map.end()) break;
				}
				if(socket_map.empty())
				{
					std::cerr << "Cleaning up empty subscriber socket map for channel \"" << channel << "\"." << std::endl;
					m_map.erase(map_it);
					return;
				}
				
				local_map = m_map;
			}
			
			auto map_it = local_map.find(channel);
			assert(map_it != local_map.end());
			auto &socket_map = (*map_it).second;
			for(auto socket_map_it = socket_map.begin(); socket_map_it != socket_map.end(); ++socket_map_it)
			{
				(*socket_map_it).second->publish(event_id);
			}
		}
		
		void add_subscriber(const std::string &channel, const std::string &subscriber_socket_path)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_map[channel][subscriber_socket_path] = std::shared_ptr<subscriber>(new subscriber(channel, subscriber_socket_path));
		}
		
		void unsubscribe(const std::string &channel, const std::string &subscriber_socket_path)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto map_it = m_map.find(channel);
			if (map_it == m_map.end()) return;
			auto &socket_map = (*map_it).second;
			socket_map.erase(subscriber_socket_path);
			if (socket_map.empty())
			{
				m_map.erase(map_it);
			}
		}
		
	private:
		std::mutex m_mutex;
		std::map<std::string, std::map<std::string, std::shared_ptr<subscriber> > > m_map;
	};
	
	class publish_server : public abstract_publish_server
	{
	public:
		publish_server(jsonrpc::AbstractServerConnector &connector);

		virtual void publish(const std::string& channel, int event_id);
		std::shared_ptr<subscriber_map> subscribers() const {return m_subscribers;}
	private:
		std::shared_ptr<subscriber_map> m_subscribers;
	};

	publish_server::publish_server(jsonrpc::AbstractServerConnector &connector) :
		abstract_publish_server(connector), m_subscribers(new subscriber_map)
	{
	}
	
	void publish_server::publish(const std::string& channel, int event_id)
	{
		std::cout << "broadcasting event " << event_id << " on channel \"" << channel << "\"." << std::endl;
		m_subscribers->publish(channel, event_id);
	}

	
	class subscribe_server : public abstract_subscribe_server
	{
	public:
		subscribe_server(jsonrpc::AbstractServerConnector &connector, const std::shared_ptr<subscriber_map> &subscribers);

		virtual void subscribe(const std::string& channel, const std::string& socket_path);
		virtual void unsubscribe(const std::string& channel, const std::string& socket_path);
	private:
		std::shared_ptr<subscriber_map> m_subscribers;
	};

	subscribe_server::subscribe_server(jsonrpc::AbstractServerConnector &connector, const std::shared_ptr<subscriber_map> &subscribers) :
		abstract_subscribe_server(connector), m_subscribers(subscribers)
	{
	}
	
	void subscribe_server::subscribe(const std::string& channel, const std::string& socket_path)
	{
		m_subscribers->add_subscriber(channel, socket_path);
	}

	void subscribe_server::unsubscribe(const std::string& channel, const std::string& socket_path)
	{
		m_subscribers->unsubscribe(channel, socket_path);
	}
}

int main()
{
	try
	{
		jsonrpc::UnixDomainSocketServer publish_server_connector("/tmp/mypublishsocket");
		cia_subpub::publish_server my_publish_server(publish_server_connector);
		
		jsonrpc::UnixDomainSocketServer subscribe_server_connector("/tmp/mysubscribesocket");
		cia_subpub::subscribe_server my_subscribe_server(subscribe_server_connector, my_publish_server.subscribers());

		if(my_publish_server.StartListening() == false)
		{
			throw std::runtime_error("Failed to start publish server.  You probably need to unlink socket.");
		}
		if(my_subscribe_server.StartListening() == false)
		{
			throw std::runtime_error("Failed to start subscribe server.  You probably need to unlink socket.");
		}
		getchar();
		my_publish_server.StopListening();
		my_subscribe_server.StopListening();
	}
	catch(const std::exception &err)
	{
		std::cerr << err.what() << std::endl;
	}
	return 0;
}
