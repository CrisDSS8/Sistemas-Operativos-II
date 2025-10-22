#include <iostream>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <iomanip>
#include <map>

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

// Estructura para Página
struct Pagina {
    int id;
    int pid; // PID del proceso dueño
    bool libre;
    bool danada;
    int errores; // contador de errores
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

// Nuevas variables para gestión de páginas
vector<Pagina> paginasRAM;
vector<Pagina> paginasVRAM;
map<int, vector<int>> paginasPorProceso; // Mapea PID -> lista de índices de páginas

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
void inicializarPaginas();
void simularErroresPagina();
void manejarErrorPagina(int paginaIndex, int pid);
void mostrarMenuReemplazoPagina(int paginaDanada, int pid);
void reemplazarPaginaExistente(int paginaDanada, int pid);
void reemplazarPaginaNueva(int paginaDanada, int pid);

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

void inicializarPaginas() {
    paginasRAM.clear();
    paginasVRAM.clear();
    paginasPorProceso.clear();
    
    for (int i = 0; i < numPaginas; i++) {
        paginasRAM.push_back({i, -1, true, false, 0});
    }
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

    // Inicializar el sistema de páginas
    inicializarPaginas();

    cout << "\nNúmero de páginas: " << numPaginas << endl;
    cout << "Tamaño total de memoria: " << tamMemKB << " KB\n";
    cout << "Tamaño de página: " << tamPaginaKB << " KB\n";
    cout << "Memoria inicializada correctamente.\n";

    presionarEnter();
}

void simularErroresPagina() {
    // Simular errores aleatorios en páginas ocupadas
    for (auto& pagina : paginasRAM) {
        if (!pagina.libre && !pagina.danada) {
            // 10% de probabilidad de error por ciclo
            if (rand() % 100 < 30) {
                pagina.errores++;
                cout << "¡Error de página! Página " << pagina.id 
                     << " (PID " << pagina.pid << ") - Errores: " 
                     << pagina.errores << "/5\n";
                
                if (pagina.errores >= 5) {
                    pagina.danada = true;
                    cout << "¡PÁGINA DAÑADA! Página " << pagina.id 
                         << " marcada como defectuosa.\n";
                    manejarErrorPagina(pagina.id, pagina.pid);
                }
            }
        }
    }
}

void manejarErrorPagina(int paginaIndex, int pid) {
    cout << "\n--- GESTIÓN DE PÁGINA DAÑADA ---\n";
    cout << "Página " << paginaIndex << " del proceso PID " << pid << " está dañada.\n";
    
    mostrarMenuReemplazoPagina(paginaIndex, pid);
}

void mostrarMenuReemplazoPagina(int paginaDanada, int pid) {
    int opcion;
    
    cout << "\nOpciones de reemplazo:\n";
    cout << "1. Reemplazar página dañada por una página existente\n";
    cout << "2. Reemplazar página dañada por una página nueva\n";
    cout << "Seleccione opción: ";
    cin >> opcion;
    
    switch(opcion) {
        case 1:
            reemplazarPaginaExistente(paginaDanada, pid);
            break;
        case 2:
            reemplazarPaginaNueva(paginaDanada, pid);
            break;
        default:
            cout << "Opción inválida. Se usará reemplazo por página nueva.\n";
            reemplazarPaginaNueva(paginaDanada, pid);
            break;
    }
}

void reemplazarPaginaExistente(int paginaDanada, int pid) {
    cout << "\n--- PÁGINAS LIBRES DISPONIBLES ---\n";
    vector<int> paginasLibres;
    
    // Buscar páginas libres
    for (const auto& pagina : paginasRAM) {
        if (pagina.libre && !pagina.danada) {
            paginasLibres.push_back(pagina.id);
        }
    }
    
    if (paginasLibres.empty()) {
        cout << "No hay páginas libres disponibles. Creando página nueva...\n";
        reemplazarPaginaNueva(paginaDanada, pid);
        return;
    }
    
    // Mostrar páginas libres
    cout << "Páginas libres: ";
    for (int i = 0; i < paginasLibres.size(); i++) {
        cout << paginasLibres[i];
        if (i < paginasLibres.size() - 1) cout << ", ";
    }
    cout << endl;
    
    // Seleccionar página de reemplazo
    int nuevaPagina;
    cout << "Seleccione el número de página para reemplazar: ";
    cin >> nuevaPagina;
    
    // Validar selección
    bool valida = false;
    for (int libre : paginasLibres) {
        if (libre == nuevaPagina) {
            valida = true;
            break;
        }
    }
    
    if (!valida) {
        cout << "Página inválida. Seleccionando automáticamente...\n";
        nuevaPagina = paginasLibres[0];
    }
    
    // Realizar reemplazo
    paginasRAM[paginaDanada].libre = true;
    paginasRAM[paginaDanada].pid = -1;
    paginasRAM[paginaDanada].errores = 0;
    
    paginasRAM[nuevaPagina].libre = false;
    paginasRAM[nuevaPagina].pid = pid;
    paginasRAM[nuevaPagina].errores = 0;
    paginasRAM[nuevaPagina].danada = false;
    
    // Actualizar mapeo de proceso
    auto& paginasProceso = paginasPorProceso[pid];
    for (auto it = paginasProceso.begin(); it != paginasProceso.end(); ++it) {
        if (*it == paginaDanada) {
            *it = nuevaPagina;
            break;
        }
    }
    
    cout << "Página " << paginaDanada << " reemplazada por página " << nuevaPagina << " para proceso PID " << pid << endl;
}

void reemplazarPaginaNueva(int paginaDanada, int pid) {
    // Buscar una página libre o crear una nueva (en este caso, simular creación)
    int nuevaPagina = -1;
    
    // Buscar página libre
    for (auto& pagina : paginasRAM) {
        if (pagina.libre && !pagina.danada) {
            nuevaPagina = pagina.id;
            break;
        }
    }
    
    if (nuevaPagina == -1) {
        // En un sistema real aquí se implementaría algún algoritmo de reemplazo
        // Por simplicidad, usamos la primera página no dañada
        for (auto& pagina : paginasRAM) {
            if (!pagina.danada) {
                nuevaPagina = pagina.id;
                cout << "Advertencia: Se está reemplazando página ocupada " << nuevaPagina << endl;
                break;
            }
        }
    }
    
    if (nuevaPagina != -1) {
        // Realizar reemplazo
        paginasRAM[paginaDanada].libre = true;
        paginasRAM[paginaDanada].pid = -1;
        paginasRAM[paginaDanada].errores = 0;
        
        paginasRAM[nuevaPagina].libre = false;
        paginasRAM[nuevaPagina].pid = pid;
        paginasRAM[nuevaPagina].errores = 0;
        paginasRAM[nuevaPagina].danada = false;
        
        // Actualizar mapeo de proceso
        auto& paginasProceso = paginasPorProceso[pid];
        for (auto it = paginasProceso.begin(); it != paginasProceso.end(); ++it) {
            if (*it == paginaDanada) {
                *it = nuevaPagina;
                break;
            }
        }
        
        cout << "Página " << paginaDanada << " reemplazada por página " << nuevaPagina << " (NUEVA) para proceso PID " << pid << endl;
    } else {
        cout << "ERROR: No se pudo encontrar página de reemplazo.\n";
    }
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

    // Asignar páginas al proceso
    vector<int> paginasAsignadas;
    int paginasAsignadasCount = 0;
    
    // Buscar páginas libres en RAM
    for (auto& pagina : paginasRAM) {
        if (pagina.libre && !pagina.danada && paginasAsignadasCount < paginasNecesarias) {
            pagina.libre = false;
            pagina.pid = nuevoProceso.pid;
            pagina.errores = 0;
            pagina.danada = false;
            paginasAsignadas.push_back(pagina.id);
            paginasAsignadasCount++;
        }
    }

    if (paginasAsignadasCount == paginasNecesarias) {
        // Todas las páginas asignadas en RAM
        RAM.push_back(nuevoProceso);
        RAMUsada += tamBytes;
        paginasPorProceso[nuevoProceso.pid] = paginasAsignadas;
        cout << "Proceso con PID " << nuevoProceso.pid << " creado en RAM (" 
             << paginasNecesarias << " páginas).\n";
    } else {
        // Liberar páginas asignadas (no hay suficiente espacio en RAM)
        for (int paginaId : paginasAsignadas) {
            paginasRAM[paginaId].libre = true;
            paginasRAM[paginaId].pid = -1;
        }
        
        // Crear proceso en VRAM (NO se asignan páginas aquí)
        nuevoProceso.estado = "wait";
        VRAM.push_back(nuevoProceso);
        VRAMUsada += tamBytes;
        cout << "RAM llena. Proceso con PID " << nuevoProceso.pid 
             << " enviado a VRAM (" << paginasNecesarias << " páginas necesarias).\n";
        cout << "Nota: Las páginas se asignarán cuando el proceso se mueva a RAM.\n";
    }

    presionarEnter();
}

void mostrarMemoria() {
    cleanSc();
    cout << "----Estado de la Memoria----\n\n";

    // Mostrar información de páginas
    cout << "Páginas RAM (" << numPaginas << " totales):\n";
    cout << "+---------+------+--------+---------+\n";
    cout << "| Página  | PID  | Estado | Errores |\n";
    cout << "+---------+------+--------+---------+\n";
    
    for (const auto& pagina : paginasRAM) {
        cout << "| " << setw(7) << pagina.id 
             << " | " << setw(4) << (pagina.pid == -1 ? "libre" : to_string(pagina.pid))
             << " | " << setw(6) << (pagina.libre ? "libre" : (pagina.danada ? "dañada" : "ocupada"))
             << " | " << setw(7) << pagina.errores << " |\n";
    }
    cout << "+---------+------+--------+---------+\n\n";

    // Mostrar procesos en RAM
    cout << "Procesos en RAM:\n";
    if (RAM.empty()) {
        cout << "   [Vacía]\n";
    } else {
        cout << "+------+-----------+----------+----------+\n";
        cout << "| PID  | Tamaño KB | Páginas  | Estado   |\n";
        cout << "+------+-----------+----------+----------+\n";
        for (const auto& p : RAM) {
            int paginas = p.tam / (tamPaginaKB * 1024);
            cout << "| " << setw(4) << p.pid 
                 << " | " << setw(9) << p.tam / 1024
                 << " | " << setw(8) << paginas
                 << " | " << setw(8) << p.estado << " |\n";
        }
        cout << "+------+-----------+----------+----------+\n";
    }

    cout << "\nProcesos en VRAM:\n";
    if (VRAM.empty()) {
        cout << "   [Vacía]\n";
    } else {
        cout << "+------+-----------+----------+----------+\n";
        cout << "| PID  | Tamaño KB | Páginas  | Estado   |\n";
        cout << "+------+-----------+----------+----------+\n";
        for (const auto& p : VRAM) {
            int paginas = p.tam / (tamPaginaKB * 1024);
            cout << "| " << setw(4) << p.pid 
                 << " | " << setw(9) << p.tam / 1024
                 << " | " << setw(8) << paginas
                 << " | " << setw(8) << p.estado << " |\n";
        }
        cout << "+------+-----------+----------+----------+\n";
    }

    cout << "\nUso de memoria: RAM " << RAMUsada/1024 << "/" << TAM_MEMORIA/1024 
         << " KB, VRAM " << VRAMUsada/1024 << "/" << TAM_MEMORIA/1024 << " KB\n";

    presionarEnter();
}

void moverVRAMaRAM() {
    // Mover procesos de VRAM a RAM si hay espacio
    for (auto it = VRAM.begin(); it != VRAM.end(); ) {
        int paginasNecesarias = it->tam / (tamPaginaKB * 1024);
        int paginasLibresCount = 0;
        
        // Contar páginas libres disponibles
        for (const auto& pagina : paginasRAM) {
            if (pagina.libre && !pagina.danada) {
                paginasLibresCount++;
            }
        }
        
        if (paginasLibresCount >= paginasNecesarias) {
            Proceso p = *it;
            it = VRAM.erase(it);
            VRAMUsada -= p.tam;

            // ASIGNAR PÁGINAS AL PROCESO
            vector<int> paginasAsignadas;
            int paginasAsignadasCount = 0;
            
            // Buscar y asignar páginas libres en RAM
            for (auto& pagina : paginasRAM) {
                if (pagina.libre && !pagina.danada && paginasAsignadasCount < paginasNecesarias) {
                    pagina.libre = false;
                    pagina.pid = p.pid;
                    pagina.errores = 0;
                    pagina.danada = false;
                    paginasAsignadas.push_back(pagina.id);
                    paginasAsignadasCount++;
                }
            }
            
            p.estado = "ready";
            RAM.push_back(p);
            RAMUsada += p.tam;
            paginasPorProceso[p.pid] = paginasAsignadas;

            cout << "Proceso con PID " << p.pid << " movido de VRAM a RAM. ";
            cout << "Páginas asignadas: " << paginasNecesarias << ". Estado: Listo.\n";
        } else {
            ++it;
        }
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
                    Proceso p = RAM.front();
                    cout << "Proceso con PID " << p.pid << " eliminado de RAM.\n";
                    RAMUsada -= p.tam;
                    
                    // Liberar páginas del proceso
                    if (paginasPorProceso.find(p.pid) != paginasPorProceso.end()) {
                        for (int paginaId : paginasPorProceso[p.pid]) {
                            if (paginaId < paginasRAM.size()) {
                                paginasRAM[paginaId].libre = true;
                                paginasRAM[paginaId].pid = -1;
                                paginasRAM[paginaId].errores = 0;
                            }
                        }
                        paginasPorProceso.erase(p.pid);
                    }
                    
                    RAM.erase(RAM.begin());
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    Proceso p = VRAM.front();
                    cout << "Proceso con PID " << p.pid << " eliminado de VRAM.\n";
                    VRAMUsada -= p.tam;
                    // NO liberar páginas porque los procesos en VRAM no tienen páginas asignadas en RAM
                    VRAM.erase(VRAM.begin());
                } else {
                    cout << "No hay procesos para eliminar.\n";
                }
                presionarEnter();
                break;

            case 2: // LIFO
                if (!RAM.empty()) {
                    Proceso p = RAM.back();
                    cout << "Proceso con PID " << p.pid << " eliminado de RAM.\n";
                    RAMUsada -= p.tam;
                    
                    // Liberar páginas del proceso
                    if (paginasPorProceso.find(p.pid) != paginasPorProceso.end()) {
                        for (int paginaId : paginasPorProceso[p.pid]) {
                            if (paginaId < paginasRAM.size()) {
                                paginasRAM[paginaId].libre = true;
                                paginasRAM[paginaId].pid = -1;
                                paginasRAM[paginaId].errores = 0;
                            }
                        }
                        paginasPorProceso.erase(p.pid);
                    }
                    
                    RAM.pop_back();
                    moverVRAMaRAM();
                } else if (!VRAM.empty()) {
                    Proceso p = VRAM.back();
                    cout << "Proceso con PID " << p.pid << " eliminado de VRAM.\n";
                    VRAMUsada -= p.tam;
                    // NO liberar páginas porque los procesos en VRAM no tienen páginas asignadas en RAM
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

    int paginasCambio = (tamCambio + tamPaginaKB - 1) / tamPaginaKB;
    int cambioBytes = paginasCambio * tamPaginaKB * 1024;

    if (opcion == 1) { // Incrementar
        if (enRAM) {
            // Verificar si hay suficientes páginas libres
            int paginasLibres = 0;
            for (const auto& pagina : paginasRAM) {
                if (pagina.libre && !pagina.danada) paginasLibres++;
            }
            
            if (paginasLibres >= paginasCambio && (RAMUsada + cambioBytes <= TAM_MEMORIA)) {
                // Asignar nuevas páginas
                vector<int> nuevasPaginas;
                for (auto& pagina : paginasRAM) {
                    if (pagina.libre && !pagina.danada && nuevasPaginas.size() < paginasCambio) {
                        pagina.libre = false;
                        pagina.pid = pid;
                        pagina.errores = 0;
                        nuevasPaginas.push_back(pagina.id);
                    }
                }
                
                // Actualizar el mapeo de páginas del proceso
                auto& paginasProceso = paginasPorProceso[pid];
                paginasProceso.insert(paginasProceso.end(), nuevasPaginas.begin(), nuevasPaginas.end());
                
                proceso->tam += cambioBytes;
                RAMUsada += cambioBytes;
                cout << "Tamaño incrementado correctamente. " << paginasCambio << " páginas asignadas.\n";
            } else {
                cout << "No hay espacio suficiente en RAM para incrementar.\n";
            }
        } else {
            // Proceso en VRAM
            if (VRAMUsada + cambioBytes <= TAM_MEMORIA) {
                proceso->tam += cambioBytes;
                VRAMUsada += cambioBytes;
                cout << "Tamaño incrementado en VRAM.\n";
            } else {
                cout << "No hay espacio suficiente en VRAM para incrementar.\n";
            }
        }
    } else if (opcion == 2) { // Decrementar
        if (cambioBytes >= proceso->tam) {
            cout << "ERROR: No se puede reducir más del tamaño actual.\n";
        } else {
            if (enRAM) {
                // Liberar páginas (las últimas páginas asignadas)
                int paginasALiberar = min(paginasCambio, (int)paginasPorProceso[pid].size());
                for (int i = 0; i < paginasALiberar; i++) {
                    int paginaId = paginasPorProceso[pid].back();
                    paginasRAM[paginaId].libre = true;
                    paginasRAM[paginaId].pid = -1;
                    paginasRAM[paginaId].errores = 0;
                    paginasPorProceso[pid].pop_back();
                }
                RAMUsada -= cambioBytes;
            } else {
                VRAMUsada -= cambioBytes;
            }
            proceso->tam -= cambioBytes;
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
                
                // Simular errores de página
                simularErroresPagina();
                
                for (size_t i = 0; i < RAM.size(); ) {
                    Proceso &p = RAM[i];
                    string nuevoEstado = nextState(p.estado);

                    cout << "PID " << p.pid << " (" << p.estado 
                         << " -> " << nuevoEstado << ")\n";
                    p.estado = nuevoEstado;

                    if (p.estado == "end") {
                        cout << "PID " << p.pid << " finalizado.\n";
                        RAMUsada -= p.tam;
                        
                        // Liberar páginas del proceso
                        if (paginasPorProceso.find(p.pid) != paginasPorProceso.end()) {
                            for (int paginaId : paginasPorProceso[p.pid]) {
                                if (paginaId < paginasRAM.size()) {
                                    paginasRAM[paginaId].libre = true;
                                    paginasRAM[paginaId].pid = -1;
                                    paginasRAM[paginaId].errores = 0;
                                }
                            }
                            paginasPorProceso.erase(p.pid);
                        }
                        
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