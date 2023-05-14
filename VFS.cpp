#include "ivfs.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <mutex>
#include <vector>

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


			file->root->fileStream.open(file->root->name, std::ios::in);

			if (file->root->fileStream.is_open()) {

				file->isReadOnly = true;

				std::cout << "File opened in ReadOnly mode " << file->name << std::endl;

				openedFiles[file->name] = file;
			}

			fileLock.unlock();
			return file;
		};

		File* Create(const char* name) {

			fileLock.lock();

			File* root;
			File* file;


			std::string fullPath = name;

			std::string rootName{};
			std::string path{};
			std::string fileName{};

			size_t firstSlash = fullPath.find_first_of("/");
			

			if (firstSlash == std::string::npos) {

				//то путь пустой и запрашиваемый файл это рут
				fileName = name;
				rootName = name;
			}
			else {
				rootName = fullPath.substr(0, firstSlash);
			}

			size_t lastSlah = fullPath.find_last_of("/");

			if (lastSlah == firstSlash) {
				//если файл сразу под рутом
				fileName = fullPath.substr(lastSlah + 1, fullPath.length());
			}
			else {
				//если файл находится дальше внутри директорий
				path = fullPath.substr(firstSlash+1, lastSlah - firstSlash - 1);
				fileName = fullPath.substr(lastSlah + 1, fullPath.length());
			}



			//проверка не открыт ли уже запрашиваемый рут
			root = openedFiles[rootName.c_str()];
			if (root == nullptr) {
				//проверка существует ли запрашиваемый рут
				root = rootFiles[rootName.c_str()];
				if (root == nullptr) {
					//создание нового рута, если его не было
					//const char* newRootName = new char[rootName.length()];

					void* newRootName = malloc(sizeof(rootName));
					strcpy_s((char*)newRootName, sizeof(rootName), rootName.c_str());
					//std::copy(rootName.begin(), rootName.end(), newRootName);
					//strcpy_s(newRootName, sizeof(rootName), rootName.c_str());

					root = CreateRoot((char*)newRootName);
				}
				else {
					//если рут уже есть, открываем его в режиме WriteOnly
					root->fileStream.open(root->name, std::ios::app);
					if (root->fileStream.is_open()) {
						root->isWriteOnly = true;
						openedFiles[root->name] = root;
					}
				}
			}

			//fileLock.unlock();
			//return root;

			//проверка, не является ли запрашиваемый файл рутом
			if (rootName == fileName) {
				if (root->isReadOnly) {
					//если рут уже открыт в режиме ReadOnly, отдаем nullptr
					fileLock.unlock();
					return nullptr;
				}
				//иначе просто отдаем рут
				fileLock.unlock();
				return root;
			}

			//если запрашиваемый файл не рут, проверяем есть ли на пути директории
			if (!path.empty()) {
				//проверяем наличие директорий и создаем, если нужно
				std::string currentPath = path;
				size_t slashPos = currentPath.find_first_of("/");
				while (slashPos != std::string::npos) {
					std::string directoryName = currentPath.substr(0, slashPos);
					File* dir = directories[directoryName.c_str()];

					currentPath.erase(0, slashPos + 1);

					slashPos = currentPath.find_first_of("/");

					if (dir == nullptr) {

						const char newDirName{ *directoryName.c_str() };
						const char newDirPath{ *currentPath.c_str() };

						dir = new File{&newDirName,&newDirPath,root,false,true,false,false,0,0,0};
						directories[dir->name] = dir;		
					}
					else {
						continue;
					}
				}

				//когда остается последний кусочек пути, проверяем еще раз
				File* dir = directories[currentPath.c_str()];

				if (dir == nullptr) {
					const char newDirPath{ *currentPath.c_str() };

					dir = new File{ &newDirPath,&newDirPath,root,false,true,false,false,0,0,0 };
					directories[dir->name] = dir;
				}
			}


			//в конце проверяем, существует ли нужный файл, создаем, если требуется
			file = allFiles[name];
			if (file == nullptr) {
				//если файла нет, создаем новый файл под рутом и отдаем указатель на него
				file = new File{ name,path.c_str(),rootFiles[root->name],false,false,false,false,0,0,0};
				allFiles[file->name] = file;
				fileLock.unlock();
				return file;
			}
			else{
				//если файл уже есть, проверяем, не открыт ли он в рпежиме ReadOnly
				if (file->isReadOnly) {
					//если рут уже открыт в режиме ReadOnly, отдаем nullptr
					fileLock.unlock();
					return nullptr;
				}
				//иначе открываем поток рута в режиме WriteOnly и отдаем файл
				file->root->fileStream.open(file->root->name, std::ios::app);
				if (file->root->fileStream.is_open()) {
					file->isWriteOnly = true;
					openedFiles[file->name] = file;
					fileLock.unlock();
					return file;
				}

				//если поток не открылся возвращаем nullptr
				fileLock.unlock();
				return nullptr;
			}

		};

		size_t Read(File* f, char* buff, size_t len) 
		{
			fileLock.lock();

			if (!f->root->fileStream.is_open()) {
				fileLock.unlock();
				return 0;
			}

			if (!f->isReadOnly) {
				fileLock.unlock();
				return 0;
			}

			size_t bytesRead{};

			if (f->root->fileStream.is_open()) {

				f->root->fileStream.seekg(f->start, std::ios::beg);

				if (f->size < len) {
					if (f->root->fileStream.read(buff, f->size)) {
						bytesRead = f->root->fileStream.gcount();
					}
					
					

					//bytesRead = f->root->fileStream.readsome(buff, f->size);
				}
				else {
					if (f->root->fileStream.read(buff, len)) {
						bytesRead = f->root->fileStream.gcount();
					}

					//bytesRead = f->root->fileStream.readsome(buff, len);
				}
			}

			std::cout << "Bytes read: " << bytesRead << std::endl;

			fileLock.unlock();

			return bytesRead;
		};

		size_t Write(File* f, char* buff, size_t len)
		{
			fileLock.lock();

			if (!f->root->fileStream.is_open()) {
				fileLock.unlock();
				return 0;
			}

			if (!f->isWriteOnly) {
				fileLock.unlock();
				return 0;
			}

			

			size_t bytesWrite{};

			if (f->root->fileStream.is_open()) {

				if (f->size == 0) {
					f->root->fileStream.seekp(0, std::ios::end);
					f->start = f->root->fileStream.tellp();
					if (f->root->fileStream.write(buff, len)) {
						f->end = f->root->fileStream.tellp();
						bytesWrite = (size_t)(f->end - f->start);
						f->size = bytesWrite;
					}
				}
				else {
					f->root->fileStream.seekp(f->end, std::ios::beg);
					size_t start = f->root->fileStream.tellp();

					if (f->root->fileStream.write(buff, len)) {
						f->end = f->root->fileStream.tellp();
						bytesWrite = (size_t)(f->end - start);
						f->size += bytesWrite;
					}
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
				if (file->root->fileStream.is_open()) {
					file->root->fileStream.close();
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
		std::unordered_map<const char*, File*> rootFiles;
		std::unordered_map<const char*, File*> directories;
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

		File* GetDirectory(const char* name)
		{
			auto iterator = directories.find(name);
			if (iterator == directories.end()) {
				return nullptr;
			}
			else {
				return iterator->second;
			}
		}

		File* GetRootFile(const char* name) {
			auto iterator = rootFiles.find(name);
			if (iterator == rootFiles.end()) {
				return nullptr;
			}
			else {
				return iterator->second;
			}
		}

		File* CreateRoot(const char* name) {
			File* file = new File{ name,name,nullptr,true,false,false,false,0,0,0 };
			file->root = file;
			rootFiles[name] = file;

			file->fileStream.open(name, std::ios::app);
			file->fileStream.close();

			return file;
		}
	};
}