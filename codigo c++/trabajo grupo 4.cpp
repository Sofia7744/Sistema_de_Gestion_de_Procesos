#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

// ============================================================================
// ESTRUCTURAS DE DATOS
// ============================================================================

// Estructura para representar un proceso
struct Proceso {
    int id;
    string nombre;
    int prioridad;
    string estado;
    Proceso* siguiente;
};

// Estructura para la cola de prioridad
struct NodoCola {
    int id;
    string nombre;
    int prioridad;
    NodoCola* siguiente;
};

// Estructura para la pila de memoria
struct NodoMemoria {
    int bloqueID;
    int tamanio;
    string proceso;
    NodoMemoria* siguiente;
};

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

Proceso* listaProcesos = NULL;
NodoCola* frenteCola = NULL;
NodoCola* finalCola = NULL;
NodoMemoria* topePila = NULL;
int contadorProcesos = 1;
int contadorBloques = 1;

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void pausar() {
    cout << "\nPresione Enter para continuar...";
    cin.ignore();
    cin.get();
}

void limpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// ============================================================================
// MODULO 1: GESTION DE PROCESOS (LISTA ENLAZADA)
// ============================================================================

// Insertar nuevo proceso al final de la lista
void insertarProceso(int id, string nombre, int prioridad, string estado) {
    Proceso* nuevo = new Proceso;
    nuevo->id = id;
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->estado = estado;
    nuevo->siguiente = NULL;
    
    if (listaProcesos == NULL) {
        listaProcesos = nuevo;
    } else {
        Proceso* aux = listaProcesos;
        while (aux->siguiente != NULL) {
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;
    }
}

// Buscar proceso por ID
Proceso* buscarProcesoPorID(int id) {
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        if (aux->id == id) {
            return aux;
        }
        aux = aux->siguiente;
    }
    return NULL;
}

// Buscar proceso por nombre
Proceso* buscarProcesoPorNombre(string nombre) {
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        if (aux->nombre == nombre) {
            return aux;
        }
        aux = aux->siguiente;
    }
    return NULL;
}

// Eliminar proceso por ID
bool eliminarProceso(int id) {
    if (listaProcesos == NULL) {
        return false;
    }
    
    // Si es el primero
    if (listaProcesos->id == id) {
        Proceso* temp = listaProcesos;
        listaProcesos = listaProcesos->siguiente;
        delete temp;
        return true;
    }
    
    // Buscar en el resto de la lista
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

// Modificar estado de un proceso
bool modificarEstado(int id, string nuevoEstado) {
    Proceso* proceso = buscarProcesoPorID(id);
    if (proceso != NULL) {
        proceso->estado = nuevoEstado;
        return true;
    }
    return false;
}

// Modificar prioridad de un proceso
bool modificarPrioridad(int id, int nuevaPrioridad) {
    Proceso* proceso = buscarProcesoPorID(id);
    if (proceso != NULL) {
        proceso->prioridad = nuevaPrioridad;
        return true;
    }
    return false;
}

// Mostrar todos los procesos
void mostrarProcesos() {
    if (listaProcesos == NULL) {
        cout << "\n[!] No hay procesos registrados.\n";
        return;
    }
    
    cout << "\n========================================\n";
    cout << "       LISTA DE PROCESOS\n";
    cout << "========================================\n";
    
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        cout << "\nID: " << aux->id;
        cout << "\nNombre: " << aux->nombre;
        cout << "\nPrioridad: " << aux->prioridad;
        cout << "\nEstado: " << aux->estado;
        cout << "\n----------------------------------------\n";
        aux = aux->siguiente;
    }
}

// Guardar procesos en archivo
void guardarProcesos() {
    ofstream archivo("procesos.txt");
    if (!archivo) {
        cout << "\n[!] Error al crear el archivo.\n";
        return;
    }
    
    Proceso* aux = listaProcesos;
    while (aux != NULL) {
        archivo << aux->id << "|" << aux->nombre << "|" 
                << aux->prioridad << "|" << aux->estado << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Procesos guardados correctamente.\n";
}

// Cargar procesos desde archivo
void cargarProcesos() {
    ifstream archivo("procesos.txt");
    if (!archivo) {
        return; // Archivo no existe aun
    }
    
    // Limpiar lista actual
    while (listaProcesos != NULL) {
        Proceso* temp = listaProcesos;
        listaProcesos = listaProcesos->siguiente;
        delete temp;
    }
    
    int id, prioridad;
    string nombre, estado, linea;
    
    while (getline(archivo, linea)) {
        size_t pos1 = linea.find('|');
        size_t pos2 = linea.find('|', pos1 + 1);
        size_t pos3 = linea.find('|', pos2 + 1);
        
        id = atoi(linea.substr(0, pos1).c_str());
        nombre = linea.substr(pos1 + 1, pos2 - pos1 - 1);
        prioridad = atoi(linea.substr(pos2 + 1, pos3 - pos2 - 1).c_str());
        estado = linea.substr(pos3 + 1);
        
        insertarProceso(id, nombre, prioridad, estado);
        
        if (id >= contadorProcesos) {
            contadorProcesos = id + 1;
        }
    }
    archivo.close();
}

// ============================================================================
// MODULO 2: PLANIFICACION DE CPU (COLA DE PRIORIDAD)
// ============================================================================

// Encolar proceso con prioridad
void encolarConPrioridad(int id, string nombre, int prioridad) {
    NodoCola* nuevo = new NodoCola;
    nuevo->id = id;
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->siguiente = NULL;
    
    // Si la cola esta vacia o tiene mayor prioridad que el frente
    if (frenteCola == NULL || prioridad > frenteCola->prioridad) {
        nuevo->siguiente = frenteCola;
        frenteCola = nuevo;
        if (finalCola == NULL) {
            finalCola = nuevo;
        }
    } else {
        // Buscar posicion segun prioridad
        NodoCola* aux = frenteCola;
        while (aux->siguiente != NULL && aux->siguiente->prioridad >= prioridad) {
            aux = aux->siguiente;
        }
        nuevo->siguiente = aux->siguiente;
        aux->siguiente = nuevo;
        
        if (nuevo->siguiente == NULL) {
            finalCola = nuevo;
        }
    }
}

// Desencolar (ejecutar proceso)
bool desencolar() {
    if (frenteCola == NULL) {
        return false;
    }
    
    NodoCola* temp = frenteCola;
    cout << "\n[EJECUTANDO] Proceso ID: " << temp->id 
         << " | Nombre: " << temp->nombre 
         << " | Prioridad: " << temp->prioridad << "\n";
    
    frenteCola = frenteCola->siguiente;
    if (frenteCola == NULL) {
        finalCola = NULL;
    }
    
    delete temp;
    return true;
}

// Mostrar cola de procesos
void mostrarCola() {
    if (frenteCola == NULL) {
        cout << "\n[!] La cola de CPU esta vacia.\n";
        return;
    }
    
    cout << "\n========================================\n";
    cout << "       COLA DE PLANIFICACION CPU\n";
    cout << "========================================\n";
    
    NodoCola* aux = frenteCola;
    int posicion = 1;
    while (aux != NULL) {
        cout << "\nPosicion: " << posicion++;
        cout << "\nID: " << aux->id;
        cout << "\nNombre: " << aux->nombre;
        cout << "\nPrioridad: " << aux->prioridad;
        cout << "\n----------------------------------------\n";
        aux = aux->siguiente;
    }
}

// Guardar cola en archivo
void guardarCola() {
    ofstream archivo("cola_cpu.txt");
    if (!archivo) {
        cout << "\n[!] Error al crear el archivo.\n";
        return;
    }
    
    NodoCola* aux = frenteCola;
    while (aux != NULL) {
        archivo << aux->id << "|" << aux->nombre << "|" 
                << aux->prioridad << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Cola guardada correctamente.\n";
}

// Cargar cola desde archivo
void cargarCola() {
    ifstream archivo("cola_cpu.txt");
    if (!archivo) {
        return;
    }
    
    // Limpiar cola actual
    while (frenteCola != NULL) {
        NodoCola* temp = frenteCola;
        frenteCola = frenteCola->siguiente;
        delete temp;
    }
    finalCola = NULL;
    
    int id, prioridad;
    string nombre, linea;
    
    while (getline(archivo, linea)) {
        size_t pos1 = linea.find('|');
        size_t pos2 = linea.find('|', pos1 + 1);
        
        id = atoi(linea.substr(0, pos1).c_str());
        nombre = linea.substr(pos1 + 1, pos2 - pos1 - 1);
        prioridad = atoi(linea.substr(pos2 + 1).c_str());
        
        encolarConPrioridad(id, nombre, prioridad);
    }
    archivo.close();
}

// ============================================================================
// MODULO 3: GESTION DE MEMORIA (PILA)
// ============================================================================

// Asignar bloque de memoria (push)
void asignarMemoria(int tamanio, string proceso) {
    NodoMemoria* nuevo = new NodoMemoria;
    nuevo->bloqueID = contadorBloques++;
    nuevo->tamanio = tamanio;
    nuevo->proceso = proceso;
    nuevo->siguiente = topePila;
    topePila = nuevo;
    
    cout << "\n[OK] Bloque " << nuevo->bloqueID 
         << " asignado (" << tamanio << " KB) a: " << proceso << "\n";
}

// Liberar bloque de memoria (pop)
bool liberarMemoria() {
    if (topePila == NULL) {
        return false;
    }
    
    NodoMemoria* temp = topePila;
    cout << "\n[LIBERADO] Bloque " << temp->bloqueID 
         << " (" << temp->tamanio << " KB) de: " << temp->proceso << "\n";
    
    topePila = topePila->siguiente;
    delete temp;
    return true;
}

// Mostrar estado de la memoria
void mostrarMemoria() {
    if (topePila == NULL) {
        cout << "\n[!] No hay bloques de memoria asignados.\n";
        return;
    }
    
    cout << "\n========================================\n";
    cout << "       PILA DE MEMORIA\n";
    cout << "========================================\n";
    
    NodoMemoria* aux = topePila;
    int totalMemoria = 0;
    
    while (aux != NULL) {
        cout << "\nBloque ID: " << aux->bloqueID;
        cout << "\nTamanio: " << aux->tamanio << " KB";
        cout << "\nProceso: " << aux->proceso;
        cout << "\n----------------------------------------\n";
        totalMemoria += aux->tamanio;
        aux = aux->siguiente;
    }
    
    cout << "\nMemoria total asignada: " << totalMemoria << " KB\n";
}

// Guardar pila en archivo
void guardarMemoria() {
    ofstream archivo("memoria.txt");
    if (!archivo) {
        cout << "\n[!] Error al crear el archivo.\n";
        return;
    }
    
    NodoMemoria* aux = topePila;
    while (aux != NULL) {
        archivo << aux->bloqueID << "|" << aux->tamanio << "|" 
                << aux->proceso << "\n";
        aux = aux->siguiente;
    }
    archivo.close();
    cout << "\n[OK] Memoria guardada correctamente.\n";
}

// Cargar pila desde archivo
void cargarMemoria() {
    ifstream archivo("memoria.txt");
    if (!archivo) {
        return;
    }
    
    // Limpiar pila actual
    while (topePila != NULL) {
        NodoMemoria* temp = topePila;
        topePila = topePila->siguiente;
        delete temp;
    }
    
    int bloqueID, tamanio;
    string proceso, linea;
    
    // Leer en orden inverso para mantener el orden LIFO
    string lineas[1000];
    int count = 0;
    
    while (getline(archivo, linea) && count < 1000) {
        lineas[count++] = linea;
    }
    
    // Insertar en orden inverso
    for (int i = count - 1; i >= 0; i--) {
        size_t pos1 = lineas[i].find('|');
        size_t pos2 = lineas[i].find('|', pos1 + 1);
        
        bloqueID = atoi(lineas[i].substr(0, pos1).c_str());
        tamanio = atoi(lineas[i].substr(pos1 + 1, pos2 - pos1 - 1).c_str());
        proceso = lineas[i].substr(pos2 + 1);
        
        NodoMemoria* nuevo = new NodoMemoria;
        nuevo->bloqueID = bloqueID;
        nuevo->tamanio = tamanio;
        nuevo->proceso = proceso;
        nuevo->siguiente = topePila;
        topePila = nuevo;
        
        if (bloqueID >= contadorBloques) {
            contadorBloques = bloqueID + 1;
        }
    }
    archivo.close();
}

// ============================================================================
// MENUS DEL SISTEMA
// ============================================================================

void menuGestionProcesos() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n========================================\n";
        cout << "    GESTION DE PROCESOS\n";
        cout << "========================================\n";
        cout << "1. Agregar nuevo proceso\n";
        cout << "2. Buscar proceso por ID\n";
        cout << "3. Buscar proceso por nombre\n";
        cout << "4. Modificar estado de proceso\n";
        cout << "5. Modificar prioridad de proceso\n";
        cout << "6. Eliminar proceso\n";
        cout << "7. Mostrar todos los procesos\n";
        cout << "8. Guardar procesos\n";
        cout << "0. Volver al menu principal\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                string nombre, estado;
                int prioridad;
                
                cout << "\n=== AGREGAR NUEVO PROCESO ===\n";
                cout << "Nombre del proceso: ";
                getline(cin, nombre);
                cout << "Prioridad (1-10): ";
                cin >> prioridad;
                cin.ignore();
                cout << "Estado (Listo/Ejecucion/Bloqueado): ";
                getline(cin, estado);
                
                insertarProceso(contadorProcesos++, nombre, prioridad, estado);
                cout << "\n[OK] Proceso agregado con ID: " << (contadorProcesos - 1) << "\n";
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                int id;
                cout << "\n=== BUSCAR PROCESO POR ID ===\n";
                cout << "Ingrese ID: ";
                cin >> id;
                cin.ignore();
                
                Proceso* proc = buscarProcesoPorID(id);
                if (proc != NULL) {
                    cout << "\n[ENCONTRADO]\n";
                    cout << "ID: " << proc->id << "\n";
                    cout << "Nombre: " << proc->nombre << "\n";
                    cout << "Prioridad: " << proc->prioridad << "\n";
                    cout << "Estado: " << proc->estado << "\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 3: {
                limpiarPantalla();
                string nombre;
                cout << "\n=== BUSCAR PROCESO POR NOMBRE ===\n";
                cout << "Ingrese nombre: ";
                getline(cin, nombre);
                
                Proceso* proc = buscarProcesoPorNombre(nombre);
                if (proc != NULL) {
                    cout << "\n[ENCONTRADO]\n";
                    cout << "ID: " << proc->id << "\n";
                    cout << "Nombre: " << proc->nombre << "\n";
                    cout << "Prioridad: " << proc->prioridad << "\n";
                    cout << "Estado: " << proc->estado << "\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 4: {
                limpiarPantalla();
                int id;
                string nuevoEstado;
                cout << "\n=== MODIFICAR ESTADO ===\n";
                cout << "ID del proceso: ";
                cin >> id;
                cin.ignore();
                cout << "Nuevo estado: ";
                getline(cin, nuevoEstado);
                
                if (modificarEstado(id, nuevoEstado)) {
                    cout << "\n[OK] Estado actualizado correctamente.\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 5: {
                limpiarPantalla();
                int id, nuevaPrioridad;
                cout << "\n=== MODIFICAR PRIORIDAD ===\n";
                cout << "ID del proceso: ";
                cin >> id;
                cout << "Nueva prioridad (1-10): ";
                cin >> nuevaPrioridad;
                cin.ignore();
                
                if (modificarPrioridad(id, nuevaPrioridad)) {
                    cout << "\n[OK] Prioridad actualizada correctamente.\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 6: {
                limpiarPantalla();
                int id;
                cout << "\n=== ELIMINAR PROCESO ===\n";
                cout << "ID del proceso a eliminar: ";
                cin >> id;
                cin.ignore();
                
                if (eliminarProceso(id)) {
                    cout << "\n[OK] Proceso eliminado correctamente.\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 7: {
                limpiarPantalla();
                mostrarProcesos();
                pausar();
                break;
            }
            case 8: {
                limpiarPantalla();
                guardarProcesos();
                pausar();
                break;
            }
            case 0:
                break;
            default:
                cout << "\n[!] Opcion invalida.\n";
                pausar();
        }
    } while (opcion != 0);
}

void menuPlanificacionCPU() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n========================================\n";
        cout << "    PLANIFICACION DE CPU\n";
        cout << "========================================\n";
        cout << "1. Encolar proceso desde lista\n";
        cout << "2. Ejecutar siguiente proceso (Desencolar)\n";
        cout << "3. Mostrar cola de ejecucion\n";
        cout << "4. Guardar cola\n";
        cout << "0. Volver al menu principal\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                int id;
                cout << "\n=== ENCOLAR PROCESO ===\n";
                mostrarProcesos();
                cout << "\nID del proceso a encolar: ";
                cin >> id;
                cin.ignore();
                
                Proceso* proc = buscarProcesoPorID(id);
                if (proc != NULL) {
                    encolarConPrioridad(proc->id, proc->nombre, proc->prioridad);
                    modificarEstado(id, "En cola");
                    cout << "\n[OK] Proceso encolado correctamente.\n";
                } else {
                    cout << "\n[!] Proceso no encontrado.\n";
                }
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                if (desencolar()) {
                    cout << "\n[OK] Proceso ejecutado y desencolado.\n";
                } else {
                    cout << "\n[!] La cola esta vacia.\n";
                }
                pausar();
                break;
            }
            case 3: {
                limpiarPantalla();
                mostrarCola();
                pausar();
                break;
            }
            case 4: {
                limpiarPantalla();
                guardarCola();
                pausar();
                break;
            }
            case 0:
                break;
            default:
                cout << "\n[!] Opcion invalida.\n";
                pausar();
        }
    } while (opcion != 0);
}

void menuGestionMemoria() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n========================================\n";
        cout << "    GESTION DE MEMORIA\n";
        cout << "========================================\n";
        cout << "1. Asignar bloque de memoria\n";
        cout << "2. Liberar ultimo bloque (pop)\n";
        cout << "3. Mostrar estado de memoria\n";
        cout << "4. Guardar estado de memoria\n";
        cout << "0. Volver al menu principal\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                int tamanio;
                string proceso;
                cout << "\n=== ASIGNAR MEMORIA ===\n";
                cout << "Tamanio del bloque (KB): ";
                cin >> tamanio;
                cin.ignore();
                cout << "Nombre del proceso: ";
                getline(cin, proceso);
                
                asignarMemoria(tamanio, proceso);
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                if (liberarMemoria()) {
                    cout << "\n[OK] Bloque liberado correctamente.\n";
                } else {
                    cout << "\n[!] No hay bloques para liberar.\n";
                }
                pausar();
                break;
            }
            case 3: {
                limpiarPantalla();
                mostrarMemoria();
                pausar();
                break;
            }
            case 4: {
                limpiarPantalla();
                guardarMemoria();
                pausar();
                break;
            }
            case 0:
                break;
            default:
                cout << "\n[!] Opcion invalida.\n";
                pausar();
        }
    } while (opcion != 0);
}

// ============================================================================
// MENU PRINCIPAL
// ============================================================================

void menuPrincipal() {
    int opcion;
    do {
        limpiarPantalla();
        cout << "\n========================================\n";
        cout << "  SISTEMA DE GESTION DE PROCESOS\n";
        cout << "========================================\n";
        cout << "1. Gestion de Procesos (Lista Enlazada)\n";
        cout << "2. Planificacion de CPU (Cola de Prioridad)\n";
        cout << "3. Gestion de Memoria (Pila)\n";
        cout << "4. Guardar todo el sistema\n";
        cout << "0. Salir\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();
        
        switch (opcion) {
            case 1:
                menuGestionProcesos();
                break;
            case 2:
                menuPlanificacionCPU();
                break;
            case 3:
                menuGestionMemoria();
                break;
            case 4:
                limpiarPantalla();
                guardarProcesos();
                guardarCola();
                guardarMemoria();
                cout << "\n[OK] Sistema completo guardado.\n";
                pausar();
                break;
            case 0:
                cout << "\n[!] Saliendo del sistema...\n";
                guardarProcesos();
                guardarCola();
                guardarMemoria();
                break;
            default:
                cout << "\n[!] Opcion invalida.\n";
                pausar();
        }
    } while (opcion != 0);
}

// ============================================================================
// FUNCION PRINCIPAL
// ============================================================================

int main() {
    // Cargar datos persistentes al iniciar
    cargarProcesos();
    cargarCola();
    cargarMemoria();
    
    cout << "\n========================================\n";
    cout << "  BIENVENIDO AL SISTEMA DE GESTION\n";
    cout << "         DE PROCESOS\n";
    cout << "========================================\n";
    cout << "\nCargando datos del sistema...\n";
    pausar();
    
    menuPrincipal();
    
    // Liberar memoria antes de salir
    while (listaProcesos != NULL) {
        Proceso* temp = listaProcesos;
        listaProcesos = listaProcesos->siguiente;
        delete temp;
    }
    
    while (frenteCola != NULL) {
        NodoCola* temp = frenteCola;
        frenteCola = frenteCola->siguiente;
        delete temp;
    }
    
    while (topePila != NULL) {
        NodoMemoria* temp = topePila;
        topePila = topePila->siguiente;
        delete temp;
    }
    
    cout << "\n[!] Gracias por usar el sistema. Hasta pronto!\n\n";
    
    return 0;
}
