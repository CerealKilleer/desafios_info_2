#include <iostream>
#include <stdint.h>

using namespace std;

static inline uint8_t xor_byte(uint8_t, uint8_t);
static inline uint8_t shift_left_byte(uint8_t, uint8_t);
static inline uint8_t shift_right_byte(uint8_t, uint8_t);
static inline uint8_t rotate_left_byte(uint8_t, uint8_t);
static inline uint8_t rotate_right_byte(uint8_t, uint8_t);

static inline uint8_t xor_byte(uint8_t byte_1, uint8_t byte_2)
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

static inline uint8_t shift_left_byte(uint8_t byte, uint8_t n)
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

static inline uint8_t shift_right_byte(uint8_t byte, uint8_t n)
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

static inline uint8_t rotate_left_byte(uint8_t byte, uint8_t n)
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

static inline uint8_t rotate_right_byte(uint8_t byte, uint8_t n)
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
