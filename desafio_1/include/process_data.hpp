#ifndef PROCESS_DATA_HPP
#define PROCESS_DATA_HPP
    #include <stdint.h>
    unsigned int* loadSeedMasking(const char* nombreArchivo, uint32_t &seed, uint32_t &n_pixels);
    bool exportImage(unsigned char* pixelData, uint16_t width, uint16_t height, QString archivoSalida);
    unsigned char* loadPixels(QString input, uint16_t &width, uint16_t &height);
    static uint8_t validate_ro_sh(uint8_t (*op)(uint8_t, uint8_t), const uint8_t *img_data,
                                  const uint8_t *reversed_mask, const uint32_t seed, const uint32_t num_pixels,
                                  uint8_t &op_code, uint32_t &max_op_sim, uint8_t curr_op_code, uint8_t curr_n_bits);
    void app_img(uint16_t n);
#endif // PROCESS_DATA_HPP
