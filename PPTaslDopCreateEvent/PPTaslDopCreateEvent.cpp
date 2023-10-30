// Доп.задание на С, CreateEvent.
//Делаем так, чтоб ОДИН поток читал из буфера общей памяти
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <iostream>
#include <windows.h>

const int imageSize = 1900; //размер изображения в байтах
const int coefficient = 1;
const int maxThreads = 8;
const int threadStep = imageSize / maxThreads;
unsigned char f[1900];

HANDLE writeEvent;

void create_event(void) {
    writeEvent = CreateEvent(NULL,TRUE,FALSE,TEXT("WriteEvent")); //создание события, вывод кода ошибки при уже существующем/не создалось создать событии
    if (writeEvent == NULL)
    {
        printf("Error! (%d)\n", GetLastError());
        return;
    }
}
void write_buffer(VOID)
{
    //меняем состояние на сигнальное
    printf("Main thread: writing to buffer\n");
    if (!SetEvent(writeEvent))
    {
        printf("Error! (%d)\n", GetLastError());
        return;
    }
}

typedef struct thread_parameters { //параметры функции для потока
    int start;
    int end;
    int threadNum;
};
DWORD __stdcall change_bytes(void* args) {
    thread_parameters* params = (thread_parameters*)args;

    for (int i = params->start; i > params->end; i -= threadStep) {
        f[i] += (i * coefficient) & 255;

    }

    DWORD dwwaitResult;

    printf("Thread: %d (waiting)\n", GetCurrentThreadId());

    dwwaitResult = WaitForSingleObject(writeEvent,INFINITE);

    switch (dwwaitResult)
    {
    case WAIT_OBJECT_0: //сигнальное состояние
        printf("Thread: %d (writing) \n",GetCurrentThreadId());
        break;
    default: //ошибка
        printf("Error! (%d)\n", GetLastError());
        return 0;
    }
    printf("Thread: %d (DONE)\n", GetCurrentThreadId()); //поток завершился
    return 0;
}
HANDLE handles[maxThreads];
DWORD thId[maxThreads];
thread_parameters params[maxThreads];
int main()
{
    FILE* original_file;
    FILE* new_file;

    original_file = fopen("C://Users//707//Downloads//corrupted_binary.jpg", "rb");
    fread(f, sizeof(f), 1, original_file);

    create_event();

    for (int i = 1; i < maxThreads + 1; i++) { //идем по потокам

        params[i - 1].start = imageSize - i;
        params[i - 1].end = 0;
        params[i - 1].threadNum = i;

        //первый поток расшифровывает от N-1, N-1-N/B, второй N-2, N-2-N/B, N-2-2*N/B...
        //N - размер файла, B = max_threads


        handles[i - 1] = CreateThread(NULL, 0, change_bytes, &params[i - 1], CREATE_SUSPENDED, &thId[i - 1]);
        write_buffer();
        ResumeThread(handles[i - 1]);
    }
    WaitForMultipleObjects(maxThreads, handles, true, INFINITE);

    new_file = fopen("C://Users//707//Downloads//changed_binary_createevent.jpg", "wb");
    fwrite(f, sizeof(f), 1, new_file);

    fclose(original_file);
    fclose(new_file);

    CloseHandle(writeEvent); //завершаем ивент
    return 0;
}