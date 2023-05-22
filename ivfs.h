#ifndef IVFS_H
#define IVFS_H

#include <cstdint>
#include <fstream>


namespace TestTask 
{
    struct File
    {
        public:
            const char* name;
            File* root;
            bool isRoot;
            bool isDirectory;
            bool isWriteOnly;
            bool isReadOnly;
            uint64_t refCount;
            uint64_t start;
            uint64_t end;
            uint64_t size;
            std::fstream fileStream;
           
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

#endif // IVFS_H