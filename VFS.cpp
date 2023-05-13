#include "ivfs.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <mutex>

namespace TestTask 
{
	class VFS : public IVFS
	{
	public:

		File* Open(const char* name) {

			fileLock.lock();

			File* file;

			file = GetOpenedFile(name);

			if (file != nullptr) {
				if (file->isWriteOnly) {
					fileLock.unlock();
					return nullptr;
				}

				fileLock.unlock();
				return file;
			}

			file = GetFile(name);

			if (file == nullptr) {
				fileLock.unlock();
				return nullptr;
			}


			file->fileStream.open(file->name, std::ios::in);

			if (file->fileStream.is_open()) {

				file->isReadOnly = true;

				std::cout << "File opened in ReadOnly mode " << file->name << std::endl;

				openedFiles[file->name] = file;
			}

			fileLock.unlock();
			return file;
		};

		File* Create(const char* name) {

			fileLock.lock();

			File* file;

			file = GetOpenedFile(name);

			if (file != nullptr) {
				if (file->isReadOnly) {
					fileLock.unlock();
					return nullptr;
				}

				fileLock.unlock();
				return file;
			}

			file = GetFile(name);

			if (file != nullptr) {

				file->isWriteOnly = true;
				file->fileStream.open(file->name, std::ios::app);

				openedFiles[file->name] = file;
				std::cout << "File opened in WriteOnly mode " << file->name << std::endl;

				fileLock.unlock();
				return file;
			}

			file = new File{ name,false,true,false,0,0,0,0 };;
			allFiles[name] = file;

			std::cout << "File created " << file->name << std::endl;

			fileLock.unlock();

			return file;
		};

		size_t Read(File* f, char* buff, size_t len) 
		{
			if (!f->fileStream.is_open()) {
				return 0;
			}

			if (!f->isReadOnly) {
				return 0;
			}

			fileLock.lock();

			size_t bytesRead{};

			if (f->fileStream.is_open()) {
				f->fileStream.read(buff, len);
				bytesRead = f->fileStream.gcount();
			}

			std::cout << "Bytes read: " << bytesRead << std::endl;

			fileLock.unlock();

			return bytesRead;
		};

		size_t Write(File* f, char* buff, size_t len)
		{
			if (!f->fileStream.is_open()) {
				return 0;
			}

			if (!f->isWriteOnly) {
				return 0;
			}

			fileLock.lock();

			size_t bytesWrite{};

			if (f->fileStream.is_open()) {

				size_t start = f->fileStream.tellp();
				if (f->fileStream.write(buff, len)) {
					bytesWrite = (size_t)(f->fileStream.tellp()) - start;
				}
			
			}

			std::cout << "Bytes written" << bytesWrite << std::endl;

			fileLock.unlock();

			return bytesWrite;
		};

		void Close(File* f)
		{
			fileLock.lock();

			File* file;

			file = GetOpenedFile(f->name);

			if (file == nullptr) {
				fileLock.unlock();
				return;
			}
			else {
				if (file->fileStream.is_open()) {
					file->fileStream.close();
					file->isReadOnly = false;
					file->isWriteOnly = false;

					openedFiles.erase(f->name);
					std::cout << "File closed " << file->name << std::endl;

				}
				fileLock.unlock();
				return;
			}
		};


	private:
		std::unordered_map <const char*, File*> allFiles;
		std::unordered_map <const char*, File*> openedFiles;
		std::mutex fileLock;

		File* GetFile(const char* name) 
		{
			auto iterator = allFiles.find(name);
			if (iterator == allFiles.end()) {
				return nullptr;
			}
			else {
				return iterator->second;
			}
		};

		File* GetOpenedFile(const char* name) 
		{
			auto iterator = openedFiles.find(name);
			if (iterator == openedFiles.end()) {
				return nullptr;
			}
			else {
				return iterator->second;
			}
		}
	};
}