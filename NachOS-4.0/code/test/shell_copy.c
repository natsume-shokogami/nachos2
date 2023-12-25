#include "syscall.h"

int strlen(char* str){
    int i;
    char ch;
    i = 0;
    ch = str[0];
    while (ch != '\0'){
        i = i + 1;
        ch = str[i];
    }
    return i;
}

int strcmp(char* str1, char* str2){
    int str1_len, str2_len, i;
    str1_len = strlen(str1);
    str2_len = strlen(str2);

    i = 0;
    while (i < str1_len && i < str2_len){
        if (str1[i] < str2[i]){
            return -1;
        } else if (str1[i] > str2[i]){
            return 1;
        }
        i = i + 1;
    }
    if (str1_len < str2_len){
        return -1;
    }
    else if (str2_len > str1_len){
        return 1;
    }
    return 0;
}

int main()
{
    SpaceId newProc1;
    SpaceId newProc2;
    SpaceId newProc3;
    SpaceId newProc4;
    OpenFileId input = 0;
    OpenFileId output = 1;
    char ch, buffer[60];
    char *prompt = "[shell] $ ";
    int i;


    while( 1 )
    {
        Write(prompt, 10, output);

        
        Read(buffer, 59, input);

        if (strcmp(buffer, "cat") == 0){
            Write("Exec cat.\n", 11, output);
            newProc1 = Exec("cat");
            Join(newProc1);
        } else if (strcmp(buffer, "copy") == 0){
            Write("Exec copy.\n", 12, output);
            newProc2 = Exec("copy");
            Join(newProc2);
        } else if (strcmp(buffer, "createfile") == 0){
            newProc2 = Exec("createfile");
            Join(newProc2);
        } else if (strcmp(buffer, "delete") == 0){
            newProc2 = Exec("delete");
            Join(newProc2);
        } else if (strcmp(buffer, "exit") == 0){
            Exit(0);
        } else {
            Write("Error: Command not supported.\n", 32, output);
        }
        
    }
}

