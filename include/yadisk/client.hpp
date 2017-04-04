#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
using std::string;

#include <list>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "url/path.hpp"

namespace yadisk
{
    class Client
    {
    public:

        Client(string token);

        auto ping() -> bool;

        auto info() -> json;

        ///
        /// \brief info, full information is here:
        ///     https://tech.yandex.ru/disk/api/reference/meta-docpage/
        /// \param resource is a path to desired file/folder
        /// \param options is json with deleted flag, sort, limit, offset, fields,
        ///     preview_size, preview_crop params
        /// \return meta information about resource, also empty json() on some errors,
        ///     on some errors json with error message.
        /// One example:
        /// \include examples/info.cpp
        /// For more info about usage the method see tests/client/info.cpp
        ///
        auto info(url::path resource, json options = nullptr) -> json;

        auto list(json options = nullptr) -> json;

        auto upload(url::path to, fs::path from, bool overwrite, std::list<string> fields = std::list<string>()) -> json;

        auto upload(url::path to, string url, std::list<string> fields = std::list<string>()) -> json;

        auto download(url::path from, url::path to, std::list<string> fields = std::list<string>()) -> json;

        auto copy(url::path from, url::path to, bool overwrite, std::list<string> fields = std::list<string>()) -> json;

        auto move(url::path from, url::path to, bool overwrite, std::list<string> fields = std::list<string>()) -> json;

        auto mkdir(url::path dir, std::list<string> fields = std::list<string>()) -> json;

        auto remove(url::path resource, bool permanently, std::list<string> fields = std::list<string>()) -> json;

        auto publish(url::path resource) -> json;

        auto unpublish(url::path resource) -> json;

        auto patch(url::path resource, json meta, std::list<string> fields = std::list<string>()) -> json;

        auto info(string public_key, url::path resource = nullptr, json options = nullptr) -> json;
      
        auto download(string public_key, fs::path to, url::path file = nullptr)-> json;
      
        auto save(string public_key, string name, url::path file = nullptr)-> json;
        
        string token;

    private:
        auto info_impl (url::path resource, json options) -> json;
    };

}
