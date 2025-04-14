#include <iostream>
#include <stdint.h>
#include <cassert>
#include "include/bitwise_pixel.hpp"
#include "include/constants.hpp"

using namespace std;

void apply_complete_rotate_shift(uint8_t (*op)(const uint8_t, const uint8_t),
                                uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight)
{
    /**
     * @brief Aplica una operación bit a bit a cada byte de una imagen.
     *
     * Esta función recorre todos los píxeles de una imagen RGB y aplica una función
     * proporcionada (`op`) a cada byte del arreglo de datos de imagen. La función
     * `op` debe aceptar un byte (un canal del pixel) y un segundo parámetro `n`,
     * que indica el número de desplazamientos o rotaciones de bit
     *
     * @param op Puntero a una función que toma dos argumentos (`uint8_t byte`, `uint8_t n`)
     *           y devuelve un `uint8_t`. Representa la operación a aplicar a cada byte.
     * @param img_data Puntero al arreglo que contiene los datos de la imagen en formato RGB plano.
     * @param n Valor constante que será pasado como segundo parámetro a la función `op`.
     * @param width Ancho de la imagen (en píxeles).
     * @param hight Altura de la imagen (en píxeles). [Nota: considera renombrarlo a 'height']
     */
    for (uint32_t i=0; i<width*hight*RGB_CHANNELS; i++)
        img_data[i] = op(img_data[i], n);
}

void apply_complete_xor(uint8_t *img_data, const uint8_t *img_noisy_data, const uint16_t width, const uint16_t hight)
{
    /**
     * @brief Aplica una operación XOR byte a byte entre dos imágenes.
     *
     * Esta función realiza un XOR entre cada byte de los datos de imagen originales y los datos
     * de una imagen ruidosa, escribiendo el resultado directamente sobre los datos originales.
     * Se asume que ambas imágenes tienen el mismo tamaño y número de canales RGB.
     *
     * @param img_data Puntero a los datos de la imagen original. Será sobrescrita con el resultado.
     * @param img_noisy_data Puntero a los datos de la imagen con ruido.
     * @param width Ancho de la imagen en píxeles.
     * @param hight Alto de la imagen en píxeles.
     */
    for (uint32_t i=0; i<width*hight*RGB_CHANNELS; i++)
        img_data[i] = xor_byte(img_data[i], img_noisy_data[i]);
}

uint32_t validate_rotate_shift_process(uint8_t(*op)(const uint8_t, const uint8_t), const uint8_t *img_data, const uint8_t *reversed_mask,
                                       const uint32_t seed, const uint32_t mask_size, const uint8_t n)
{
    /**
     * @brief Valida una operación bit a bit comparando los datos enmascarados con una imagen original.
     *
     * Aplica una operación de rotación o desplazamiento (`op`) sobre cada byte de la máscara invertida
     * y calcula la suma de las distancias de Hamming entre el resultado y los bytes correspondientes
     * en los datos de imagen originales.
     *
     * @param op Función que realiza una operación bit a bit sobre un byte, usando un parámetro adicional `n`.
     * @param img_data Puntero a los datos de la imagen original.
     * @param reversed_mask Puntero a los datos enmascarados a procesar.
     * @param seed Posición inicial dentro de `img_data` desde donde se comparará.
     * @param mask_size Tamaño de la máscara (en píxeles o unidades).
     * @param n Parámetro adicional usado por la función `op` (por ejemplo, número de bits a rotar).
     * @return La suma total de las distancias de Hamming entre los datos procesados y los datos originales.
     */
    uint32_t total_hamm_dist = 0;

    for (uint32_t i=0; i < mask_size*RGB_CHANNELS; i++) {
        total_hamm_dist += hamming_distance(
            op(reversed_mask[i], n), img_data[i + seed]);
    }

    return total_hamm_dist;
}

uint32_t validate_xor(const uint8_t *img_data, const uint8_t *noisy_img_data,
                    const uint8_t *reversed_mask, const uint32_t seed, const uint32_t mask_size)
{
    /**
     * @brief Valida el desciframiento comparando la imagen transformada y la ruidosa.
     *
     * Realiza una operación XOR entre cada byte de la imagen transformada y la imagen con ruido,
     * luego calcula la distancia de Hamming entre ese resultado y el byte correspondiente
     * de la máscara revertida.
     *
     * @param img_data Puntero a los bytes muestra de la imagen transformada.
     * @param noisy_img_data Puntero a los bytes de la muestra de imagen con ruido.
     * @param reversed_mask Puntero a los bytes de la máscara revertida.
     * @param mask_size Número de píxeles totales de la máscara (cada píxel tiene 3 canales RGB).
     * @return Distancia total de Hamming entre los bytes XOR y la máscara revertida. (Menor distancia hamming son bytes más parecidos)
     */
    uint32_t total_hamm_dist = 0;

    for (uint32_t i=0; i < mask_size*RGB_CHANNELS; i++) {
        total_hamm_dist += hamming_distance(
            xor_byte(img_data[i + seed], noisy_img_data[i + seed]), reversed_mask[i]);
    }

    return total_hamm_dist;
}

uint8_t shift_left_byte(const uint8_t byte, const uint8_t n)
{

    /**
     * @brief Aplica la operación de corrimiento de n bits en un byte.
     * Cuando n es mayor a MAX_SHIFT se reinicia la cantidad de bits a desplazar
     * @param byte_1 Primer operando.
     * @param n Número de bits a desplazar
     * @return uint8_t Resultado de (byte << (n % MAX_SHIFT));
     */
    return (byte << (n % MAX_SHIFT));
}

uint8_t shift_right_byte(const uint8_t byte, const uint8_t n)
{
    /**
     * @brief Realiza un desplazamiento a la derecha sobre un byte.
     *
     * Esta función desplaza los bits del byte hacia la derecha `n` posiciones.
     * El valor de `n` se reduce módulo MAX_SHIFT para evitar desplazamientos mayores a 8.
     * Los bits desplazados se descartan y se introducen ceros por la izquierda.
     *
     * @param byte Byte a desplazar.
     * @param n Número de posiciones a desplazar (modulo MAX_SHIFT).
     * @return uint8_t Resultado del desplazamiento (byte >> (n % MAX_SHIFT)).
     */
    return (byte >> (n % MAX_SHIFT));
}

uint8_t rotate_left_byte(const uint8_t byte, const uint8_t n)
{
    /**
     * @brief Realiza una rotación circular a la izquierda sobre un byte.
     *
     * Esta función rota los bits del byte `n` posiciones hacia la izquierda.
     * Los bits que se desplazan fuera por la izquierda se reintegran por la derecha.
     * Se asegura que el número de posiciones esté en el rango válido (0–7) usando módulo.
     *
     * @param byte Byte a rotar.
     * @param n Número de posiciones a rotar (se reduce módulo BITS_ON_BYTE).
     * @return uint8_t Resultado del byte rotado a la izquierda.
     */
    uint8_t shift = n % BITS_ON_BYTE;
    return ((byte << shift) | (byte >> (BITS_ON_BYTE - shift)));
}

uint8_t rotate_right_byte(const uint8_t byte, const uint8_t n)
{
    /**
     * @brief Realiza una rotación circular a la derecha sobre un byte.
     *
     * Esta función rota los bits del byte `n` posiciones hacia la derecha.
     * Los bits que se desplazan fuera por la derecha se reintegran por la izquierda.
     * Se asegura que el número de posiciones esté en el rango válido (0–7) usando módulo.
     *
     * @param byte Byte a rotar.
     * @param n Número de posiciones a rotar (se reduce módulo BITS_ON_BYTE).
     * @return uint8_t Resultado del byte rotado a la derecha.
     */
    uint8_t shift = n % BITS_ON_BYTE;
    return ((byte >> shift) | (byte << (BITS_ON_BYTE - shift)));
}

uint8_t xor_byte(const uint8_t byte_1, const uint8_t byte_2)
{
    /**
     * @brief Aplica la operación XOR bit a bit entre dos bytes.
     *
     * Esta función realiza una operación XOR (exclusiva) entre dos valores de 8 bits.
     * El resultado tendrá un bit activado en cada posición donde los bits de entrada difieran.
     *
     * @param byte_1 Primer operando.
     * @param byte_2 Segundo operando.
     * @return uint8_t Resultado de (byte_1 ^ byte_2).
     */
    return (byte_1 ^ byte_2);
}

uint8_t hamming_distance(const uint8_t byte_1, const uint8_t byte_2)
{
    /**
     * @brief Calcula la distancia de Hamming entre dos bytes.
     *
     * Esta función compara dos valores de 8 bits (`byte_1` y `byte_2`) y devuelve
     * la cantidad de bits en los que difieren. Internamente realiza una operación
     * XOR bit a bit para identificar las diferencias, y luego cuenta la cantidad
     * de bits en 1 usando el algoritmo de Brian Kernighan.
     *
     * @param byte_1 Primer byte de entrada.
     * @param byte_2 Segundo byte de entrada.
     * @return uint8_t Número de bits diferentes entre `byte_1` y `byte_2`.
     */
    uint8_t hamming_byte = xor_byte(byte_1, byte_2);
    uint8_t hamming_cnt = 0;

    while(hamming_byte) {
        hamming_byte &= (hamming_byte - 1);
        hamming_cnt++;
    }

    return hamming_cnt;
}

