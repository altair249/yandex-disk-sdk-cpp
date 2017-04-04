#include <yadisk/client.hpp>
using ydclient = yadisk::Client;
#include <url/path.hpp>

// Вася хочет узнать когда был создан файл, который он загрузил на свой yandex
// disk. К сожалению Вася забыл пароль от аккаунта, чтобы посмотреть через web
// страницу. К счастью Вася сохранил токен от аккаунта, немного знает С++ и
// слышал о API yadiskhttps://github.com/designerror/yandex-disk-sdk-cpp/
int main()
{
	ydclient client("token");
	url::path resource{ "/file.dat" };
	// Васе нужна только дата создания
	json options;
	options["fields"] = "created";
	bool notFound = true;
	// Вася не помнит удалил ли он этот файл или нет, поэтому постмотрит в обоих
	// местах
	// сначала на диске
	auto meta = client.info (resource, options);
	if (!meta.empty() && meta.find ("error") == meta.end()) {
		std::cout << "The file created: " << meta["created"].get<std::string>()
		          << std::endl;
		notFound = false;
	}
	// потом в корзине
	options["deleted"] = true;
	meta = client.info (resource, options);
	if (!meta.empty() && meta.find ("error") == meta.end()) {
		std::cout << "The file in the trash created: "
		          << meta["created"].get<std::string>() << std::endl;
		notFound = false;
	}
	// если файла нет ни в корзине, ни на диске, то значит Вася что-то напутал
	if (notFound) {
		std::cout << "File didn't exist" << std::endl;
	}
}
