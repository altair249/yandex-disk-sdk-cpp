#include <catch.hpp>
#include <yadisk/client.hpp>
using ydclient = yadisk::Client;

#include <url/path.hpp>

static ydclient invalid_client{ "JS1w4zmPUdrsJNR1FATxEM" };
static ydclient client{ "AQAAAAATPnx3AAQXOJS1w4zmPUdrsJNR1FATxEM" };

TEST_CASE ("info with invalid token and valid resource", "[client][info]")
{
    url::path resource{ "/file.dat" };
    auto meta = invalid_client.info (resource);
    REQUIRE (not meta.empty());
    REQUIRE (meta["error"].get<std::string>() == "UnauthorizedError");
}

TEST_CASE ("info with valid token and valid file", "[client][info]")
{
    url::path resource{ "/file.dat" };
    auto meta = client.info (resource);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["created"].get<std::string>() == "2017-04-07T10:41:09+00:00");
    REQUIRE (meta["media_type"].get<std::string>() == "data");
    REQUIRE (meta["mime_type"].get<std::string>() == "application/octet-stream");
    REQUIRE (meta["name"].get<std::string>() == "file.dat");
    REQUIRE (meta["path"].get<std::string>() == "disk:/file.dat");
    REQUIRE (meta["type"].get<std::string>() == "file");
}

TEST_CASE ("info with valid token and invalid file", "[client][info]")
{
    url::path resource{ "/invalid_file.dat" };
    auto meta = client.info (resource);
    REQUIRE (not meta.empty());
    REQUIRE (meta["error"].get<std::string>() == "DiskNotFoundError");
}

TEST_CASE ("info with valid token and valid directory", "[client][info]")
{
    url::path resource{ "/empty_directory" };
    auto meta = client.info (resource);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["created"].get<std::string>() == "2016-09-26T10:14:48+00:00");
    REQUIRE (meta["name"].get<std::string>() == "empty_directory");
    REQUIRE (meta["path"].get<std::string>() == "disk:/empty_directory");
    REQUIRE (meta["type"].get<std::string>() == "dir");
    REQUIRE (meta["_embedded"]["items"].is_array());
    REQUIRE (meta["_embedded"]["items"].empty());
    REQUIRE (meta["_embedded"]["limit"].get<int>() == 20);
    REQUIRE (meta["_embedded"]["offset"].get<int>() == 0);
    REQUIRE (meta["_embedded"]["total"].get<int>() == 0);
    REQUIRE (meta["_embedded"]["sort"].get<std::string>() == "");
}

TEST_CASE ("info with option deleted", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["deleted"] = true;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["path"].get<std::string>() == "trash:/");
}

TEST_CASE ("info with invalid option deleted", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["deleted"] = 1;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["path"].get<std::string>() == "disk:/");
}

TEST_CASE ("info with option sort", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["sort"] = "-size";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["sort"].get<std::string>() == "-size");
}

TEST_CASE ("info with invalid option sort", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["sort"] = "35";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["sort"].get<std::string>() == "");
}

TEST_CASE ("info with invalid option sort, not a string", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["sort"] = 35;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["sort"].get<std::string>() == "");
}

TEST_CASE ("info with option limit", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["limit"] = 5;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["limit"].get<int>() == 5);
}

TEST_CASE ("info with invalid option limit", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["limit"] = -11;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["limit"].get<int>() == 20);
}

TEST_CASE ("info with invalid option limit, not a number", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["limit"] = "five";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["limit"].get<int>() == 20);
}

TEST_CASE ("info with option offset", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["offset"] = 5;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["offset"].get<int>() == 5);
}

TEST_CASE ("info with invalid option offset", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["offset"] = -11;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["offset"].get<int>() == 0);
}

TEST_CASE ("info with invalid option offset, not a number", "[client][info]")
{
    url::path resource{ "/" };
    json options;
    options["offset"] = "five";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta["_embedded"]["offset"].get<int>() == 0);
}

static std::string get_preview_size (std::string url)
{
    std::string place = "&size=";
    std::string res;
    for (size_t i = url.find (place) + place.size(); i < url.size() && url[i] != '&'; ++i)
    {
        res.push_back (url[i]);
    }
    return res;
}

TEST_CASE ("info with option preview_size", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_size"] = "L";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (get_preview_size (meta["preview"].get<std::string>()) == "L");
}

TEST_CASE ("info with invalid string option preview_size", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_size"] = "haha";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta["error"].get<std::string>() == "FieldValidationError");
}

TEST_CASE ("info with number option preview_size", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_size"] = 120;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (get_preview_size (meta["preview"].get<std::string>()) == "120");
}

TEST_CASE ("info with invalid number option preview_size", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_size"] = -120;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta["error"].get<std::string>() == "FieldValidationError");
}

TEST_CASE ("info with option fields", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    std::vector<std::string> fields {"name", "created"};
    options["fields"] = fields;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") == meta.end());
    REQUIRE (meta.find ("name") != meta.end());
    REQUIRE (meta.find ("created") != meta.end());
}

TEST_CASE ("info with some invalid options fields", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    std::vector<std::string> fields {"name", "42"};
    options["fields"] = fields;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") == meta.end());
    REQUIRE (meta.find ("name") != meta.end());
	REQUIRE (meta.find ("created") == meta.end());
}

TEST_CASE ("info with one option fields", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["fields"] = "name";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") == meta.end());
    REQUIRE (meta.find ("name") != meta.end());
    REQUIRE (meta.find ("created") == meta.end());
}

TEST_CASE ("info with invalid string option fields", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["fields"] = "42";
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    // full answer is expected, like without fields
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (meta.find ("name") != meta.end());
    REQUIRE (meta.find ("created") != meta.end());
}

TEST_CASE ("info with invalid not a string option fields", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["fields"] = 42;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    // full answer is expected, like without fields
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (meta.find ("name") != meta.end());
    REQUIRE (meta.find ("created") != meta.end());
}

static std::string get_preview_crop (std::string url)
{
    std::string place = "&crop=";
    std::string res;
    for (size_t i = url.find (place) + place.size(); i < url.size() && url[i] != '&'; ++i)
    {
        res.push_back (url[i]);
    }
    return res;
}

TEST_CASE ("info with false option preview_crop", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_crop"] = false;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (get_preview_crop (meta["preview"].get<std::string>()) == "0");
}

TEST_CASE ("info with true option preview_crop", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_crop"] = true;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (get_preview_crop (meta["preview"].get<std::string>()) == "1");
}

TEST_CASE ("info with invalid option preview_crop", "[client][info]")
{
    url::path resource{ "/image.jpg" };
    json options;
    options["preview_crop"] = 21;
    auto meta = client.info (resource, options);
    REQUIRE (not meta.empty());
    REQUIRE (meta.find ("error") == meta.end());
    REQUIRE (meta.find ("preview") != meta.end());
    REQUIRE (get_preview_crop (meta["preview"].get<std::string>()) == "0");
}
