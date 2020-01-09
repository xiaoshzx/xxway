#ifndef __CONNECTIONS_HPP__
#define __CONNECTIONS_HPP__

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include <xfinal.hpp>
using namespace xfinal;

// websocket 链接池对象
class Connections {
public:
	// 链接信息
	struct ConnInfo {
		// 链接类型
		enum class ConnType { None, Gateway, Service, Client } m_conn_type = ConnType::None;
		// 链接对象
		std::shared_ptr<websocket> m_ws;
		// 对象名称，当为网关或服务时有效
		std::string m_name = "";

		ConnInfo (std::shared_ptr<websocket> _ws): m_ws (_ws) {}
	};

	void add (std::shared_ptr<websocket> _ws) {
		std::unique_lock<std::recursive_mutex> ul (m_mutex);
		m_conns.insert (std::make_pair (_ws->uuid (), std::make_shared<ConnInfo> (_ws)));
	}

	void on_msg (std::shared_ptr<websocket> _ws, std::string _cnt) {
		std::unique_lock<std::recursive_mutex> ul (m_mutex);
		auto _ptr = m_conns.find (_ws->uuid ());
		if (_ptr == m_conns.end ()) {
			add (_ws);
			_ptr = m_conns.find (_ws->uuid ());
		}
		auto _conn_info = _ptr->second;
		if (_conn_info->m_conn_type == ConnInfo::ConnType::None) {
			// 初始连接

		} else if (_conn_info->m_conn_type == ConnInfo::ConnType::Gateway) {
			// 其他网关
		} else if (_conn_info->m_conn_type == ConnInfo::ConnType::Service) {
			// 微服务端
		} else if (_conn_info->m_conn_type == ConnInfo::ConnType::Client) {
			// 客户端
		}
	}

	void remove (std::string _uuid) {
		std::unique_lock<std::recursive_mutex> ul (m_mutex);
		m_conns.erase (_uuid);
	}

	std::tuple<std::string, std::shared_ptr<ConnInfo>> get_conn (std::function<bool (std::shared_ptr<ConnInfo>)> _callback) {
		std::unique_lock<std::recursive_mutex> ul (m_mutex);
		for (decltype (auto) _pair : m_conns) {
			if (_callback (_pair.second))
				return std::make_tuple (_pair.first, _pair.second);
		}
		return std::make_tuple ("", nullptr);
	}

	std::vector<std::tuple<std::string, std::shared_ptr<ConnInfo>>> get_conns (std::function<bool (std::shared_ptr<ConnInfo>)> _callback) {
		std::unique_lock<std::recursive_mutex> ul (m_mutex);
		std::vector<std::tuple<std::string, std::shared_ptr<ConnInfo>>> v;
		for (decltype (auto) _pair : m_conns) {
			if (_callback (_pair.second))
				v.push_back (std::make_tuple (_pair.first, _pair.second));
		}
		return v;
	}

private:
	std::map<std::string, std::shared_ptr<ConnInfo>> m_conns;
	std::recursive_mutex m_mutex;
};

#endif //__CONNECTIONS_HPP__
