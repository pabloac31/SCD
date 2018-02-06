#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


//**********************************************************************
// variables compartidas

const int num_items = 100 ,   // número de items
	       tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int buffer[tam_vec];		// buffer intermedio
unsigned primera_libre = 0;		// índice en el vector de la primera celda libre
Semaphore libres = tam_vec,		// número de entradas libres del buffer
					ocupadas = 0;				// número de entradas ocupadas del buffer
unsigned producidos = 0;


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int i)
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "hebra " << i << ", producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( int prod )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato(prod) ;
			sem_wait(libres);
			assert( primera_libre < tam_vec );
      buffer[primera_libre++] = dato;		// insertar el dato en el vector
																				// y actualizar posicion
			//cout << dato << " -> buffer" << endl; // para depurar
			sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
			sem_wait(ocupadas);
			assert( primera_libre < tam_vec );
      dato = buffer[--primera_libre];		// actualizar posicion
																				// y extraer valor del vector
			//cout << dato << " <- buffer" << endl; // para depurar
			sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
        thread hebra_fumador[3];

   thread hebra_productora[3];
   thread hebra_consumidora[3];
   for(int i = 0; i < 3; i++) {
     hebra_productora[i] = thread( funcion_hebra_productora, i );
     hebra_consumidora[i] = thread( funcion_hebra_consumidora );
   }

   for (int i = 0; i < 3; i++) {
     hebra_productora[i].join();
     hebra_consumidora[i].join();
   }

   test_contadores();

	 cout << "fin" << endl;
}
