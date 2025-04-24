#ifndef PROCESS_DATA_HPP
#define PROCESS_DATA_HPP
    #include <stdint.h>
    unsigned int* loadSeedMasking(const char* nombreArchivo, uint32_t &seed, uint32_t &n_pixels);
    bool exportImage(unsigned char* pixelData, uint16_t width, uint16_t height, QString archivoSalida);
    unsigned char* loadPixels(QString input, uint16_t &width, uint16_t &height);
    void app_img(uint8_t n);
#endif // PROCESS_DATA_HPP
