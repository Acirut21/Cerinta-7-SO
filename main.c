#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define THREAD_RETURN unsigned __stdcall
#define THREAD_TYPE HANDLE

typedef HANDLE Mutex;
void create_mutex(Mutex* m) { *m = CreateMutex(NULL, FALSE, NULL); }
void lock_mutex(Mutex* m) { WaitForSingleObject(*m, INFINITE); }
void unlock_mutex(Mutex* m) { ReleaseMutex(*m); }
void destroy_mutex(Mutex* m) { CloseHandle(*m); }
void sleep_ms(int ms) { Sleep(ms); }
void wait_thread(THREAD_TYPE t) { WaitForSingleObject(t, INFINITE); }

#else
#include <pthread.h>
#include <unistd.h>
#define THREAD_RETURN void*
#define THREAD_TYPE pthread_t

typedef pthread_mutex_t Mutex;
void create_mutex(Mutex* m) { pthread_mutex_init(m, NULL); }
void lock_mutex(Mutex* m) { pthread_mutex_lock(m); }
void unlock_mutex(Mutex* m) { pthread_mutex_unlock(m); }
void destroy_mutex(Mutex* m) { pthread_mutex_destroy(m); }
void sleep_ms(int ms) { usleep(ms * 1000); }
void wait_thread(THREAD_TYPE t) { pthread_join(t, NULL); }
#endif

Mutex resourceMutex;
Mutex turnstileMutex;
Mutex whiteCountMutex;
Mutex blackCountMutex;

int whiteCount = 0;
int blackCount = 0;

void white_enter() {
    lock_mutex(&turnstileMutex);
    lock_mutex(&whiteCountMutex);
    whiteCount++;
    if (whiteCount == 1) {
        lock_mutex(&resourceMutex);
    }
    unlock_mutex(&whiteCountMutex);
    unlock_mutex(&turnstileMutex);
}

void white_exit() {
    lock_mutex(&whiteCountMutex);
    whiteCount--;
    if (whiteCount == 0) {
        unlock_mutex(&resourceMutex);
    }
    unlock_mutex(&whiteCountMutex);
}

void black_enter() {
    lock_mutex(&turnstileMutex);
    lock_mutex(&blackCountMutex);
    blackCount++;
    if (blackCount == 1) {
        lock_mutex(&resourceMutex);
    }
    unlock_mutex(&blackCountMutex);
    unlock_mutex(&turnstileMutex);
}

void black_exit() {
    lock_mutex(&blackCountMutex);
    blackCount--;
    if (blackCount == 0) {
        unlock_mutex(&resourceMutex);
    }
    unlock_mutex(&blackCountMutex);
}

THREAD_RETURN white_thread_func(void* arg) {
    int id = *((int*)arg);
    free(arg);

    sleep_ms(rand() % 100);

    printf("White Thread %d is requesting access.\n", id);

    white_enter();

    printf(" -> White Thread %d is USING the resource. (Total Whites: %d)\n", id, whiteCount);
    sleep_ms(200 + (rand() % 300));

    white_exit();

    printf("White Thread %d finished.\n", id);
    return 0;
}

THREAD_RETURN black_thread_func(void* arg) {
    int id = *((int*)arg);
    free(arg);

    sleep_ms(rand() % 100);

    printf("Black Thread %d is requesting access.\n", id);

    black_enter();

    printf(" -> Black Thread %d is USING the resource. (Total Blacks: %d)\n", id, blackCount);
    sleep_ms(200 + (rand() % 300));

    black_exit();

    printf("Black Thread %d finished.\n", id);
    return 0;
}

int main() {
    srand(time(NULL));

    create_mutex(&resourceMutex);
    create_mutex(&turnstileMutex);
    create_mutex(&whiteCountMutex);
    create_mutex(&blackCountMutex);

    const int NUM_WHITE = 5;
    const int NUM_BLACK = 5;
    const int TOTAL_THREADS = NUM_WHITE + NUM_BLACK;

    THREAD_TYPE threads[10];

    for (int i = 0; i < TOTAL_THREADS; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;

        if (i % 2 == 0) {
#ifdef _WIN32
            threads[i] = (HANDLE)_beginthreadex(NULL, 0, white_thread_func, id, 0, NULL);
#else
            pthread_create(&threads[i], NULL, white_thread_func, id);
#endif
        }
        else {
#ifdef _WIN32
            threads[i] = (HANDLE)_beginthreadex(NULL, 0, black_thread_func, id, 0, NULL);
#else
            pthread_create(&threads[i], NULL, black_thread_func, id);
#endif
        }
    }

    for (int i = 0; i < TOTAL_THREADS; i++) {
        wait_thread(threads[i]);
    }

    destroy_mutex(&resourceMutex);
    destroy_mutex(&turnstileMutex);
    destroy_mutex(&whiteCountMutex);
    destroy_mutex(&blackCountMutex);

    printf("All threads completed execution.\n");
    return 0;
}
