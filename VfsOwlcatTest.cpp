// VfsOwlcatTest.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "VFS.cpp"
#include <vector>

TestTask::VFS* vfs = new TestTask::VFS;


void Test();
void TestVirtualFile();

int main()
{
    Test();
    TestVirtualFile();

    return 0;
}


void Test()
{
    char firstLine[] = "abc";
    char secondLine[] = "123";
    char thirdLine[] = "cde";
    char fourthLine[] = "345";

    char* readBufferSmall = new char[3];
    char* readBufferEqual = new char[6];
    char* readBufferBig = new char[9];

    TestTask::File* fA = vfs->Create("a.bin");
    TestTask::File* fB = vfs->Create("b.bin");
    vfs->Write(fA, firstLine, 3);
    vfs->Write(fB, secondLine, 3);
    vfs->Write(fA, thirdLine, 3);
    vfs->Write(fB, fourthLine, 3);
    vfs->Close(fA);
    vfs->Close(fB);

    fA = vfs->Open("a.bin");
    fB = vfs->Open("b.bin");

    vfs->Read(fA, readBufferSmall, 3);
    vfs->Read(fA, readBufferEqual, 6);
    vfs->Read(fB, readBufferBig, 9);
    vfs->Close(fA);
    vfs->Close(fB);

    std::cout << readBufferSmall << std::endl;
    std::cout << readBufferEqual << std::endl;
    std::cout << readBufferBig << std::endl;
}

void TestVirtualFile() {
    char firstLine[] = "zxc";
    char secondLine[] = "098";
    TestTask::File* fA = vfs->Create("a.bin/virtualA");
    TestTask::File* fB = vfs->Create("b.bin/virtualB");
    TestTask::File* fC = vfs->Create("a.bin/folder/virtualC");
    vfs->Write(fA, firstLine, 3);
    vfs->Write(fB, secondLine, 3);
    vfs->Write(fC, secondLine, 3);
    vfs->Close(fA);
    vfs->Close(fB);
    vfs->Close(fC);

    char* readBufferPart = new char[3];
    char* readBufferFull = new char[12];
    fC = vfs->Open("a.bin/folder/virtualC");
    fA = vfs->Open("a.bin");
    vfs->Read(fC, readBufferPart, 3);
    vfs->Read(fA, readBufferFull, 12);
    vfs->Close(fC);
    vfs->Close(fA);

    std::cout << readBufferPart << std::endl;
    std::cout << readBufferFull << std::endl;
}