#include <curl/curl.h>

#include <url/params.hpp>
#include <yadisk/client.hpp>
#include <boost/algorithm/string/join.hpp>

#include <sstream>
#include <set>
using std::stringstream;

#include "callbacks.hpp"
#include "quote.hpp"
#include "wrappers.hpp"

static void parse_path (url::params_t& url_params, const std::string& resource, CURL * curl) {
	url_params["path"] = quote(resource, curl);
}

static void parse_sort (url::params_t& url_params, const json& options) {
	if (options.find("sort") != options.end())
	{
		if (options["sort"].is_string())
		{
			std::string temp = options["sort"].get<std::string>();
			std::set<std::string> valid_sort_options { "name", "-name",
				"path", "-path", "created", "-created", "modified",
				"-modified", "size", "-size"};
			if (valid_sort_options.find(temp) != valid_sort_options.end())
			{
				url_params["sort"] = temp;
			}
		}
	}
}

static void parse_limit (url::params_t& url_params, const json& options) {
	if (options.find("limit") != options.end())
	{
		if (options["limit"].is_number())
		{
			int temp = options["limit"].get<int>();
			if (temp > 0)
			{
				url_params["limit"] = std::to_string(temp);
			}
		}
	}
}

static void parse_offset (url::params_t& url_params, const json& options) {
	if (options.find("offset") != options.end())
	{
		if (options["offset"].is_number())
		{
			int temp = options["offset"].get<int>();
			if (temp >= 0)
			{
				url_params["offset"] = std::to_string(temp);
			}
		}
	}
}

static void parse_fields (url::params_t& url_params, const json& options) {
	if (options.find("fields") != options.end())
	{
		if (options["fields"].is_array())
		{
			std::string temp;
			for (json::const_iterator it = options["fields"].begin(); it != options["fields"].end(); ++it) {
				if (it != options["fields"].begin()) {
					temp.push_back(',');
				}
				temp += it->get<std::string>();
			}
			url_params["fields"] = temp;
		}
		else if (options["fields"].is_string())
		{
			url_params["fields"] = options["fields"].get<std::string>();
		}
	}
}

static void parse_preview_size (url::params_t& url_params, const json& options) {
	if (options.find("preview_size") != options.end())
	{
		if (options["preview_size"].is_string())
		{
			url_params["preview_size"] = options["preview_size"].get<std::string>();
		}
		else if (options["preview_size"].is_number())
		{
			url_params["preview_size"] = std::to_string (options["preview_size"].get<int>());
		}
	}
}

static void parse_preview_crop (url::params_t& url_params, const json& options) {
	if (options.find("preview_crop") != options.end())
	{
		if (options["preview_crop"].is_boolean())
		{
			url_params["preview_crop"] = options["preview_crop"].get<bool>();
		}
	}
}

static url::params_t parse_params_for_info (const std::string& resource, const json& options, CURL * curl) {
	url::params_t url_params;
	parse_path (url_params, resource, curl);
	parse_sort (url_params, options);
	parse_limit (url_params, options);
	parse_offset (url_params, options);
	parse_fields (url_params, options);
	parse_preview_size (url_params, options);
	parse_preview_crop (url_params, options);
	return url_params;
}

static std::string is_resource_in_trash(const json& options) {
	std::string trash = "";
	if (options.find("deleted") != options.end())
	{
		if (options["deleted"].is_boolean())
		{
			if (options["deleted"].get<bool>())
			{
				trash = "/trash";
			}
		}
	}
	return trash;
}

static std::stringstream perform_request (CURL * curl,
        const char* url, curl_slist * header_list,
        const char* type = "GET", int ssl_verify_host = 0,
        int ssl_verify_peer = 0) {
	stringstream response;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, type);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write<stringstream>);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, ssl_verify_host);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, ssl_verify_peer);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

	auto response_code = curl_easy_perform(curl);

	if (response_code != CURLE_OK) {
		throw std::runtime_error("curl_easy_perform");
	}
	return response;
}

namespace yadisk
{
	static const std::string api_url = "https://cloud-api.yandex.net/v1/disk";

	Client::Client(string token_) : token{token_} {}

	auto Client::ping() -> bool {

		CURL * curl = curl_easy_init();
		if (curl == nullptr) return false;

		std::string url = api_url;

		struct curl_slist *header_list = nullptr;
		std::string auth_header = "Authorization: OAuth " + token;
		header_list = curl_slist_append(header_list, auth_header.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

		auto response_code = curl_easy_perform(curl);
		
		long http_response_code = 0;
		if (response_code == CURLE_OK) {
			response_code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);			
		}	
		
		curl_slist_free_all(header_list);
		curl_easy_cleanup(curl);

		if (response_code != CURLE_OK) return false;

		return http_response_code == 200;
	}

	auto Client::info(url::path resource, json options/*= nullptr*/) -> json {
		try {
			return info_impl(resource, options);
		}
		catch(...) {
			return json();
		}
	}

	auto Client::info_impl (url::path resource, json options) -> json {
		CurlWrapper curl_wrapper;

		auto url_params = parse_params_for_info(resource.string(), options, curl_wrapper.getCurl());

		auto trash = is_resource_in_trash(options);
		std::string url = api_url + trash + "/resources" + "?" + url_params.string();

		std::string auth_header = "Authorization: OAuth " + token;
		CurlSlistWrapper header_list {auth_header.c_str()};

		auto response = perform_request (curl_wrapper.getCurl(), url.c_str(),
			header_list.getCurlSlist());
		auto response_data = json::parse (response);

		return response_data;
	}

	auto Client::copy(url::path from, url::path to, bool overwrite, std::list<std::string> fields) -> json {

		CURL * curl = curl_easy_init();
		if (!curl) return json();

		url::params_t url_params;
		url_params["from"] = quote(from.string(), curl);
		url_params["path"] = quote(to.string(), curl);
		url_params["overwrite"] = overwrite;
		url_params["fields"] = boost::algorithm::join(fields, ",");
		std::string url = api_url + "/resources/copy" + "?" + url_params.string();

		struct curl_slist *header_list = nullptr;
		std::string auth_header = "Authorization: OAuth " + token;
		header_list = curl_slist_append(header_list, auth_header.c_str());

		stringstream response;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_READDATA, &response);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, write<stringstream>);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

		auto response_code = curl_easy_perform(curl);

		curl_slist_free_all(header_list);
		curl_easy_cleanup(curl);

		if (response_code != CURLE_OK) return json();

		auto response_data = json::parse(response);
		return response_data;
	}
    
	auto Client::patch(url::path resource, json meta, std::list<string> fields) -> json {

		// init http request
		CURL * curl = curl_easy_init();
		if (curl == nullptr) return json();

		// fill http url
		url::params_t url_params;
		url_params["fields"] = boost::algorithm::join(fields, ",");
		url_params["path"] = quote(resource.string(), curl);
		std::string url = api_url + "/resources" + "?" + url_params.string();

		// fill http headers
		curl_slist * header_list = nullptr;
		std::string auth_header = "Authorization: OAuth " + token;
		header_list = curl_slist_append(header_list, "Content-Type: application/json");
		header_list = curl_slist_append(header_list, auth_header.c_str());

		// fill http body
		auto request_body = meta.dump();

		// build http request
		stringstream response_body;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write<stringstream>);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_body.size());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

		// perform http request
		auto response_code = curl_easy_perform(curl);

		// clean resources
		curl_slist_free_all(header_list);
		curl_easy_cleanup(curl);

		// check response code
		if ( response_code != CURLE_OK ) return json();

		// handle body of http response
		auto info = json::parse(response_body);
		return info;
	}
}

class curl_environment {
public:
    curl_environment() {
        curl_global_init(CURL_GLOBAL_ALL);
    }
    ~curl_environment() {
        curl_global_cleanup();
    }
};

static const curl_environment env;
