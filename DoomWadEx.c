
/******************************************************************************************
* Extracts all of the files from a DOOM WAD file. 
* Not very usefull, it is very basic and there are better tools for this. 
* This project was just to show some basic c binary IO.
*
* Wad structure from http://doom.wikia.com/wiki/WAD
*
*Author James Kozlowski https://github.com/jameskozlowski
******************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 


#ifdef _WIN32
    #include <direct.h>
#endif

#ifdef linux
    #include <sys/stat.h>
#endif

/************************************************************************************************************
*   A WAD file always starts with a 12-byte header. It contains three values:
*    
*   Bytes 	    Content
*   0x00-0x03 	string "PWAD" or "IWAD", defines whether the WAD is a PWAD or an IWAD
*   0x04-0x07 	An integer specifying the number of entries in the directory
*   0x08-0x0b 	An integer holding a pointer to the location of the directory 
**************************************************************************************************************/
struct WADHeader
{
    char    fileType[5];
    int     directoryPointer;
    int     directorySize;
};


/************************************************************************************************************
*   The directory associates names of lumps with the data that belong to them. It consists 
*   of a number of entries, each with a length of 16 bytes. The length of the directory is 
*   determined by the number given in the WAD header. The structure of each entry is as follows:
*
*   Bytes 	    Content
*   0x00-0x03 	An integer holding a pointer to the start of the lump's data in the file
*   0x04-0x07 	An integer representing the size of the lump in bytes
*   0x08-0x0f 	ASCII string defining the lump's name. Only the characters A-Z (uppercase), 0-9 and []-_ 
*               should be used in Lump names (an exception has to be made for some of the ArchVile 
*               sprites, which use "\"). The string must be null-terminated if shorter than 8 bytes. 
**************************************************************************************************************/
struct WADDirectory
{
    char    lumpName[9];
    int     lumpPointer;
    int     lumpSize;
};

int main(int argc, char *argv[])
{

    //Make sure the user ran with the doom WAD file name
    if (argc != 2)
    {
        printf("Please specify a wad file to open");
        return 1;
    }

    //try to open the WAD file
    FILE *wadFile = fopen(argv[1],"r");

    //we where unable to open the wad file
    if(!wadFile)
    {
        printf("Unable to open file");
        return 1;
    }

    //create a header struct
    struct WADHeader wadHeader;

    //read in the header information
    fread(wadHeader.fileType, 4, 1, wadFile);
    fread(&wadHeader.directorySize, 4, 1, wadFile);
    fread(&wadHeader.directoryPointer, 4, 1, wadFile);

    //create an array of wad lump
    struct WADDirectory wadFiles[wadHeader.directorySize];

    //seek to the beginning of the directory
    fseek(wadFile, wadHeader.directoryPointer, SEEK_SET);

    //read in the directory
    for (int i = 0; i < wadHeader.directorySize; i++)
    {
        fread(&wadFiles[i].lumpPointer, 4, 1, wadFile);
        fread(&wadFiles[i].lumpSize, 4, 1, wadFile);
        fread(wadFiles[i].lumpName, 8, 1, wadFile);
    }

    //create the dir...not tested in windows
    #ifdef _WIN32
        int dirOK = _mkdir("./ExportedLumps");
    #endif

    #ifdef linux
        int dirOK = mkdir("./ExportedLumps", 0777);
    #endif

    //make sure the directory was created ok
    if (!dirOK)
    {
        printf("Error creating output directory");
        return 1;
    }

    //extract the files
    for (int i = 0; i < wadHeader.directorySize; i++)
    {
        //extract real files only, not markers
        if (wadFiles[i].lumpSize > 0)
        {
            //create a buffer for the lump size
            char *buffer = malloc(wadFiles[i].lumpSize);

            //seek to the lump
            fseek(wadFile, wadFiles[i].lumpPointer, SEEK_SET);

            //read the lump into the buffer
            fread(buffer, wadFiles[i].lumpSize, 1, wadFile);

            //create a var to hold the file name and combine with the directory
            char destFile[30];
            strcpy(destFile, "./ExportedLumps/");
            strcat(destFile, wadFiles[i].lumpName);

            //open a new file to save
            FILE *outFile = fopen(destFile,"w");

            //if the file was created ok then write to it, other wise skip over this file
            if (outFile)
            {
                fwrite(buffer, wadFiles[i].lumpSize, 1, outFile);
                fclose(outFile);
                
                //output the file saved
                printf("%s\n", destFile);
            }

            //be sure to free the buffer
            free(buffer);
        }
    }

    //close the WAD file
    fclose(wadFile);

}