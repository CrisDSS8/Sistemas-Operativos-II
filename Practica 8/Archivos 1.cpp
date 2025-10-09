#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

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

// === VARIABLES GLOBALES ===
int TAM_DISCO = 0; // Tamaño del disco
vector<int> FAT;   // Tabla de asignación de archivos
vector<char> disco;
map<char, int> inicioArchivo;

// === FUNCIONES DE DISCO Y ARCHIVOS ===
void inicializarDisco(int tam) {
    TAM_DISCO = tam;
    FAT.assign(TAM_DISCO, -2);   // -2 = libre
    disco.assign(TAM_DISCO, '.'); // '.' = bloque vacío
    inicioArchivo.clear();
    cout << "\nDisco inicializado con " << TAM_DISCO << " bloques.\n";
}

void crearArchivo(char nombre, int bloques) {
    if (TAM_DISCO == 0) {
        cout << "\nError: primero debes definir el tamaño del disco.\n";
        return;
    }

    int prev = -1;
    int start = -1;
    int disponibles = 0;
    for (char c : disco) if (c == '.') disponibles++;

    if (bloques > disponibles) {
        cout << "\nError: espacio insuficiente en disco (" << disponibles << " libres).\n";
        return;
    }

    cout << "\nCreando archivo '" << nombre << "' con " << bloques << " bloques...\n";

    for (int i = 0; i < TAM_DISCO && bloques > 0; i++) {
        if (disco[i] == '.') {
            disco[i] = nombre;
            if (start == -1)
                start = i;
            if (prev != -1)
                FAT[prev] = i;
            prev = i;
            FAT[i] = -1; // último bloque temporalmente
            bloques--;
        }
    }

    inicioArchivo[nombre] = start;
    cout << "Archivo '" << nombre << "' creado exitosamente.\n";
}

void leerArchivo(char nombre) {
    if (inicioArchivo.find(nombre) == inicioArchivo.end()) {
        cout << "\nEl archivo '" << nombre << "' no existe.\n";
        return;
    }

    cout << "\nLeyendo archivo '" << nombre << "': ";
    int pos = inicioArchivo[nombre];
    while (pos != -1) {
        cout << pos << " ";
        pos = FAT[pos];
    }
    cout << "(fin)\n";
}

void mostrarEstado() {
    if (TAM_DISCO == 0) {
        cout << "\nError: primero debes definir el tamaño del disco.\n";
        return;
    }

    cout << "\n=== Estado del Disco ===\n";
    for (char c : disco) cout << c << " ";
    cout << "\n\n=== Tabla FAT ===\n";
    for (int i = 0; i < TAM_DISCO; i++) {
        cout << setw(2) << i << " ->" << setw(3) << FAT[i] << "  ";
        if ((i + 1) % 5 == 0) cout << "\n";
    }

    cout << "\n\n=== Archivos Creados ===\n";
    for (auto &par : inicioArchivo) {
        cout << par.first << " (inicio en bloque " << par.second << ")\n";
    }

    cout << "\n";
}

void eliminarArchivo(char nombre) {
    if (inicioArchivo.find(nombre) == inicioArchivo.end()) {
        cout << "\nError: el archivo '" << nombre << "' no existe.\n";
        return;
    }

    cout << "\nEliminando archivo '" << nombre << "'...\n";

    // Liberar los bloques ocupados por el archivo
    int pos = inicioArchivo[nombre];
    while (pos != -1) {
        int siguiente = FAT[pos];
        FAT[pos] = -2;      // -2 = libre
        disco[pos] = '.';   // espacio vacío
        pos = siguiente;
    }

    // Eliminar registro del mapa
    inicioArchivo.erase(nombre);

    // === COMPACTAR DISCO ===
    vector<char> nuevoDisco(TAM_DISCO, '.');
    int indiceNuevo = 0;

    // Recorremos los bloques del disco original en orden
    for (int i = 0; i < TAM_DISCO; i++) {
        if (disco[i] != '.') {
            nuevoDisco[indiceNuevo++] = disco[i];
        }
    }

    // Actualizamos FAT y posiciones de inicio
    vector<int> nuevaFAT(TAM_DISCO, -2);
    map<char, int> nuevoInicio;
    int prev = -1;

    for (int i = 0; i < indiceNuevo; i++) {
        char actual = nuevoDisco[i];
        nuevaFAT[i] = -1;

        // Si es el primer bloque de un nuevo archivo
        if (i == 0 || nuevoDisco[i] != nuevoDisco[i - 1]) {
            nuevoInicio[actual] = i;
        }

        // Si el bloque anterior era del mismo archivo, enlazamos en la FAT
        if (i > 0 && nuevoDisco[i] == nuevoDisco[i - 1]) {
            nuevaFAT[i - 1] = i;
        }
    }

    disco = nuevoDisco;
    FAT = nuevaFAT;
    inicioArchivo = nuevoInicio;

    cout << "Archivo eliminado y disco compactado exitosamente.\n";
}


// === MAIN ===
int main() {
    int opcion;
    int tamBits = 0;
    int contadorArchivos = 0; // para nombres automáticos A, B, C, ...

    do {
        cleanSc();
        cout << "====== MENU PRINCIPAL ======\n";
        cout << "1. Definir tamaño del disco (en bits)\n";
        cout << "2. Crear archivo automáticamente (nombre A, B, C...)\n";
        cout << "3. Mostrar estado del disco y leer archivos\n";
        cout << "4. Eliminar archivo\n";
        cout << "5. Salir\n";
        cout << "Selecciona una opción: ";
        cin >> opcion;

        switch (opcion) {
        case 1: {
            cleanSc();
            cout << "\nIngrese el número de bits (bloques) del disco: ";
            cin >> tamBits;
            inicializarDisco(tamBits);
            contadorArchivos = 0;
            presionarEnter();
            break;
        }

        case 2: {
            cleanSc();
            if (TAM_DISCO == 0) {
                cout << "\nPrimero define el tamaño del disco (opción 1).\n";
                presionarEnter();
                break;
            }

            if (contadorArchivos >= 26) {
                cout << "\nError: límite de archivos alcanzado (Z).\n";
                presionarEnter();
                break;
            }

            char nombre = 'A' + contadorArchivos;
            int bloques;
            cout << "\nTamaño del archivo (en bloques): ";
            cin >> bloques;
            crearArchivo(nombre, bloques);
            contadorArchivos++;
            presionarEnter();
            break;
        }

        case 3: {
            cleanSc();
            mostrarEstado();
            /*cout << "\nLectura de archivos:\n";
            for (auto &par : inicioArchivo) {
                leerArchivo(par.first);
            }*/
            presionarEnter();
            break;
        }
        
        case 4: {
            cleanSc();
            if (inicioArchivo.empty()) {
                cout << "\nNo hay archivos para eliminar.\n";
                presionarEnter();
                break;
            }
        
            cout << "\nArchivos existentes: ";
            for (auto &p : inicioArchivo) cout << p.first << " ";
            cout << "\nIngrese el nombre del archivo a eliminar: ";
            char nombre;
            cin >> nombre;
            eliminarArchivo(nombre);
            presionarEnter();
            break;
        }

        case 5:
            cout << "\nSaliendo del programa...\n";
            segundosPausa(1);
            break;

        default:
            cout << "\nOpción no válida.\n";
            presionarEnter();
        }

    } while (opcion != 5);

    return 0;
}
