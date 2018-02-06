#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

unsigned fumados = 0;
Semaphore mostr_vacio = 1,
          ingr_disp[4] = {0,0,0,0},
          fum = 1;


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

//----------------------------------------------------------------------
// funcion que produce un número aleatorio entre 0 y 2 con un retardo aleatorio

int producir(  )
{
    // calcular milisegundos aleatorios de duración
    chrono::milliseconds duracion( aleatorio<20,200>() );

    // espera bloqueada por un tiempo igual a 'duracion'
    this_thread::sleep_for( duracion );

    // devuelve el ingrediente producido
    return aleatorio<0,3>();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
    int i;
    while( true )
    {
      i = producir();   // producir ingrediente
      sem_wait( mostr_vacio );  // espera a que el mostrador este vacío
      cout << "estanquero produce ingrediente " << i << endl;
      sem_signal( ingr_disp[i] );
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   int b;
   while( true )
   {
      sem_wait( ingr_disp[num_fumador] );   // espera a que el ingrediente esté disponible
      cout << "el fumador " << num_fumador << " retira su ingrediente" << endl;
      sem_signal( mostr_vacio );
      fumar( num_fumador );
      sem_wait(fum);
      fumados++;
      cout << "Se han fumado " << fumados << " cigarrillos en total" << endl;
      sem_signal(fum);
   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------" << endl
       << "Problema de los fumadores." << endl
       << "--------------------------" << endl
       << flush ;

  // Creación de las hebras necesarias para el problema
  thread hebra_estanquero ( funcion_hebra_estanquero );
  thread hebra_fumador[4];
  for (int i = 0; i < 4; i++) {
    hebra_fumador[i] = thread( funcion_hebra_fumador, i );
  }

  // Finalización de las hebras
  hebra_estanquero.join() ;
  for (int i = 0; i < 4; i++) {
    hebra_fumador[i].join();
  }

}
