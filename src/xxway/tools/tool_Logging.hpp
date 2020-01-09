#ifndef __TOOL_LOGGING_HPP__
#define __TOOL_LOGGING_HPP__

#include <ctime>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <exception>

#include <Windows.h>

class tool_Logging {
public:
	static void init (int argc, char *argv []) {
		if (argc < 1 || argv [0][0] == '\0')
			return;
		std::string _path = argv [0];
		if (_path [0] == '"') {
			_path.erase (_path.begin ());
			_path.erase (_path.begin () + _path.size () - 1);
		}
		size_t _p = 0;
		while ((_p = _path.find ('\\', _p)) != std::string::npos)
			_path [_p] = '/';
		_path.erase (_path.begin () + _p, _path.end ());
		_path += "log/";
		::CreateDirectory (_path.c_str (), nullptr);
		_get_obj ().m_path = _path;
	}

	void write (std::string _state, const char *_file, const int _line, const char *fmt_str, ...) {
		auto &_obj = _get_obj ();
		std::string _file = _obj.m_path + _get_date (false) + ".log", str_result = "";
		try {
			int _len = lstrlenA (fmt_str);
			if (_len > 0) {
				va_list ap;
				//来源：http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
				ptrdiff_t final_n, n = ((ptrdiff_t) _len) * 2;
				std::unique_ptr<char []> formatted;
				while (true) {
					formatted.reset (new char [n]);
					va_start (ap, fmt_str);
					final_n = _vsnprintf_s ((char *) &formatted [0], n, _TRUNCATE, fmt_str, ap);
					va_end (ap);
					if (final_n < 0 || final_n >= n)
						n += abs (final_n - n + 1);
					else
						break;
				}
				str_result = formatted.get ();
			}
		} catch (...) {
		}
		std::string str_file = _file;
		size_t _p = str_file.rfind ('\\');
		if (_p != std::string::npos)
			str_file.erase (str_file.begin () + _p, str_file.end ());
		_p = str_file.rfind ('/');
		if (_p != std::string::npos)
			str_file.erase (str_file.begin () + _p, str_file.end ());
		{
			static std::recursive_mutex m;
			std::lock_guard<std::recursive_mutex> lg (m);
			std::ofstream ofs (_file, std::ios::app | std::ios::binary);
			ofs << "[" << _get_date (true) << "][" << _state << " in " << str_file << ":" << _line << "] " << str_result << std::endl;
			ofs.close ();
		}
	}

private:
	std::string m_path = "";
	static tool_Logging &_get_obj () {
		static tool_Logging _logging;
		return _logging;
	}

	static std::string _get_date (bool _long) {
		char buf_time [32] = { 0 };
		auto time_now = std::chrono::system_clock::now ();
		time_t raw_time = std::chrono::system_clock::to_time_t (time_now);
		tm *local_time_now = localtime (&raw_time);//_localtime64_s
		if (_long) {
			char buf_time2 [32] = { 0 };
			strftime (buf_time2, sizeof (buf_time2), "%Y%m%d_%H%M%S", local_time_now);
			auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch ());
			auto ms_part = duration_in_ms - std::chrono::duration_cast<std::chrono::seconds>(duration_in_ms);
			sprintf (buf_time, "%s_%03d", buf_time2, (int) ms_part.count ());
		} else {
			strftime (buf_time, sizeof (buf_time), "%Y%m%d", local_time_now);
		}
		return buf_time;
	}
};

#define LOG_DEBUG(...)    tool_Logging::write("debug   ",__FILE__,__LINE__,__VA_ARGS__)
#define LOG_INFO(...)     tool_Logging::write("info    ",__FILE__,__LINE__,__VA_ARGS__)
#define LOG_WARNING(...)  tool_Logging::write("warning ",__FILE__,__LINE__,__VA_ARGS__)
#define LOG_ERROR(...)    tool_Logging::write("error   ",__FILE__,__LINE__,__VA_ARGS__)
#define LOG_CRITICAL(...) tool_Logging::write("critical",__FILE__,__LINE__,__VA_ARGS__)

#endif //__TOOL_LOGGING_HPP__
