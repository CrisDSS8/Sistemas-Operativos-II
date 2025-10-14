#include <iostream>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// Estructura de Proceso
struct Proceso {
    int pid;
    int tam; // tamaño en bytes
    string estado; // ready, execute, wait, end
};

// Variables globales
int TAM_MEMORIA = 40 * 1024; // Default: 40 KB en bytes, pero se sobreescribe
int RAMUsada = 0, VRAMUsada = 0;
int siguientePID = 1;

vector<Proceso> RAM;
vector<Proceso> VRAM;

// Variables para paginación
int tamMemKB = 0;
int tamPaginaKB = 0;
int numPaginas = 0;
bool memoriaInicializada = false;

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
void modificarProceso();
void calcularPaginas();
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

void calcularPaginas() {
    cleanSc();
    cout << "----Inicializar Memoria (Calcular Numero de Paginas)----\n\n";

    cout << "Tamaño total de memoria (KB): ";
    cin >> tamMemKB;
    cout << "Tamaño de página (KB): ";
    cin >> tamPaginaKB;

    if (tamPaginaKB <= 0 || tamMemKB <= 0 || tamMemKB < tamPaginaKB) {
        cout << "ERROR: Valores inválidos.\n";
        presionarEnter();
        return;
    }

    numPaginas = tamMemKB / tamPaginaKB;
    TAM_MEMORIA = tamMemKB * 1024; // bytes
    memoriaInicializada = true;

    cout << "\nNúmero de páginas: " << numPaginas << endl;
    cout << "Tamaño total de memoria: " << tamMemKB << " KB\n";
    cout << "Tamaño de página: " << tamPaginaKB << " KB\n";
    cout << "Memoria inicializada correctamente.\n";

    presionarEnter();
}

void crearProceso() { 
    int tamKB;

    cleanSc();
    cout << "----Crear Proceso----\n\n";
    cout << "Tamaño del proceso (en KB): ";
    cin >> tamKB;

    if (tamKB <= 0) {
        cout << "ERROR: El tamaño debe ser mayor a 0.\n";
        presionarEnter();
        return;
    }

    int paginasNecesarias = (tamKB + tamPaginaKB - 1) / tamPaginaKB; // Redondeo hacia arriba
    int tamBytes = paginasNecesarias * tamPaginaKB * 1024; // Tamaño real en bytes (paginado)

    Proceso nuevoProceso = {siguientePID++, tamBytes, "ready"};

    if (tamBytes <= (TAM_MEMORIA - RAMUsada)) {
        RAM.push_back(nuevoProceso);
        RAMUsada += tamBytes;
        cout << "Proceso con PID " << nuevoProceso.pid << " creado en RAM (" 
             << paginasNecesarias << " páginas).\n";
    } else if (tamBytes <= (TAM_MEMORIA - VRAMUsada)) {
        nuevoProceso.estado = "wait";
        VRAM.push_back(nuevoProceso);
        VRAMUsada += tamBytes;
        cout << "RAM llena. Proceso con PID " << nuevoProceso.pid 
             << " enviado a VRAM (" << paginasNecesarias << " páginas).\n";
    } else {
        cout << "ERROR: No hay espacio suficiente en RAM ni en VRAM.\n";
    }

    presionarEnter();
}

void mostrarMemoria() {
    cleanSc();
    cout << "----Estado de la Memoria----\n\n";

    auto imprimirTabla = [](const vector<Proceso>& lista, int usada, const string& nombre, int paginaInicial) -> int {
        cout << nombre << " (" << usada / 1024 << "/" << TAM_MEMORIA / 1024 << " KB usados)\n";

        if (lista.empty()) {
            cout << "   [Vacía]\n\n";
            return paginaInicial;
        }

        cout << "+------+-----------+----------+----------+----------------------+\n";
        cout << "| PID  | Tamaño KB | Páginas  | Estado   | Páginas ocupadas     |\n";
        cout << "+------+-----------+----------+----------+----------------------+\n";

        for (const auto& p : lista) {
            int paginas = p.tam / (tamPaginaKB * 1024);

            // Formatear las páginas ocupadas
            cout << "| " << setw(4) << left << p.pid
                 << " | " << setw(9) << left << p.tam / 1024
                 << " | " << setw(8) << left << paginas
                 << " | " << setw(8) << left << p.estado
                 << " | {";

            for (int i = 0; i < paginas; ++i) {
                cout << paginaInicial++;
                if (i < paginas - 1) cout << ", ";
            }
            cout << "}" << string(22 - (paginas * 3), ' ') << "|\n";
        }

        cout << "+------+-----------+----------+----------+----------------------+\n\n";

        return paginaInicial;
    };

    int paginaActual = 0;
    paginaActual = imprimirTabla(RAM, RAMUsada, "RAM", paginaActual);
    imprimirTabla(VRAM, VRAMUsada, "VRAM", paginaActual);

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

        cout << "Proceso con PID " << p.pid << " movido de VRAM a RAM. Estado: Listo.\n";
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
                    cout << "Proceso con PID " << RAM.front().pid << " eliminado de RAM.\n";
                    RAMUsada -= RAM.front().tam;
                    RAM.erase(RAM.begin());
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    cout << "Proceso con PID " << VRAM.front().pid << " eliminado de VRAM.\n";
                    VRAMUsada -= VRAM.front().tam;
                    VRAM.erase(VRAM.begin());
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;

            case 2: // LIFO
                if (!RAM.empty()) {
                    cout << "Proceso con PID " << RAM.back().pid << " eliminado de RAM.\n";
                    RAMUsada -= RAM.back().tam;
                    RAM.pop_back();
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    cout << "Proceso con PID " << VRAM.back().pid << " eliminado de VRAM.\n";
                    VRAMUsada -= VRAM.back().tam;
                    VRAM.pop_back();
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;
        }
    } while(opc != 3);
}

void modificarProceso() {
    cleanSc();
    cout << "----Modificar Proceso----\n\n";

    int pid, opcion, tamCambio;
    cout << "Ingrese el PID del proceso a modificar: ";
    cin >> pid;

    Proceso* proceso = nullptr;
    bool enRAM = false;

    for (auto& p : RAM)
        if (p.pid == pid) { proceso = &p; enRAM = true; break; }

    if (!proceso)
        for (auto& p : VRAM)
            if (p.pid == pid) { proceso = &p; enRAM = false; break; }

    if (!proceso) {
        cout << "No se encontró el proceso con PID " << pid << ".\n";
        presionarEnter();
        return;
    }

    cout << "1. Incrementar tamaño\n2. Decrementar tamaño\nOpcion: ";
    cin >> opcion;

    cout << "Cantidad en KB: ";
    cin >> tamCambio;

    if (tamCambio <= 0) {
        cout << "ERROR: Tamaño inválido.\n";
        presionarEnter();
        return;
    }

    int cambioBytes = ((tamCambio + tamPaginaKB - 1) / tamPaginaKB) * tamPaginaKB * 1024;

    if (opcion == 1) { // Incrementar
        if (enRAM && (RAMUsada + cambioBytes <= TAM_MEMORIA)) {
            proceso->tam += cambioBytes;
            RAMUsada += cambioBytes;
            cout << "Tamaño incrementado correctamente.\n";
        } else if (!enRAM && (VRAMUsada + cambioBytes <= TAM_MEMORIA)) {
            proceso->tam += cambioBytes;
            VRAMUsada += cambioBytes;
            cout << "Tamaño incrementado en VRAM.\n";
        } else {
            cout << "No hay espacio suficiente para incrementar.\n";
        }
    } else if (opcion == 2) { // Decrementar
        if (cambioBytes >= proceso->tam) {
            cout << "ERROR: No se puede reducir más del tamaño actual.\n";
        } else {
            proceso->tam -= cambioBytes;
            if (enRAM) RAMUsada -= cambioBytes;
            else VRAMUsada -= cambioBytes;
            cout << "Tamaño decrementado correctamente.\n";
        }
    }

    moverVRAMaRAM();
    presionarEnter();
}

string nextState(const string& estado) {
    int r = rand() % 100;
    
    if (estado == "ready") {
        if (r < 80) return "execute";
        else if (r < 90) return "end";
        else return "ready";
    } else if (estado == "execute") {
        if (r < 40) return "ready";
        else if (r < 80) return "wait";
        else if (r < 90) return "end";
        else return "execute";
    } else if (estado == "wait") {
        if (r < 40) return "ready";
        else if (r < 70) return "end";
        else return "wait";
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
                cout << "Ingrese el número de ciclos: ";
                cin >> n;
            }

            for (int ciclo = 1; ciclo <= n; ciclo++) {
                cout << "\n--- Ciclo " << ciclo << " ---\n";
                for (size_t i = 0; i < RAM.size(); ) {
                    Proceso &p = RAM[i];
                    string nuevoEstado = nextState(p.estado);

                    cout << "PID " << p.pid << " (" << p.estado 
                         << " -> " << nuevoEstado << ")\n";
                    p.estado = nuevoEstado;

                    if (p.estado == "end") {
                        cout << "PID " << p.pid << " finalizado.\n";
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
    srand(time(NULL)); 
    int opc;
    do {
        cleanSc();
        cout << "----Simulador RAM & VRAM----\n\n";
        cout << "1. Inicializar memoria y calcular páginas\n";
        cout << "2. Crear proceso\n";
        cout << "3. Eliminar proceso\n";
        cout << "4. Mostrar memoria\n";
        cout << "5. Modificar proceso\n";
        cout << "6. Simular CPU\n";
        cout << "7. Salir\n\n"; 	
        cout << "Opcion: ";
        cin >> opc;
        cout << endl;

        if (!memoriaInicializada && opc != 1 && opc != 7) {
            cout << "Primero debe inicializar la memoria desde la opción 1.\n";
            presionarEnter();
            continue;
        }

        switch (opc) {
            case 1: calcularPaginas(); break;
            case 2: crearProceso(); break;
            case 3: eliminarProceso(); break;
            case 4: mostrarMemoria(); break;
            case 5: modificarProceso(); break;
            case 6: simularCPU(); break;
            case 7: cout << "Saliendo del programa...\n"; break;
            default: cout << "Opción no válida.\n"; segundosPausa(1); break;
        }
    } while(opc != 7);
}

int main() {
    menu();
    return 0;
}
