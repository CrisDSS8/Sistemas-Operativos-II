#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

// Estructura de Proceso
struct Proceso {
    string nombre;
    int tam; // tamaño en bytes
    string estado; // ready, execute, wait, end
};

// Variables globales
const int TAM_MEMORIA = 40; // RAM y VRAM tienen 40 bytes cada una
int RAMUsada = 0, VRAMUsada = 0;
vector<Proceso> RAM;
vector<Proceso> VRAM;

// Declaración de funciones
void menu();
void cleanSc();
void segundosPausa (int segundos);
void presionarEnter();
void crearProceso();
void mostrarMemoria();
void eliminarProceso();
void moverVRAMaRAM();
void simularCPU();
string nextState(const string& estado);

// ---------------------- FUNCIONES ----------------------

void segundosPausa(int segundos) {
    #ifdef _WIN32
        Sleep(segundos * 1000);
    #else
        sleep(segundos);
    #endif
}

void cleanSc() {
    #ifdef _WIN32
        system("cls");
    #else 
        system("clear");
    #endif
}

void presionarEnter() {
    cout << "\nPresione Enter para continuar...";
    cin.ignore();
    cin.get();
}

void crearProceso() { 
    string nombreProceso;
    int tamProceso;

    cleanSc();
    cout << "----Crear Proceso----\n\n";
    cout << "Nombre del proceso: ";
    cin >> nombreProceso;
    cout << "Tamaño del proceso (en bytes): ";
    cin >> tamProceso;

    if (tamProceso <= 0) {
        cout << "ERROR: El tamaño debe ser mayor a 0.\n";
        presionarEnter();
        return;
    }

    if (tamProceso <= (TAM_MEMORIA - RAMUsada)) {
        RAM.push_back({nombreProceso, tamProceso, "ready"});
        RAMUsada += tamProceso;
        cout << "Proceso '" << nombreProceso << "' creado en RAM. Estado: Listo.\n";
    } else if (tamProceso <= (TAM_MEMORIA - VRAMUsada)) {
        VRAM.push_back({nombreProceso, tamProceso, "wait"});
        VRAMUsada += tamProceso;
        cout << "RAM llena. Proceso '" << nombreProceso << "' enviado a VRAM. Estado: Wait.\n";
    } else {
        cout << "ERROR: No hay espacio suficiente en RAM ni en VRAM.\n";
    }
    presionarEnter();
}

void mostrarMemoria() {
    cleanSc();
    cout << "----Estado de la Memoria----\n\n";

    cout << "RAM (" << RAMUsada << "/" << TAM_MEMORIA << " bytes usados):\n";
    if (RAM.empty()) cout << "   [Vacia]\n";
    for (auto &p : RAM) {
        cout << "   Proceso " << p.nombre << " (" << p.tam << " bytes, Estado: " << p.estado << ")\n";
    }

    cout << "\nVRAM (" << VRAMUsada << "/" << TAM_MEMORIA << " bytes usados):\n";
    if (VRAM.empty()) cout << "   [Vacia]\n";
    for (auto &p : VRAM) {
        cout << "   Proceso " << p.nombre << " (" << p.tam << " bytes, Estado: " << p.estado << ")\n";
    }

    presionarEnter();
}

void moverVRAMaRAM() {
    while (!VRAM.empty() && (VRAM.front().tam <= (TAM_MEMORIA - RAMUsada))) {
        Proceso p = VRAM.front();
        VRAM.erase(VRAM.begin());
        VRAMUsada -= p.tam;

        p.estado = "ready";
        RAM.push_back(p);
        RAMUsada += p.tam;

        cout << "Proceso '" << p.nombre << "' movido de VRAM a RAM. Estado: Listo.\n";
    }
}

void eliminarProceso() {
    int opc;
    do {
        cleanSc();
        cout << "----Eliminar Proceso----\n\n";
        cout << "1. Eliminar por Cola (FIFO)\n";
        cout << "2. Eliminar por Pila (LIFO)\n";
        cout << "3. Regresar\n";
        cout << "Opcion: ";
        cin >> opc;

        switch (opc) {
            case 1: // FIFO
                if (!RAM.empty()) {
                    RAMUsada -= RAM.front().tam;
                    cout << "Proceso '" << RAM.front().nombre << "' eliminado de RAM.\n";
                    RAM.erase(RAM.begin());
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    VRAMUsada -= VRAM.front().tam;
                    cout << "Proceso '" << VRAM.front().nombre << "' eliminado de VRAM.\n";
                    VRAM.erase(VRAM.begin());
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;
            case 2: // LIFO
                if (!RAM.empty()) {
                    RAMUsada -= RAM.back().tam;
                    cout << "Proceso '" << RAM.back().nombre << "' eliminado de RAM.\n";
                    RAM.pop_back();
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    VRAMUsada -= VRAM.back().tam;
                    cout << "Proceso '" << VRAM.back().nombre << "' eliminado de VRAM.\n";
                    VRAM.pop_back();
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;
        }
    } while(opc != 3);
}

string nextState(const string& estado) {
    int r = rand() % 100;
    if (estado == "ready") {
        return (r < 75) ? "execute" : "end";
    } else if (estado == "execute") {
        if (r < 50) return "ready";
        else if (r < 80) return "wait";
        else return "end";
    } else if (estado == "wait") {
        return (r < 80) ? "ready" : "end";
    }
    return "end";
}

void simularCPU() {
    int opc, n;
    do {
        cleanSc();
        cout << "----Simulacion de CPU----\n\n";
        cout << "1. Ejecutar 1 ciclo\n";
        cout << "2. Ejecutar N ciclos\n";
        cout << "3. Regresar\n";
        cout << "Opcion: ";
        cin >> opc;

        if (opc == 1 || opc == 2) {
            n = (opc == 1) ? 1 : 0;
            if (opc == 2) {
                cout << "Ingrese el numero de ciclos: ";
                cin >> n;
            }

            for (int ciclo = 1; ciclo <= n; ciclo++) {
                cout << "\n--- Ciclo " << ciclo << " ---\n";
                for (size_t i = 0; i < RAM.size(); ) {
                    Proceso &p = RAM[i];
                    string nuevoEstado = nextState(p.estado);

                    cout << "Proceso '" << p.nombre << "' (" << p.estado 
                         << " -> " << nuevoEstado << ")\n";
                    p.estado = nuevoEstado;

                    if (p.estado == "end") {
                        cout << "Proceso '" << p.nombre << "' ha terminado y se elimina.\n";
                        RAMUsada -= p.tam;
                        RAM.erase(RAM.begin() + i);
                        moverVRAMaRAM();
                    } else {
                        i++;
                    }
                }
            }
            presionarEnter();
        }
    } while(opc != 3);
}

void menu() { 
    srand(time(NULL)); // inicializar aleatorio
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "----Simulador RAM & VRAM----\n\n";
        cout << "\tMENU\n\n";
        cout << "1. Crear proceso\n";
        cout << "2. Eliminar proceso\n";
        cout << "3. Mostrar memoria\n";
        cout << "4. Simular CPU\n";
        cout << "5. Salir\n\n"; 	
        cout << "Opcion que desea: ";
        cin >> opc;
        cout << endl;

        switch (opc) {
            case 1:
                crearProceso();
                break;
            case 2: 
                eliminarProceso();
                break;
            case 3:
                mostrarMemoria();
                break;
            case 4:
                simularCPU();
                break;
            case 5:
                cout << "Saliendo del programa...\n" << endl;
                break;
            default:
                cout << "Opcion no valida... Ingrese nuevamente\n" << endl;
                segundosPausa(1);
                break;
        }
    } while(opc != 5);
}

// ---------------------- MAIN ----------------------
int main () {
    menu();
    return 0;
}
