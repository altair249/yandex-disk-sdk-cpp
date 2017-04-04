#ifndef __WRAPPERS_HPP__
#define __WRAPPERS_HPP__

#include <curl/curl.h>

class CurlWrapper
{
public:
	CurlWrapper() {
		curl = curl_easy_init();
		if (curl == nullptr) {
			throw std::runtime_error("curl_easy_init");
		}
	}
	virtual ~CurlWrapper() {
		curl_easy_cleanup(curl);
	}
	CURL * getCurl() {
		return curl;
	}

private:
	CURL * curl;
};

class CurlSlistWrapper
{
public:
	explicit CurlSlistWrapper(const char* s)
		: header_list(nullptr)
	{
		header_list = curl_slist_append(header_list, s);
	}
	virtual ~CurlSlistWrapper() {
		curl_slist_free_all(header_list);
	}
	curl_slist * getCurlSlist() {
		return header_list;
	}

private:
	curl_slist * header_list;
};

#endif // __WRAPPERS_HPP__
