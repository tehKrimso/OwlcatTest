#include <cstdint>

namespace TestTask 
{
    struct File
    {
    };
    struct IVFS 
    {
    public:
        virtual File* Open(const char* name) = 0;
        virtual File* Create(const char* name) = 0;
        virtual size_t Read(File* f, char* buff, size_t len) = 0;
        virtual size_t Write(File* f, char* buff, size_t len) = 0;
        virtual void Close(File* f) = 0;
    };
}
