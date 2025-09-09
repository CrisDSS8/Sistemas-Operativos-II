#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

// === ESTRUCTURAS ===
struct EntradaTablaPagina {
    int pagina;
    int posicionFisica;
};

// === VARIABLES GLOBALES ===
int tamRAM = 0;
int tamPagina = 0;
vector<string> memoriaFisica;
map<string, vector<EntradaTablaPagina>> tablasDePaginas;
int contadorProcesos = 1;

// === FUNCIONES ===
void cleanSc() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void segundosPausa(int segundos) {
    #ifdef _WIN32
        Sleep(segundos * 1000);
    #else
        sleep(segundos);
    #endif
}

void presionarEnter() {
    cout << "\nPresione Enter para continuar...";
    cin.ignore();
    cin.get();
}

void definirMemoria() {
    if (!memoriaFisica.empty()) {
        cout << "ERROR: La memoria ya fue definida. Reinicia el programa para redefinirla.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    cout << "----Definir Memoria----\n\n";
    cout << "Tamaño total de la RAM (en bytes): ";
    cin >> tamRAM;
    cout << "Tamaño de cada página (en bytes): ";
    cin >> tamPagina;

    if (tamRAM <= 0 || tamPagina <= 0 || tamPagina > tamRAM) {
        cout << "Tamaños inválidos. Intenta nuevamente.\n";
        presionarEnter();
        return;
    }

    int totalPaginas = tamRAM / tamPagina;
    memoriaFisica = vector<string>(totalPaginas, "LIBRE");

    cout << "\nRAM inicializada con " << totalPaginas << " páginas de " << tamPagina << " bytes.\n";
    presionarEnter();
}

void crearProceso() {
    if (memoriaFisica.empty()) {
        cout << "ERROR: Define la memoria primero.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    string nombre = "P" + to_string(contadorProcesos++);
    int tamProceso;
    cout << "----Crear Proceso----\n\n";
    cout << "Nombre del proceso: " << nombre << endl;
    cout << "Tamaño del proceso (en bytes): ";
    cin >> tamProceso;

    if (tamProceso <= 0) {
        cout << "Tamaño inválido.\n";
        presionarEnter();
        return;
    }

    int paginasNecesarias = ceil((float)tamProceso / tamPagina);
    int paginasLibres = 0;
    for (const auto& p : memoriaFisica) if (p == "LIBRE") paginasLibres++;

    if (paginasNecesarias > paginasLibres) {
        cout << "No hay suficientes páginas libres.\n";
        presionarEnter();
        return;
    }

    vector<EntradaTablaPagina> tabla;
    int asignadas = 0;

    for (int i = 0; i < memoriaFisica.size() && asignadas < paginasNecesarias; ++i) {
        if (memoriaFisica[i] == "LIBRE") {
            memoriaFisica[i] = nombre;
            tabla.push_back({asignadas, i});
            asignadas++;
        }
    }

    tablasDePaginas[nombre] = tabla;
    cout << "\nProceso '" << nombre << "' creado con " << paginasNecesarias << " páginas.\n";
    presionarEnter();
}

void traducirDireccionLogica() {
    if (tablasDePaginas.empty()) {
        cout << "ERROR: No hay procesos cargados.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    cout << "----Traducción de Dirección Lógica----\n\n";
    string nombre;
    int pagina, desplazamiento;

    cout << "Nombre del proceso: ";
    cin >> nombre;

    if (tablasDePaginas.find(nombre) == tablasDePaginas.end()) {
        cout << "Proceso no encontrado.\n";
        presionarEnter();
        return;
    }

    cout << "Número de página: ";
    cin >> pagina;
    cout << "Desplazamiento (0 - " << tamPagina - 1 << "): ";
    cin >> desplazamiento;

    if (desplazamiento < 0 || desplazamiento >= tamPagina) {
        cout << "Desplazamiento inválido.\n";
        presionarEnter();
        return;
    }

    auto tabla = tablasDePaginas[nombre];
    bool encontrada = false;
    int posicionFisica = -1;

    for (const auto& entrada : tabla) {
        if (entrada.pagina == pagina) {
            posicionFisica = entrada.posicionFisica;
            encontrada = true;
            break;
        }
    }

    if (!encontrada) {
        cout << "Página no encontrada para ese proceso.\n";
    } else {
        int dirFisica = posicionFisica * tamPagina + desplazamiento;
        cout << "\nDirección física = Página física " << posicionFisica
             << " * " << tamPagina << " + Desplazamiento(" << desplazamiento
             << ") = " << dirFisica << " bytes" << endl;
    }

    presionarEnter();
}

void mostrarRAM() {
    if (memoriaFisica.empty()) {
        cout << "ERROR: Define la memoria primero.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    cout << "----Estado de la Memoria (Paginación)----\n\n";
    for (int i = 0; i < memoriaFisica.size(); ++i) {
        cout << "Página física " << i << ": " << memoriaFisica[i] << endl;
    }

    presionarEnter();
}

void mostrarTablasDePaginas() {
    cleanSc();
    cout << "----Tablas de Páginas por Proceso----\n\n";

    if (tablasDePaginas.empty()) {
        cout << "No hay procesos cargados.\n";
    } else {
        for (const auto& par : tablasDePaginas) {
            cout << "Proceso: " << par.first << endl;
            for (const auto& entrada : par.second) {
                cout << "  Página lógica " << entrada.pagina << " → Página física " << entrada.posicionFisica << endl;
            }
            cout << endl;
        }
    }

    presionarEnter();
}

void menu() {
    int opc;
    do {
        cleanSc();
        cout << "---- Simulador de Memoria con Paginación ----\n\n";
        cout << "1. Definir Memoria\n";
        cout << "2. Crear Proceso\n";
        cout << "3. Traducir Dirección Lógica a Física\n";
        cout << "4. Mostrar Estado de Memoria\n";
        cout << "5. Ver Tablas de Páginas\n";
        cout << "6. Salir\n\n";
        cout << "Seleccione una opción: ";
        cin >> opc;

        switch (opc) {
            case 1: definirMemoria(); break;
            case 2: crearProceso(); break;
            case 3: traducirDireccionLogica(); break;
            case 4: mostrarRAM(); break;
            case 5: mostrarTablasDePaginas(); break;
            case 6: cout << "Saliendo del programa...\n"; break;
            default: cout << "Opción inválida.\n"; segundosPausa(1); break;
        }
    } while (opc != 6);
}

int main() {
    menu();
    return 0;
}
