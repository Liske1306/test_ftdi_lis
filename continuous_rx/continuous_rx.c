#include <windows.h>        // For Windows-specific APIs
#include <stdio.h>          // For printf and debugging
#include <conio.h>          // For getch()
#include "../FTD3xx.h"         // FTDI D3XX API header (include path required)

#define BUFFER_SIZE 1000000    // Define the buffer size (adjust as needed)
static char* str_ftStatus[] = {
    "FT_OK",
    "FT_INVALID_HANDLE",
    "FT_DEVICE_NOT_FOUND",
    "FT_DEVICE_NOT_OPENED",
    "FT_IO_ERROR",
    "FT_INSUFFICIENT_RESOURCES",
    "FT_INVALID_PARAMETER",
    "FT_INVALID_BAUD_RATE",
    "FT_DEVICE_NOT_OPENED_FOR_ERASE",
    "FT_DEVICE_NOT_OPENED_FOR_WRITE",
    "FT_FAILED_TO_WRITE_DEVICE",
    "FT_EEPROM_READ_FAILED",
    "FT_EEPROM_WRITE_FAILED",
    "FT_EEPROM_ERASE_FAILED",
    "FT_EEPROM_NOT_PRESENT",
    "FT_EEPROM_NOT_PROGRAMMED",
    "FT_INVALID_ARGS",
    "FT_NOT_SUPPORTED",
    "FT_NO_MORE_ITEMS",
    "FT_TIMEOUT",
    "FT_OPERATION_ABORTED",
    "FT_RESERVED_PIPE",
    "FT_INVALID_CONTROL_REQUEST_DIRECTION",
    "FT_INVALID_CONTROL_REQUEST_TYPE",
    "FT_IO_PENDING",
    "FT_IO_INCOMPLETE",
    "FT_HANDLE_EOF",
    "FT_BUSY",
    "FT_NO_SYSTEM_RESOURCES",
    "FT_DEVICE_LIST_NOT_READY",
    "FT_DEVICE_NOT_CONNECTED",
	"FT_INCORRECT_DEVICE_PATH",

    "FT_OTHER_ERROR"
};


BOOL Receiver_test(void) {
    FT_STATUS ftStatus = FT_OK;
    FT_HANDLE ftHandle;
    GUID DeviceGUID[2] = { 0 };

    OVERLAPPED vOverlapped = { 0 };
    UINT32 acWriteBuf[BUFFER_SIZE / 4] = { 0xFF };
    ULONG checksum = 0;
    ULONG ulBytesWritten = 0;
    UCHAR ucPipeId = 0x02;
    ULONG ulTimeoutInMs = 10000;

    ftStatus = FT_Create("ByttEHH8yTnWTeO", FT_OPEN_BY_SERIAL_NUMBER, &ftHandle); 
    printf("Device Creation: %s\n",ftStatus ? "ERROR" : "OK");
    // Prepare data to sent
    for (ULONG i = 0; i < BUFFER_SIZE / 4; i++) {
        acWriteBuf[i] = i % 32;
        checksum += acWriteBuf[i];
    }
   
    // Initialize resource for overlapped parameter
    ftStatus = FT_InitializeOverlapped(ftHandle, &vOverlapped);
    printf("InitializeOverlapped: %s\n",ftStatus ? "ERROR" : "OK");
    // Set pipe timeout
    ftStatus = FT_SetPipeTimeout(ftHandle, ucPipeId, ulTimeoutInMs);
    printf("SetPipeTimeout: %s\n",ftStatus ? "ERROR" : "OK");

    // Wait for key press
    getch();

    // Write asynchronously
    ftStatus = FT_WritePipe(ftHandle, ucPipeId, acWriteBuf, sizeof(acWriteBuf), &ulBytesWritten, &vOverlapped);
    printf("WritePipe: %s\n",str_ftStatus[ftStatus]);

    if (ftStatus == FT_IO_PENDING) {
        // Wait until all requested data is sent
        do {
        // will return FT_IO_INCOMPLETE if not yet finish
        ftStatus = FT_GetOverlappedResult(ftHandle, &vOverlapped, &ulBytesWritten, FALSE);
        if(ftStatus != FT_IO_INCOMPLETE)
            printf("%s\n",ftStatus ? str_ftStatus[ftStatus] : "GetOverlappedResult: OK");

        if (ftStatus == FT_IO_INCOMPLETE) {
            continue;
        }
        else if (FT_FAILED(ftStatus)) {
            ftStatus = FT_ReleaseOverlapped(ftHandle, &vOverlapped);
            FT_Close(ftHandle);
            return FALSE;
        }
        else {
            break;
        }
        }while (1);
    }
    else{
        return FALSE;
    }

    // Release Overlapped
    ftStatus = FT_ReleaseOverlapped(ftHandle, &vOverlapped);
    printf("ReleaseOverlapped: %s\n",ftStatus ? "ERROR" : "OK");

    // Close device
    ftStatus = FT_Close(ftHandle);
    printf("Close: %s\n",ftStatus ? "ERROR" : "OK");
    printf("Checksum: %x\n",checksum);
    return TRUE;
}

int main(){
    DWORD library_version = 0;
    FT_STATUS ftdefStatus = FT_GetLibraryVersion(&library_version);
    printf("Lib. ver.: %s\n",ftdefStatus ? "ERROR" : "OK");
    DWORD lib1 = (library_version >> 24) & 0xFF;
    DWORD lib2 = (library_version >> 16) & 0xFF;
    DWORD lib3 = (library_version >> 8) & 0xFF;
    DWORD lib4 = library_version & 0xFF;

    printf("FTDI D3XX Library Version: %x.%x.%x.%x\n", lib1, lib2, lib3, lib4);
    
    DWORD numDevs = 0;   
    ftdefStatus = FT_CreateDeviceInfoList(&numDevs);

    printf("Device Nr: %d\n",numDevs);

    printf("%s\n", Receiver_test()?"RX_PASSED":"RX_ERROR");
    return 0;
} 