/**
 * Programa de comunicación MPI usando tipos de datos derivados
 * Este programa demuestra la comunicación entre procesos MPI
 * utilizando un tipo de datos personalizado para la matriz triangular superior
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Constantes para la configuración del programa */
#define ETIQUETA_MENSAJE 10
#define PROCESO_ORIGEN 0  
#define PROCESO_DESTINO 1
#define NUM_ITERACIONES 1

/**
 * Crea un tipo de datos MPI para representar la parte triangular superior de una matriz cuadrada
 * @param dimension La dimensión de la matriz cuadrada
 * @return El tipo de datos MPI creado
 */
MPI_Datatype generar_tipo_triangular_superior(int dimension) {
    MPI_Datatype tipo_triang_sup;
    int* longitudes_bloques = (int*)malloc(dimension * sizeof(int));
    int* desplazamientos = (int*)malloc(dimension * sizeof(int));
    
    // Configuración de los bloques para el tipo de datos indexado
    for (int fila = 0; fila < dimension; fila++) {
        // Cada fila contiene (dimension - fila) elementos
        longitudes_bloques[fila] = dimension - fila;
        // Desplazamiento para cada fila
        desplazamientos[fila] = (fila * dimension) + fila;
    }
    
    // Crear el tipo de datos indexado
    MPI_Type_indexed(dimension, longitudes_bloques, desplazamientos, MPI_INT, &tipo_triang_sup);
    MPI_Type_commit(&tipo_triang_sup);
    
    // Liberar memoria
    free(longitudes_bloques);
    free(desplazamientos);
    
    return tipo_triang_sup;
}

/**
 * Muestra una matriz en la consola
 * @param datos Puntero a los datos de la matriz
 * @param dimension Dimensión de la matriz
 */
void mostrar_matriz(int* datos, int dimension) {
    for (int fila = 0; fila < dimension; fila++) {
        for (int columna = 0; columna < dimension; columna++) {
            printf("%3d ", datos[fila * dimension + columna]);
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * Crea y inicializa una matriz
 * @param dimension Dimensión de la matriz cuadrada
 * @param inicializar Si es 1, inicializa la matriz con valores secuenciales; si es 0, la inicializa con ceros
 * @return Puntero a la matriz creada
 */
int* crear_matriz(int dimension, int inicializar) {
    int* matriz = (int*)malloc(dimension * dimension * sizeof(int));
    
    for (int fila = 0; fila < dimension; fila++) {
        for (int columna = 0; columna < dimension; columna++) {
            if (inicializar) {
                matriz[fila * dimension + columna] = fila * dimension + columna;
            } else {
                matriz[fila * dimension + columna] = 0;
            }
        }
    }
    
    return matriz;
}

/**
 * Verifica las precondiciones del programa
 * @param total_procesos Número total de procesos
 */
void verificar_configuracion(int total_procesos) {
    if (NUM_ITERACIONES != 1) {
        printf("Error: NUM_ITERACIONES debe ser 1\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    if (PROCESO_ORIGEN == PROCESO_DESTINO) {
        printf("Error: PROCESO_ORIGEN y PROCESO_DESTINO no pueden ser iguales\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    if (total_procesos < 2) {
        printf("Error: Se requieren al menos 2 procesos para ejecutar este programa\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/**
 * Función principal
 */
int main(int argc, char** argv) {
    // Inicializar el entorno MPI
    MPI_Init(&argc, &argv);
    
    // Obtener información sobre los procesos
    int rango_proceso, total_procesos;
    MPI_Comm_rank(MPI_COMM_WORLD, &rango_proceso);
    MPI_Comm_size(MPI_COMM_WORLD, &total_procesos);
    
    // Verificar la configuración del programa
    if (rango_proceso == PROCESO_ORIGEN) {
        verificar_configuracion(total_procesos);
    }
    
    // Sincronizar todos los procesos
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Definir el tamaño de la matriz
    int dimension_matriz = 4;
    
    // Crear la matriz
    int* matriz = crear_matriz(dimension_matriz, 1);
    
    // Crear el tipo de datos personalizado
    MPI_Datatype tipo_triang_sup = generar_tipo_triangular_superior(dimension_matriz);
    
    if (rango_proceso == PROCESO_ORIGEN) {
        // Proceso emisor
        printf("Matriz enviada por el proceso %d:\n", PROCESO_ORIGEN);
        mostrar_matriz(matriz, dimension_matriz);
        
        // Enviar la matriz
        MPI_Send(matriz, NUM_ITERACIONES, tipo_triang_sup, PROCESO_DESTINO, 
                 ETIQUETA_MENSAJE, MPI_COMM_WORLD);
    } 
    else if (rango_proceso == PROCESO_DESTINO) {
        // Proceso receptor
        int* matriz_recibida = crear_matriz(dimension_matriz, 0);
        
        // Recibir la matriz
        MPI_Recv(matriz_recibida, NUM_ITERACIONES, tipo_triang_sup, PROCESO_ORIGEN,
                 ETIQUETA_MENSAJE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Mostrar la matriz recibida
        printf("Matriz recibida por el proceso %d:\n", PROCESO_DESTINO);
        mostrar_matriz(matriz_recibida, dimension_matriz);
        
        // Liberar memoria
        free(matriz_recibida);
    }
    
    // Liberar recursos
    free(matriz);
    MPI_Type_free(&tipo_triang_sup);
    
    // Finalizar el entorno MPI
    MPI_Finalize();
    
    return 0;
}