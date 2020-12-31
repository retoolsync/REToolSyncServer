#include "CivetServer.h"
#include <cstring>
#include <atomic>
#include <map>

// System headers for Sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::atomic_bool exitNow;

// https://github.com/civetweb/civetweb/blob/2261bfc7a15b66705bf82905d6f8a32324bf9c6e/examples/embedded_cpp/embedded_cpp.cpp#L343
class WebSocketHandler : public CivetWebSocketHandler
{
	int m_connectionIndex = 0;
	std::map<const struct mg_connection*, int> m_connections;

	void handleClose(CivetServer* server,
		const struct mg_connection* conn) override
	{
		auto found = m_connections.find(conn);
		if (found == m_connections.end())
			throw std::exception("Invalid connection state (disconnect)");
		printf("[%d] WS closed\n", found->second);
		m_connections.erase(found);
	}

	bool handleConnection(CivetServer* server,
		const struct mg_connection* conn) override
	{
		printf("WS connected\n");
		return true;
	}

	void handleReadyState(CivetServer* server,
		struct mg_connection* conn) override
	{
		if (m_connections.find(conn) != m_connections.end())
			throw std::exception("Invalid connection state (connect)");
		m_connections.emplace(conn, m_connectionIndex++);

		printf("[%d] WS ready\n", m_connections[conn]);

		//const char* text = "Hello from the websocket ready handler";
		//mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	}

	bool handleData(CivetServer* server,
		struct mg_connection* conn,
		int bits,
		char* data,
		size_t data_len) override
	{
		printf("[%d] WS got %lu bytes: ", m_connections[conn], (long unsigned)data_len);
		fwrite(data, 1, data_len, stdout);
		printf("\n");

		// mutex for m_connections?
		for (const auto [c, idx] : m_connections)
			if (c != conn)
				mg_websocket_write(const_cast<struct mg_connection*>(c), MG_WEBSOCKET_OPCODE_TEXT, data, data_len);

		// Keep connection alive
		return true;
	}
};

// This server broadcasts all messages received to all clients but the sender
int main(int argc, char* argv[])
{
	// TODO: (see https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md for help)
	// enable_websocket_ping_pong on
	// websocket_timeout_ms 2000
	// ssl_certificate (TODO: ssl support with mbedtls)
	// add command line
	// support running as a service
	// gracefully exit on ctrl+c/signal in a portable way
	std::string listening_ports("127.0.0.1:6969");
	std::string endpoint("/REToolSync");
	auto server = CivetServer({ "listening_ports", listening_ports });
	WebSocketHandler h_websocket;
	server.addWebSocketHandler(endpoint, h_websocket);
	printf("Listening on %s%s...\n", listening_ports.c_str(), endpoint.c_str());
	while (!exitNow)
	{
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}
}