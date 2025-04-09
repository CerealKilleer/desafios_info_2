#ifndef PROCESS_DATA_HPP
#define PROCESS_DATA_HPP
    #include <QCoreApplication>
    #include <QImage>
    uint8_t *reverse_mask(const uint32_t *, const uint8_t *, const int);
    unsigned char* loadPixels(QString input, int &width, int &height);
    bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
    unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
    uint8_t *get_reversed_mask(const char *, const QString, int &seed, int &n_pixels);
    int app_prueba();
    void probar_operaciones();
#endif // PROCESS_DATA_HPP
