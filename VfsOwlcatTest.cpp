// VfsOwlcatTest.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "VFS.cpp"
#include <vector>

TestTask::VFS* vfs = new TestTask::VFS;


void Test();

int main()
{
    //::VFS vfs;

    Test();
    return 0;
    //create file
    TestTask::File* test = vfs->Create("testRoot/testDir/testDir2/testFile");

    //open file in WriteOnly mode
    TestTask::File* openedTestWrite = vfs->Create("testRoot/testDir/testDir2/testFile");

    //create write buffer
    std::vector<char> writeBuffer(8 * 1000 * 1000, 'a');
    
    //write to the file
    vfs->Write(openedTestWrite, &writeBuffer[0], writeBuffer.size());
    
    //close file
    vfs->Close(openedTestWrite);

    TestTask::File* test2 = vfs->Create("testRoot/testDir/testDir2/testFile2");
    TestTask::File* openedtest2 = vfs->Create("testRoot/testDir/testDir2/testFile2");

    std::vector<char> writeBuffer2(4 * 1000 * 1000, 'a');

    vfs->Write(openedtest2, &writeBuffer2[0], writeBuffer2.size());
    vfs->Close(openedtest2);

    //open file for ReadOnlyMode
    TestTask::File* openedTestRead = vfs->Open("testRoot/testDir/testDir2/testFile");

    //create read buffer
    char* readBuff = new char[writeBuffer.size() / 2];

    //read from file
    vfs->Read(openedTestRead, &readBuff[0], writeBuffer.size()/2);

    //close file
    vfs->Close(openedTestRead);

    return 0;
}


void Test()
{
    char firstLine[] = "abc";
    char secondLine[] = "123";
    char thirdLine[] = "cde";
    char fourthLine[] = "345";

    //FILE* fA = vfs->Create("a.bin");
    //FILE* fB = vfs->Create("b.bin");
    TestTask::File* fA = vfs->Create("a.bin");
    TestTask::File* fB = vfs->Create("b.bin");
    vfs->Write(fA, firstLine, 3);
    vfs->Write(fB, secondLine, 3);
    vfs->Write(fA, thirdLine, 3);
    vfs->Write(fB, fourthLine, 3);
    vfs->Close(fA);
    vfs->Close(fB);
}