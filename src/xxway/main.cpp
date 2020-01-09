#define WIN32_WINNT 0x0601//_WIN32_WINNT_WIN7
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#pragma warning (disable: 4819)
#include <xfinal.hpp>
using namespace xfinal;

#include <gzip/compress.hpp>
#include <gzip/decompress.hpp>

#include "tools/tool_Logging.hpp"
#include "tools/tool_ThreadPool.hpp"
tool_ThreadPool _pool { 1 };

#include "Connections.hpp"
Connections _conn;

int main (int argc, char *argv []) {
	tool_Logging::init (argc, argv);

	http_server _server { 1 };
	_server.listen ("0.0.0.0", "8080");

	websocket_event ws_event;
	ws_event.on ("open", [] (websocket &ws) {
		_conn.add (ws.shared_from_this ());
	});
	ws_event.on ("message", [] (websocket &ws) {
		_pool.enqueue ([] (std::shared_ptr<websocket> _ws) {
			auto _msg_code = _ws->message_code ();
			std::string _cnt = "";
			std::string_view _raw = _ws->messages ();
			if (_msg_code == 1) { // 文本
				_cnt = _raw;
			} else if (_msg_code == 2) { // 二进制
				_cnt = gzip::decompress (_raw.data (), _raw.size ());
			}
			if (_cnt != "") {
				_conn.on_msg (_ws, _cnt);
			}
		}, ws.shared_from_this ());
	});
	ws_event.on ("close", [] (websocket &ws) {
		_conn.remove (ws.uuid ());
	});
	_server.router ("/_transpack_/ws", ws_event);
	_server.run ();
	return 0;
}
