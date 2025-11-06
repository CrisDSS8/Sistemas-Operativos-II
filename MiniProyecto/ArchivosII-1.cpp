#include <iostream>
#include <limits>
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

// ==== CONFIG ====
const int TOTAL_BLOQUES = 100;
const string FS_FILENAME = "fs.json";
const string USERS_FILENAME = "usuarios.json";

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
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// trim helpers
static inline void ltrim(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}
static inline void rtrim(string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
static inline void trim(string &s) { ltrim(s); rtrim(s); }

// ==== SISTEMA DE ARCHIVOS (estructura) ====

struct Nodo {
    string nombre;
    bool esArchivo;
    Nodo* padre;
    vector<unique_ptr<Nodo>> hijos;

    // Metadatos para archivos
    string owner; // nombre del propietario
    bool ownerR = true;
    bool ownerW = true;
    bool othersR = true;
    bool othersW = true;
    vector<int> bloques; // indices de bloques ocupados (0..TOTAL_BLOQUES-1)

    Nodo(string nombre, bool esArchivo = false, Nodo* padre = nullptr)
        : nombre(nombre), esArchivo(esArchivo), padre(padre) {}
};

bool tienePermisoLectura(Nodo* nodo);

// Estructura de usuario
struct Usuario {
    string nombre;
    bool puedeLeer;
    bool puedeEscribir;
    bool esAdmin;
};

Nodo* raiz = nullptr;
Nodo* actual = nullptr;
string usuarioActual = "admin";

vector<Usuario> usuarios;
vector<int> disco(TOTAL_BLOQUES, -1); // -1 libre, si ocupado: unique id of file (we use pointer id hashed)
int nextFileId = 1; // para identificar archivos en el disco (no es visible al usuario)
unordered_map<Nodo*, int> fileIdMap; // mapa Nodo* -> id (solo para archivos)

// ==== FUNCIONES DE DISCO ====

int asignarIdArchivo(Nodo* f) {
    if (fileIdMap.count(f)) return fileIdMap[f];
    fileIdMap[f] = nextFileId++;
    return fileIdMap[f];
}

vector<int> bloquesLibres() {
    vector<int> libres;
    for (int i = 0; i < TOTAL_BLOQUES; ++i) if (disco[i] == -1) libres.push_back(i);
    return libres;
}

int contarBloquesLibres() {
    int c = 0;
    for (int i = 0; i < TOTAL_BLOQUES; ++i) if (disco[i] == -1) ++c;
    return c;
}

// asigna n bloques libres y marca disco con idArchivo; devuelve vector de bloques asignados (o vacío si no hay suficientes)
vector<int> asignarBloques(int n, int idArchivo) {
    vector<int> asignados;
    if (n <= 0) return asignados;
    for (int i = 0; i < TOTAL_BLOQUES && (int)asignados.size() < n; ++i) {
        if (disco[i] == -1) {
            disco[i] = idArchivo;
            asignados.push_back(i);
        }
    }
    if ((int)asignados.size() < n) {
        // no alcanzaron; revertir
        for (int b : asignados) disco[b] = -1;
        asignados.clear();
    }
    return asignados;
}

void liberarBloques(const vector<int>& bloques) {
    for (int b : bloques) {
        if (b >= 0 && b < TOTAL_BLOQUES) disco[b] = -1;
    }
}

// ==== NAVEGACIÓN Y UTILIDADES SOBRE NODOS ====
void mostrarContenido(Nodo* nodo) {
    if (!tienePermisoLectura(nodo)) return;
    
    cout << "\nContenido de \"" << nodo->nombre << "\":\n";

    for (auto& hijo : nodo->hijos) {
        cout << (hijo->esArchivo ? "[A] " : "[D] ") << hijo->nombre;

        // Información común para ambos tipos
        cout << "  (owner: " << hijo->owner;
        cout << ", perms: " 
             << (hijo->ownerR ? 'r' : '-') 
             << (hijo->ownerW ? 'w' : '-') << "/"
             << (hijo->othersR ? 'r' : '-') 
             << (hijo->othersW ? 'w' : '-');

        // Solo los archivos muestran tamaño
        if (hijo->esArchivo)
            cout << ", size: " << hijo->bloques.size() << " bloques";

        cout << ")";
        cout << endl;
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

Usuario* buscarUsuario(const string& nombre) {
    for (auto& u : usuarios) {
        if (u.nombre == nombre)
            return &u;
    }
    return nullptr;
}

// -----------------------------
// PERMISOS DE LECTURA
// -----------------------------
bool tienePermisoLectura(Nodo* nodo) {
    if (nodo == nullptr) return false;

    Usuario* u = buscarUsuario(usuarioActual);
    if (!u) {
        cout << "Error: usuario no encontrado.\n";
        return false;
    }

    // El admin siempre puede todo
    if (u->esAdmin) return true;

    // Si es la carpeta raíz y el usuario tiene permiso global de lectura, permitir
    if (nodo == raiz && u->puedeLeer) return true;

    // Verificar permiso global del usuario
    if (!u->puedeLeer) {
        cout << "Acceso denegado: el usuario \"" << usuarioActual
             << "\" no tiene permiso global de lectura.\n";
        return false;
    }

    // Verificar permiso definido por el propietario del nodo
    bool permitidoPorNodo = false;
    if (nodo->owner == usuarioActual)
        permitidoPorNodo = nodo->ownerR;
    else
        permitidoPorNodo = nodo->othersR;

    if (!permitidoPorNodo) {
        cout << "Acceso denegado: el propietario del archivo/directorio \""
             << nodo->nombre << "\" no permite lectura para este usuario.\n";
        return false;
    }

    // Si pasa ambas verificaciones
    return true;
}

// -----------------------------
// PERMISOS DE ESCRITURA
// -----------------------------
bool tienePermisoEscritura(Nodo* nodo) {
    if (nodo == nullptr) return false;

    Usuario* u = buscarUsuario(usuarioActual);
    if (!u) {
        cout << "Error: usuario no encontrado.\n";
        return false;
    }

    // El admin siempre puede
    if (u->esAdmin) return true;

    // Si es la carpeta raíz y el usuario tiene permiso global, permitir
    if (nodo == raiz && u->puedeEscribir) return true;

    // Verificar permiso global
    if (!u->puedeEscribir) {
        cout << "Acceso denegado: el usuario \"" << usuarioActual
             << "\" no tiene permiso global de escritura.\n";
        return false;
    }

    // Verificar permiso local (del propietario del nodo)
    bool permitidoPorNodo = false;
    if (nodo->owner == usuarioActual)
        permitidoPorNodo = nodo->ownerW;
    else
        permitidoPorNodo = nodo->othersW;

    if (!permitidoPorNodo) {
        cout << "Acceso denegado: el propietario del archivo/directorio \"" 
             << nodo->nombre << "\" no permite escritura para este usuario.\n";
        return false;
    }

    return true;
}

// ==== OPERACIONES PRINCIPALES ====

void crearElemento() {
    cleanSc();

    // El admin siempre puede crear
    Usuario* u = buscarUsuario(usuarioActual);
    if (!u) { cout << "Error: usuario no encontrado.\n"; presionarEnter(); return; }
    if (!u->esAdmin && !tienePermisoEscritura(actual)) {
        cout << "No tienes permiso de escritura en este directorio.\n";
        presionarEnter(); return;
    }

    cout << "\n== CREAR ==\n";
    cout << "1. Directorio\n";
    cout << "2. Archivo\n";
    cout << "Seleccione una opción: ";
    int sub; cin >> sub;
    if (sub != 1 && sub != 2) {
        cout << "Opción inválida.\n";
        presionarEnter(); return;
    }

    cout << "Ingrese nombre: ";
    string nombre; cin >> nombre;

    if (buscarHijo(actual, nombre)) {
        cout << "Ya existe un elemento con ese nombre.\n";
        presionarEnter(); return;
    }

    bool esArchivo = (sub == 2);
    auto nuevo = std::unique_ptr<Nodo>(new Nodo(nombre, esArchivo, actual));

    // Asignar propietario y permisos base
    nuevo->owner = usuarioActual;
    nuevo->ownerR = true;
    nuevo->ownerW = true;

    // Preguntar permisos para otros usuarios
    char resp;
    cout << "¿Permitir lectura a otros usuarios? (s/n): ";
    cin >> resp;
    nuevo->othersR = (resp == 's' || resp == 'S');

    cout << "¿Permitir escritura a otros usuarios? (s/n): ";
    cin >> resp;
    nuevo->othersW = (resp == 's' || resp == 'S');

    if (esArchivo) {
        cout << "Tamaño inicial en bloques (0 = vacío): ";
        int tam = 0; cin >> tam;
        if (tam < 0) tam = 0;

        int id = asignarIdArchivo(nuevo.get());
        vector<int> asignados = asignarBloques(tam, id);

        if ((int)asignados.size() < tam) {
            cout << "No hay espacio suficiente. Se asignaron "
                 << asignados.size() << " bloques (archivo creado con ese tamaño).\n";
        } else {
            cout << "Se asignaron " << asignados.size() << " bloques.\n";
        }

        nuevo->bloques = asignados;
    }

    actual->hijos.push_back(move(nuevo));

    cout << (esArchivo ? "Archivo" : "Directorio")
         << " creado correctamente con permisos:\n";
    cout << "  Propietario: rw\n";
    cout << "  Otros: "
         << (actual->hijos.back()->othersR ? 'r' : '-')
         << (actual->hijos.back()->othersW ? 'w' : '-') << endl;

    presionarEnter();
}


void eliminarElementoMenu() {
    cleanSc();
    // El admin siempre puede crear
    Usuario* u = buscarUsuario(usuarioActual);
    if (!u) { cout << "Error: usuario no encontrado.\n"; presionarEnter(); return; }
    if (!u->esAdmin && !tienePermisoEscritura(actual)) {
        cout << "No tienes permiso de escritura en este directorio.\n";
        presionarEnter(); return;
    }
    
    cout << "\n== ELIMINAR ==\n";
    cout << "1. Directorio\n";
    cout << "2. Archivo\n";
    cout << "Seleccione una opción: ";
    int sub; cin >> sub;
    if (sub != 1 && sub != 2) { cout << "Opción inválida.\n"; presionarEnter(); return; }
    cout << "Ingrese nombre: ";
    string nombre; cin >> nombre;
    bool esArchivo = (sub==2);
    for (auto it = actual->hijos.begin(); it != actual->hijos.end(); ++it) {
        if ((*it)->nombre == nombre && (*it)->esArchivo == esArchivo) {
            // permisos: solo owner o root puede eliminar
            Nodo* target = it->get();
            /*if (usuarioActual != "root" && target->owner != usuarioActual) {
                cout << "No tienes permisos para eliminar este elemento (no eres el propietario).\n";
                break;
            }*/
            Usuario* u = buscarUsuario(usuarioActual);
                bool esAdmin = (u && u->esAdmin);
                if (!esAdmin && target->owner != usuarioActual) {
                    cout << "No tienes permisos para eliminar este elemento (no eres el propietario).\n";
                    break;
                }
            if (!(*it)->esArchivo && !(*it)->hijos.empty()) {
                cout << "No se puede eliminar. El directorio contiene elementos.\n";
                break;
            }
            // si es archivo, liberar bloques
            if ((*it)->esArchivo) {
                liberarBloques((*it)->bloques);
                fileIdMap.erase(it->get());
            }
            actual->hijos.erase(it);
            cout << (esArchivo ? "Archivo" : "Directorio") << " eliminado.\n";
            presionarEnter();
            return;
        }
    }
    cout << (esArchivo ? "Archivo" : "Directorio") << " no encontrado.\n";
    presionarEnter();
}

void accederDirectorio() {
    cleanSc();
    
    if (!tienePermisoLectura(actual)) {
        cout << "No tienes permiso de lectura en este directorio.\n";
        presionarEnter(); return;
    }
    cout << "\nIngrese nombre del directorio a acceder: ";
    string nombre; cin >> nombre;
    Nodo* destino = buscarHijo(actual, nombre);
    if (!destino || destino->esArchivo) {
        cout << "Directorio no encontrado.\n";
    } else {
        actual = destino;
        cout << "Ahora estás en: " << actual->nombre << endl;
    }
    presionarEnter();
}

void subirNivel() {
    cleanSc();
    if (actual->padre) {
        actual = actual->padre;
        cout << "Subiste al directorio: " << actual->nombre << endl;
    } else {
        cout << "Ya estás en la raíz.\n";
    }
    presionarEnter();
}

void subirARaiz() {
    cleanSc();
    actual = raiz;
    cout << "Volviste a la raíz: " << actual->nombre << endl;
    presionarEnter();
}

void mostrarMapaBloques() {
    cleanSc();
    if (!tienePermisoLectura(actual)) {
        cout << "No tienes permiso de lectura en este directorio.\n";
        presionarEnter(); return;
    }
    cout << "Mapa de bloques (0.."<<TOTAL_BLOQUES-1<<"):\n";
    for (int i = 0; i < TOTAL_BLOQUES; ++i) {
        cout << setw(3) << i << ": " << (disco[i] == -1 ? string("Libre") : ("ID" + to_string(disco[i]))) << "\n";
    }
    cout << "Bloques libres: " << contarBloquesLibres() << "\n";
    presionarEnter();
}

void guardarUsuarios() {
    ofstream f("usuarios.json");
    if (!f) {
        cout << "Error al guardar usuarios.\n";
        return;
    }

    f << "[\n";
    for (size_t i = 0; i < usuarios.size(); ++i) {
        const auto& u = usuarios[i];
        f << "  {\n"
          << "    \"nombre\": \"" << u.nombre << "\",\n"
          << "    \"puedeLeer\": " << (u.puedeLeer ? "true" : "false") << ",\n"
          << "    \"puedeEscribir\": " << (u.puedeEscribir ? "true" : "false") << ",\n"
          << "    \"esAdmin\": " << (u.esAdmin ? "true" : "false") << "\n"
          << "  }";
        if (i < usuarios.size() - 1) f << ",";
        f << "\n";
    }
    f << "]";
    f.close();
}

void cargarUsuarios() {
    ifstream f("usuarios.json");
    if (!f.good()) {
        cout << "No se encontró archivo de usuarios. Creando administrador...\n";
        Usuario admin = {"admin", true, true, true};
        usuarios.push_back(admin);
        guardarUsuarios();
        return;
    }

    usuarios.clear(); // evitar duplicados si se llama más de una vez

    string json((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    f.close();

    size_t pos = 0;
    while ((pos = json.find("\"nombre\":", pos)) != string::npos) {
        Usuario u;

        // Nombre
        size_t start = json.find("\"", pos + 9) + 1;
        size_t end = json.find("\"", start);
        u.nombre = json.substr(start, end - start);

        // Crear bloque acotado para evitar lecturas cruzadas
        size_t bloqueInicio = json.rfind("{", start);
        size_t bloqueFin = json.find("}", end);
        string bloque = json.substr(bloqueInicio, bloqueFin - bloqueInicio);

        // Buscar permisos solo dentro del bloque
        u.puedeLeer = (bloque.find("\"puedeLeer\": true") != string::npos);
        u.puedeEscribir = (bloque.find("\"puedeEscribir\": true") != string::npos);
        u.esAdmin = (bloque.find("\"esAdmin\": true") != string::npos);

        usuarios.push_back(u);
        pos = bloqueFin;
    }

    // Verificación final: si no existe admin, se crea automáticamente
    bool existeAdmin = false;
    for (auto& u : usuarios)
        if (u.nombre == "admin") { existeAdmin = true; break; }

    if (!existeAdmin) {
        Usuario admin = {"admin", true, true, true};
        usuarios.push_back(admin);
        guardarUsuarios();
        cout << "Administrador creado automáticamente (faltaba en el archivo).\n";
    }
}

void login() {
    cleanSc();
    cout << "Ingrese usuario para hacer login: ";
    string u; cin >> u;
    
    Usuario* us = buscarUsuario(u);
    if (/*!u.empty()*/us) {
        usuarioActual = us->nombre;
        cout << "Login exitoso como: " << usuarioActual << "\n";
    } else {
        cout << "Nombre inválido.\n";
    }
    presionarEnter();
}

void modificarTamArchivo() {
    cleanSc();
    if (!tienePermisoEscritura(actual)) {
        cout << "No tienes permiso para cambiar permisos aquí.\n";
        presionarEnter(); return;
    }
    cout << "\nModificar tamaño de archivo. Ingrese nombre del archivo: ";
    string nombre; cin >> nombre;
    Nodo* f = buscarHijo(actual, nombre);
    if (!f || !f->esArchivo) {
        cout << "Archivo no encontrado.\n"; presionarEnter(); return;
    }
    if (!tienePermisoEscritura(actual)) {
        cout << "No tienes permiso de escritura sobre este archivo.\n"; presionarEnter(); return;
    }
    cout << "Tamaño actual (bloques): " << f->bloques.size() << "\n";
    cout << "1) Incrementar\n2) Disminuir\nSeleccione: ";
    int op; cin >> op;
    if (op == 1) {
        cout << "¿Cuántos bloques desea añadir?: "; int n; cin >> n;
        if (n <= 0) { cout << "Número inválido.\n"; presionarEnter(); return; }
        int libres = contarBloquesLibres();
        if (libres < n) {
            cout << "No hay suficientes bloques libres. Solo quedan " << libres << " bloques.\n";
            presionarEnter(); return;
        }
        int id = asignarIdArchivo(f);
        vector<int> nuevos = asignarBloques(n, id);
        if (nuevos.empty()) {
            cout << "Error asignando bloques.\n";
        } else {
            f->bloques.insert(f->bloques.end(), nuevos.begin(), nuevos.end());
            cout << "Se agregaron " << nuevos.size() << " bloques. Nuevo tamaño: " << f->bloques.size() << "\n";
        }
    } else if (op == 2) {
        cout << "¿Cuántos bloques desea quitar?: "; int n; cin >> n;
        if (n <= 0) { cout << "Número inválido.\n"; presionarEnter(); return; }
        if (n > (int)f->bloques.size()) n = f->bloques.size();
        // liberar los últimos n bloques
        vector<int> aLiberar;
        for (int i = 0; i < n; ++i) {
            aLiberar.push_back(f->bloques.back());
            f->bloques.pop_back();
        }
        liberarBloques(aLiberar);
        cout << "Se liberaron " << aLiberar.size() << " bloques. Nuevo tamaño: " << f->bloques.size() << "\n";
    } else {
        cout << "Opción inválida.\n";
    }
    presionarEnter();
}

void cambiarPermisos() {
    cleanSc();
    
    cout << "\nIngrese nombre: ";
    string nombre; cin >> nombre;
    
    Nodo* n = buscarHijo(actual, nombre);
    if (!n) { cout << "No existe.\n"; presionarEnter(); return; }
    
    //Usuario* usu = buscarUsuario(usuarioActual);
    //bool esAdmin = (usu && usu->esAdmin);
    Usuario* u = buscarUsuario(usuarioActual);
    bool esAdmin = (u && u->esAdmin);
    if (!esAdmin && usuarioActual != n->owner) {
        cout << "No tienes permiso para cambiar permisos de este elemento.\n"; presionarEnter(); return;
    }
    /*Usuario* u = buscarUsuario(usuarioActual);
        bool esAdmin = (u && u->esAdmin);
        if (!esAdmin && usuarioActual != n->owner) {
            cout << "No tienes permiso para cambiar permisos de este elemento.\n"; presionarEnter(); return;
        }*/
    cout << "Permisos actuales owner: " << (n->ownerR?'r':'-') << (n->ownerW?'w':'-')
         << " others: " << (n->othersR?'r':'-') << (n->othersW?'w':'-') << "\n";
    cout << "Editar (0 = no cambiar):\n";
    cout << "Owner read (1/0): "; int v; cin >> v; if (v==0 || v==1) n->ownerR = (v==1);
    cout << "Owner write (1/0): "; cin >> v; if (v==0 || v==1) n->ownerW = (v==1);
    cout << "Others read (1/0): "; cin >> v; if (v==0 || v==1) n->othersR = (v==1);
    cout << "Others write (1/0): "; cin >> v; if (v==0 || v==1) n->othersW = (v==1);
    cout << "Permisos actualizados.\n";
    presionarEnter();
}

// ==== SERIALIZACIÓN A JSON (generación y parseo simple) ====

string escapeJson(const string& s) {
    string out;
    for (char c : s) {
        if (c == '\"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

void nodoToJson(const Nodo* n, ostream& os, int indent = 2) {
    string ind(indent, ' ');
    os << ind << "{\n";
    os << ind << "  \"nombre\": \"" << escapeJson(n->nombre) << "\",\n";
    os << ind << "  \"esArchivo\": " << (n->esArchivo ? "true" : "false") << ",\n";
    os << ind << "  \"owner\": \"" << escapeJson(n->owner) << "\",\n";
    os << ind << "  \"ownerR\": " << (n->ownerR ? "true":"false") << ",\n";
    os << ind << "  \"ownerW\": " << (n->ownerW ? "true":"false") << ",\n";
    os << ind << "  \"othersR\": " << (n->othersR ? "true":"false") << ",\n";
    os << ind << "  \"othersW\": " << (n->othersW ? "true":"false") << ",\n";
    // bloques
    os << ind << "  \"bloques\": [";
    if (!n->bloques.empty()) {
        for (size_t i = 0; i < n->bloques.size(); ++i) {
            if (i) os << ", ";
            os << n->bloques[i];
        }
    }
    os << "],\n";
    // children
    os << ind << "  \"hijos\": [\n";
    for (size_t i = 0; i < n->hijos.size(); ++i) {
        nodoToJson(n->hijos[i].get(), os, indent + 4);
        if (i + 1 < n->hijos.size()) os << ",\n";
        else os << "\n";
    }
    os << ind << "  ]\n";
    os << ind << "}";
}

void saveFS(const string& filename) {
    ofstream ofs(filename);
    if (!ofs) {
        cerr << "Error al abrir archivo para guardar: " << filename << "\n";
        return;
    }
    ofs << "{\n";
    ofs << "  \"usuarioActual\": \"" << escapeJson(usuarioActual) << "\",\n";
    ofs << "  \"disco\": [";
    for (int i = 0; i < TOTAL_BLOQUES; ++i) {
        if (i) ofs << ", ";
        ofs << disco[i];
    }
    ofs << "],\n";
    ofs << "  \"root\": \n";
    nodoToJson(raiz, ofs, 2);
    ofs << "\n}\n";
    ofs.close();
    cout << "Sistema guardado en " << filename << "\n";
}

// ======= SIMPLE PARSER JSON (solo para el JSON que genera este programa) =======
//
// Este parser es específico y asume el formato que genera saveFS.
// No es un parser JSON genérico. Pero es suficiente para reconstruir la estructura.
// =============================================================================

void skipWs(const string& s, size_t& pos) {
    while (pos < s.size() && isspace((unsigned char)s[pos])) ++pos;
}

bool consume(const string& s, size_t& pos, const string& token) {
    skipWs(s,pos);
    if (s.compare(pos, token.size(), token) == 0) {
        pos += token.size();
        return true;
    }
    return false;
}

string parseString(const string& s, size_t& pos) {
    skipWs(s,pos);
    if (pos >= s.size() || s[pos] != '"') return "";
    ++pos;
    string out;
    while (pos < s.size()) {
        char c = s[pos++];
        if (c == '\\' && pos < s.size()) {
            char n = s[pos++];
            if (n == 'n') out.push_back('\n');
            else if (n == '"') out.push_back('"');
            else if (n == '\\') out.push_back('\\');
            else out.push_back(n);
        } else if (c == '"') {
            break;
        } else out.push_back(c);
    }
    return out;
}

int parseInt(const string& s, size_t& pos) {
    skipWs(s,pos);
    int sign = 1;
    if (pos < s.size() && s[pos] == '-') { sign = -1; ++pos; }
    int val = 0;
    bool any = false;
    while (pos < s.size() && isdigit((unsigned char)s[pos])) {
        any = true;
        val = val*10 + (s[pos]-'0');
        ++pos;
    }
    return any ? val*sign : 0;
}

bool parseBool(const string& s, size_t& pos) {
    skipWs(s,pos);
    if (s.compare(pos,4,"true")==0) { pos+=4; return true; }
    if (s.compare(pos,5,"false")==0) { pos+=5; return false; }
    return false;
}

// forward
unique_ptr<Nodo> parseNodo(const string& s, size_t& pos, Nodo* padre);

// parse array of ints [a, b, c]
vector<int> parseArrayInts(const string& s, size_t& pos) {
    vector<int> out;
    skipWs(s,pos);
    if (!consume(s,pos,"[")) return out;
    skipWs(s,pos);
    if (consume(s,pos,"]")) return out;
    while (true) {
        skipWs(s,pos);
        int v = parseInt(s,pos);
        out.push_back(v);
        skipWs(s,pos);
        if (consume(s,pos,"]")) break;
        consume(s,pos,",");
    }
    return out;
}

// parse array of nodos
vector<unique_ptr<Nodo>> parseArrayNodos(const string& s, size_t& pos, Nodo* padre) {
    vector<unique_ptr<Nodo>> out;
    skipWs(s,pos);
    if (!consume(s,pos,"[")) return out;
    skipWs(s,pos);
    if (consume(s,pos,"]")) return out;
    while (true) {
        skipWs(s,pos);
        unique_ptr<Nodo> child = parseNodo(s,pos,padre);
        if (child) out.push_back(move(child));
        skipWs(s,pos);
        if (consume(s,pos,"]")) break;
        consume(s,pos,",");
    }
    return out;
}

unique_ptr<Nodo> parseNodo(const string& s, size_t& pos, Nodo* padre) {
    skipWs(s,pos);
    if (!consume(s,pos,"{")) return nullptr;
    string nombre;
    bool esArchivo = false;
    string owner = "root";
    bool ownerR=true, ownerW=true, othersR=true, othersW=false;
    vector<int> bloques;
    vector<unique_ptr<Nodo>> hijos;
    while (true) {
        skipWs(s,pos);
        if (consume(s,pos,"}")) break;
        string key = parseString(s,pos);
        skipWs(s,pos);
        consume(s,pos,":");
        skipWs(s,pos);
        if (key == "nombre") {
            nombre = parseString(s,pos);
        } else if (key == "esArchivo") {
            esArchivo = parseBool(s,pos);
        } else if (key == "owner") {
            owner = parseString(s,pos);
        } else if (key == "ownerR") {
            ownerR = parseBool(s,pos);
        } else if (key == "ownerW") {
            ownerW = parseBool(s,pos);
        } else if (key == "othersR") {
            othersR = parseBool(s,pos);
        } else if (key == "othersW") {
            othersW = parseBool(s,pos);
        } else if (key == "bloques") {
            bloques = parseArrayInts(s,pos);
        } else if (key == "hijos") {
            hijos = parseArrayNodos(s,pos,nullptr); // padre set later
        } else {
            // unknown: skip somewhat
            // try to skip value by consuming until comma or }
            size_t brace = s.find_first_of(",}", pos);
            if (brace == string::npos) break;
            pos = brace;
        }
        skipWs(s,pos);
        consume(s,pos,",");
    }
    auto nodo = std::unique_ptr<Nodo>(new Nodo(nombre, esArchivo, padre));
    nodo->owner = owner;
    nodo->ownerR = ownerR; nodo->ownerW = ownerW;
    nodo->othersR = othersR; nodo->othersW = othersW;
    nodo->bloques = bloques;
    // set padres correctos for hijos
    for (auto &h : hijos) {
        h->padre = nodo.get();
        nodo->hijos.push_back(move(h));
    }
    return nodo;
}

bool loadFS(const string& filename) {
    ifstream ifs(filename);
    if (!ifs) {
        cout << "No se encontró archivo de sistema. Se inicia uno nuevo.\n";
        return false;
    }
    stringstream ss;
    ss << ifs.rdbuf();
    string content = ss.str();
    size_t pos = 0;
    skipWs(content,pos);
    if (!consume(content,pos,"{")) {
        cout << "Formato inválido (no json esperado). Inicio limpio.\n";
        return false;
    }
    // reset structures
    fill(disco.begin(), disco.end(), -1);
    fileIdMap.clear();
    nextFileId = 1;
    usuarioActual = "admin";
    unique_ptr<Nodo> rootParsed = nullptr;
    while (true) {
        skipWs(content,pos);
        if (consume(content,pos,"}")) break;
        string key = parseString(content,pos);
        skipWs(content,pos);
        consume(content,pos,":");
        skipWs(content,pos);
        if (key == "usuarioActual") {
            usuarioActual = parseString(content,pos);
        } else if (key == "disco") {
            // parse array of ints
            vector<int> arr;
            skipWs(content,pos);
            if (consume(content,pos,"[")) {
                skipWs(content,pos);
                while (true) {
                    skipWs(content,pos);
                    if (consume(content,pos,"]")) break;
                    int v = parseInt(content,pos);
                    arr.push_back(v);
                    skipWs(content,pos);
                    if (consume(content,pos,"]")) break;
                    consume(content,pos,",");
                }
            }
            // assign disco
            for (size_t i=0;i<arr.size() && i<disco.size();++i) disco[i] = arr[i];
            // update nextFileId guess
            int mx = 0;
            for (int v : disco) if (v > mx) mx = v;
            nextFileId = max(nextFileId, mx+1);
        } else if (key == "root") {
            // parse node
            rootParsed = parseNodo(content,pos,nullptr);
        } else {
            // skip unknown
            size_t brace = content.find_first_of(",}", pos);
            if (brace == string::npos) break;
            pos = brace;
        }
        skipWs(content,pos);
        consume(content,pos,",");
    }
    if (rootParsed) {
        // set padres recursivamente and rebuild fileIdMap by assigning new ids and marking disco mapping accordingly
        // First, replace current root if exists
        if (raiz) {
            // free previous memory
            delete raiz;
            raiz = nullptr;
            actual = nullptr;
        }
        raiz = rootParsed.release();
        // fix parent pointers recursively
        function<void(Nodo*)> fixParents = [&](Nodo* node) {
            for (auto &ch : node->hijos) {
                ch->padre = node;
                fixParents(ch.get());
            }
            if (node->esArchivo) {
                int id = asignarIdArchivo(node);
                // mark disco according to bloques vector (overwrite with id)
                for (int b : node->bloques) {
                    if (b >= 0 && b < TOTAL_BLOQUES) disco[b] = id;
                }
            }
        };
        raiz->padre = nullptr;
        fixParents(raiz);
        actual = raiz;
        cout << "Sistema cargado desde " << filename << "\n";
        return true;
    }
    cout << "No se pudo parsear root. Iniciando nuevo sistema.\n";
    return false;
}

void gestionarUsuarios() {
    if (usuarioActual != "admin") {
        cout << "Solo el administrador puede gestionar usuarios.\n";
        return;
    }

    int opcion;
    do {
        cout << "\n=== Gestión de Usuarios ===\n";
        cout << "1. Listar usuarios\n";
        cout << "2. Agregar usuario\n";
        cout << "3. Eliminar usuario\n";
        cout << "4. Volver\n";
        cout << "Seleccione: ";
        cin >> opcion;

        if (opcion == 1) {
            cout << "\nUsuarios registrados:\n";
            for (const auto& u : usuarios) {
                cout << "- " << u.nombre 
                     << (u.esAdmin ? " (admin)" : "")
                     << " [lectura: " << (u.puedeLeer ? "sí" : "no")
                     << ", escritura: " << (u.puedeEscribir ? "sí" : "no")
                     << "]\n";
            }

        } else if (opcion == 2) {
            Usuario nuevo;
            cout << "\n=== Crear nuevo usuario ===\n";
            cout << "Nombre: ";
            cin >> nuevo.nombre;

            // Evitar duplicados
            bool existe = false;
            for (const auto& u : usuarios) {
                if (u.nombre == nuevo.nombre) {
                    existe = true;
                    break;
                }
            }
            if (existe) {
                cout << "El usuario \"" << nuevo.nombre << "\" ya existe.\n";
                continue;
            }

            nuevo.esAdmin = false;

            char resp;
            cout << "¿Permiso de lectura (s/n)? ";
            cin >> resp; 
            nuevo.puedeLeer = (resp == 's' || resp == 'S');

            cout << "¿Permiso de escritura (s/n)? ";
            cin >> resp; 
            nuevo.puedeEscribir = (resp == 's' || resp == 'S');

            usuarios.push_back(nuevo);
            guardarUsuarios();

            cout << "\nUsuario agregado correctamente.\n";

        } else if (opcion == 3) {
            string nombre;
            cout << "\nNombre a eliminar: ";
            cin >> nombre;
            if (nombre == "admin") {
                cout << "No puedes eliminar al administrador.\n";
                continue;
            }

            auto sizeAntes = usuarios.size();
            usuarios.erase(remove_if(usuarios.begin(), usuarios.end(),
                [&](const Usuario& u){ return u.nombre == nombre; }), usuarios.end());

            if (usuarios.size() < sizeAntes) {
                guardarUsuarios();
                cout << "Usuario eliminado correctamente.\n";
            } else {
                cout << "Usuario no encontrado.\n";
            }
        }

    } while (opcion != 4);
}



// ==== MAIN ====

int main() {
    cleanSc();
    cargarUsuarios();
    if (!buscarUsuario("admin")) {
        usuarios.push_back({"admin", true, true, true});
        guardarUsuarios();
    }
    cout << "====== MINI SISTEMA DE ARCHIVOS (Simulación) ======\n\n";
    cout << "Cargando sistema desde " << FS_FILENAME << " (si existe)...\n";
    if (!loadFS(FS_FILENAME)) {
        // inicializar nuevo
        cout << "Defina el nombre de la carpeta raíz: ";
        string nombreRaiz;
        cin >> nombreRaiz;
        raiz = new Nodo(nombreRaiz.empty() ? "root" : nombreRaiz, false, nullptr);
        raiz->owner = "admin";
        raiz->ownerR = true; raiz->ownerW = true;
        raiz->othersR = true; raiz->othersW = false;
        actual = raiz;
    }

    int opcion;
    do {
        cleanSc();
        cout << "\n==== Carpeta actual: " << actual->nombre << "  (usuario: " << usuarioActual << ") ====\n\n";
        cout << "1. Crear\n";
        cout << "2. Eliminar\n";
        cout << "3. Mostrar contenido\n";
        cout << "4. Acceder a carpeta\n";
        cout << "5. Subir un nivel\n";
        cout << "6. Volver a la raíz\n";
        cout << "7. Mostrar mapa de bloques\n";
        cout << "8. Login (cambiar usuario)\n";
        cout << "9. Modificar tamaño de archivo (inc/dec)\n";
        cout << "10. Cambiar permisos\n";
        cout << "11. Guardar ahora\n";
        cout << "12. Cargar desde archivo (sobrescribe memoria)\n";
        cout << "13. Gestionar usuarios\n";
        cout << "14. Salir\n\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
        case 1:
            crearElemento();
            break;
        case 2:
            eliminarElementoMenu();
            break;
        case 3:
            cleanSc();
            mostrarContenido(actual);
            presionarEnter();
            break;
        case 4:
            accederDirectorio();
            break;
        case 5:
            subirNivel();
            break;
        case 6:
            subirARaiz();
            break;
        case 7:
            mostrarMapaBloques();
            break;
        case 8:
            login();
            break;
        case 9:
            modificarTamArchivo();
            break;
        case 10:
            cambiarPermisos();
            break;
        case 11:
            cleanSc();
            saveFS(FS_FILENAME);
            presionarEnter();
            break;
        case 12:
            cleanSc();
            if (loadFS(FS_FILENAME)) {
                cout << "Carga completa.\n";
            } else {
                cout << "Error al cargar.\n";
            }
            presionarEnter();
            break;
        case 13:
            cleanSc();
            gestionarUsuarios();
            //presionarEnter();
            break;
        case 14:
            cout << "\nGuardando sistema antes de salir...\n";
            saveFS(FS_FILENAME);
            segundosPausa(1);
            break;
        default:
            cout << "\nOpción no válida.\n";
            presionarEnter();
        }

    } while (opcion != 14);

    // liberar memoria (recursivo)
    function<void(Nodo*)> liberar = [&](Nodo* n) {
        for (auto &h : n->hijos) liberar(h.get());
        delete n;
    };
    if (raiz) liberar(raiz);
    return 0;
}
