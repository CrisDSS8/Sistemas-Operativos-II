#include <iostream>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

//DECLARACION DE FUNCIONES
void menu();
void cleanSc();
int definirBloque();
void operacionRAM();
int crearProceso();

//VARIABLES GLOBALES
int CantBloques = 0;

//PROGRAMA PRINCIPAL
int main () {
    menu();
    return 0;
}

void segundosPausa(int segundos) {
    #ifdef _WIN32
        sleep(segundos * 1000);
    #else
        sleep(segundos);
    #endif
}

void cleanSc() { //Limpiar pantalla
    #ifdef _WIN32
        system("cls")
    #else 
        system("clear");
    #endif
}

void presionarEnter() {
    cout << "\nPresiona Enter para continuar...";
    cin.ignore();
    cin.get();
}

void menu() { //MENU general 
    int opc;
    do {
        cleanSc();
        opc = 0;
        cout << "----Practica RAM----" << endl << endl;
		cout << "\tMENU\n" << endl;
		cout << "1. Definir bloque" << endl;
		cout << "2. Operacion en RAM" << endl;
		cout << "3. Crear proceso"<< endl;
		cout << "4. Salir\n"<< endl; 	
		cout << "Opcion que desea: ";
		cin >> opc;
		cout << endl;
		switch (opc) {
			case 1:
                //definirBloque();
				break;
			case 2:
                //operacionRAM();
				break;
			case 3:
                //crearProceso();
				break;
			case 4:
                cout << "Saliendo del programa...\n" << endl;
				break;
			default:
				cout << "Opcion no valida... Ingrese nuevamente\n" << endl;
				segundosPausa(2);
				break;
		}
    } while(opc != 4);
}