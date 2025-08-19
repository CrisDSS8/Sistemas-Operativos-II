#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
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
void definirBloque();
void operacionRAM();
void crearProceso();
void mostrarRAM();
void eliminarProceso();
void compactarMemoria();

//VARIABLES GLOBALES
int tamBloque = 0;
int RAMDisponible = 0;
vector<string> memoria;
vector<string> historialProcesos; // Para FIFO y LIFO

//PROGRAMA PRINCIPAL
int main () {
    menu();
    return 0;
}

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

void definirBloque() { //Definir bloque de memoria
    int numBloques;
    if (memoria.empty()) {
        cleanSc();
        cout << "----Definir Bloque de Memoria----\n" << endl;
        cout << "Ingrese el tamano de la RAM disponible (en bytes): ";
        cin >> RAMDisponible;

        cout << "Ingrese el tamano de cada bloque (en bytes): ";
        cin >> tamBloque;

        if (RAMDisponible <= 0 || tamBloque <= 0 || tamBloque > RAMDisponible) {
            cout << "Tamano invalido. Debe ser mayor que 0 y menor al total de RAM." << endl;
            return;
        }

        numBloques = RAMDisponible / tamBloque;
        memoria = vector<string>(numBloques, "LIBRE");

        cout << "\nRAM Inicializada con " << numBloques << " bloques de " << tamBloque << " bytes." << endl;
        presionarEnter();
    } else {
        cout << "ERROR: La Memoria RAM y el tamano de bloques ya han sido definidos." << endl;
        cout << "Si desea cambiar los valores, reinicie el programa." << endl;
        presionarEnter();
        return;
    }
}

void crearProceso() { //Crear un proceso en la RAM
    string nombreProceso;
    int tamProceso;
    bool procesoExiste = false;
    if (memoria.empty() || tamBloque == 0) {
        cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
        presionarEnter();
    } else {
        cleanSc();
        do {
            cout << "----Crear Proceso----\n" << endl;
            cout << "Nombre del proceso: ";
            cin >> nombreProceso;
            procesoExiste = false;
            for (size_t i = 0; i < memoria.size(); ++i) {
                if (memoria[i] == nombreProceso) {
                    procesoExiste = true;
                    cout << "\nERROR: El proceso '" << nombreProceso << "' ya existe en la RAM." << endl;
                    segundosPausa(2);
                    cleanSc();
                    break;
                }
            }
        } while(procesoExiste == true);

        cout << "Tamano del proceso (en bytes): ";
        cin >> tamProceso;

        int bloquesNecesarios = (tamProceso + tamBloque - 1) / tamBloque;
        int bloquesLibres = 0;

        for (const auto &bloque : memoria) {
            if (bloque == "LIBRE") bloquesLibres++;
        }

        if (bloquesLibres < bloquesNecesarios) {
            cout << "\nNo hay suficiente memoria para el proceso." << endl;
        } else {
            int asignados = 0;
            for (size_t i = 0; i < memoria.size() && asignados < bloquesNecesarios; ++i) {
                if (memoria[i] == "LIBRE") {
                    memoria[i] = nombreProceso;
                    asignados++;
                }
            }
            historialProcesos.push_back(nombreProceso); // Guardamos el orden
            cout << "\nProceso '" << nombreProceso << "' creado con " << bloquesNecesarios << " bloque(s)." << endl;
        }
        presionarEnter();
    }
}

void operacionRAM() { //Verificar si una direccion pertenece a un proceso
    string nombreProceso;
    int direccion;

    if (memoria.empty() || tamBloque == 0) {
        cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
        presionarEnter();
    } else {
        //Verificar si hay procesos en memoria
        bool hayProcesos = false;
        for (const auto& bloque : memoria) {
            if (bloque != "LIBRE") {
                hayProcesos = true;
                break;
            }
        }

        if (!hayProcesos) {
            cout << "Memoria RAM totalmente libre. No hay procesos cargados." << endl;
            presionarEnter();
            return;
        }

        //Si hay procesos, continuar con la operacion
        cleanSc();
        cout << "----Operacion en RAM----\n" << endl;
        cout << "Nombre del proceso: ";
        cin >> nombreProceso;
        cout << "Direccion (en bytes): ";
        cin >> direccion;

        int bloque = direccion / tamBloque;

        if (bloque < 0 || bloque >= memoria.size()) {
            cout << "\nDireccion invalida (fuera de rango de RAM)." << endl;
        } else if (memoria[bloque] == nombreProceso) {
            cout << "\nDireccion valida. Pertenece al proceso '" << nombreProceso << "'." << endl;
        } else {
            cout << "\nDireccion invalida. No pertenece al proceso '" << nombreProceso << "'." << endl;
        }
        presionarEnter();
    }
}

void mostrarRAM() { //Mostrar procesos en RAM 
    if (memoria.empty() || tamBloque == 0) {
        cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
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
        cout << "\nTamano de cada bloque: " << tamBloque << " bytes" << endl;
        cout << "RAM Disponible: " << RAMDisponible << " bytes" << endl;
    }
    presionarEnter();
}

// FUNCION DE COMPACTACION
void compactarMemoria() {
    vector<string> memoriaCompactada(memoria.size(), "LIBRE");
    int idx = 0;

    for (size_t i = 0; i < memoria.size(); i++) {
        if (memoria[i] != "LIBRE") {
            memoriaCompactada[idx] = memoria[i];
            idx++;
        }
    }
    memoria = memoriaCompactada;
    cout << "\nMemoria compactada exitosamente." << endl;
}

// FUNCION DE ELIMINAR PROCESO
void eliminarProceso() {
    if (memoria.empty() || tamBloque == 0) {
        cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
        presionarEnter();
        return;
    }

    int opcion;
    cleanSc();
    cout << "----Eliminar Proceso----\n" << endl;
    cout << "1. Eliminar por nombre" << endl;
    cout << "2. Eliminar ultimo proceso (LIFO)" << endl;
    cout << "3. Eliminar primer proceso (FIFO)" << endl;
    cout << "Opcion: ";
    cin >> opcion;

    string nombreEliminar = "";
    bool encontrado = false;

    switch (opcion) {
        case 1: {
            cout << "Ingrese el nombre del proceso a eliminar: ";
            cin >> nombreEliminar;
            break;
        }
        case 2: {
            if (!historialProcesos.empty()) {
                nombreEliminar = historialProcesos.back();
                historialProcesos.pop_back();
                cout << "Eliminando proceso (LIFO): " << nombreEliminar << endl;
            }
            break;
        }
        case 3: {
            if (!historialProcesos.empty()) {
                nombreEliminar = historialProcesos.front();
                historialProcesos.erase(historialProcesos.begin());
                cout << "Eliminando proceso (FIFO): " << nombreEliminar << endl;
            }
            break;
        }
        default:
            cout << "Opcion invalida." << endl;
            presionarEnter();
            return;
    }

    if (nombreEliminar == "") {
        cout << "No hay procesos para eliminar." << endl;
        presionarEnter();
        return;
    }

    for (size_t i = 0; i < memoria.size(); i++) {
        if (memoria[i] == nombreEliminar) {
            memoria[i] = "LIBRE";
            encontrado = true;
        }
    }

    if (encontrado) {
        cout << "Proceso '" << nombreEliminar << "' eliminado correctamente." << endl;
        compactarMemoria(); // compactamos despues de eliminar
    } else {
        cout << "No se encontro el proceso '" << nombreEliminar << "' en memoria." << endl;
    }

    presionarEnter();
}

// MENU PRINCIPAL
void menu() { 
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "----Practica RAM----" << endl << endl;
		cout << "\tMENU\n" << endl;
		cout << "1. Definir cantidad de RAM y tamano de bloques" << endl;
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
			    definirBloque(); 
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
