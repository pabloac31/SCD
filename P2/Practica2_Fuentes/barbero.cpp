#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM;

constexpr int num_clientes = 4;   // número de hebras cliente

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// *****************************************************************************
// clase para monitor Barbería

class Barberia : public HoareMonitor
{
private:
  CondVar cola_clientes,    // cola de espera de los clientes
          terminado,        // espera a que termine de pelarse
          despierta;        // barbero durmiendo

public:
  Barberia();
  void cortarPelo(int i);
  void siguienteCliente();
  void finCliente();
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia()
{
  cola_clientes = newCondVar();
  terminado = newCondVar();
  despierta = newCondVar();

}
// -----------------------------------------------------------------------------

void Barberia::cortarPelo(int i)
{
  cout << "Cliente " << i << ": Buenos días!" << endl;
  if( despierta.get_nwt() > 0 )     // Si el barbero está dormido
    despierta.signal();             // lo despierta y pasa a pelarse

  else{                             // Si no, espera su turno en la sala de espera
    cout << "Cliente " << i << ": Voy a la sala de espera" << endl;
    cola_clientes.wait();
    cout << "Cliente " << i << ": Me toca!" << endl;
  }

  terminado.wait();                 // Espera a que el barbero termine
  cout << "Cliente " << i << ": Hasta luego!" << endl;
}
// -----------------------------------------------------------------------------

void Barberia::siguienteCliente()
{
  if( cola_clientes.empty() ){    // Si no hay nadie esperando
    cout << "Barbero: No hay nadie...zzz..." << endl;
    despierta.wait();             // se duerme
    cout << "Barbero: ...zzz... Ah! hola, pase" << endl;  // cuando lo despiertan
  }
  else{
    cout << "Barbero: Que pase el siguiente!" << endl;
    cola_clientes.signal();       // pasa el siguiente de la cola
  }
}
// -----------------------------------------------------------------------------

void Barberia::finCliente()
{
  cout << "Barbero: Ya hemos terminado, puede irse" << endl;
  terminado.signal();   // indica al cliente que ha teminado
}
// *****************************************************************************

//-------------------------------------------------------------------------
// Función que simula la espera fuera de la barbería como un retardo aleatorio de la hebra
void esperarFueraBarberia(int i)
{
  // calcular milisegundos aleatorios de duración
  chrono::milliseconds duracion( aleatorio<500,1000>() );

  cout << "Cliente " << i << " espera fuera de la barbería " << duracion.count()
       << " milisegundos..." << endl;

  //espera bloqueada por un tiempo igual a 'duracion'
  this_thread::sleep_for( duracion );
}

//----------------------------------------------------------------------
// función que ejecutan las hebras de los clientes
void funcion_hebra_cliente( MRef<Barberia> monitor, int num_cliente )
{
  while (true) {
    monitor->cortarPelo(num_cliente);
    esperarFueraBarberia(num_cliente);
  }
}

//-------------------------------------------------------------------------
// Función que simula el proceso de cortar el pelo como un retardo aleatorio de la hebra
void cortarPeloACliente()
{
  // calcular milisegundos aleatorios de duración
  chrono::milliseconds duracion( aleatorio<100,500>() );

  cout << "Cortando pelo al cliente... (" << duracion.count() << " milisegundos)"
       << endl;

  //espera bloqueada por un tiempo igual a 'duracion'
  this_thread::sleep_for( duracion );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del barbero
void funcion_hebra_barbero( MRef<Barberia> monitor )
{
  while (true) {
    monitor->siguienteCliente();
    cortarPeloACliente();
    monitor->finCliente();
  }
}

// *****************************************************************************

int main()
{
  cout << "---------------------------------------------" << endl
       << "Problema del barbero durmiente, monitores SU." << endl
       << "---------------------------------------------" << endl
       << flush;

  // crear monitor
  logM("voy a crear el monitor");
  auto monitor = Create<Barberia>();
  logM("monitor creado");

  // Creación de las hebras necesarias para el Problema
  thread hebra_barbero( funcion_hebra_barbero, monitor );
  thread hebra_cliente[num_clientes];
  for (int i = 0; i < num_clientes; i++) {
    hebra_cliente[i] = thread( funcion_hebra_cliente, monitor, i );
  }

  // Finalización de las hebras
  hebra_barbero.join();
  for(int i = 0; i < num_clientes; i++){
    hebra_cliente[i].join();
  }

}
