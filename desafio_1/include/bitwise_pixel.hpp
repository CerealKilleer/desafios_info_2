#ifndef BITWISE_PIXEL_HPP
#define BITWISE_PIXEL_HPP
    #include <stdint.h>
    uint32_t validate_xor(const uint8_t *img_data, const uint8_t *noisy_img_data,
                                const uint8_t *reversed_mask, const uint32_t seed, const uint32_t mask_size);
    uint32_t validate_ror(const uint8_t *img_data, const uint8_t *reversed_mask,
                                const uint32_t seed, const uint32_t mask_size, const uint8_t n);
    uint32_t validate_rol(const uint8_t *img_data, const uint8_t *reversed_mask,
                                 const uint32_t seed, const uint32_t mask_size, const uint8_t n);
    uint32_t validate_shl(const uint8_t *img_data, const uint8_t *reversed_mask,
                          const uint32_t seed, const uint32_t mask_size, const uint8_t n);
    uint32_t validate_shr(const uint8_t *img_data, const uint8_t *reversed_mask,
                          const uint32_t seed, const uint32_t mask_size, const uint8_t n);
    void apply_complete_shr(uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight);
    void apply_complete_shl(uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight);
    void apply_complete_rol(uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight);
    void apply_complete_ror(uint8_t *img_data, const uint8_t n, const uint16_t width, const uint16_t hight);
    void apply_complete_xor(uint8_t *img_data, const uint8_t *img_noisy_data, const uint16_t width, const uint16_t hight);
    void pruebas_bitwise_byte_ops(void);

#endif // BITWISE_PIXEL_HPP
