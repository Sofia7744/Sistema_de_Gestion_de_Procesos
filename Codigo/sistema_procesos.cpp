#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

// ============= ESTRUCTURAS =============

struct Proceso {
    int id;
    string nombre;
    int prioridad;
    string estado;
    Proceso* siguiente;
};

struct NodoMemoria {
    int bloqueID;
    int tamanio;
    string proceso;
    NodoMemoria* siguiente;
};

struct NodoCola {
    int procesID;
    string nombreProceso;
    int prioridad;
    NodoCola* siguiente;
};

// ============= VARIABLES GLOBALES =============

Proceso* listaProcesos = NULL;
NodoMemoria* topePila = NULL;
NodoCola* inicialCola = NULL;
int contadorProcesos = 1;
int contadorBloques = 1;

// ============= FUNCIONES AUXILIARES =============

void limpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pausar() {
    cout << "\nPresione Enter para continuar...";
    cin.ignore();
    cin.get();
}

// ============= GESTION DE PROCESOS =============

void insertarProceso(string nombre, int prioridad, string estado) {
    Proceso* nuevo = new Proceso;
    nuevo->id = contadorProcesos++;
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->estado = estado;
    nuevo->siguiente = NULL;
    
    if (listaProcesos == NULL) {
        listaProcesos = nuevo;
    } else {
        Proceso* aux = listaProcesos;
        while (aux->siguiente != NULL) aux = aux->siguiente;
        aux->siguiente = nuevo;
    }
    cout << "\n[OK] Proceso '" << nombre << "' agregado con ID: " << (nuevo->id) << "\n";
}

Proceso* buscarProcesoPorID(int id) {
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        if (aux->id == id) return aux;
        aux = aux->siguiente;
    }
    return NULL;
}

bool modificarProceso(int id, string nuevoEstado, int nuevaPrioridad) {
    Proceso* proc = buscarProcesoPorID(id);
    if (proc != NULL) {
        if (nuevoEstado != "") proc->estado = nuevoEstado;
        if (nuevaPrioridad > 0) proc->prioridad = nuevaPrioridad;
        return true;
    }
    return false;
}

bool eliminarProceso(int id) {
    if (listaProcesos == NULL) return false;
    
    if (listaProcesos->id == id) {
        Proceso* temp = listaProcesos;
        listaProcesos = listaProcesos->siguiente;
        delete temp;
        return true;
    }
    
    Proceso* aux = listaProcesos;
    while (aux->siguiente != NULL) {
        if (aux->siguiente->id == id) {
            Proceso* temp = aux->siguiente;
            aux->siguiente = temp->siguiente;
            delete temp;
            return true;
        }
        aux = aux->siguiente;
    }
    return false;
}

void mostrarProcesos() {
    if (listaProcesos == NULL) {
        cout << "\n[!] No hay procesos registrados.\n";
        return;
    }
    
    cout << "\n========== PROCESOS ==========\n";
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        cout << "ID: " << aux->id << " | Nombre: " << aux->nombre 
             << " | Prioridad: " << aux->prioridad << " | Estado: " << aux->estado << "\n";
        aux = aux->siguiente;
    }
    cout << "==============================\n";
}

void guardarProcesos() {
    ofstream archivo("procesos.txt");
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        archivo << aux->id << "|" << aux->nombre << "|" 
                << aux->prioridad << "|" << aux->estado << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Procesos guardados.\n";
}

void cargarProcesos() {
    ifstream archivo("procesos.txt");
    if (!archivo) return;
    
    string linea;
    while (getline(archivo, linea)) {
        size_t p1 = linea.find('|'), p2 = linea.find('|', p1 + 1), p3 = linea.find('|', p2 + 1);
        int id = atoi(linea.substr(0, p1).c_str());
        string nombre = linea.substr(p1 + 1, p2 - p1 - 1);
        int prioridad = atoi(linea.substr(p2 + 1, p3 - p2 - 1).c_str());
        string estado = linea.substr(p3 + 1);
        
        Proceso* nuevo = new Proceso;
        nuevo->id = id;
        nuevo->nombre = nombre;
        nuevo->prioridad = prioridad;
        nuevo->estado = estado;
        nuevo->siguiente = listaProcesos;
        listaProcesos = nuevo;
        
        if (id >= contadorProcesos) contadorProcesos = id + 1;
    }
    archivo.close();
}

// ============= GESTION DE PLANIFICADOR CPU (COLA DE PRIORIDAD) =============

void encolarProceso(int procesID, string nombreProceso, int prioridad) {
    NodoCola* nuevo = new NodoCola;
    nuevo->procesID = procesID;
    nuevo->nombreProceso = nombreProceso;
    nuevo->prioridad = prioridad;
    nuevo->siguiente = NULL;
    
    if (inicialCola == NULL) {
        inicialCola = nuevo;
    } else if (nuevo->prioridad > inicialCola->prioridad) {
        nuevo->siguiente = inicialCola;
        inicialCola = nuevo;
    } else {
        NodoCola* aux = inicialCola;
        while (aux->siguiente != NULL && aux->siguiente->prioridad > nuevo->prioridad) {
            aux = aux->siguiente;
        }
        nuevo->siguiente = aux->siguiente;
        aux->siguiente = nuevo;
    }
    cout << "\n[OK] Proceso '" << nombreProceso << "' (Prioridad: " << prioridad << ") encolado.\n";
}

bool desencolaYEjecutar() {
    if (inicialCola == NULL) {
        cout << "\n[!] Cola vacía, no hay procesos para ejecutar.\n";
        return false;
    }
    
    NodoCola* temp = inicialCola;
    cout << "\n[EJECUTANDO] Proceso: " << temp->nombreProceso << " (ID: " << temp->procesID 
         << ", Prioridad: " << temp->prioridad << ")\n";
    
    Proceso* proc = buscarProcesoPorID(temp->procesID);
    if (proc != NULL) {
        proc->estado = "Terminado";
    }
    
    inicialCola = inicialCola->siguiente;
    delete temp;
    return true;
}

void mostrarCola() {
    if (inicialCola == NULL) {
        cout << "\n[!] Cola vacía.\n";
        return;
    }
    
    cout << "\n========== COLA DE PROCESOS ==========\n";
    int posicion = 1;
    NodoCola* aux = inicialCola;
    while (aux != NULL) {
        cout << posicion << ". ID: " << aux->procesID << " | Nombre: " << aux->nombreProceso 
             << " | Prioridad: " << aux->prioridad << "\n";
        aux = aux->siguiente;
        posicion++;
    }
    cout << "======================================\n";
}

void guardarCola() {
    ofstream archivo("cola_procesos.txt");
    NodoCola* aux = inicialCola;
    while (aux != NULL) {
        archivo << aux->procesID << "|" << aux->nombreProceso << "|" << aux->prioridad << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Cola guardada.\n";
}

void cargarCola() {
    ifstream archivo("cola_procesos.txt");
    if (!archivo) return;
    
    string linea;
    while (getline(archivo, linea)) {
        size_t p1 = linea.find('|'), p2 = linea.find('|', p1 + 1);
        int procesID = atoi(linea.substr(0, p1).c_str());
        string nombreProceso = linea.substr(p1 + 1, p2 - p1 - 1);
        int prioridad = atoi(linea.substr(p2 + 1).c_str());
        
        encolarProceso(procesID, nombreProceso, prioridad);
    }
    archivo.close();
}

void vaciarCola() {
    while (inicialCola != NULL) {
        NodoCola* temp = inicialCola;
        inicialCola = inicialCola->siguiente;
        delete temp;
    }
    cout << "\n[OK] Cola vaciada.\n";
}

// ============= GESTION DE MEMORIA =============

void asignarMemoria(int tamanio, string proceso) {
    NodoMemoria* nuevo = new NodoMemoria;
    nuevo->bloqueID = contadorBloques++;
    nuevo->tamanio = tamanio;
    nuevo->proceso = proceso;
    nuevo->siguiente = topePila;
    topePila = nuevo;
    cout << "\n[OK] Bloque " << nuevo->bloqueID << " (" << tamanio << " KB) asignado a: " << proceso << "\n";
}

bool liberarMemoria() {
    if (topePila == NULL) return false;
    
    NodoMemoria* temp = topePila;
    cout << "\n[OK] Bloque " << temp->bloqueID << " (" << temp->tamanio << " KB) liberado.\n";
    topePila = topePila->siguiente;
    delete temp;
    return true;
}

void mostrarMemoria() {
    if (topePila == NULL) {
        cout << "\n[!] No hay bloques de memoria asignados.\n";
        return;
    }
    
    cout << "\n========== MEMORIA ==========\n";
    NodoMemoria* aux = topePila;
    int total = 0;
    while (aux != NULL) {
        cout << "Bloque " << aux->bloqueID << ": " << aux->tamanio 
             << " KB - Proceso: " << aux->proceso << "\n";
        total += aux->tamanio;
        aux = aux->siguiente;
    }
    cout << "Total: " << total << " KB\n";
    cout << "=============================\n";
}

void guardarMemoria() {
    ofstream archivo("memoria.txt");
    NodoMemoria* aux = topePila;
    while (aux != NULL) {
        archivo << aux->bloqueID << "|" << aux->tamanio << "|" << aux->proceso << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Memoria guardada.\n";
}

void cargarMemoria() {
    ifstream archivo("memoria.txt");
    if (!archivo) return;
    
    string linea;
    string lineas[1000];
    int count = 0;
    
    while (getline(archivo, linea) && count < 1000) lineas[count++] = linea;
    
    for (int i = count - 1; i >= 0; i--) {
        size_t p1 = lineas[i].find('|'), p2 = lineas[i].find('|', p1 + 1);
        int bloqueID = atoi(lineas[i].substr(0, p1).c_str());
        int tamanio = atoi(lineas[i].substr(p1 + 1, p2 - p1 - 1).c_str());
        string proceso = lineas[i].substr(p2 + 1);
        
        NodoMemoria* nuevo = new NodoMemoria;
        nuevo->bloqueID = bloqueID;
        nuevo->tamanio = tamanio;
        nuevo->proceso = proceso;
        nuevo->siguiente = topePila;
        topePila = nuevo;
        
        if (bloqueID >= contadorBloques) contadorBloques = bloqueID + 1;
    }
    archivo.close();
}

// ============= MENUS =============

void menuProcesos() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n=== GESTION DE PROCESOS ===\n";
        cout << "1. Agregar proceso\n2. Buscar proceso\n3. Modificar proceso\n4. Eliminar proceso\n5. Listar procesos\n6. Guardar\n0. Volver\n";
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                string nombre;
                int prioridad;
                char estadoChar;
                string estado;
            
                cout << "\n=== AGREGAR NUEVO PROCESO ===\n";
            
                // Validar nombre no vacío
                do {
                    cout << "Nombre: ";
                    getline(cin, nombre);
                    if (nombre.empty()) cout << "[!] El nombre no puede estar vacío.\n";
                } while (nombre.empty());
            
                // Validar prioridad (1–10)
                do {
                    cout << "Prioridad (1-10): ";
                    cin >> prioridad;
                    if (cin.fail() || prioridad < 1 || prioridad > 10) {
                        cout << "[!] Ingrese un número válido entre 1 y 10.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                        prioridad = -1; // reinicia el valor
                    }
                } while (prioridad < 1 || prioridad > 10);
                cin.ignore();
            
                // Validar estado
                cout << "Estado [L = Listo, E = Ejecucion, B = Bloqueado]: ";
                cin >> estadoChar;
                cin.ignore();
            
                estadoChar = toupper(estadoChar);
            
                if (estadoChar == 'L') estado = "Listo";
                else if (estadoChar == 'E') estado = "Ejecucion";
                else if (estadoChar == 'B') estado = "Bloqueado";
                else {
                    cout << "[!] Estado inválido. Se asignará 'Listo' por defecto.\n";
                    estado = "Listo";
                }

                insertarProceso(nombre, prioridad, estado);
                pausar();
                break;
            }
            case 2: {
                int id;
                cout << "\n=== BUSCAR PROCESO ===\n";
                // Validar entrada numérica
                while (true) {
                    cout << "Ingrese el ID del proceso: ";
                    cin >> id;
            
                    if (cin.fail()) {
                        cout << "[!] Ingrese un número válido.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                Proceso* proc = buscarProcesoPorID(id);
                if (proc) {
                    cout << "\n--- PROCESO ENCONTRADO ---\n";
                    cout << "ID: " << proc->id << "\nNombre: " << proc->nombre
                         << "\nPrioridad: " << proc->prioridad
                         << "\nEstado: " << proc->estado << "\n";
                } else {
                    cout << "\n[!] No se encontró un proceso con ID " << id << ".\n";
                }
                pausar();
                break;
            }
            case 3: {
                int id;
                int nuevaPrioridad;
                char estadoChar;
                string nuevoEstado = "";
                cout << "\n=== MODIFICAR PROCESO ===\n";
                // Validar entrada de ID
                while (true) {
                    cout << "Ingrese el ID del proceso: ";
                    cin >> id;
                    if (cin.fail()) {
                        cout << "[!] Ingrese un número válido.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                Proceso* proc = buscarProcesoPorID(id);
                if (!proc) {
                    cout << "[!] No se encontró un proceso con ese ID.\n";
                    pausar();
                    break;
                }
                // Mostrar proceso actual
                cout << "\n--- DATOS ACTUALES ---\n";
                cout << "Nombre: " << proc->nombre << "\n";
                cout << "Prioridad actual: " << proc->prioridad << "\n";
                cout << "Estado actual: " << proc->estado << "\n\n";
                // Modificar prioridad
                do {
                    cout << "Nueva prioridad (1-10, 0 = no cambiar): ";
                    cin >> nuevaPrioridad;
                    if (cin.fail()) {
                        cout << "[!] Ingrese un número válido.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                        nuevaPrioridad = -1;
                    } else if (nuevaPrioridad != 0 && (nuevaPrioridad < 1 || nuevaPrioridad > 10)) {
                        cout << "[!] Valor fuera de rango.\n";
                        nuevaPrioridad = -1;
                    }
                } while (nuevaPrioridad < 0);
                cin.ignore();
                // Modificar estado
                cout << "Nuevo estado [L = Listo, E = Ejecucion, B = Bloqueado, Enter = no cambiar]: ";
                estadoChar = cin.get();
                if (estadoChar != '\n') {
                    estadoChar = toupper(estadoChar);
                    if (estadoChar == 'L') nuevoEstado = "Listo";
                    else if (estadoChar == 'E') nuevoEstado = "Ejecucion";
                    else if (estadoChar == 'B') nuevoEstado = "Bloqueado";
                    else {
                        cout << "[!] Estado inválido. No se modificará.\n";
                    }
                }
                if (modificarProceso(id, nuevoEstado, nuevaPrioridad)) {
                    cout << "[OK] Proceso actualizado.\n";
                } else {
                    cout << "[!] No se pudo modificar el proceso.\n";
                }
                pausar();
                break;
            }
            case 4: {
                int id;
                char confirmacion;
                cout << "\n=== ELIMINAR PROCESO ===\n";
                // Validar entrada numérica
                while (true) {
                    cout << "Ingrese el ID del proceso a eliminar: ";
                    cin >> id;
                    if (cin.fail()) {
                        cout << "[!] Ingrese un número válido.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                Proceso* proc = buscarProcesoPorID(id);
                if (!proc) {
                    cout << "[!] No se encontró un proceso con ese ID.\n";
                    pausar();
                    break;
                }
                cout << "¿Está seguro que desea eliminar el proceso '" << proc->nombre << "'? (S/N): ";
                cin >> confirmacion;
                cin.ignore();
                confirmacion = toupper(confirmacion);
                if (confirmacion == 'S') {
                    if (eliminarProceso(id)) cout << "[OK] Proceso eliminado correctamente.\n";
                    else cout << "[!] Ocurrió un error al eliminar el proceso.\n";
                } else {
                    cout << "[!] Operación cancelada.\n";
                }
                pausar();
                break;
            }
            case 5: {
                mostrarProcesos();
                pausar();
                break;
            }
            case 6: {
                guardarProcesos();
                pausar();
                break;
            }
        }
    } while (opcion != 0);
}

void menuPlanificador() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n=== PLANIFICADOR DE CPU ===\n";
        cout << "1. Encolar proceso\n2. Ejecutar proceso (desencolar)\n3. Ver cola\n4. Guardar cola\n5. Vaciar cola\n0. Volver\n";
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                int id, prioridad;
                // Mostrar procesos disponibles
                mostrarProcesos();
                // Validar ID del proceso
                while (true) {
                    cout << "Ingrese el ID del proceso a encolar: ";
                    cin >> id;
                    if (cin.fail()) {
                        cout << "[!] Ingrese un número válido.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                // Buscar proceso existente
                Proceso* proc = buscarProcesoPorID(id);
                if (!proc) {
                    cout << "[!] No se encontró un proceso con ese ID.\n";
                    pausar();
                    break;
                }
                // Validar prioridad (1 a 10)
                while (true) {
                    cout << "Ingrese la prioridad en la cola (1-10, mayor = más urgente): ";
                    cin >> prioridad;
                    if (cin.fail() || prioridad < 1 || prioridad > 10) {
                        cout << "[!] Prioridad inválida. Intente nuevamente.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                encolarProceso(proc->id, proc->nombre, prioridad);
                cout << "[OK] Proceso agregado a la cola de ejecución.\n";
                pausar();
                break;
            }
            case 2: {
                if (!desencolaYEjecutar()) {
                    cout << "[!] No hay procesos en la cola.\n";
                } else {
                    cout << "[OK] Proceso ejecutado correctamente.\n";
                }
                pausar();
                break;
            }
            case 3: {
                mostrarCola();
                pausar();
                break;
            }
            case 4: {
                guardarCola();
                pausar();
                break;
            }
            case 5: {
                vaciarCola();
                pausar();
                break;
            }
        }
    } while (opcion != 0);
}

void menuMemoria() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n=== GESTION DE MEMORIA ===\n";
        cout << "1. Asignar memoria\n2. Liberar bloque\n3. Mostrar memoria\n4. Guardar\n0. Volver\n";
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                cout << "\n=== ASIGNAR BLOQUE DE MEMORIA ===\n";
                int tamanio;
                string proceso;
                // Validar tamaño del bloque
                while (true) {
                    cout << "Tamaño del bloque (en KB): ";
                    cin >> tamanio;
                    if (cin.fail() || tamanio <= 0) {
                        cout << "[!] Ingrese un número válido mayor que 0.\n";
                        cin.clear();
                        cin.ignore(1000, '\n');
                    } else {
                        cin.ignore();
                        break;
                    }
                }
                // Validar nombre del proceso (no vacío)
                do {
                    cout << "Nombre del proceso: ";
                    getline(cin, proceso);
                    if (proceso.empty()) {
                        cout << "[!] El nombre no puede estar vacío.\n";
                    }
                } while (proceso.empty());
                asignarMemoria(tamanio, proceso);
                cout << "[OK] Bloque de memoria asignado correctamente.\n";
                pausar();
                break;
            }
            case 2: {
                if (liberarMemoria()) {
                    cout << "[OK] Bloque liberado.\n";
                } else {
                    cout << "[!] No hay bloques para liberar.\n";
                }
                pausar();
                break;
            }
            case 3: {
                mostrarMemoria();
                pausar();
                break;
            }
            case 4: {
                guardarMemoria();
                pausar();
                break;
            }
        }
    } while (opcion != 0);
}

int main() {
    cargarProcesos();
    cargarMemoria();
    cargarCola();
    
    cout << "\n=== SISTEMA DE GESTION DE PROCESOS ===\n";
    cout << "Cargando...\n";
    pausar();
    
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n=== MENU PRINCIPAL ===\n";
        cout << "1. Gestion de Procesos (Lista Enlazada)\n2. Planificador de CPU (Cola Prioridad)\n3. Gestion de Memoria (Pila)\n4. Guardar todo\n0. Salir\n";
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1:
                menuProcesos();
                break;
            case 2:
                menuPlanificador();
                break;
            case 3:
                menuMemoria();
                break;
            case 4: {
                guardarProcesos();
                guardarMemoria();
                guardarCola();
                cout << "[OK] Sistema guardado.\n";
                pausar();
                break;
            }
        }
    } while (opcion != 0);
    
    while (inicialCola != NULL) {
        NodoCola* temp = inicialCola;
        inicialCola = inicialCola->siguiente;
        delete temp;
    }
    
    while (listaProcesos != NULL) {
        Proceso* temp = listaProcesos;
        listaProcesos = listaProcesos->siguiente;
        delete temp;
    }
    
    while (topePila != NULL) {
        NodoMemoria* temp = topePila;
        topePila = topePila->siguiente;
        delete temp;
    }
    
    cout << "\n[!] Hasta pronto!\n\n";
    return 0;
}
