#include <windows.h>        // For Windows-specific APIs
#include <stdio.h>          // For printf and debugging
#include <conio.h>          // For getch()
#include "../FTD3xx.h"         // FTDI D3XX API header (include path required)

#define BUFFER_SIZE 8192    // Define the buffer size (adjust as needed)

BOOL Receiver_test(void) {
    FT_STATUS ftStatus = FT_OK;
    printf("%d\n",ftStatus);
    FT_HANDLE ftHandle;
    GUID DeviceGUID[2] = { 0 };

    OVERLAPPED vOverlapped = { 0 };
    UINT32 acWriteBuf[BUFFER_SIZE / 4] = { 0xFF };
    ULONG ulBytesWritten = 0;
    UCHAR ucPipeId = 0x02;
    ULONG ulTimeoutInMs = 10000;

    //memcpy(&DeviceGUID[0], &GUID_DEVINTERFACE_FOR_D3XX, sizeof(GUID));
    //ftStatus = FT_Create(&DeviceGUID[0], FT_OPEN_BY_GUID, &ftHandle);
    ftStatus = FT_Create("000000000001", FT_OPEN_BY_SERIAL_NUMBER, &ftHandle); 
    printf("%s\n",ftStatus ? "Error not Connected" : "Connected");
    // Prepare data to sent
    for (ULONG i = 0; i < BUFFER_SIZE / 4; i++) {
    acWriteBuf[i] = i;
    }

    // Initialize resource for overlapped parameter
    ftStatus = FT_InitializeOverlapped(ftHandle, &vOverlapped);
    printf("%s\n",ftStatus ? "Error Overlap" : "Overlap PASS");
    // Set pipe timeout
    ftStatus = FT_SetPipeTimeout(ftHandle, ucPipeId, ulTimeoutInMs);
    printf("%s\n",ftStatus ? "Error Timeout" : "Timeout PASS");

    // Wait for anykey press
    getch();

    // Write asynchronously
    ftStatus = FT_WritePipe(ftHandle, ucPipeId, acWriteBuf, sizeof(acWriteBuf), &ulBytesWritten, &vOverlapped);
    printf("%d - Pipe (24 = FT_IO_PENDING expected)\n",ftStatus);

    if (ftStatus == FT_IO_PENDING) {
        // Wait until all requested data is sent
        do {
        // will return FT_IO_INCOMPLETE if not yet finish
        ftStatus = FT_GetOverlappedResult(ftHandle, &vOverlapped,
        &ulBytesWritten, FALSE);

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
    printf("%s\n",ftStatus ? "Error Release Overlap" : "Release Overlap PASS");

    // Close device
    FT_Close(ftHandle);
    return TRUE;
}

int main(){
    printf("%s\n", Receiver_test()?"RX_PASSED":"RX_ERROR");
    return 0;
} 