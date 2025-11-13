#ifndef __VITERBI_H__
#define __VITERBI_H__
//////////////////////////////////////////////////////////////////////////////
/*!
 * \file viterbi.h
 * \brief Algoritmo de Viterbi genérico con consolidación de camino
 * 
 * (C) 2010 TSR/Aholab - ETSI Bilbao (UPV/EHU)
 */
//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <list>
#include <deque>
#include "uti.h"




/*!
 * \brief Clase base para algoritmos de viterbi generalizados
 * 
 * \author ikerl \<ikerl(ARROBA)aholab.ehu.es\>
 * \version
 * \code
 * 		Version  dd/mm/aa  Autor     Proposito de la edicion
 * 		-------  --------  --------  -----------------------
 * 		1.0.0    21/09/10  ikerl     Codificacion inicial
 * \endcode
 * 
 * ¿Estas harto de programar viterbi's para una y otra aplicación? Aquí está tu solución:
 * Un algoritmo de Viterbi generalizable, con una gestión eficiente de memoria, y con la posibilidad
 * de realizar consolidaciones parciales de la secuencia cuando los caminos abiertos convergenen un punto.
 * 
 * \section viterbi_properties Propiedades
 * 
 * \subsection viterbi_generalization Generalizable
 * 
 * 		Gracias a la herencia, es posible generalizar este algoritmo para adaptarse a diferentes
 * 		definiciones de los costes locales o de transición. De hecho, esta es una función
 * 		virtual pura, que no sirve más que para definir la arquitectura común.
 * 
 * 		Para poder
 * 		usar esta arquitectura, es necesario derivar una nueva clase de ella, y sobrecargar
 * 		las funciones get_localcost() y get_transitioncost(), que se encargan de
 * 		calcular el coste local de un nodo y el coste de transición entre dos nodos cualesquiera.
 * 		¡Ya tenemos personalizado el viterbi!
 * 
 * 		Pero podemos hacer más cosas. Por ejemplo, imaginemos que nuestra arquitectura impone una serie de
 * 		restricciones en cuanto a cuáles pueden ser los predecesores de un cierto nodo. Una solución
 * 		es sobrecargar get_transitioncost() de forma que para dos nodos cuya transición está prohibida,
 * 		devuelva un coste de transición infinito. Pero también podemos sobrecargar la función get_parent() para
 * 		que ni siquiera intente emparejar esos dos nodos, ahorrando la computación correspondiente.
 * 
 * 		Además de ser una clase virtual pura, también es una plantilla, que admite un tio de datos T.
 * 		En este caso T representa toda la información interna que debe tener un nodo, bien para permitir
 * 		el cálculo de los costes, o bien para permitir recuperar el valor que representa.
 * 
 * 		Por ejemplo, supongamos que se trata de un viterbi para decodificar una secuencia de símbolos QAM
 * 		transmitidos por radio. La información interna puede ser el valor del módulo y la fase del vector
 * 		QAM, y el símbolo que representa el nodo. Esta información es utilizada por get_localcost()
 * 		para estimar el coste/probabilidad de ese símbolo dado ese vector. Igualmente, es utilizada por
 * 		get_transitioncost() para determinar el coste/probabilidad de transición entre dos símbolos
 * 		(supongamos que no todas las transiciones son válidas). Por úlitmo, una vez deducido el camino completo,
 * 		se puede saber qué símbolo representa cada uno de los nodos de dicho camino.
 * 
 * \subsection viterbi_consolidation Consolidación de caminos parciales
 * 
 * 		Cuando la secuencia a determinar es muy larga, podemos tener problemas de memoria. Imagina que
 * 		tenemos que decodificar una secuencia de 1000 símbolos, y por cada símbolo, tenemos 10 candidatos.
 * 		la memoria necesaria para almacenar el trellis es de 10*1000=10000 nodos. Cada nodo
 * 		consta de información de costes locales, de transición, acumulados, el predecesor, y la información
 * 		interna de tipo T, que puede ser de varios cientos de bytes. Total, alrededor del megabyte.
 * 
 * 		¡Bah, eso no es nada! dirás. Bien, ahora prueba a determinar una secuencia de 10 millones de
 * 		símbolos, y ya hemos llegado a los 10 gigabytes. Memoria insuficiente. Ale, a casa.
 * 		Ah, y lo de los 10 millones de símbolos no me lo he inventado, es lo necesario para
 * 		estimar el pitch de una señal de 4 horas con una resolución de 1 ms entre cada muestra.
 * 
 * 		Pero en secuencias tan largas, lo normal es que muchos de los caminos abiertos vayan muriendo,
 * 		de forma que parte del trellis converja en un punto. Se puede hacer un backtracking desde ese punto
 * 		hacia atrás con total seguridad, recuperar parte del camino, y borrar toda esa parte del trellis, que
 * 		una vez recuperado el camino que representa, no sirve de nada. Ya hemos liberado memoria. Y podemos
 * 		seguir.
 * 
 * 		Bueno, pues esto de comprobar si el trellis converge en un punto dado, recuperar el trozo de camino
 * 		correspondiente y liberar la memoria que ya no se necesita, lo hace la función consolidate(). Si esta
 * 		función encuentra un punto de convergencia, devuelve el camino parcial hasta ese punto y libera memoria.
 * 		Si el trellis no ha llegado a converger, devuelve una secuencia vacía, y todo sigue como antes.
 * 
 * 		Ahora bien, esto de recuperar caminos parciales no es gratuito. Para hacerlo de forma eficiente, es
 * 		necesario mantener una estructura paralela al trellis, un trellis compactado, que no es sino
 * 		un esquema en forma de árbol de los caminos que quedan abiertos y los puntos de bifurcación.
 * 		Esto permite saber rápidamente si el trellis converge, a costa de tener que mantener actualizada
 * 		la estructura compactada. Eso suponen ciclos de reloj cada vez que se añade un nuevo símbolo a la secuencia.
 * 
 * 		Con secuencias muy largas, compensa: Irá un poco más despacio, pero al menos el programa no muere
 * 		por falta de memoria. Sin embargo, en secuencias cortas puede no tener mucho sentido
 * 		soportar esta sobrecarga. Si la secuencia es corta, la probabilidad de que el trellis converja
 * 		es reducida, y el ahorro de memoria que vamos a tener, totalmente irrelevante. En estos casos podemos
 * 		sobrevivir sin consolidar partes parciales del camino. Por tanto,
 * 		se ha implementado un mecanismo para evitar el mantenimiento del trellis compactado si se desea.
 * 		Esto hace que nunca se van a poder extraer trozos de camino convergentes, y que consolidate()
 * 		siempre devolverá una secuencia vacía.
 * 
 * 		Por cierto, este mecanismo es tan sencillo como crear el viterbi con parámetro true/false en el constructor.
 * 
 * 
 * \subsection viterbi_asymetry Trellis asimétrico
 * 
 * 		El número de candidatos de una etapa del trellis no tiene porque ser constante. Se permite que
 * 		en una cierta etapa haya 10 candidatos, y en la siguiente 20. En algunas ocasiones esto es muy interesante.
 * 
 * 		El número de candidatos en cada etapa no está definido de antemano, y se deducen en tiempo de ejecución:
 * 		cada vez que se llama a add_candidate() se añade un nuevo candidato en la última etapa. Y puedes llamarlo
 * 		todas las veces que quieras. Para empezar una nuev etapa y dar la actual por terminada (ya no tiene más candidatos),
 * 		basta con llamar a begin_step().
 * 
 * 		Una vez finalizada la inserción de candidatos en una etapa y haber llamado a begin_step(), ya no es posible
 * 		modificar el número de candidatos de etapas anteriores. add_candidate() siempre añade en la última etapa.
 * 
 * 		Por supuesto, si se conoce de antemano el número de candidatos de una etapa, puede darse este dato
 * 		en la llamada de begin_step(), lo que hará que se reserve la memoria necesaria para todos ellos, mejorando
 * 		todavía más la gestión de memoria. Ni siquiera es necesario que este número sea el mismo para todas las etapas.
 * 
 * \section viterbi_limitations Limitaciones
 * 
 * 		- El modelo de costes se limita a un cálculo del tipo coste_total = coste_local+coste_transición+coste_total_predecesor.
 * 		  Si el problema a resolver tiene un modelo de costes multiplicativos (por ejemplo, definido mediante probabilidades),
 * 		  es necesario convertirlo a sumas (por ejemplo, tomando logaritmos).
 * 
 * 		- El coste local sólo puede depender del nodo actual, no de los otros candidatos, ni tampoco de los predecesores.
 * 
 * 		- Igualmente, el coste de transición sólo depende del actual y del predecesor, nunca de otros nodos candidatos
 * 		  ni de otros posibles predecesores.
 * 
 * 		
 * \section viterbi_use Utilización
 * 
 * Vamos a ver el manejo de la clase mediante un ejemplo. Supongamos que transmitimos una secuencia
 * de dígitos binarios, con la siguiente particularidad:
 * 
 * 		- Cada símbolo consta de 2 dígitos.
 * 		- Dados dos símbolos consecutivos, sólo uno de los dos dígitos puede cambiar de uno a otro, y nunca los dos.
 * 		  Es decir, 00 -> 01 es válido, como también 01 -> 11, pero no 10 -> 01.
 * 		- La probabilidad de error de un bit en ese canal es del 10% (p=0.1)
 * 		- Nada indica que no pueda darse una inversión de los dos bits de un mismo símbolo, (que un 00 transmitido
 * 		  se convierta a un 11 recibido), pero esto se dará con una probabilidad de p*p=0.01
 * 
 * Recibimos una secuencia de 1000 símbolos (2000 dígitos), posiblemente con errores, y queremos estimar
 * la secuencia transmitida y escribirla en un fichero.
 * 
 * \subsection viterbi_nodeinfo Estructura de información de nodo
 * 
 * Lo primero que necesitamos definir es la estructura que va a almacenar la información interna de cada nodo.
 * En este caso, la información será el símbolo 'recibido' y el símbolo candidato al que representa:
 * 
 * \code
 * 
 * 		struct symbol
 * 		{
 * 			bool first;
 * 			bool second;
 * 		};
 * 
 * 		struct myinfo
 * 		{
 * 			symbol received;
 * 			symbol candidate;
 * 		};
 * \endcode
 * 
 * En esta estructura se puede meter lo que se quiera. La única condición es que sea un elemento copyable,
 * es decir, que el constructor copya y las operaciones de copia estén correctamente definidas.
 * 
 * \subsection viterbi_derive Crear el viterbi personalizado
 * 
 * La personalización del viterbi se da a dos niveles: Por herencia, y por argumento de plantilla. Y las
 * dos deben realizarse simultaneamente. Además, es necesario sobrecargar las funciones virtuales puras
 * get_localcost() y get_transitioncost() según la definición de costes del problema.
 * 
 * En este caso, los costes nos vienen dados en forma de probabilidades. Puesto que este viterbi
 * aplica la regla de suma a los costes, es necesario convertir estas probabilidades a logarítmico,
 * para que la suma tenga algún sentido. También es necesario convertir las probabilidades (algo
 * positivo, se debería buscar lo más probable) a costes (el viterbi busca el mínimo coste), lo que puede
 * conseguirse si convertimos la probabilidad de que algo ocurra (p) en la probabilidad de error
 * en caso de tomar esa opción, o coste de error (1-p).
 * 
 * \note Puesto que en este caso también tenemos restricciones en los predecesores posibles, se podría
 * 		sobrecargar get_parent(), pero vamos a introducir estas restricciones en get_transitioncost().
 * 
 * \code
 * 		class myviterbi : public viterbi<myinfo>
 * 		{
 * 
 *			//Constructor. Vamos a hacer que sea consolidable por narices
 * 			myviterbi()
 * 			: viterbi\<myinfo\>(true),	//Consolidable por narices
 * 			  p(0.1)					//probabilidad de una inversión de bit
 * 			{ }
 * 
 * 			virtual
 * 			double
 * 			get_localcost(const Vnode &node)
 * 			{
 *				//El coste local depende del número de inversiones de bit entre lo recibido y el candidato
 * 				double cost;
 * 				if (node.info.received.first != node.info.candidate.first)
 * 					cost = p;
 * 				else
 * 					cost = (1-p);
 * 
 * 				if (node.info.received.second != node.info.candidate.second)
 * 					cost *= p;
 * 				else
 * 					cost *= (1-p);
 * 
 *				//Esto es la probabilidad de que el símbolo sea correcto.
 *				//Hay que calcular la probabilidad de queno lo sea.
 *				//Además, hay que convertir a log
 * 				return (log(1-cost));
 * 			}
 * 
 * 			virtual
 * 			double
 * 			get_transitioncost(const Vnode &node1, const Vnode &node2)
 * 			{
 *				//El coste de transición es 0 si entre ambos nodos no difiere más que un bit, y INF en caso contrario.
 *				//(suponiendo que INF está definido como algo MUY grande en algún sitio)
 * 				if (node1.info.candidate.first != node2.info.candidate.first &&
 * 				    node1.info.candidate.second != node2.info.candidate.second)
 * 					return INF;
 * 				return 0;
 * 			}
 * 
 * 		protected:
 * 			double p;		//La probabilidad de UNA inversión de bit
 * 		};
 * \endcode
 * 
 * 
 * \subsection viterbi_program Programa general
 * 
 * Una vez que tenemos el viterbi personalizado, sólo falta ponerlo en marcha!
 * Puesto que hemos forzado a que sea consolidable, vamos a tratar de consolidar los caminos
 * cada cierto tiempo. Por ejemplo, vamos a mirar si parte del camino ha convergido cada
 * 50 símbolos recibidos. Esto también demostrará una de las particularidades de consolidate() y terminate(),
 * y es que devuelven el camino AL REVÉS.
 * 
 * \code
 * 
 *		//Función que devuelve un nuevo símbolo recibido. Devuelve true si hay un símbolo más, false si ya no quedan
 * 		bool get_new_symbol(symbol& s)
 * 		{ //... }
 * 
 * 		int convergence_check_delay = 50;	//Comprovamos convergencia cada 50 símbolos
 * 		myviterbi v;
 * 		myviterbi::path_type path;			//Para recoger el camino parcial
 * 
 * 		int symbol_count = 0;		//Contador para saber cuándo toca chequear convergencia
 * 		symbol received;			//símbolo recibido
 * 		while (get_new_symbol (received)
 * 		{
 *			//Tenemos 4 símbolos posibles -> 4 candidatos
 *			//Podemos optimizar el uso de memoria indicando elnúmero de candidatos
 * 			v.begin_step(4);
 * 
 *			//La parte de received será la misma para todos los candidatos
 * 			myinfo node;
 * 			node.received = received;
 * 
 *			//candidato 00
 * 			node.candidate.first = false;
 * 			node.candidate.second = false;
 * 			v.add_candidate(node);
 * 			
 *			//candidato 01
 * 			node.candidate.first = false;
 * 			node.candidate.second = true;
 * 			v.add_candidate(node);
 * 			
 *			//candidato 10
 * 			node.candidate.first = true;
 * 			node.candidate.second = false;
 * 			v.add_candidate(node);
 * 			
 *			//candidato 11
 * 			node.candidate.first = true;
 * 			node.candidate.second = true;
 * 			v.add_candidate(node);
 * 
 * 			if (++symbol_count == convergence_check_delay)
 * 			{
 * 				v.consolidate(path);
 * 
 *				//Es necesario recorrer el path al revés, el primer símbolo está al final
 * 				myviterbi::path_type::const_reverse_iterator it;
 * 				for (it = path.rbegin(); it != path.rend(); ++it)
 * 					print_symbol_in_file (it->candidate);
 * 			}
 * 		}
 * 
 *		//Falta la última parte del camino por recoger
 * 		v.terminate(path);
 * 
 *		//Es necesario recorrer el path al revés, el primer símbolo está al final
 * 		myviterbi::path_type::const_reverse_iterator it;
 * 		for (it = path.rbegin(); it != path.rend(); ++it)
 * 			print_symbol_in_file (it->candidate);
 * \endcode
 * 
 */
template <typename T>
class viterbi
{
	
protected:
	
	
	//Forward declaration
	struct Vnode;
	
public:
	typedef std::allocator<Vnode>						node_alloc;		///<Allocator para un Vnode
	typedef std::vector<Vnode, node_alloc>				trellis_step;	///<Una etapa del trellis (con los candidatos)
	typedef std::allocator<trellis_step>				step_alloc;		///<Allocator para una etapa del trellis
	typedef std::list<trellis_step, step_alloc>			trellis;		///<Trellis completo
	typedef std::vector<T, std::allocator<T> >			path_type;		///<Contenedor utilizado para devolver el path

protected:
	
	/*!
	 * Define un nodo del trellis. Contiene un campo de información genérica \p info
	 * donde se almacenará toda la información específica que se necesite almacenar.
	 * Esta información, lógicamente, es totalmente dependiente de la implementación
	 * y del objetivo del viterbi
	 */
	struct Vnode
	{
		double localcost;		///<Coste local del nodo
		double transitioncost;	///<Coste de transición
		double totalcost;		///<Coste total hasta el nodo (localcost + transitioncost + prev->totalcost)
		Vnode* prev;			///<Predecesor más probable según el tratamiento de costes
		
		T info;					///<Placeholder para todo tipo de información añadida (dependiente de aplicación)
		
		/*!
		 * Un constructor permite que todo sea siempre coherente (sobre todo, los punteros)
		 */
		Vnode (const T& data = T())
		: localcost(0),
		  transitioncost(0),
		  totalcost(0),
		  prev(0),
		  info(data)
		{ }
	};
	
	
	class Ztrellis
	{
	protected:
		
		//Forward declaration
		struct Znode;
		
		typedef std::allocator<Znode>						Znode_alloc;	///<Allocator para un Znode
		typedef std::list<Znode, Znode_alloc>				ztrellis;		///<Trellis comrimido
	
		/*!
		* Define un nodo en el trellis comprimido
		*/
		struct Znode
		{
			const Vnode* cand;						///<Puntero al candidato representado por este nodo
			long children;							///<Número de hijos del nodo
			typename ztrellis::iterator parent;		///<Nodo padre del actual (dentro del Ztrellis)
		};
		
		
		
		
		
	public:
		/*!
		* Crea un trellis con sólo un nodo base, sin padre, sin hijos, y sin
		* candidato asociado
		*/
		inline
		Ztrellis ()
		{	
			//El nodo padre es un nodo sin padre ni puntero a candidato.
			//Y de momento, tiene 0 hijos
			Znode root;
			root.cand = 0;
			root.children = 0;
			root.parent = m_ztrellis.end();
			m_ztrellis.push_back(root);
			m_old_nodes_begin = m_ztrellis.begin();
			m_old_nodes_end = m_ztrellis.end();
		}
		
		inline
		void
		add_and_compress (const trellis_step& nodes)
		{
			//Primero añadimos
			add(nodes);
			
			//Y luego comprimimos
			compress();
			
			//Actualizamos los iteradores a nodos base
			m_old_nodes_end = m_old_nodes_begin;
			m_old_nodes_begin = m_ztrellis.begin();
		}
		
		/*!
		* \brief Devuelve la dirección Vnode correspondiente al nodo base en el trellis comprimido
		* 
		* Básicamente, todo lo que esté antes del nodo base es camino consolidado, que se puede
		* extraer sin peligro.
		*/
		inline
		const Vnode*
		get_base_node () const
		{ return m_ztrellis.back().cand; }
		

	protected:
		
		void
		add (const trellis_step& nodes)
		{
			//Añadimos los nuevos nodos
			for (typename trellis_step::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
			{
				Znode newnode;
				newnode.cand = &(*it);
				newnode.children = 0;
				
				//Hay que buscar el Znode correspondiente a it->prev
				//Esto debería funcionar incluso para la primera etapa, con it->prev==0, ya que 
				//en el primer Znode cand==0
				typename ztrellis::iterator zit;
				for (zit = m_old_nodes_begin; zit != m_old_nodes_end; ++zit)
				{
					if (zit->cand == newnode.cand->prev)
					{
						newnode.parent = zit;
						++ newnode.parent->children;
						break;
					}
				}
				if (zit == m_old_nodes_end)
					die ("No se ha encontrado el candidato del padre en el Ztrellis");
				m_ztrellis.push_front(newnode);
			}
		}
		
		void
		compress ()
		{
			//Primero eliminamos nodos VIEJOS (no toques los nodos recién metidos) con 0 hijos
			for (typename ztrellis::iterator zit = m_old_nodes_begin; zit != m_ztrellis.end(); ++zit)
			{
				if (!(zit->children))
				{
					//Cuidado. zit apunta a algo que se va a eliminar.
					//zit vuelve al elemento anterior (ya se recuperará con el ++zit,
					//y estamos seguros de que nuca es cabeza de lista)
					//y borramos desde un temporal.
					zit->parent->children --;
					typename ztrellis::iterator tmp = zit;
					--zit;
					m_ztrellis.erase(tmp);
				}
			}
			
			//Ahora eliminamos nodos con un único hijo
			for (typename ztrellis::iterator zit = m_ztrellis.begin(); zit != m_ztrellis.end(); ++zit)
			{
				while (zit->parent->children == 1)
				{
					typename ztrellis::iterator tmp = zit->parent;
					zit->parent = zit->parent->parent;
					m_ztrellis.erase(tmp);
				}
			}
		}


		
	protected:
		
		ztrellis m_ztrellis;							///<El trellis comprimido
		typename ztrellis::iterator m_old_nodes_begin;	///<Iterador al inicio de los padres primerizos
		typename ztrellis::iterator m_old_nodes_end;	///<Iterador al final de los padres primerizos
	};

	

	
public:
	
	viterbi(bool consolidable = true)
	: m_trellis(trellis()),
	  m_prev_step(m_trellis.end()),
	  m_Ztrellis(Ztrellis()),
	  m_last_consolidation(m_Ztrellis.get_base_node()),
	  m_consolidable(consolidable)
	{ /*Sólo hace falta para inicializar el anterior step a end*/ }
			
	  
	//Sólo defino el destructor para hacerlo virtual
	virtual
	~viterbi()
	{ }
	
	/*!
	 * \brief Añade una nueva etapa al trellis
	 * 
	 * La etapa añadida estará vacía y sin candidatos.
	 * 
	 * Si el número de candidatos en cada etapa
	 * es conocida de antemano, se puede optimizar la gestión de memoria indicando el número de
	 * candidatos, que se utilizará para realizar la reserva de memoria inicial. Cuidado, la etapa
	 * seguirá estando vacía, pero ya tendrá la memoria necesaria reservada.
	 * 
	 * Además, se encarga de añadir la etapa anterior en el trellis compacto y de
	 * recompactarlo (la etapa anterior, hasta ahora, existía en el trellis normal, pero no en el compacto).
	 * Pero esta operación supone un gasto de tiempo innecesario si resulta que
	 * el viterbi no es consolidable, por lo que esta actualización sólo se realiza en el caso de un 
	 * viterbi consolidable
	 * 
	 * \note Si por lo que sea \p nc es MENOR que el verdadero número de candidatos, no ocurre nada
	 * más grave que algo de memoria perdida. Cuando se llame a add_candidate() y ya no haya más memoria,
	 * vector automáticamente reservará más, y no habrá error. Pero posiblemente reserve más de la necesaria,
	 * provocando un desperdicio de la misma.
	 */
	inline
	void
	begin_step( size_t nc = 0 )		///<Número de candidatos
	{
		//Primero, actualizamos el puntero a los padres
		if (!m_trellis.empty())
		{
			typename trellis::const_iterator it = m_trellis.begin();
			std::advance(it, 1);
			if(it == m_trellis.end())
				m_prev_step = m_trellis.begin();
			else
				++m_prev_step;	//avanzar el iterador es más fácil que buscar el último
			
			//Si el viterbi es consolidable, añadimos la última etapa al Ztrellis y recompactamos
			if (m_consolidable)
				m_Ztrellis.add_and_compress (m_trellis.back()); 
		}
	
		//Metemos una nueva etapa en el trellis
		m_trellis.push_back(trellis_step());
		if (nc)
			m_trellis.back().reserve(nc);
	}
	
	/*!
	 * \brief Añade un nuevo candidato a la etapa actual
	 * 
	 * Calcula los costes local, total, de transición y el predecesor más
	 * probable.
	 */
	inline
	void
	add_candidate( const T& data )	///<Datos del candidato
	{
		//Creamos el nodo. Esto ya hace que los punteros tengar el valor adecuado
		Vnode node(data);
		
		//Hemos de calcular sus costes
		node.localcost = get_localcost(node);
		node.prev = get_parent(node, node.transitioncost);
		if (node.prev)
			node.totalcost = node.localcost + node.transitioncost + node.prev->totalcost;
		else
			node.totalcost = node.localcost + node.transitioncost;
		
		//Por último, se añade
		m_trellis.back().push_back(node);
	}
	
	/*!
	 * \brief Consolida la parte del camino que haya convergido
	 * 
	 * Este proceso permite consolidar la parte del camino que haya convergido, devolviendo esta
	 * parte del camino, y liberando la memoria asociada en el trellis.
	 * 
	 * \return un vector con la información (tipo T) contenida en los nodos del camino
	 * 		consolidado. CUIDADO!! La primera posición de este vector representa
	 * 		el final del camino consolidado (cosas de tener que hacer backtrack y no forwardtrack),
	 * 		por lo que para escribir luego el camino correctamente, será necesario usar
	 * 		reverse_iterators.
	 * 
	 * \note Puesto que el trellis compactado sólo se actualiza con las llamadas a begin_step()
	 * 		(es el único momento en el que podemos estar seguros de que la etapa anterior ha terminado,
	 * 		y por tanto, es seguro añadir los nodos al trellis compactado), consolidate() no puede
	 * 		tomar en cosideración los hipotéticos candidatos de la última etapa creada (la etapa creada
	 * 		en la última llamada a begin_step()), ya que estos no se han añadido al trellis compacatado
	 * 		todavía. Dicho de otra forma:
	 * 
	 * \code
	 * 		vitebi<myinfo> v(true);		//Un viterbi consolidable
	 *		//...
	 * 		v.begin_step();				//Primera etapa
	 *		//...
	 * 		v.begin_step();				//Segunda etapa. La primera entra al trellis compactado
	 *		//...
	 * 		v.begin_step();				//Tercera etapa. La segunda entra al trellis compactado
	 *		//...
	 *		viterbi<myinfo>::path_type path
	 * 		v.consolidate(path);		//Consolidamos parte del camino
	 * \endcode
	 * 
	 * 		Esta operación de consolidación sólo utiliza las dos primeras etapas, o lo que es lo mismo,
	 * 		el camino consolidado puede tener como mucho UNA etapa, ya que para el trellis conpactado,
	 * 		los candidatos de la segunda etapa siguen siendo nodos finales.
	 * 
	 * 		En general esto no afecta para nada al sistema: cuando se consolida, se consolida lo que se puede
	 * 		y punto, generalmente no se sabe cuánto se va a consolidar. Pero prefiero dejarlo documentadito
	 * 		por si a alguien se le ocurre alguna aplicación extraña presuponiendo que
	 * 		la última etapa también entra en la consolidación.
	 */
	inline
	void
	consolidate(path_type& path)	///<Vector donde se dejará el camino consolidado
	{ consolidate_helper(m_Ztrellis.get_base_node(), path); }
	
	
	/*!
	 * \brief Devuelve el segmento de camino que no haya sido consolidado al final del algoritmo
	 * 
	 * Cuando terminemos el algoritmo, quedará al menos un trozo de camino sin consolidar, que tendremos
	 * que procesar con el método clásico: Buscar el mínimo coste en la última etapa, y backtracking desde ahí.
	 * 
	 * \note Después de llamar a esta función, el viterbi queda inutilizado. No sé qué puede pasar si luego
	 * 		se añaden más nodos, o se llama a consolidate(). Esta operación, además, elimina todo el trellis.
	 * 
	 * \return un contenedor tipo viterbi<T>::path_type con la información (tipo T)
	 * 		contenida en los nodos del trozo de camino
	 * 		que falta. CUIDADO!! La primera posición de este contenedor representa
	 * 		el final del camino (cosas de tener que hacer backtrack y no forwardtrack),
	 * 		por lo que para escribir luego el camino correctamente, será necesario usar
	 * 		reverse_iterators.
	 */
	void
	terminate(path_type& path)	///<Vector donde se dejará el camino final
	{
		/* La última etapa, esa que está en el limbo de 'pertenece al trellis general pero todavía no está
		 * en el terllis compactado', nunca llega al trellis compactado. En realidad da igual, puesto
		 * que vamos a sacar lo que falta del camino, no hace falta saber si converge o no, sólo
		 * necesito saber el final (por búsqueda de mínimo coste) y el último punto consolidado (m_last_consolidation)
		 */

		//Buscamos el mínimo coste en la última etapa
		typename trellis_step::const_iterator pos = m_trellis.back().begin();
		double cost = pos->totalcost;
		for (typename trellis_step::const_iterator it = pos+1; it != m_trellis.back().end(); ++it)
		{
			if (cost > it->totalcost)
			{
				cost = it->totalcost;
				pos = it;
			}
		}
		
		//Vale, el camino acaba en pos == &(*pos)
		consolidate_helper(&(*pos), path);
	}
	
	
	
protected:
	
	void
	consolidate_helper (const Vnode* new_consolidation, path_type& path)
	{
		//Usaremos el viejo truco del swap para asegurarnos de que borramos el contenido de path
		path_type sal;

		if (new_consolidation == m_last_consolidation)
		{
			//Nada que hacer, no se puede consolidar nada nuevo
			std::swap(sal, path);
			return;
		}
		
		//Se puede consolidar desde new_consolidation hasta m_last_consolidation
		//Se supone que m_last_consolidation es null o ya está metido desde la última consolidación
		long i = 0;
		for (const Vnode* it = new_consolidation; it != m_last_consolidation; it = it->prev)
		{
			sal.push_back(it->info);
			++i;
		}
		std::swap(sal, path);
		
		//Ahora hay que eliminar toda la memoria inservible. Exactamente las primeras i etapas :)
		typename trellis::iterator endpoint = m_trellis.begin();
		std::advance(endpoint, i-1);
		m_trellis.erase(m_trellis.begin(), endpoint);
		
		//Actualizamos el punto de consolidación
		m_last_consolidation = new_consolidation;
	}
	

	
	/*!
	 * \brief Estima el coste local de un nodo
	 * 
	 * Esta función es totalmente dependiente de la implementación, por lo que debe ser sobrecargada
	 * por las clases derivadas.
	 */
	virtual
	double
	get_localcost (Vnode &node) = 0;
	
	/*!
	 * \brief Estima el predecesor de un nodo, y también calcula su coste de transición
	 * 
	 * Esta función posiblemente pueda ser optimizada en función de la implementación, por lo que
	 * es un buen candidato a ser sobrecargada en las clases derivadas. Por ejemplo, si hay
	 * restricciones en los posibles predecesores, no tiene sentido comprobarlos todos.
	 * 
	 * La implementación por defecto prueba con TODOS los predecesores posibles, y por cada uno
	 * de ellos calcula el coste de transición.
	 * 
	 * \note En los nodos de la primera etapa el coste de transición está definido a 0 por defecto.
	 * 		Este coste de transición puede ser utilizado como el coste de que el camino empiece en ese
	 * 		nodo (algo así como la probabilidad de inicio de un HMM). Para ello es necesario sobrecargar
	 * 		esta función para que asigne los costes adecuados en la primera etapa.
	 * 
	 * \bug Para comprobar si existen padres, se debería mirar m_prev_step y comprobar alguna condición
	 * 		como if(m_prev_step == m_trellis.end()). Sin embargo, esta condición falla, posiblemente porque
	 * 		durante la construcción el trellis está totalmente vacío, y m_trellis.end() apunta a alguna
	 * 		otra cosa que cuando ya se ha metido una etapa. De momento, la condición impuesta es
	 * 		if(m_trellis.size() == 1), pero esto no contempla todos los casos. 
	 */
	virtual
	Vnode*
	get_parent (Vnode &node, double& transitioncost)
	{
		typename trellis::const_iterator it = m_trellis.begin();
		std::advance(it, 1);
		if(it == m_trellis.end())
		{
			transitioncost = 0;
			return 0;
		}
		
		//Buscamos el mínimo coste
		typename trellis_step::iterator pos = m_prev_step->begin();
		double cost = pos->totalcost + get_transitioncost(*pos, node);
		for (typename trellis_step::iterator it = pos+1; it != m_prev_step->end(); ++it)
		{
			double tmp = it->totalcost + get_transitioncost(*it, node);
			if (cost > tmp)
			{
				cost = tmp;
				pos = it;
			}
		}
		
		//Vale, conocemos el predecesor en pos
		transitioncost = cost - pos->totalcost;
		return &(*pos);
	}
	
	/*!
	 * \brief Estima el coste de transición entre dos nodos.
	 * 
	 * Esta función es totalmente dependiente de la implementación, por lo que debe ser sobrecargada
	 * por las clases derivadas.
	 */
	virtual
	double
	get_transitioncost (Vnode &node1, Vnode &node2) = 0;
	
	
protected:
	/*
	 * El trellis es una lista de vectores. Cada posicion de la lista representa una etapa del trellis, mientras
	 * que cada elemento del vector es un candidato considerado dentro de esa etapa. Esto supone varias ventajas:
	 * 		- Cada etapa del trellis se añade una a una. Por tanto, el contenedor debe permitir un push_back() sin
	 * 		  coste. La lista cumple esto.
	 * 		- Cuando se consolida parte del camino, la parte del trellis correspondiente a ese trozo se elimina
	 * 		  de la memoria. Esa parte estará al principio del trellis, por lo que también debe permitir
	 * 		  un pop_front() sin coste. Otra vez, la lista cumple este requisito
	 * 		- El número de candidatos en cada etapa debe poder ser variable, por lo que se ha utilizado
	 * 		  un vector para representar a los candidatos de cada etapa, y no un array de tamaño fijo.
	 * 
	 * En lugar de utilizar una lista, se podría haber utilizado un deque, que sigue cumpliendo los
	 * requisitos. Sin embargo, m_prev_step es un iterador a m_trellis, y resulta que los deque invalidan
	 * todos sus iteradores con cada inserción, aunque sea en los extremos. Por tanto, me quedo con la lista
	 * 
	 * En lugar de utilizar un vector, también se podría haber utilizado una lista, pero eso provocaría la
	 * imposibilidad de optimizar las clases derivadas en las que el número de candidatos sea conocido
	 * de antemano mediante std::vector::reserve()
	 */
	trellis m_trellis;						///<Trellis con los nodos del viterbi
	typename trellis::iterator m_prev_step;	///<Iterador a la etapa anterior, para saber dónde buscar predecesores
	Ztrellis m_Ztrellis;					///<Trellis comprimido con los caminos abiertos
	const Vnode* m_last_consolidation;		///<Último nodo consolidado
	bool m_consolidable;					///<Indica si el viterbi es consolidable
	/* Un viterbi no consolidable no pierde el tiempo actualizando el Ztrellis. Una llamada a consolidate()
	 * no tiene ningún efecto (siempre devuelve un vector vacío), y el único momento para extraer el
	 * camino es con terminate(), que lo extrae de golpe.
	 */
	
};


#endif //__VITERBI_H__
