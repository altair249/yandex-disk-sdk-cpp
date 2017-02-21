#ifndef URL_PATH_HPP
#define URL_PATH_HPP

#include <string>
#include <ostream>

namespace url 
{
    class path
    {
        static std::string separator;
        
        std::string str;
    
    public:
        
        path() = delete;

        path(const path&) = default;

        path(path&&) = default;

        path(const char * str);

        path(std::string str);

        path& operator+=(const path&);

        path& operator/=(const path&);

        path& operator=(const path&) = default;

        path& operator=(path&&) = default;

        bool operator==(const path&);

        bool operator!=(const path&);

        void swap(path& rhs);

        auto string() -> std::string;

        friend auto operator/(const path& lhs, const path& rhs) -> path; 

        friend auto operator+(const path& lhs, const path& rhs) -> path;

        friend auto equivalent(const path& p1, const path& p2) -> bool;

        friend auto operator<<(std::ostream & out, const path& p) -> std::ostream&;
    };
}

void swap(url::path& p1, url::path& p2);

#endif
