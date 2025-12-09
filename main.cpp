#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;


// Global mutexes folosite pentru sincronizare intre thread-uri


// Mutex care protejeaza resursa comuna. Doar un tip de thread
// (white sau black) are voie sa o foloseasca la un moment dat.
mutex resourceMutex;

// Turnichet care forteaza thread-urile sa intre pe rand,
// evitand intrarea simultana in logica de incrementare.
mutex turnstileMutex;

// Mutex pentru numarul de thread-uri white care folosesc resursa
mutex whiteCountMutex;

// Mutex pentru numarul de thread-uri black care folosesc resursa
mutex blackCountMutex;

// Contoare pentru thread-uri active din fiecare categorie
int whiteCount = 0;
int blackCount = 0;

// Functie standard de pauza pentru C++
void sleep_ms(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}


//LOGICA DE INTRARE / IESIRE WHITE


void white_enter() {
    // Turnichetul se blocheaza pentru a preveni interferentele
    // cand white si black incearca sa intre simultan.
    turnstileMutex.lock();

    // Actualizam numarul de white active
    whiteCountMutex.lock();
    whiteCount++;

    // Daca acest thread este primul white care vrea resursa,
    // atunci white blocheaza resursa comuna.
    if (whiteCount == 1) {
        resourceMutex.lock();
    }

    // Eliberam mutex-urile
    whiteCountMutex.unlock();
    turnstileMutex.unlock();
}

void white_exit() {
    // Cand un thread white termina, scade whiteCount
    whiteCountMutex.lock();
    whiteCount--;

    // Daca nu mai exista thread-uri white,
    // atunci white elibereaza resursa comuna.
    if (whiteCount == 0) {
        resourceMutex.unlock();
    }

    whiteCountMutex.unlock();
}


//LOGICA DE INTRARE / IESIRE BLACK


void black_enter() {
    // Blocheaza turnichetul pentru sincronizare
    turnstileMutex.lock();

    // Actualizeaza contor black
    blackCountMutex.lock();
    blackCount++;

    // Daca acest thread este primul black care vrea resursa,
    // atunci black blocheaza resursa comuna.
    if (blackCount == 1) {
        resourceMutex.lock();
    }

    blackCountMutex.unlock();
    turnstileMutex.unlock();
}

void black_exit() {
    // Scade contorul de black active
    blackCountMutex.lock();
    blackCount--;

    // Daca nu mai exista thread-uri black,
    // se elibereaza resursa comuna.
    if (blackCount == 0) {
        resourceMutex.unlock();
    }

    blackCountMutex.unlock();
}


//FUNCTIA RULATA DE FIECARE WHITE THREAD


void white_thread(int id) {
    // Pausa aleatorie pentru simulat concurenta reala
    sleep_ms(rand() % 100);

    cout << "White Thread " << id << " is requesting access." << endl;

    // Thread-ul incearca sa intre in zona protejata
    white_enter();

    // Momentul in care thread-ul foloseste resursa
    cout << " -> White Thread " << id << " is USING the resource. (Total Whites: "
         << whiteCount << ")" << endl;

    // Simulare folosire resursa
    sleep_ms(200 + (rand() % 300));

    // Thread-ul iese din zona critica
    white_exit();

    cout << "White Thread " << id << " finished." << endl;
}


//FUNCTIA RULATA DE FIECARE BLACK THREAD


void black_thread(int id) {
    // Pauza aleatorie
    sleep_ms(rand() % 100);

    cout << "Black Thread " << id << " is requesting access." << endl;

    // Thread-ul incearca sa intre in zona protejata
    black_enter();

    // Momentul cand folosește resursa
    cout << " -> Black Thread " << id << " is USING the resource. (Total Blacks: "
         << blackCount << ")" << endl;

    // Simulare folosire resursa
    sleep_ms(200 + (rand() % 300));

    // Thread-ul iese din zona critica
    black_exit();

    cout << "Black Thread " << id << " finished." << endl;
}


//MAIN


int main() {
    srand(time(NULL));

    const int NUM_WHITE = 5;
    const int NUM_BLACK = 5;
    const int TOTAL_THREADS = NUM_WHITE + NUM_BLACK;

    // Vector care contine thread-urile create
    vector<thread> threads;
    threads.reserve(TOTAL_THREADS);

    // Creeaza alternativ un thread white si unul black
    for (int i = 0; i < TOTAL_THREADS; i++) {
        if (i % 2 == 0) {
            threads.emplace_back(white_thread, i);
        } else {
            threads.emplace_back(black_thread, i);
        }
    }

    // Asteapta ca toate thread-urile sa se termine
    for (auto& t : threads) {
        t.join();
    }

    cout << "All threads completed execution." << endl;
    return 0;
}
