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

			//check if file is already opened
			file = FindFileInCollection(openedFiles, name);
			if (file != nullptr) {
				if (file->isWriteOnly) {
					fileLock.unlock();
					return nullptr;
				}

				fileLock.unlock();
				return file;
			}

			//check if file is root file
			file = FindFileInCollection(rootFiles, name);
			if (file == nullptr) {
				//check if file is virtual file
				file = FindFileInCollection(virtualFiles, name);
				if (file == nullptr) {
					fileLock.unlock();
					return nullptr;
				}
				
			}

			//check if root already opened
			if (file->root->fileStream.is_open()) {
				if (file->root->isReadOnly) {
					file->root->refCount++;
					file->isReadOnly = true;

					std::cout << "File opened in ReadOnly mode " << file->name << std::endl;

					openedFiles[file->name] = file;

					fileLock.unlock();
					return file;
				}
				else {
					//attempt to open root in readonly mode while root already opened in writeonly mode
					fileLock.unlock();
					return nullptr;
				}
			}

			//if file exists, open root filestream in read mode
			file->root->fileStream.open(file->root->name, std::ios::binary | std::ios::in);
			if (file->root->fileStream.is_open()) {
				file->root->refCount++;
				file->root->isReadOnly = true;

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

				//file is root
				fileName = name;
				rootName = name;
			}
			else {
				rootName = fullPath.substr(0, firstSlash);
			}

			size_t lastSlah = fullPath.find_last_of("/");

			if (lastSlah == firstSlash) {
				//virtual file direct child of root
				fileName = fullPath.substr(lastSlah + 1, fullPath.length());
			}
			else {
				//virtual file inside a directory
				path = fullPath.substr(firstSlash+1, lastSlah - firstSlash - 1);
				fileName = fullPath.substr(lastSlah + 1, fullPath.length());
			}



			//check if root opened
			root = FindFileInCollection(openedFiles, rootName.c_str());
			if (root == nullptr) {
				//check if root exists
				root = FindFileInCollection(rootFiles, rootName.c_str());
				if (root == nullptr) {
					//if there is no root, create one
					void* newRootName = malloc(sizeof(rootName));
					strcpy_s((char*)newRootName, sizeof(rootName), rootName.c_str());

					root = CreateRoot((char*)newRootName);
				}
			}

			//if file we are looking for is root
			if (rootName == fileName) {
				if (root->isReadOnly) {
					//if root already opened in ReadOnlyMode, return nullptr
					fileLock.unlock();
					return nullptr;
				}

				if (root->isWriteOnly) {
					//if root already opened in WriteOnlyMode, return root pointer
					fileLock.unlock();
					return root;
				}

				//if root exists and we are looking for root, and root closed, open root in write mode
				root->fileStream.open(root->name, std::ios::binary | std::ios::app);
				if (root->fileStream.is_open()) {
					root->refCount++;
					root->isWriteOnly = true;
					openedFiles[root->name] = root;

					std::cout << "File opened in WriteOnly mode " << root->name << std::endl;
				}
				
				fileLock.unlock();
				return root;
			}

			//check for directories
			if (!path.empty()) {
				//create directories if needed
				std::string currentPath = path;
				size_t slashPos = currentPath.find_first_of("/");
				while (slashPos != std::string::npos) {
					std::string directoryName = currentPath.substr(0, slashPos);
					File* dir = directories[directoryName.c_str()];

					currentPath.erase(0, slashPos + 1);

					slashPos = currentPath.find_first_of("/");

					if (dir == nullptr) {

						void* newDirName = malloc(sizeof(directoryName));
						void* newDirPath = malloc(sizeof(currentPath));

						strcpy_s((char*)newDirName, sizeof(directoryName), directoryName.c_str());
						strcpy_s((char*)newDirPath, sizeof(currentPath), currentPath.c_str());

						dir = new File{(char*)newDirName,root,false,true,false,false,0,0,0};
						directories[dir->name] = dir;		
					}
					else {
						continue;
					}
				}

				//check last directory
				File* dir = FindFileInCollection(directories, currentPath.c_str());

				if (dir == nullptr) {

					void* newDirPath = malloc(sizeof(currentPath));
					strcpy_s((char*)newDirPath, sizeof(currentPath), currentPath.c_str());

					dir = new File{ (char*)newDirPath,root,false,true,false,false,0,0,0 };
					directories[dir->name] = dir;
				}
			}


			//check if file exists
			file = FindFileInCollection(virtualFiles, name);
			if (file == nullptr) {
				//create new file if we dont have one
				file = new File{ name,rootFiles[root->name],false,false,false,false,0,0,0};
				virtualFiles[file->name] = file;
			}
			else{
				//check if file opened in ReadOnly mode
				if (file->isReadOnly) {
					fileLock.unlock();
					return nullptr;
				}
			}

			//check if root already opened on WriteOnly mode
			if (file->root->fileStream.is_open()) {
				if (file->root->isWriteOnly) {
					file->root->refCount++;
					file->isWriteOnly = true;
					openedFiles[file->name] = file;
					fileLock.unlock();
					return file;
				}
				else {
					//attempt to open root in readonly mode while root is already opened in writeonly mode
					fileLock.unlock();
					return nullptr;
				}
				
			}

			//open root in write mode and return file;
			file->root->fileStream.open(file->root->name, std::ios::binary | std::ios::app);
			if (file->root->fileStream.is_open()) {
				file->root->refCount++;
				file->root->isWriteOnly = true;
				file->isWriteOnly = true;
				openedFiles[file->name] = file;

				std::cout << "File opened in WriteOnly mode " << file->name << std::endl;

				fileLock.unlock();
				return file;
			}

			//if filestream failed to open
			fileLock.unlock();
			return nullptr;

		};

		size_t Read(File* f, char* buff, size_t len) 
		{
			fileLock.lock();

			//check if file stream opened
			if (!f->root->fileStream.is_open()) {
				fileLock.unlock();
				return 0;
			}

			//check if file opened in ReadOnlyMode
			if (!f->isReadOnly) {
				fileLock.unlock();
				return 0;
			}

			size_t bytesRead{};

			if (f->root->fileStream.is_open()) {

				//set pointer to virtual file start
				f->root->fileStream.seekg(f->start, std::ios::beg);

				if (f->size < len) {
					//read full file if buffer larger than file size
					if (f->root->fileStream.read(buff, f->size)) {
						bytesRead = f->root->fileStream.gcount();
					}
				}
				else {
					//read part of the file
					if (f->root->fileStream.read(buff, len)) {
						bytesRead = f->root->fileStream.gcount();
					}
				}
			}

			std::cout << "Bytes read: " << bytesRead << std::endl;

			//end line null
			buff[len] = '\0';

			fileLock.unlock();

			return bytesRead;
		};

		size_t Write(File* f, char* buff, size_t len)
		{
			fileLock.lock();

			//check if file stream opened
			if (!f->root->fileStream.is_open()) {
				fileLock.unlock();
				return 0;
			}

			//check if file opened in WriteOnly mode
			if (!f->isWriteOnly) {
				fileLock.unlock();
				return 0;
			}

			size_t bytesWrite{};

			if (f->root->fileStream.is_open()) {

				if (f->size == 0) {
					//if virtual file is empty, write to the end of root file
					f->root->fileStream.seekp(0, std::ios::end);
					f->start = f->root->fileStream.tellp();
					if (f->root->fileStream.write(buff, len)) {
						f->end = f->root->fileStream.tellp();
						bytesWrite = (size_t)(f->end - f->start);
						f->size = bytesWrite;
					}
				}
				else {
					//if file is not empty, set pointer to the end of virtual file
					f->root->fileStream.seekp(f->end, std::ios::beg);
					size_t start = f->root->fileStream.tellp();

					if (f->root->fileStream.write(buff, len)) {
						f->end = f->root->fileStream.tellp();
						bytesWrite = (size_t)(f->end - start);
						f->size += bytesWrite;
					}
				}
			}

			std::cout << "Bytes written: " << bytesWrite << std::endl;

			fileLock.unlock();

			return bytesWrite;
		};

		void Close(File* f)
		{
			fileLock.lock();

			File* file;

			file = openedFiles[f->name];

			if (file == nullptr) {
				fileLock.unlock();
				return;
			}
			else {
				if (file->root->fileStream.is_open()) {
					file->root->refCount--;
					file->isReadOnly = false;
					file->isWriteOnly = false;

					openedFiles.erase(f->name);

					if (file->root->refCount <= 0) {
						file->root->refCount = 0;
						file->root->fileStream.close();
					}

					std::cout << "File closed " << file->name << std::endl;

				}
				fileLock.unlock();
				return;
			}
		};

	private:
		std::unordered_map<std::string, File*> rootFiles;
		std::unordered_map<std::string, File*> directories;
		std::unordered_map <std::string, File*> virtualFiles;
		std::unordered_map <std::string, File*> openedFiles;
		std::mutex fileLock;

		File* CreateRoot(const char* name) {
			File* file = new File{ name,nullptr,true,false,false,false,0,0,0 };
			file->root = file;
			rootFiles[file->name] = file;

			file->fileStream.open(name, std::ios::binary | std::ios::app);
			if (file->fileStream.is_open()) {
				file->refCount++;

				file->isWriteOnly = true;
				openedFiles[file->name] = file;

				std::cout << "File opened in WriteOnly mode " << file->name << std::endl;
			}

			return file;
		}

		File* FindFileInCollection(std::unordered_map<std::string, File*> collection, std::string name) {
			auto iterator = collection.find(name);
			if (iterator == collection.end()) {
				return nullptr;
			}
			else {
				return iterator->second;
			}
		}

		~VFS()
		{

		};
	};
}