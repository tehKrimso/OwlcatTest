// VfsOwlcatTest.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "VFS.cpp"
#include <vector>


int main()
{
    TestTask::VFS myVfs;

    //create file
    TestTask::File* test = myVfs.Create("testRoot/testDir/testDir2/testFile");

    //open file in WriteOnly mode
    TestTask::File* openedTestWrite = myVfs.Create("testRoot/testDir/testDir2/testFile");

    //create write buffer
    std::vector<char> writeBuffer(8 * 1000 * 1000, 'a');
    
    //write to the file
    myVfs.Write(openedTestWrite, &writeBuffer[0], writeBuffer.size());
    
    //close file
    myVfs.Close(openedTestWrite);

    //open file for ReadOnlyMode
    TestTask::File* openedTestRead = myVfs.Open("testRoot/testDir/testDir2/testFile");

    //create read buffer
    char* readBuff = new char[writeBuffer.size() / 2];

    //read from file
    myVfs.Read(openedTestRead, &readBuff[0], writeBuffer.size()/2);

    //close file
    myVfs.Close(openedTestRead);

    return 0;
}
