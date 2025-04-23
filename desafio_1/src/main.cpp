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
#define MAX_NUMBER_DIGITS 6

static uint32_t str_len(const char *num)
{
    /**
     * @brief Calcula la longitud de una cadena de caracteres.
     *
     * Recorre la cadena `num` hasta encontrar el carácter nulo (`'\0'`)
     * y devuelve el número de caracteres contados.
     *
     * @param num Puntero a la cadena de caracteres terminada en nulo.
     * @return uint8_t Longitud de la cadena.
     */
    uint32_t cnt = 0;
    while (*num++ != '\0')
        cnt++;

    return cnt;
}

bool get_int16_t(char *num, int16_t &number)
{
    /**
     * @brief Convierte una cadena de caracteres a un número entero de 16 bits con validación.
     *
     * Esta función toma una cadena de caracteres que representa un número entero,
     * verifica que no exceda la longitud máxima permitida definida por `MAX_NUMBER`,
     * convierte la cadena a un entero de 32 bits usando `atoi`, y luego valida que el valor
     * se encuentre dentro del rango de un `int16_t`. Si la conversión es exitosa y el número
     * es válido, lo almacena en la variable `number` y retorna `true`. Si hay un error por longitud
     * o el valor está fuera del rango, retorna `false`.
     *
     * @param num Puntero a la cadena de caracteres que representa el número.
     * @param number Referencia donde se almacenará el valor convertido si es válido.
     * @return true Si la conversión fue exitosa y el número es válido.
     * @return false Si la cadena es demasiado larga o el número está fuera del rango de `int16_t`.
     */
    int32_t aux_num;

    if (str_len(num) > MAX_NUMBER_DIGITS)
        return false;

    /// Devolverá 0 si la cadena contiene caracteres no numéricos, en nuestro contexto no es problemático
    aux_num = atoi(num);
    if (aux_num < std::numeric_limits<int16_t>::min() || aux_num > std::numeric_limits<int16_t>::max())
        return false;

    number = static_cast<int16_t>(aux_num);

    return true;
}

int main(int argc, char* argv[])
{

    if (argc != 2) {
        cout << "Uso reto_1 [num_ops]" << endl;
        return EXIT_FAILURE;
    }

    int16_t num_ops;

    if (!get_int16_t(argv[1], num_ops)) {
        cout << "No ingresó un número válido. Vuelva a intentarlo" << endl;
        return EXIT_FAILURE;
    }

    if (num_ops < 1) {
        cout << "¿Un número negativo de operaciones? ¿Ninguna operación? Vuelva a intenarlo" << endl;
        return EXIT_FAILURE;
    }

    app_img(num_ops);

    return 0;
}













