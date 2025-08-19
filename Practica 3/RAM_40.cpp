#include <iostream>
#include <stdlib.h>
#include <map>
#include <iomanip>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

//DECLARACION DE FUNCIONES
void segundosPausa(int segundos);
void cleanSc();
void presionarEnter();
void menu();
void definirManejo();
void crearProceso();
void eliminarProceso();
void estadistica();
void reiniciarPrograma();

//VARIABLES GLOBALES
int RAMDisponible = 40; 
int cantBloques = 0;
string manejoRAM = " ";
int tamBloque = 0;
int bloquesLibres = 0;
vector<string> memoria;
int contadorProcesos = 0;

//PROGRAMA PRINCIPAL
int main() {
    menu();
    return 0;
}

//FUNCIONES
void segundosPausa(int segundos) { //Pausa de segundos
    #ifdef _WIN32
        sleep(segundos * 1000);
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
    cin.ignore();
    cin.get();
}

void menu() { //MENU general
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "\n----Practica RAM----\n" << endl;
        cout << "1. Definir manejo de RAM" << endl;
        cout << "2. Crear proceso" << endl;
        cout << "3. Eliminar proceso" << endl;
        cout << "4. Estadistica" << endl;
        cout << "5. Reiniciar programa" << endl;
        cout << "6. Salir\n" << endl;
        cout << "Opcion que desea: ";
        cin >> opc;
        cout << endl;
        switch(opc) {
            case 1:
                definirManejo();
                break;
            case 2:
                crearProceso();
                break;
            case 3:
                eliminarProceso();
                break;
            case 4:
                estadistica();
                break;
            case 5:   
                reiniciarPrograma();
                break;
            case 6:
                cout << "Saliendo del programa...\n" << endl;
                break;
            default:
                cout << "Opcion no valida... Ingrese nuevamente\n" << endl;
                segundosPausa(2);
        }
    } while (opc != 6);
}

void definirManejo() { //Definir manejo de RAM
    int opcion;
    if (manejoRAM == " ") {
        do {
            cleanSc();
            cout << "----Definir Manejo de RAM----\n" << endl;
            cout << "1. Por Bytes" << endl;
            cout << "2. Por Bloques" << endl;
            cout << "Seleccione una opcion: ";
            cin >> opcion;
            switch (opcion) {
                case 1:
                    manejoRAM = "Por Bytes";
                    memoria = vector<string>(RAMDisponible, "0");
                    cout << "\nManejo de RAM definido como 'Por Bytes'." << endl;
                    presionarEnter();
                    break;
                case 2:
                    manejoRAM = "Por Bloques";
                    do {
                        cout << "\nIngrese la cantidad de bloques que desea:" << endl;
                        cin >> cantBloques;
                        if (cantBloques <= 0 || cantBloques > RAMDisponible) {
                            cout << "\nERROR: Cantidad de bloques invalida. Debe ser mayor que 0 y menor o igual a " << RAMDisponible << "..." << endl;
                            segundosPausa(2);
                            break;
                        }
                    } while (cantBloques <= 0 || cantBloques > RAMDisponible);
                    memoria = vector<string>(cantBloques, "0");
                    tamBloque = RAMDisponible / cantBloques;
                    cout << "\nManejo de RAM definido como 'Por Bloques'." << endl;
                    cout << "\nCantidad de bloques: " << cantBloques << " de " << tamBloque << " bytes cada uno." << endl;
                    presionarEnter();
                    break;
                default:
                    cout << "\nOpcion no valida. Intente nuevamente." << endl;
                    segundosPausa(2);
                    break;
            }
        } while(opcion != 1 && opcion != 2);    
    } else {
        cout << "ERROR: El manejo de RAM ya ha sido definido." << endl;
        cout << "Si desea cambiar los valores, reinicie el programa." << endl;
        segundosPausa(2);
    }
}

void crearProceso() {
    string nombreProceso;
    int tamProceso;
    int asignados = 0;
    bool procesoExiste = false;

    if (manejoRAM == " ") {
        cout << "ERROR: Debe definir el manejo de RAM antes de crear un proceso." << endl;
        segundosPausa(2);
        return;
    }

    // Verificar si la memoria está llena antes de pedir datos
    if (manejoRAM == "Por Bytes") {
        int libres = 0;
        for (const auto& b : memoria) if (b == "0") libres++;
        if (libres == 0) {
            cout << "La memoria está llena. No se pueden crear más procesos." << endl;
            presionarEnter();
            return;
        }
    } else if (manejoRAM == "Por Bloques") {
        int bloquesLibres = 0;
        for (const auto& bloque : memoria) if (bloque == "0") bloquesLibres++;
        if (bloquesLibres == 0) {
            cout << "La memoria está llena. No se pueden crear más procesos." << endl;
            presionarEnter();
            return;
        }
    }

    cleanSc();
    
    // Asignar nombre automático
    nombreProceso = "T" + to_string(contadorProcesos);
    contadorProcesos++;

    cout << "----Crear Proceso----\n" << endl;
    cout << "Nombre del proceso asignado: " << nombreProceso << endl;

    cout << "Tamano del proceso (en bytes): ";
    cin >> tamProceso;
    if (tamProceso <= 0 || tamProceso > RAMDisponible) {
        cout << "\nERROR: Tamano invalido. Debe ser mayor que 0 y menor o igual a " << RAMDisponible << " bytes." << endl;
        presionarEnter();
        return;
    }

    if (manejoRAM == "Por Bytes") {
        // Contar bytes libres
        int libres = 0;
        for (const auto& b : memoria) if (b == "0") libres++;
        if (tamProceso > libres) {
            cout << "\nERROR: No hay suficiente memoria disponible para el proceso." << endl;
            presionarEnter();
            return;
        }
        // Asignar bytes
        asignados = 0;
        for (size_t i = 0; i < memoria.size() && asignados < tamProceso; ++i) {
            if (memoria[i] == "0") {
                memoria[i] = nombreProceso;
                asignados++;
            }
        }
        RAMDisponible -= tamProceso;
        cout << "\nProceso '" << nombreProceso << "' creado con " << tamProceso << " byte(s)." << endl;
    } else if (manejoRAM == "Por Bloques") {
        int bloquesNecesarios = (tamProceso + tamBloque - 1) / tamBloque;
        int bloquesLibres = 0;
        for (const auto& bloque : memoria) if (bloque == "0") bloquesLibres++;
        if (bloquesNecesarios > bloquesLibres) {
            cout << "\nNo hay suficiente memoria para el proceso." << endl;
            presionarEnter();
            return;
        }
        // Asignar bloques
        asignados = 0;
        for (size_t i = 0; i < memoria.size() && asignados < bloquesNecesarios; ++i) {
            if (memoria[i] == "0") {
                memoria[i] = nombreProceso;
                asignados++;
            }
        }
        RAMDisponible -= bloquesNecesarios * tamBloque;
        cout << "\nProceso '" << nombreProceso << "' creado con " << bloquesNecesarios << " bloque(s)." << endl;
    }
    presionarEnter();
}

void eliminarProceso() {
    if (manejoRAM == " ") {
        cout << "\nERROR: Debe definir el manejo de RAM antes de eliminar un proceso." << endl;
        segundosPausa(2);
        return;
    }
    if (memoria.empty()) {
        cout << "\nNo hay procesos en la RAM para eliminar." << endl;
        segundosPausa(2);
        return;
    }

    int opcion;
    cleanSc();
    cout << "----Eliminar Proceso----\n" << endl;
    cout << "1. Eliminar por Pila." << endl;
    cout << "2. Eliminar por Cola." << endl;
    cout << "\nSeleccione una opcion: ";
    cin >> opcion;

    // Obtener el orden de llegada de los procesos
    vector<string> ordenProcesos;
    map<string, bool> yaAgregado;
    for (const auto& bloque : memoria) {
        if (bloque != "0" && !yaAgregado[bloque]) {
            ordenProcesos.push_back(bloque);
            yaAgregado[bloque] = true;
        }
    }
    if (ordenProcesos.empty()) {
        cout << "\nNo hay procesos en la RAM para eliminar." << endl;
        segundosPausa(2);
        return;
    }

    string procesoEliminar;
    if (opcion == 1) { // Pila
        procesoEliminar = ordenProcesos.back();
        cout << "\nEliminando proceso (Pila): " << procesoEliminar << endl;
    } else if (opcion == 2) { // Cola
        procesoEliminar = ordenProcesos.front();
        cout << "\nEliminando proceso (Cola): " << procesoEliminar << endl;
    } else {
        cout << "\nOpcion no valida." << endl;
        segundosPausa(2);
        return;
    }

    int liberados = 0;
    for (size_t i = 0; i < memoria.size(); ++i) {
        if (memoria[i] == procesoEliminar) {
            memoria[i] = "0";
            if (manejoRAM == "Por Bytes") {
                liberados++;
            } else {
                liberados += tamBloque;
            }
        }
    }
    if (manejoRAM == "Por Bytes") {
        RAMDisponible += liberados;
        cout << "\nProceso '" << procesoEliminar << "' eliminado. Se liberaron " << liberados << " byte(s)." << endl;
    } else {
        RAMDisponible += liberados;
        int bloquesEliminados = liberados / tamBloque;
        cout << "\nProceso '" << procesoEliminar << "' eliminado. Se liberaron " << bloquesEliminados << " bloque(s) (" << liberados << " bytes)." << endl;
    }
    presionarEnter();
}

void estadistica() {
    int opc;
    int totalBytes = 0, totalBloques = 0, totalDesperdiciado = 0;
    if (manejoRAM == " ") {
        cout << "ERROR: Debe definir el manejo de RAM antes de ver las estadisticas." << endl;
        segundosPausa(2);
        return;
    } else {
        cleanSc();
        do {
            cout << "----Estadisticas de RAM----\n" << endl;
            cout << "1. Estado de la RAM" << endl;
            cout << "2. Lista de Procesos" << endl;
            cout << "Seleccione una opcion: ";
            cin >> opc;
            switch (opc) {
                case 1:
                    if (manejoRAM == "Por Bytes") {
                        cleanSc();
                        cout << "----Procesos en RAM----\n" << endl;
                        cout << "Estado de la RAM: [";
                        for (size_t i = 0; i < memoria.size(); ++i) {
                            cout << "'" << memoria[i] << "'";
                            if (i != memoria.size() - 1) {
                                cout << ", ";
                            }
                        }
                        cout << "]" << endl;    
                        cout << "\nEstado de la RAM: " << RAMDisponible << " bytes disponibles." << endl;
                        presionarEnter();
                    } else {
                        cleanSc();
                        cout << "----Procesos en RAM----\n" << endl;
                        cout << "Estado de la RAM: [";
                        for (size_t i = 0; i < memoria.size(); ++i) {
                            cout << "'" << memoria[i] << "'";
                            if (i != memoria.size() - 1) {
                                cout << ", ";
                            }
                        }
                        cout << "]" << endl;
                        cout << "\nCantidad de bloques: " << cantBloques << " de " << tamBloque << " bytes cada uno." << endl;
                        cout << "RAM Disponible: " << RAMDisponible << " bytes" << endl;
                        presionarEnter();
                    }
                    break;
                case 2:
                    cleanSc();
                    cout << "----Lista de Procesos----\n" << endl;
                    if (memoria.empty()) {
                        cout << "No hay procesos en la RAM." << endl;
                    } else {
                        map<string, int> contador;
                        for (const auto& bloque : memoria) {
                            if (bloque != "0") contador[bloque]++;
                        }
                        if (contador.empty()) {
                            cout << "No hay procesos en la RAM." << endl;
                        } else {
                            cout << left << setw(12) << "PID" << setw(12) << "Bytes" << setw(12) << "Bloques" << endl;
                            cout << string(36, '-') << endl;
                            for (const auto& par : contador) {
                                cout << left << setw(12) << par.first;
                                if (manejoRAM == "Por Bytes") {
                                    cout << setw(12) << par.second << setw(12) << 0 << endl; 
                                    totalBytes += par.second;
                                    totalBloques += 0;
                                } else {
                                    cout << setw(12) << par.second * tamBloque << setw(12) << par.second << endl;
                                    totalBytes += par.second * tamBloque;
                                    totalBloques += par.second;
                                }
                            }
                            cout << setw(12) << "\nTotal Byte(s): " << totalBytes << "\nTotal Bloques: " << totalBloques << "\nTotal Desperdiciado: " << RAMDisponible << endl; 
                        }
                    }
                    presionarEnter();
                    break;
                default:
                    cout << "\nOpcion no valida. Intente nuevamente." << endl;
                    segundosPausa(2);
                    break;
            }
        } while (opc != 1 && opc != 2); 
    }

}

void reiniciarPrograma() {
    string respuesta;
    cout << "¿Esta seguro de que desea reiniciar el programa? (s/n): ";
    cin >> respuesta;
    if (respuesta == "s" || respuesta == "S") {
        RAMDisponible = 40;
        manejoRAM = " ";
        cantBloques = 0;
        tamBloque = 0;
        memoria.clear();
        contadorProcesos = 0;
        cout << "\nPrograma reiniciado. Puede definir nuevamente el manejo de RAM." << endl;
        segundosPausa(2);      
    } else if (respuesta == "n" || respuesta == "N") {
        cout << "\nReinicio cancelado..." << endl;
        segundosPausa(2);
    }
}

