#include "Execute.h"
#include <unistd.h>


static vector<unsigned> Inst = 
{
    0x400190, 0x400194, 0x400198, 0x40019C, 0x4001A0, 0x4001A4, 0x4001A8, 0x4001AC, 0x4001B0, 0x4001B4, 
    0x4001B8, 0x4001BC, 0x4001C0, 0x4001C4, 0x4202B0, 0x4202B4, 0x4202B8, 0x4202BC, 0x4202C0, 0x4202C4, 
    0x4202C8, 0x4202CC, 0x4202D0, 0x4202D4, 0x4202D8, 0x4202DC, 0x42AC30, 0x42AC34, 0x42AC38, 0x42AC3C, 
    0x42AC40, 0x42AC44, 0x42AC48, 0x42AC5C, 0x42AC60, 0x42AC64, 0x42AC68, 0x42AC6C, 0x42AC70, 0x42AC74,
    0x42AC78, 0x42AC7C, 0x42AC80, 0x42AC84, 0x42AC88, 0x42AC8C, 0x42AC90, 0x42AC94, 0x42AC98, 0x42AC9C,
    0x42ACA0
};
    
int main (int argc, char* argv[])
{
    string FileName = "";
    unsigned PType = P_CORRELATOR;
    unsigned Size  = 1024;

    char ch;
    while((ch = getopt(argc, argv, "f:b:lgch")) != -1)
    {
        switch(ch)
        {
            case 'f':
                FileName = optarg;
                break;
            case 'b':
                Size = atoi(optarg);
                break;
            case 'l':
                PType = P_LOCAL;
                break;
            case 'g':
                PType = P_GLOBAL;
                break;
            case 'c':
                PType = P_CORRELATOR;
                break;
            case 'h':
                printf("*************** help information ***************\r\n");
                printf("-f file_path \r\n");
				printf("-l: local prediction\r\n");
				printf("-g: global prediction\r\n");
				printf("-c: correlator prediction\r\n");
                printf("-h ");
                return 0;
            default:
			    printf("-h for help");
                return 0;
        }
    }

    if (Size < 1024)
    {
        Size = 1024;
    }
    
    Execute Exe(Size, PType);
    if (FileName != "")
    {
        Exe.Run (FileName);
    }
    else
    {
        Exe.Run (Inst);
    }

    Exe.Result ();

    return 0;
}

