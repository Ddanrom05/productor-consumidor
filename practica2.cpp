#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>

using namespace std;


/*
 * ============================================================
 * CONFIGURACIÓN
 * ============================================================
 */

const int BUFFER_SIZE = 5;

const char* ARCHIVO_RECURSO = "recurso.txt";

const char* ARCHIVO_BANDERA_P = "bandera_productor.txt";
const char* ARCHIVO_BANDERA_C = "bandera_consumidor.txt";
const char* ARCHIVO_TURNO     = "turno.txt";


/*
 * ============================================================
 * CLASE NODO
 * ============================================================
 */

class Nodo
{
private:

    int valor;

    Nodo* siguiente;


public:

    Nodo(int valor)
    {
        this->valor = valor;
        this->siguiente = nullptr;
    }


    int getValor()
    {
        return valor;
    }


    Nodo* getSiguiente()
    {
        return siguiente;
    }


    void setSiguiente(Nodo* siguiente)
    {
        this->siguiente = siguiente;
    }
};


/*
 * ============================================================
 * CLASE LISTA LIGADA
 * ============================================================
 *
 * Productor:
 *
 *      agrega al final.
 *
 * Consumidor:
 *
 *      elimina del inicio.
 *
 * ============================================================
 */

class ListaLigada
{
private:

    Nodo* cabeza;

    int cantidad;


public:

    ListaLigada()
    {
        cabeza = nullptr;
        cantidad = 0;
    }


    ~ListaLigada()
    {
        liberar();
    }

    bool estaVacia()
    {
        return cabeza == nullptr;
    }

    bool estaLlena()
    {
        return cantidad >= BUFFER_SIZE;
    }

    int getCantidad()
    {
        return cantidad;
    }


    /*
     * --------------------------------------------------------
     * insertarAlFinal() agrega un nodo al final.
     * --------------------------------------------------------
     */

    bool insertarAlFinal(int valor)
    {
        if(estaLlena())
        {
            return false;
        }


        Nodo* nuevo = new Nodo(valor);


        // Si la lista está vacía, el nuevo nodo se convierte en la cabeza.

        if(cabeza == nullptr)
        {
            cabeza = nuevo;

            cantidad++;

            return true;
        }


        // Buscar el último nodo.

        Nodo* actual = cabeza;


        while(actual->getSiguiente() != nullptr)
        {
            actual = actual->getSiguiente();
        }


        // Conecta el último nodo con el nuevo nodo.

        actual->setSiguiente(nuevo);

        cantidad++;


        return true;
    }


    /*
     * --------------------------------------------------------
     * eliminarDelInicio() elimina el primer nodo.
     * --------------------------------------------------------
     */

    bool eliminarDelInicio(int& valor)
    {
        if(cabeza == nullptr)
        {
            return false;
        }


        // Guardar el valor que será consumido.

        valor = cabeza->getValor();


        // Guardar el puntero al siguiente nodo, que se convertirá en la nueva cabeza.

        Nodo* siguiente = cabeza->getSiguiente();


        // Liberar la memoria del nodo que se va a eliminar.

        delete cabeza;


        // Actualizar la cabeza al siguiente nodo.

        cabeza = siguiente;

        cantidad--;


        return true;
    }

    bool eliminarPorValor(int valor)
    {
        if(cabeza == nullptr)
        {
            return false;
        }


        //Si el primer nodo tiene el valor, eliminarlo.

        if(cabeza->getValor() == valor)
        {
            Nodo* siguiente = cabeza->getSiguiente();


            delete cabeza;


            cabeza = siguiente;

            cantidad--;


            return true;
        }


        // Buscar el nodo con el valor.

        Nodo* anterior = cabeza;

        Nodo* actual =
            cabeza->getSiguiente();


        while(actual != nullptr)
        {
            if(actual->getValor() == valor)
            {
                anterior->setSiguiente(actual->getSiguiente());


                delete actual;

                cantidad--;


                return true;
            }


            anterior = actual;

            actual = actual->getSiguiente();
        }


        // No se encontró el valor.

        return false;
    }

    void imprimir(const string& nombre)
    {
        cout << "[" << nombre << "] Lista: ";


        if(cabeza == nullptr)
        {
            cout << "(vacia)" << endl;

            return;
        }


        Nodo* actual = cabeza;


        while(actual != nullptr)
        {
            cout << actual->getValor();


            if(actual->getSiguiente() != nullptr)
            {
                cout << " -> ";
            }


            actual = actual->getSiguiente();
        }


        cout << endl;
    }

    void liberar()
    {
        Nodo* actual = cabeza;


        while(actual != nullptr)
        {
            Nodo* siguiente = actual->getSiguiente();

            delete actual;

            actual = siguiente;
        }


        cabeza = nullptr;

        cantidad = 0;
    }
};


/*
 * ============================================================
 * CLASE PROTOCOLO
 * ============================================================
 *
 * recurso.txt funciona únicamente como canal de comunicación.
 *
 *
 * Formato:
 *
 *      2 0
 *
 *      Canal libre.
 *
 *
 *      1 valor
 *
 *      Operación: AGREGAR valor.
 *
 *
 *      0 valor
 *
 *      Operación: ELIMINAR valor.
 *
 * ============================================================
 */

class Protocolo
{
private:

    const char* archivo;


public:

    Protocolo()
    {
        archivo = ARCHIVO_RECURSO;
    }


    void inicializar()
    {
        ofstream salida(archivo);


        if(!salida)
        {
            cerr << "Error al crear "
                 << archivo
                 << endl;

            exit(1);
        }


        // Canal libre.

        salida << "2 0" << endl;


        salida.close();
    }

    void escribir(
        int operacion,
        int valor
    )
    {
        ofstream salida(archivo);


        if(!salida)
        {
            cerr << "Error al escribir "
                 << archivo
                 << endl;

            exit(1);
        }


        salida << operacion
               << " "
               << valor
               << endl;


        salida.close();
    }


    /*
     * --------------------------------------------------------
     * leer()
     * --------------------------------------------------------
     *
     * Devuelve:
     *
     *      0 = eliminar
     *      1 = agregar
     *      2 = libre
     *
     * El valor se devuelve mediante referencia.
     *
     * --------------------------------------------------------
     */

    int leer(int& valor)
    {
        ifstream entrada(archivo);


        if(!entrada)
        {
            valor = 0;

            return 2;
        }


        int operacion;


        entrada >> operacion;


        if(operacion == 0 || operacion == 1)
        {
            entrada >> valor;
        }
        else
        {
            valor = 0;
        }


        entrada.close();


        return operacion;
    }

    bool libre()
    {
        int valor;

        return leer(valor) == 2;
    }


    //Esperar hasta que el canal esté libre.

    void esperarLibre()
    {
        while(!libre())
        {
            // Espera activa.

            for(volatile long i = 0; i < 2000000; i++)
            {
                ;
            }
        }
    }
};


/*
 * ============================================================
 * CLASE PETERSON
 * ============================================================
 *
 * Adaptación de Peterson utilizando archivos.
 *
 *
 * bandera_productor:
 *
 *      0 = no quiere entrar.
 *      1 = quiere entrar.
 *
 *
 * bandera_consumidor:
 *
 *      0 = no quiere entrar.
 *      1 = quiere entrar.
 *
 *
 * turno:
 *
 *      P = productor.
 *      C = consumidor.
 *
 * ============================================================
 */

class Peterson
{
private:

    const char* banderaP;

    const char* banderaC;

    const char* archivoTurno;

    void escribirBandera(
        const char* archivo,
        int valor
    )
    {
        ofstream salida(archivo);


        if(!salida)
        {
            cerr << "Error al abrir "
                 << archivo
                 << endl;

            exit(1);
        }


        salida << valor << endl;


        salida.close();
    }

    int leerBandera(const char* archivo)
    {
        ifstream entrada(archivo);


        if(!entrada)
        {
            return 0;
        }


        int valor = 0;


        entrada >> valor;


        entrada.close();


        return valor;
    }

    void escribirTurno(char turno)
    {
        ofstream salida(archivoTurno);


        if(!salida)
        {
            cerr << "Error al abrir "
                 << archivoTurno
                 << endl;

            exit(1);
        }


        salida << turno << endl;


        salida.close();
    }

    char leerTurno()
    {
        ifstream entrada(archivoTurno);


        if(!entrada)
        {
            return 'P';
        }


        char turno;


        entrada >> turno;


        entrada.close();


        return turno;
    }

    void esperaActiva()
    {
        for(volatile long i = 0; i < 2000000; i++)
        {
            ;
        }
    }


public:

    /*
     * --------------------------------------------------------
     * Constructor
     * --------------------------------------------------------
     */

    Peterson()
    {
        banderaP = ARCHIVO_BANDERA_P;

        banderaC = ARCHIVO_BANDERA_C;

        archivoTurno = ARCHIVO_TURNO;
    }

    void inicializar()
    {
        escribirBandera(banderaP, 0);


        escribirBandera(banderaC, 0);


        // Inicializar el turno para la sincronización. El productor empieza primero porque el buffer está vacío.

        escribirTurno('P');
    }

    void entrarProductor()
    {
        // Indicar que el productor quiere entrar.

        escribirBandera(banderaP, 1);


        // Dar turno al consumidor.

        escribirTurno('C');


        // Esperar mientras el consumidor quiera entrar y sea su turno.

        while(leerBandera(banderaC) == 1 && leerTurno() == 'C')
        {
            esperaActiva();
        }
    }

    void salirProductor()
    {
        escribirBandera(banderaP, 0);
    }

    void entrarConsumidor()
    {
        // Indicar que el consumidor quiere entrar.

        escribirBandera(banderaC, 1);


        // Dar turno al productor.

        escribirTurno('P');


        //Esperar mientras el productor quiera entrar y sea su turno.

        while(leerBandera(banderaP) == 1 && leerTurno() == 'P'
        )
        {
            esperaActiva();
        }
    }


    /*
     * --------------------------------------------------------
     * salirConsumidor()
     * --------------------------------------------------------
     */

    void salirConsumidor()
    {
        escribirBandera(banderaC,0);
    }
};


/*
 * ============================================================
 * CLASE PRODUCTOR
 * ============================================================
 */

class Productor
{
private:

    ListaLigada lista;

    Protocolo protocolo;

    Peterson peterson;

    int siguienteDato;


public:

    Productor()
    {
        siguienteDato = 1;
    }


    /*
     * --------------------------------------------------------
     * replicarEliminacion()
     * --------------------------------------------------------
     */

    void replicarEliminacion(int valor)
    {
        bool eliminado = lista.eliminarPorValor(valor);


        if(eliminado)
        {
            cout << "[PRODUCTOR] "
                 << "Replica eliminacion: "
                 << valor
                 << endl;
        }
        else
        {
            cout << "[PRODUCTOR] "
                 << "No se encontro el valor "
                 << valor
                 << " para eliminar."
                 << endl;
        }


        lista.imprimir("PRODUCTOR");


        // Liberar el canal.

        protocolo.escribir(2,0);
    }


    /*
     * --------------------------------------------------------
     * producir()
     * --------------------------------------------------------
     */

    void producir()
    {
        // El canal debe estar libre antes de enviar una nueva operación.

        protocolo.esperarLibre();


        // Agregar el siguiente dato a la lista local del productor.

        int valor = siguienteDato;


        bool insertado = lista.insertarAlFinal(valor);


        if(!insertado)
        {
            return;
        }


        cout << endl;

        cout << "[PRODUCTOR] "
             << "Produjo: "
             << valor
             << endl;


        lista.imprimir("PRODUCTOR");


        // Comunicar la operación al consumidor.

        protocolo.escribir(1,valor);


        siguienteDato++;
    }


    void ejecutar()
    {
        while(true)
        {
            //Entrar a sección crítica para revisar el canal de comunicación.

            peterson.entrarProductor();


            // Revisar el valor que el consumidor haya eliminado de SU lista.

            int valor;

            int operacion = protocolo.leer(valor);


            if(operacion == 0)
            {
                // El consumidor eliminó un valor de SU lista.

                replicarEliminacion(valor);


                // Ya procesamos el mensaje.

                peterson.salirProductor();


                sleep(1);


                continue;
            }


            // Si el canal está libre y hay espacio, producir.

            if(operacion == 2 && !lista.estaLlena())
            {
                producir();
            }


            //Salir de sección crítica.

            peterson.salirProductor();


            // Tiempo de producción.

            sleep(1);
        }
    }
};


/*
 * ============================================================
 * CLASE CONSUMIDOR
 * ============================================================
 */

class Consumidor
{
private:

    ListaLigada lista;

    Protocolo protocolo;

    Peterson peterson;


public:

    /*
     * --------------------------------------------------------
     * replicarInsercion()
     * --------------------------------------------------------
     */

    void replicarInsercion(int valor)
    {
        // Verificar que la lista local del consumidor no esté llena.

        if(lista.estaLlena())
        {
            return;
        }


        // Agregar el valor a la lista local del consumidor.

        lista.insertarAlFinal(valor);


        cout << "[CONSUMIDOR] "
             << "Replica insercion: "
             << valor
             << endl;


        lista.imprimir("CONSUMIDOR");


        // Liberar el canal.

        protocolo.escribir(2, 0);
    }


    /*
     * --------------------------------------------------------
     * consumir() elimina el primer nodo.
     * --------------------------------------------------------
     */

    void consumir()
    {
        //El canal debe estar libre antes de enviar una nueva operación.

        protocolo.esperarLibre();


        int valor;


        // Eliminar el primer nodo de la lista local.

        bool eliminado = lista.eliminarDelInicio(valor);

        if(!eliminado)
        {
            return;
        }


        cout << endl;

        cout << "[CONSUMIDOR] "
             << "Consumio: "
             << valor
             << endl;


        lista.imprimir("CONSUMIDOR");


        // Comunicar la operación al productor.

        protocolo.escribir(
            0,
            valor
        );
    }


    void ejecutar()
    {
        while(true)
        {
            //Entrar a sección crítica para revisar el canal de comunicación.

            peterson.entrarConsumidor();


            //Revisar el valor que el productor haya agregado a su lista.

            int valor;

            int operacion =
                protocolo.leer(valor);


            if(operacion == 1)
            {
                // El productor solicita agregar "valor" y se releja la misma operación en la lista local.

                replicarInsercion(valor);


                // Ya procesamos el mensaje.

                peterson.salirConsumidor();


                sleep(1);


                continue;
            }


            //Si el canal está libre y hay elementos, consumir.

            if(operacion == 2 && !lista.estaVacia())
            {
                consumir();
            }


            //Salir de sección crítica.

            peterson.salirConsumidor();


            //Tiempo de consumo.

            sleep(2);
        }
    }
};


/*
 * ============================================================
 * MAIN
 * ============================================================
 */

int main()
{
   
    Protocolo protocolo;

    Peterson peterson;


    protocolo.inicializar();


    peterson.inicializar();


    cout << "Iniciando memoria compartida distribuida..."
         << endl;


    pid_t productor = fork();


    if(productor < 0)
    {
        cerr << "Error al crear el productor."
             << endl;

        return 1;
    }


    if(productor == 0)
    {
        Productor nodoProductor;


        cout << "[PRODUCTOR] "
             << "Proceso iniciado. PID = "
             << getpid()
             << endl;


        nodoProductor.ejecutar();


        return 0;
    }

    pid_t consumidor = fork();


    if(consumidor < 0)
    {
        cerr << "Error al crear el consumidor."
             << endl;

        return 1;
    }

    if(consumidor == 0)
    {
        Consumidor nodoConsumidor;


        cout << "[CONSUMIDOR] "
             << "Proceso iniciado. PID = "
             << getpid()
             << endl;


        nodoConsumidor.ejecutar();


        return 0;
    }


    cout << "[PADRE] "
         << "Productor PID = "
         << productor
         << endl;


    cout << "[PADRE] "
         << "Consumidor PID = "
         << consumidor
         << endl;


    while(true)
    {
        sleep(10);
    }


    return 0;
}