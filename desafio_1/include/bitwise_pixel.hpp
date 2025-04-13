#ifndef BITWISE_PIXEL_HPP
#define BITWISE_PIXEL_HPP
    #include <stdint.h>
    #include "include/constants.hpp"

    uint32_t validate_xor(const uint8_t *img_data, const uint8_t *noisy_img_data,
                                const uint8_t *reversed_mask, const uint32_t seed, const uint32_t mask_size);

    uint32_t validate_rotate_shift_process(uint8_t(*op)(const uint8_t, const uint8_t), const uint8_t *img_data, const uint8_t *reversed_mask,
                                           const uint32_t seed, const uint32_t mask_size, const uint8_t n);

    uint8_t hamming_distance(const uint8_t byte_1, const uint8_t byte_2);

    uint8_t xor_byte(const uint8_t byte_1, const uint8_t byte_2);

    uint8_t rotate_right_byte(const uint8_t byte, const uint8_t n);

    uint8_t rotate_left_byte(const uint8_t byte, const uint8_t n);

    uint8_t shift_right_byte(const uint8_t byte, const uint8_t n);

    uint8_t shift_left_byte(const uint8_t byte, const uint8_t n);

    void apply_rotate_shift_process(uint8_t (*op)(uint8_t byte, uint8_t n),
                                    uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight);

    void apply_complete_xor(uint8_t *img_data, const uint8_t *img_noisy_data, const uint16_t width, const uint16_t hight);

    void pruebas_bitwise_byte_ops(void);

#endif // BITWISE_PIXEL_HPP
