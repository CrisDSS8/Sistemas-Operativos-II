#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

// ==== UTILS ====

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

// ==== SISTEMA DE ARCHIVOS ====

struct Nodo {
    string nombre;
    bool esArchivo;
    Nodo* padre;
    vector<unique_ptr<Nodo>> hijos;

    Nodo(string nombre, bool esArchivo = false, Nodo* padre = nullptr)
        : nombre(nombre), esArchivo(esArchivo), padre(padre) {}
};

Nodo* raiz = nullptr;
Nodo* actual = nullptr;

void mostrarContenido(Nodo* nodo) {
    cout << "\nContenido de \"" << nodo->nombre << "\":\n";
    for (auto& hijo : nodo->hijos) {
        cout << (hijo->esArchivo ? "[A] " : "[D] ") << hijo->nombre << endl;
    }
}

Nodo* buscarHijo(Nodo* padre, const string& nombre) {
    for (auto& hijo : padre->hijos) {
        if (hijo->nombre == nombre) {
            return hijo.get();
        }
    }
    return nullptr;
}

void crearElemento() {
    int tipo;
    string nombre;
    cout << "\nCrear: 1) Directorio  2) Archivo: ";
    cin >> tipo;
    cout << "Nombre: ";
    cin >> nombre;

    if (buscarHijo(actual, nombre)) {
        cout << "Ya existe un elemento con ese nombre.\n";
        return;
    }

    bool esArchivo = (tipo == 2);
    actual->hijos.push_back(unique_ptr<Nodo>(new Nodo(nombre, esArchivo, actual)));
    cout << (esArchivo ? "Archivo" : "Directorio") << " creado correctamente.\n";
}

void eliminarElemento() {
    int tipo;
    string nombre;
    cout << "\nEliminar: 1) Directorio  2) Archivo: ";
    cin >> tipo;
    cout << "Nombre: ";
    cin >> nombre;

    bool esArchivo = (tipo == 2);

    for (auto it = actual->hijos.begin(); it != actual->hijos.end(); ++it) {
        if ((*it)->nombre == nombre && (*it)->esArchivo == esArchivo) {
            if (!(*it)->esArchivo && !(*it)->hijos.empty()) {
                cout << "No se puede eliminar. El directorio contiene elementos.\n";
                return;
            }
            actual->hijos.erase(it);
            cout << (esArchivo ? "Archivo" : "Directorio") << " eliminado.\n";
            return;
        }
    }

    cout << (esArchivo ? "Archivo" : "Directorio") << " no encontrado.\n";
}

void accederDirectorio() {
    string nombre;
    cout << "\nIngrese nombre del directorio a acceder: ";
    cin >> nombre;

    Nodo* destino = buscarHijo(actual, nombre);
    if (!destino || destino->esArchivo) {
        cout << "Directorio no encontrado.\n";
    } else {
        actual = destino;
        cout << "Ahora estás en: " << actual->nombre << endl;
    }
}

void subirNivel() {
    if (actual->padre) {
        actual = actual->padre;
        cout << "Subiste al directorio: " << actual->nombre << endl;
    } else {
        cout << "Ya estás en la raíz.\n";
    }
}

void subirARaiz() {
    actual = raiz;
    cout << "Volviste a la raíz: " << actual->nombre << endl;
}

// ==== MAIN ====

int main() {
    int opcion;
    string nombreRaiz;

    cleanSc();
    cout << "====== MINI SISTEMA DE ARCHIVOS ======\n\n";
    cout << "Defina el nombre de la carpeta raíz: ";
    cin >> nombreRaiz;

    raiz = new Nodo(nombreRaiz, false);
    actual = raiz;

    do {
        cleanSc();
        cout << "\n==== Carpeta actual: " << actual->nombre << " ====\n\n";
        cout << "1. Crear\n";
        cout << "2. Eliminar\n";
        cout << "3. Mostrar contenido\n";
        cout << "4. Acceder a carpeta\n";
        cout << "5. Subir un nivel\n";
        cout << "6. Volver a la raíz\n";
        cout << "7. Salir\n\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
        case 1: {
            int sub;
            cleanSc();
            cout << "\n== CREAR ==\n";
            cout << "1. Directorio\n";
            cout << "2. Archivo\n";
            cout << "Seleccione una opción: ";
            cin >> sub;
            if (sub == 1 || sub == 2) {
                cleanSc();
                string nombre;
                cout << "Ingrese nombre: ";
                cin >> nombre;

                if (buscarHijo(actual, nombre)) {
                    cout << "Ya existe un elemento con ese nombre.\n";
                } else {
                    bool esArchivo = (sub == 2);
                    actual->hijos.push_back(unique_ptr<Nodo>(new Nodo(nombre, esArchivo, actual)));
                    cout << (esArchivo ? "Archivo" : "Directorio") << " creado correctamente.\n";
                }
            } else {
                cout << "Opción inválida.\n";
            }
            presionarEnter();
            break;
        }

        case 2: {
            int sub;
            cleanSc();
            cout << "\n== ELIMINAR ==\n";
            cout << "1. Directorio\n";
            cout << "2. Archivo\n";
            cout << "Seleccione una opción: ";
            cin >> sub;

            if (sub == 1 || sub == 2) {
                cleanSc();
                string nombre;
                cout << "Ingrese nombre: ";
                cin >> nombre;
                bool esArchivo = (sub == 2);
                for (auto it = actual->hijos.begin(); it != actual->hijos.end(); ++it) {
                    if ((*it)->nombre == nombre && (*it)->esArchivo == esArchivo) {
                        if (!(*it)->esArchivo && !(*it)->hijos.empty()) {
                            cout << "No se puede eliminar. El directorio contiene elementos.\n";
                            break;
                        }
                        actual->hijos.erase(it);
                        cout << (esArchivo ? "Archivo" : "Directorio") << " eliminado.\n";
                        break;
                    }
                }
            } else {
                cout << "Opción inválida.\n";
            }
            presionarEnter();
            break;
        }

        case 3:
            cleanSc();
            mostrarContenido(actual);
            presionarEnter();
            break;

        case 4:
            cleanSc();
            accederDirectorio();
            presionarEnter();
            break;

        case 5:
            cleanSc();
            subirNivel();
            presionarEnter();
            break;

        case 6:
            cleanSc();
            subirARaiz();
            presionarEnter();
            break;

        case 7:
            cout << "\nSaliendo del programa...\n";
            segundosPausa(1);
            break;

        default:
            cout << "\nOpción no válida.\n";
            presionarEnter();
        }

    } while (opcion != 7);

    delete raiz;
    return 0;
}
