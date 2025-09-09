#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm> 
#include <cmath>
#include <cstdlib>
#include <ctime>
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

int errores = 0;
const int maxErrores = 5;
const float probError = 1;
const int maxPaginasTotales = 20;

// === FUNCIONES AUXILIARES ===
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

// === DEFINIR MEMORIA ===
void definirMemoria() {
    if (!memoriaFisica.empty()) {
        cout << "ERROR: La memoria ya fue definida.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    cout << "---- Definir Memoria ----\n\n";
    cout << "Tamaño total de la RAM (bytes): ";
    cin >> tamRAM;
    cout << "Tamaño de cada página (bytes): ";
    cin >> tamPagina;

    if (tamRAM <= 0 || tamPagina <= 0 || tamPagina > tamRAM) {
        cout << "Valores inválidos.\n";
        presionarEnter();
        return;
    }

    int totalPaginas = tamRAM / tamPagina;
    if (totalPaginas > maxPaginasTotales) {
        totalPaginas = maxPaginasTotales;
        cout << "Se limitaron las páginas a " << maxPaginasTotales << " por simulación.\n";
    }

    memoriaFisica = vector<string>(totalPaginas, "LIBRE");

    cout << "\nRAM inicializada con " << totalPaginas << " páginas de " << tamPagina << " bytes.\n";
    presionarEnter();
}

// === CREAR PROCESO ===
void crearProceso() {
    if (memoriaFisica.empty()) {
        cout << "ERROR: Define la memoria primero.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    string nombre = "P" + to_string(contadorProcesos++);
    int tamProceso;
    cout << "---- Crear Proceso ----\n\n";
    cout << "Nombre del proceso: " << nombre << endl;
    cout << "Tamaño del proceso (bytes): ";
    cin >> tamProceso;

    if (tamProceso <= 0) {
        cout << "Tamaño inválido.\n";
        presionarEnter();
        return;
    }

    int paginasNecesarias = ceil((float)tamProceso / tamPagina);
    int paginasLibres = count(memoriaFisica.begin(), memoriaFisica.end(), "LIBRE");

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

void reemplazarPagina(const string& proceso) {
    cout << "\n== Reemplazo de Página ==\n";

    auto& tabla = tablasDePaginas[proceso];
    if (tabla.empty()) {
        cout << "El proceso no tiene páginas para reemplazar.\n";
        return;
    }

    // Mostrar páginas físicas actuales del proceso
    cout << "Páginas físicas actuales del proceso " << proceso << ":\n";
    for (size_t i = 0; i < tabla.size(); ++i) {
        cout << "  [" << i << "] Lógica " << tabla[i].pagina 
             << " → Física " << tabla[i].posicionFisica << "\n";
    }

    int logicaAReemplazar;
    cout << "\nIngresa el número de página lógica que deseas reemplazar: ";
    cin >> logicaAReemplazar;

    int indiceReemplazo = -1;
    for (size_t i = 0; i < tabla.size(); ++i) {
        if (tabla[i].pagina == logicaAReemplazar) {
            indiceReemplazo = i;
            break;
        }
    }

    if (indiceReemplazo == -1) {
        cout << "Página lógica no encontrada en el proceso.\n";
        return;
    }

    cout << "\nSelecciona tipo de reemplazo:\n";
    cout << "  1. Reemplazar por una página física existente (disponible)\n";
    cout << "  2. Crear nueva página física (expandir RAM)\n";
    cout << "Opción: ";
    int opcion;
    cin >> opcion;

    if (opcion == 1) {
        // Mostrar páginas físicas disponibles
        vector<int> disponibles;
        for (size_t i = 0; i < memoriaFisica.size(); ++i) {
            if (memoriaFisica[i] == "LIBRE") {
                disponibles.push_back(i);
            }
        }

        if (disponibles.empty()) {
            cout << "No hay páginas libres disponibles para reemplazo.\n";
            return;
        }

        cout << "\nPáginas físicas disponibles:\n";
        for (int pf : disponibles) {
            cout << "  Página física " << pf << "\n";
        }

        int seleccion;
        cout << "Ingresa el número de la página física a usar para reemplazo: ";
        cin >> seleccion;

        if (find(disponibles.begin(), disponibles.end(), seleccion) == disponibles.end()) {
            cout << "Página seleccionada no es válida.\n";
            return;
        }

        // Reemplazo manual
        int paginaAntigua = tabla[indiceReemplazo].posicionFisica;
        memoriaFisica[paginaAntigua] = "LIBRE";
        memoriaFisica[seleccion] = proceso;
        tabla[indiceReemplazo].posicionFisica = seleccion;

        cout << "Reemplazo realizado: página física " << paginaAntigua 
             << " → " << seleccion << " para el proceso " << proceso << ".\n";

    } else if (opcion == 2) {
        // Crear nueva página física
        memoriaFisica.push_back(proceso);
        int nuevaPosicion = memoriaFisica.size() - 1;

        int paginaAntigua = tabla[indiceReemplazo].posicionFisica;
        memoriaFisica[paginaAntigua] = "LIBRE";
        tabla[indiceReemplazo].posicionFisica = nuevaPosicion;

        cout << "Se agregó una nueva página física (" << nuevaPosicion << ") y se usó para el reemplazo.\n";
    } else {
        cout << "Opción inválida.\n";
    }

    presionarEnter();
}

// === TRADUCIR DIRECCIÓN ===
void traducirDireccionLogica() {
    if (tablasDePaginas.empty()) {
        cout << "No hay procesos cargados.\n";
        presionarEnter();
        return;
    }

    cleanSc();
    string nombre;
    int pagina, desplazamiento;

    cout << "---- Traducción de Dirección Lógica ----\n\n";
    cout << "Nombre del proceso: ";
    cin >> nombre;

    if (tablasDePaginas.find(nombre) == tablasDePaginas.end()) {
        cout << "Proceso no encontrado.\n";
        presionarEnter();
        return;
    }

    cout << "Número de página lógica: ";
    cin >> pagina;
    cout << "Desplazamiento (0 - " << tamPagina - 1 << "): ";
    cin >> desplazamiento;

    if (desplazamiento < 0 || desplazamiento >= tamPagina) {
        cout << "Desplazamiento inválido.\n";
        presionarEnter();
        return;
    }

    auto& tabla = tablasDePaginas[nombre];
    int posicionFisica = -1;
    for (auto& entrada : tabla) {
        if (entrada.pagina == pagina) {
            posicionFisica = entrada.posicionFisica;
            break;
        }
    }

    if (posicionFisica == -1) {
        cout << "Página lógica no mapeada.\n";
        presionarEnter();
        return;
    }

    float r = (float)rand() / RAND_MAX;
    if (r < probError) {
        errores++;
        cout << "\n¡Page Fault! Total errores: " << errores << "\n";

        if (errores >= maxErrores) {
            errores = 0;
            reemplazarPagina(nombre);
        } else {
            cout << "Aún no se realiza reemplazo. (Faltan " << (maxErrores - errores) << " errores).\n";
            presionarEnter();
        }
        return;
    }

    int dirFisica = posicionFisica * tamPagina + desplazamiento;
    cout << "\nDirección física: " << dirFisica << " bytes.\n";
    presionarEnter();
}

// === MOSTRAR ESTADO DE RAM ===
void mostrarRAM() {
    cleanSc();
    cout << "---- Estado de la Memoria ----\n\n";
    for (int i = 0; i < memoriaFisica.size(); i++) {
        cout << "Página física " << i << ": " << memoriaFisica[i] << endl;
    }
    presionarEnter();
}

// === MOSTRAR TABLAS DE PÁGINAS ===
void mostrarTablasDePaginas() {
    cleanSc();
    cout << "---- Tablas de Páginas ----\n\n";
    if (tablasDePaginas.empty()) {
        cout << "No hay procesos.\n";
    } else {
        for (auto& par : tablasDePaginas) {
            cout << "Proceso: " << par.first << endl;
            for (auto& entrada : par.second) {
                cout << "  Lógica " << entrada.pagina << " → Física " << entrada.posicionFisica << endl;
            }
            cout << endl;
        }
    }
    presionarEnter();
}

// === MENÚ ===
void menu() {
    srand(time(NULL));
    int opc;
    do {
        cleanSc();
        cout << "---- Simulador de Memoria con Paginación ----\n\n";
        cout << "1. Definir Memoria\n";
        cout << "2. Crear Proceso\n";
        cout << "3. Traducir Dirección Lógica\n";
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
            case 6: cout << "Saliendo...\n"; break;
            default: cout << "Opción inválida.\n"; segundosPausa(1); break;
        }
    } while (opc != 6);
}

int main() {
    menu();
    return 0;
}
