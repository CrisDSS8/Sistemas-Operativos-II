#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <tuple>
#include <queue>
#include <stack>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

//DECLARACION DE FUNCIONES
void menu();
void cleanSc();
void segundosPausa (int segundos);
void presionarEnter();
void definirRAM();
void operacionRAM();
void crearProceso();
void mostrarRAM();
void eliminarProceso();
void actualizarMapaBits();

//VARIABLES GLOBALES
int RAMDisponible = 0;
vector<string> memoria;                 // RAM con nombres de procesos
vector<vector<int>> mapaBits;           // 0 = libre, 1 = ocupado
queue<string> colaProcesos;   
stack<string> pilaProcesos;   

//PROGRAMA PRINCIPAL
int main () {
    menu();
    return 0;
}

void segundosPausa(int segundos) { //Pausa de segundos
    #ifdef _WIN32
        Sleep(segundos * 1000);
    #else
        sleep(segundos);
    #endif
}

void cleanSc() { //Limpiar pantalla
    #ifdef _WIN32
        system("cls");
    #else 
        system("clear");
    #endif
}

void presionarEnter() { //Pedir al usuario que presione Enter
    cout << "\nPresione Enter para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar buffer
    cin.get();
}

void definirRAM() { //Definir memoria RAM (libre continua)
    if (memoria.empty()) {
        cleanSc();
        cout << "----Definir Memoria RAM----\n" << endl;
        cout << "Ingrese el tamano de la RAM disponible (en bytes): ";
        cin >> RAMDisponible;

        if (RAMDisponible <= 0) {
            cout << "Tamano invalido. Debe ser mayor que 0." << endl;
            return;
        }

        memoria = vector<string>(RAMDisponible, "LIBRE");

        // Crear matriz de bits (8 columnas x n filas)
        int filas = (RAMDisponible + 7) / 8; // redondeo hacia arriba
        mapaBits = vector<vector<int>>(filas, vector<int>(8, 0));

        cout << "\nRAM Inicializada con " << RAMDisponible << " bytes." << endl;
        presionarEnter();
    } else {
        cout << "ERROR: La Memoria RAM ya ha sido definida." << endl;
        cout << "Si desea cambiar los valores, reinicie el programa." << endl;
        presionarEnter();
    }
}

void actualizarMapaBits() {
    int filas = (RAMDisponible + 7) / 8;
    mapaBits.assign(filas, vector<int>(8, 0));

    for (int i = 0; i < RAMDisponible; i++) {
        int fila = i / 8;
        int col  = i % 8;
        if (memoria[i] != "LIBRE") {
            mapaBits[fila][col] = 1;
        }
    }
}

void crearProceso() { //Crear un proceso en la RAM
    string nombreProceso;
    int tamProceso;
    if (memoria.empty()) {
        cout << "ERROR: Debe definir la cantidad de RAM antes de utilizar esta opcion." << endl;
        presionarEnter();
        return;
    }
    cleanSc();
    cout << "----Crear Proceso----\n" << endl;
    cout << "Nombre del proceso: ";
    cin >> nombreProceso;

    // Verificar que no exista el proceso
    for (const auto& byte : memoria) {
        if (byte == nombreProceso) {
            cout << "\nERROR: El proceso '" << nombreProceso << "' ya existe en la RAM." << endl;
            presionarEnter();
            return;
        }
    }

    cout << "Tamano del proceso (en bytes): ";
    cin >> tamProceso;

    if (tamProceso <= 0 || tamProceso > RAMDisponible) {
        cout << "\nTamano de proceso invalido." << endl;
        presionarEnter();
        return;
    }

    // Buscar espacio contiguo en RAM
    int libres = 0, inicio = -1;
    for (size_t i = 0; i < memoria.size(); i++) {
        if (memoria[i] == "LIBRE") {
            if (inicio == -1) inicio = i;
            libres++;
            if (libres == tamProceso) {
                for (int j = inicio; j < inicio + tamProceso; j++) {
                    memoria[j] = nombreProceso;
                }
                colaProcesos.push(nombreProceso);
                pilaProcesos.push(nombreProceso);
                cout << "\nProceso '" << nombreProceso << "' creado con " 
                     << tamProceso << " bytes." << endl;
                actualizarMapaBits();
                presionarEnter();
                return;
            }
        } else {
            libres = 0;
            inicio = -1;
        }
    }

    cout << "\nNo hay suficiente memoria contigua para el proceso." << endl;
    presionarEnter();
}

void eliminarProcesoPorCola() {
    if (colaProcesos.empty()) {
        cout << "No hay procesos en cola para eliminar." << endl;
        presionarEnter();
        return;
    }

    string nombreProceso = colaProcesos.front();
    colaProcesos.pop();

    bool encontrado = false;
    int liberados = 0;
    for (auto &byte : memoria) {
        if (byte == nombreProceso) {
            byte = "LIBRE";
            liberados++;
            encontrado = true;
        }
    }

    if (encontrado) {
        cout << "\nProceso '" << nombreProceso << "' eliminado (por COLA). Se liberaron "
             << liberados << " bytes." << endl;
        actualizarMapaBits();
    } else {
        cout << "\nERROR: El proceso '" << nombreProceso << "' no se encontró en la RAM." << endl;
    }

    presionarEnter();
}

void eliminarProcesoPorPila() {
    if (pilaProcesos.empty()) {
        cout << "No hay procesos en pila para eliminar." << endl;
        presionarEnter();
        return;
    }

    string nombreProceso = pilaProcesos.top();
    pilaProcesos.pop();

    bool encontrado = false;
    int liberados = 0;
    for (auto &byte : memoria) {
        if (byte == nombreProceso) {
            byte = "LIBRE";
            liberados++;
            encontrado = true;
        }
    }

    if (encontrado) {
        cout << "\nProceso '" << nombreProceso << "' eliminado (por PILA). Se liberaron "
             << liberados << " bytes." << endl;
        actualizarMapaBits();
    } else {
        cout << "\nERROR: El proceso '" << nombreProceso << "' no se encontró en la RAM." << endl;
    }

    presionarEnter();
}

void eliminarProceso() {
    if (memoria.empty()) {
        cout << "ERROR: Debe definir la cantidad de RAM antes de utilizar esta opcion." << endl;
        presionarEnter();
        return;
    }

    int opc;
    do {
        cleanSc();
        cout << "----Eliminar Proceso----\n" << endl;
        cout << "1. Eliminar por nombre" << endl;
        cout << "2. Eliminar por COLA (FIFO)" << endl;
        cout << "3. Eliminar por PILA (LIFO)" << endl;
        cout << "4. Volver al menu principal\n" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opc;

        switch (opc) {
            case 1: {
                string nombreProceso;
                cout << "\nNombre del proceso a eliminar: ";
                cin >> nombreProceso;

                bool encontrado = false;
                int liberados = 0;
                for (auto &byte : memoria) {
                    if (byte == nombreProceso) {
                        byte = "LIBRE";
                        liberados++;
                        encontrado = true;
                    }
                }

                if (encontrado) {
                    cout << "\nProceso '" << nombreProceso << "' eliminado. Se liberaron "
                         << liberados << " bytes." << endl;
                    actualizarMapaBits();
                } else {
                    cout << "\nERROR: El proceso '" << nombreProceso << "' no existe en la RAM." << endl;
                }

                presionarEnter();
                break;
            }

            case 2:
                eliminarProcesoPorCola();
                break;

            case 3:
                eliminarProcesoPorPila();
                break;

            case 4:
                break;
            default:
                cout << "\nOpcion no valida. Intente nuevamente.\n";
                segundosPausa(1);
                break;
        }
    } while (opc != 4);
}

void operacionRAM() { //Verificar direccion
    string nombreProceso;
    int direccion;

    if (memoria.empty()) {
        cout << "ERROR: Debe definir la cantidad de RAM antes de utilizar esta opcion." << endl;
        presionarEnter();
        return;
    }

    bool hayProcesos = false;
    for (const auto& celda : memoria) {
        if (celda != "LIBRE") {
            hayProcesos = true;
            break;
        }
    }
    if (!hayProcesos) {
        cout << "ERROR: No hay procesos en la RAM. Cree un proceso primero." << endl;
        presionarEnter();
        return;
    }

    cleanSc();
    cout << "----Operacion en RAM----\n" << endl;
    cout << "Nombre del proceso: ";
    cin >> nombreProceso;
    cout << "Direccion (en bytes, 0 a " << RAMDisponible-1 << "): ";
    cin >> direccion;

    if (direccion < 0 || direccion >= RAMDisponible) {
        cout << "\nDireccion invalida (fuera de rango de RAM)." << endl;
    } else if (memoria[direccion] == nombreProceso) {
        cout << "\nDireccion valida. Pertenece al proceso '" << nombreProceso << "'." << endl;
    } else {
        cout << "\nDireccion invalida. No pertenece al proceso '" << nombreProceso << "'." << endl;
    }
    presionarEnter();
}

void mostrarRAM() { //Mostrar procesos en RAM 
    if (memoria.empty()) {
        cout << "ERROR: Debe definir la RAM antes de utilizar esta opcion." << endl;
        presionarEnter();
        return;
    }
    cleanSc();
    cout << "----Procesos en RAM----\n" << endl;

    // Mostrar resumen (procesos y bytes ocupados)
    vector<pair<string,int>> resumen;
    string actual = memoria[0];
    int count = 1;
    int inicio = 0;

    vector<tuple<char,int,int,string>> detalle; 
    // (P/H, inicio, longitud, nombreProceso)

    for (size_t i = 1; i < memoria.size(); i++) {
        if (memoria[i] == actual) {
            count++;
        } else {
            if (actual == "LIBRE")
                detalle.push_back({'H', inicio, count, ""});
            else
                detalle.push_back({'P', inicio, count, actual});
            resumen.push_back({actual, count});
            actual = memoria[i];
            inicio = i;
            count = 1;
        }
    }
    // último bloque
    if (actual == "LIBRE")
        detalle.push_back({'H', inicio, count, ""});
    else
        detalle.push_back({'P', inicio, count, actual});
    resumen.push_back({actual, count});

    // Mostrar resumen simple
    for (auto &p : resumen) {
        cout << p.first << " -> " << p.second << " bytes" << endl;
    }

    cout << "\nRAM Total: " << RAMDisponible << " bytes" << endl;

    // Mostrar formato P/H con dirección y longitud
    cout << "\nLista de bloques (Procesos y Huecos):\n" << endl;
    cout << left << setw(3) << "T" << setw(10) << "Inicio" << setw(10) << "Longitud" << "Nombre\n";
    cout << "-------------------------------------\n";
    for (auto &d : detalle) {
        char tipo = get<0>(d);
        int ini   = get<1>(d);
        int len   = get<2>(d);
        string nombre = get<3>(d);
        cout << setw(3) << tipo 
             << setw(10) << ini 
             << setw(10) << len 
             << (tipo == 'P' ? nombre : "-") << endl;
    }

    // Mostrar mapa de bits
    cout << "\nMapa de bits (0 = Libre, 1 = Ocupado):\n" << endl;
    for (size_t f = 0; f < mapaBits.size(); f++) {
        cout << setw(5) << f << " | ";
        for (size_t c = 0; c < mapaBits[f].size(); c++) {
            if (f*8 + c < RAMDisponible) // Evitar mostrar posiciones fuera de RAM
                cout << mapaBits[f][c] << " ";
        }
        cout << endl;
    }

    presionarEnter();
}

void menu() { //MENU general 
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "----Practica RAM (Mapa de Bits)----" << endl << endl;
        cout << "\tMENU\n" << endl;
        cout << "1. Definir cantidad de RAM" << endl;
        cout << "2. Operacion en RAM" << endl;
        cout << "3. Crear proceso"<< endl;
        cout << "4. Mostrar procesos en RAM" << endl;
        cout << "5. Eliminar proceso" << endl;
        cout << "6. Salir\n"<< endl; 	
        cout << "Opcion que desea: ";
        cin >> opc;
        cout << endl;
        switch (opc) {
            case 1:
                definirRAM();
                break;
            case 2: 
                operacionRAM();
                break;
            case 3:
                crearProceso();
                break;
            case 4:
                mostrarRAM();
                break;
            case 5:
                eliminarProceso();
                break;
            case 6:
                cout << "Saliendo del programa...\n" << endl;
                break;
            default:
                cout << "Opcion no valida... Ingrese nuevamente\n" << endl;
                segundosPausa(1);
                break;
        }
    } while(opc != 6);
}
