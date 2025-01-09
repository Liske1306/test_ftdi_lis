#include <windows.h>
#include <stdio.h>
#include <initguid.h>
#include "../FTD3XX.h"

#define  RX_SIZE (200000000) 

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

DEFINE_GUID(GUID_DEVINTERFACE_FOR_D3XX, 0xd1e8fe6a, 0xab75, 0x4d9e, 0x97, 0xd2, 0x6, 0xfa, 0x22, 0xc7, 0x73, 0x6c);
UINT32 uiReadBuf[RX_SIZE];

int main() {

    FT_STATUS ftStatus = FT_OK;
    FT_HANDLE ftHandle;
    GUID DeviceGUID[2] = { 0 };
    DWORD dwVersion;
    DWORD dwNumDevs;
    OVERLAPPED vOverlapped = { 0 };
    UCHAR ucPipeId = 0x82;
    ULONG ulTimeoutInMs = 100000;
    ULONG ulBytesRead = 0;

    printf("Lib. ver.: ");

    ftStatus = FT_GetLibraryVersion(&dwVersion);

    if (FT_SUCCESS(ftStatus)) { printf("OK\n"); }
    else { printf("Error: %d\n", ftStatus); };

    for (int i = 0; i < 4; i++) {
        printf("%x.", (unsigned char)(dwVersion >> ((3 - i) * 8)));
    }
    printf("\n");

    printf("Device Nr: ");
    ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
    printf("%d\n", dwNumDevs);

    printf("Device Creation: ");
    memcpy(&DeviceGUID[0], &GUID_DEVINTERFACE_FOR_D3XX, sizeof(GUID));
    ftStatus = FT_Create(&DeviceGUID[0], FT_OPEN_BY_GUID, &ftHandle);
    if (FT_SUCCESS(ftStatus)) {
        printf("OK\n");
    }
    else {
        printf("Error: %d\n", ftStatus);
        goto exit;
    };

    printf("InitializeOverlapped: ");
    ftStatus = FT_InitializeOverlapped(ftHandle, &vOverlapped);
    if (FT_SUCCESS(ftStatus)) printf("OK\n"); else  printf("Error: %d\n", ftStatus);

    printf("SetPipeTimeout: ");
    ftStatus = FT_SetPipeTimeout(ftHandle, ucPipeId, ulTimeoutInMs);
    if (FT_SUCCESS(ftStatus)) printf("OK\n"); else  printf("Error: %d\n", ftStatus);

    printf("Read data from pipe: ");
    ftStatus = FT_ReadPipe(ftHandle, ucPipeId, uiReadBuf, sizeof(uiReadBuf), &ulBytesRead, &vOverlapped);
    if (FT_SUCCESS(ftStatus)) printf("OK\n"); else  printf("%s\n", str_ftStatus[ftStatus]);

    printf("FT_GetOverlappedResult:");
    if (ftStatus == FT_IO_PENDING) ftStatus = FT_GetOverlappedResult(ftHandle, &vOverlapped, &ulBytesRead, TRUE);
    if (FT_SUCCESS(ftStatus)) printf("OK\n"); else  printf("Error: %d\n", ftStatus);

close:
    ftStatus = FT_Close(ftHandle);
    printf("close: ");
    if (FT_SUCCESS(ftStatus)) printf("OK\n"); else  printf("Error: %d\n", ftStatus);

    printf("Read32bitWords: % lu\n", ulBytesRead/4);

    printf("Begin: %lu\n", (unsigned long) uiReadBuf[0]);
    printf("End:   %lu\n", (unsigned long) uiReadBuf[RX_SIZE-1]);

    for (UINT32 i = 1; i < RX_SIZE; i++) {
        if (uiReadBuf[i] != uiReadBuf[i+1]-1) { //
            printf("One before %d\n", (unsigned long)uiReadBuf[i-1]);
            printf("Received: %d\n", (unsigned long)uiReadBuf[i]);
            printf("Next: %d\n", (unsigned long)uiReadBuf[i+1]);
            break;
        }        
    }


exit:;
}