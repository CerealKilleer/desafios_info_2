/*
 * Programa que implementa la solución para el primer reto de informática 2.
 * El software utiliza 3 librerías de creación propia, se utilizó Chat-GPT para
 * generar comentarios compatibles con Doxygen.
 * Las imágenes deben agregarse en el mismo directorio donde está el ejecutable de la aplicación
 * Forma de ejecución por consola en Linux: ./reto_1 [num_operaciones]
 *
 * Realizado por: Yonathan López Mejía y Daniela Escobar Velandia.
 */

#include <QCoreApplication>
#include <QImage>
#include <iostream>
#include "include/bitwise_pixel.hpp"
#include "include/process_data.hpp"
#include "include/constants.hpp"

using namespace std;

int main(int argc, char* argv[])
{

    if (argc != 2) {
        cout << "Uso reto_1 [num_ops]" << endl;
        return EXIT_FAILURE;
    }

    int16_t num_ops = atoi(argv[1]);
    if (num_ops < 0) {
        cout << "¿Un número negativo de operaciones? Vuelva a intenarlo" << endl;
    }

    app_img(num_ops);

    return 0;
}














