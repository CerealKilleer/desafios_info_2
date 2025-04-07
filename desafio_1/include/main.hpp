#ifndef MAIN_HPP
#define MAIN_HPP
    unsigned char* loadPixels(QString input, int &width, int &height);
    bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
    unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
    int app_prueba();
#endif
