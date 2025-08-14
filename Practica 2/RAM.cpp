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
void definirBloque();
void operacionRAM();
void crearProceso();

//VARIABLES GLOBALES
int tamBloque = 0;
int RAMDisponible = 0;
vector<string> memoria;

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
}

void crearProceso() { //Crear un proceso en la RAM
    string nombreProceso;
    int tamProceso;
    cleanSc();
    cout << "----Crear Proceso----\n" << endl;
    cout << "Nombre del proceso: ";
    cin >> nombreProceso;
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
        cout << "\nProceso '" << nombreProceso << "' creado con " << bloquesNecesarios << " bloques." << endl;
    }
    presionarEnter();
}

void operacionRAM() { //Verificar si una direccion pertenece a un proceso
    string nombreProceso;
    int direccion;

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

void menu() { //MENU general 
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "----Practica RAM----" << endl << endl;
		cout << "\tMENU\n" << endl;
		cout << "1. Definir cantidad de RAM y tamano de bloques" << endl;
		cout << "2. Operacion en RAM" << endl;
		cout << "3. Crear proceso"<< endl;
		cout << "4. Salir\n"<< endl; 	
		cout << "Opcion que desea: ";
		cin >> opc;
		cout << endl;
		switch (opc) {
			case 1:
                definirBloque();
				break;
			case 2: 
                if (RAMDisponible == 0 || tamBloque == 0) {
                    cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
                    presionarEnter();
                }
                else {
                    operacionRAM();
                }
				break;
			case 3:
            if (RAMDisponible == 0 || tamBloque == 0) {
                    cout << "ERROR: Debe definir la cantidad de RAM y el tamano de bloques antes de utilizar esta opcion." << endl;
                    presionarEnter();
                }
                else {
                    crearProceso();
                }
				break;
			case 4:
                cout << "Saliendo del programa...\n" << endl;
				break;
			default:
				cout << "Opcion no valida... Ingrese nuevamente\n" << endl;
				segundosPausa(1);
				break;
		}
    } while(opc != 4);
}
