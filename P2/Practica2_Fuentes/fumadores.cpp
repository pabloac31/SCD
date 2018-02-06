#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM;

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
// clase para monitor Estanco

class Estanco : public HoareMonitor
{
    private:
    bool    ingr_disp[3],
            mostr_vacio;
    CondVar disponible[3],
            mostrador_vacio;

    public:
    Estanco( );
    void obtenerIngrediente( int i );   
    void ponerIngrediente(int i );
    void esperarRecogidaIngrediente(); 
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
    for(int i=0; i<3; i++){
        ingr_disp[i] = false;
        disponible[i] = newCondVar();
    }
    mostr_vacio = true;
    mostrador_vacio = newCondVar();
}
// -----------------------------------------------------------------------------

void Estanco::obtenerIngrediente( int i )
{
    if( !ingr_disp[i] )
        disponible[i].wait();
    ingr_disp[i] = false;
    mostr_vacio = true;
    mostrador_vacio.signal();
}
// -----------------------------------------------------------------------------
void Estanco::ponerIngrediente( int i )
{
    ingr_disp[i] = true;
    //cout << "estanquero produce ingrediente " << i << endl;
    mostr_vacio = false;
    disponible[i].signal();
}
// -----------------------------------------------------------------------------
void Estanco::esperarRecogidaIngrediente()
{
    if( !mostr_vacio )
        mostrador_vacio.wait();
}

//----------------------------------------------------------------------
// funcion que produce un número aleatorio entre 0 y 2 con un retardo aleatorio

int ProducirIngrediente(  )
{
    // calcular milisegundos aleatorios de duración
    chrono::milliseconds duracion( aleatorio<20,200>() );

    // espera bloqueada por un tiempo igual a 'duracion'
    this_thread::sleep_for( duracion );

    // devuelve el ingrediente producido
    return aleatorio<0,2>();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
    int ingre;
    while( true )
    {
      ingre = ProducirIngrediente();   // producir ingrediente
      monitor->ponerIngrediente( ingre );
      monitor->esperarRecogidaIngrediente();
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
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      monitor->obtenerIngrediente( num_fumador );   // espera a que el ingrediente esté disponible
      //cout << "el fumador " << num_fumador << " retira su ingrediente" << endl;
      fumar( num_fumador );
   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "----------------------------------------" << endl
       << "Problema de los fumadores, monitores SU." << endl
       << "----------------------------------------" << endl
       << flush ;

  // crear monitor
  logM("voy a crear el monitor");
  auto monitor = Create<Estanco>();
  logM("monitor creado");

  // Creación de las hebras necesarias para el problema
  thread hebra_estanquero ( funcion_hebra_estanquero, monitor );
  thread hebra_fumador[3];
  for (int i = 0; i < 3; i++) {
    hebra_fumador[i] = thread( funcion_hebra_fumador, monitor, i );
  }

  // Finalización de las hebras
  hebra_estanquero.join() ;
  for (int i = 0; i < 3; i++) {
    hebra_fumador[i].join();
  }

}
