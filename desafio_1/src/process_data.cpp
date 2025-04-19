#include <stdint.h>
#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include "include/process_data.hpp"
#include "include/bitwise_pixel.hpp"
#include "include/constants.hpp"

using namespace std;

static uint8_t apply_ops(const int32_t op, const uint8_t *img_data, const uint8_t *img_noisy, const uint8_t *reversed_mask,
                                    const uint32_t seed, const uint32_t num_pixels, uint8_t &op_code);
static uint8_t *get_reversed_mask(const char *path_masking_data, const uint8_t *mask_data, uint32_t &seed, uint32_t &n_pixels);
static uint8_t *reverse_mask(const uint32_t *bytes_masked, const uint8_t *mask, const uint32_t size);
static void reverse_operations(uint8_t *img_data, const uint8_t *img_noisy_data,
                               const uint16_t width, const uint16_t hight, const uint8_t op, const uint8_t n);
static uint8_t validate_ro_sh(uint8_t (*op)(uint8_t, uint8_t), const uint8_t *img_data,
                              const uint8_t *reversed_mask, const uint32_t seed, const uint32_t num_pixels,
                              uint8_t &op_code, uint32_t &max_op_sim, uint8_t curr_op_code, uint8_t curr_n_bits);
void app_img(uint16_t n)
{
    /**
     * @brief Aplica un proceso de desenmascaramiento y reversión de transformaciones bit a bit sobre una imagen codificada.
     *
     * Esta función realiza `n` iteraciones de reversión sobre una imagen (`I_D.bmp`) que ha sido sometida a operaciones de
     * enmascaramiento y transformaciones binarias. Utiliza una imagen de referencia (`I_M.bmp`) y una máscara (`M.bmp`),
     * junto con archivos de datos de enmascaramiento secuenciales (`M(n-1).txt`), para deshacer paso a paso las transformaciones.
     *
     * En cada iteración se realiza:
     * - Carga y validación de los datos de máscara y semilla desde el archivo correspondiente.
     * - Cálculo de la operación aplicada mediante similitud con la imagen original enmascarada.
     * - Aplicación de la operación inversa correspondiente a la imagen transformada.
     *
     * Finalmente, se exporta la imagen restaurada como `I_O.bmp`.
     *
     * @param n Número de transformaciones (y archivos Mx.txt) a revertir. Se asume que las transformaciones fueron aplicadas en orden.
     *
     * @note Esta función depende de otras funciones auxiliares como `loadPixels`, `get_reversed_mask`, `aplicar_operaciones`,
     * `reverse_operations` y `exportImage`. También se apoya en las constantes globales como `RGB_CHANNELS` y `MAX_SIMILARITY`.
     *
     * @warning Si algún archivo no puede abrirse o si las dimensiones de las imágenes son inconsistentes,
     * la función se aborta inmediatamente liberando la memoria utilizada hasta ese momento.
     */

    uint16_t img_width = 0;
    uint16_t img_height = 0;
    uint16_t mask_width = 0;
    uint16_t mask_height = 0;
    uint16_t img_noisy_width = 0;
    uint16_t img_noisy_height = 0;
    uint8_t op_code = 0;
    uint8_t op_n = 0;
    bool ok_img = true;

    uint8_t *mask_data = loadPixels("M.bmp", mask_width, mask_height);

    if (mask_data == nullptr) {
        cout << "No se pudo leer el archivo de máscara M.bmp" << endl;
        return;
    }

    uint8_t *img_noisy_data = loadPixels("I_M.bmp", img_noisy_width, img_noisy_height);

    if (img_noisy_data == nullptr) {
        cout << "No se pudo leer la imagen de entropía I_M.bmp" << endl;
        delete[] mask_data;
        return;
    }

    uint8_t *img_data = loadPixels("I_D.bmp", img_width, img_height);

    if (img_data == nullptr) {
        cout << "Error abriendo I_D.bmp" << endl;
        delete[] mask_data;
        delete[] img_noisy_data;
        return;
    }

    if ((img_width != img_noisy_width) || (img_height != img_noisy_height)) {
        cout << "La imagen objetivo y la imagen de entropía no tienen las mismas dimensiones" << endl;
        delete[] mask_data;
        delete[] img_noisy_data;
        delete[] img_data;
        return;
    }

    //Se aplicarán las n transformaciones
    for (int32_t i=n; i > 0; i--) {
        //Variables para el archivo de enmascaramiento
        uint32_t seed = 0;
        uint32_t num_pixels = 0;
        //Se lee el archivo M(n-1).txt
        QByteArray masked_data_path = "M" + QByteArray::number(i-1) + ".txt";

        //Se aplica el desenmascaramiento
        uint8_t *reversed_mask = get_reversed_mask(masked_data_path.data(), mask_data, seed, num_pixels);
        if (reversed_mask == nullptr) {
            ok_img = false;
            break;
        }

        if ((num_pixels > (img_width*img_height))
            || num_pixels != (mask_width*mask_height)) {

            cout << "La imagen máscara y el archivo de máscara son inconsistentes" << endl;
            delete[] reversed_mask;
            ok_img = false;
            break;
        }

        op_n = apply_ops(i, img_data, img_noisy_data, reversed_mask, seed, num_pixels, op_code);
        reverse_operations(img_data, img_noisy_data, img_width, img_height, op_code, op_n);

        delete[] reversed_mask;
    }

    if (ok_img)
        exportImage(img_data, img_width, img_height, "I_O.bmp");

    delete[] mask_data;
    delete[] img_noisy_data;
    delete[] img_data;
}

static void reverse_operations(uint8_t *img_data, const uint8_t *img_noisy_data,
                               const uint16_t width, const uint16_t hight, const uint8_t op, const uint8_t n)
{
    /**
     * @brief Aplica la operación inversa correspondiente a una imagen procesada para restaurar sus datos originales.
     *
     * Esta función utiliza el código de operación `op` y la cantidad de bits `n` para aplicar la operación inversa adecuada
     * sobre los datos de la imagen (`img_data`), con el objetivo de revertir los efectos de una transformación previa.
     *
     * Para la operación XOR, se utiliza también la imagen ruidosa original (`img_noisy_data`). Para rotaciones y desplazamientos,
     * se invoca la operación inversa correspondiente (por ejemplo, si fue una rotación a la derecha, se aplica una a la izquierda).
     *
     * @param img_data Puntero a los datos de la imagen que será modificada in-place para revertir la operación.
     * @param img_noisy_data Puntero a los datos originales ruidosos usados para revertir la operación XOR.
     * @param width Ancho de la imagen (en píxeles).
     * @param hight Alto de la imagen (en píxeles).
     * @param op Código de la operación original que se desea revertir (XOR, ROR, ROL, SHL, SHR).
     * @param n Cantidad de bits utilizados originalmente en la operación (usado para rotaciones/desplazamientos).
     */

    switch(op) {
    case XOR_OP:
        apply_complete_xor(img_data, img_noisy_data, width, hight);
        break;
    case ROR_OP:
        apply_complete_rotate_shift(rotate_left_byte, img_data, n, width, hight);
        break;
    case ROL_OP:
        apply_complete_rotate_shift(rotate_right_byte, img_data, n, width, hight);
        break;
    case SHR_OP:
        apply_complete_rotate_shift(shift_left_byte, img_data, n, width, hight);
        break;
    case SHL_OP:
        apply_complete_rotate_shift(shift_right_byte, img_data, n, width, hight);
        break;
    default:
        cout << "Valor de operación desconocido" << endl;
    }
}

static uint8_t validate_ro_sh(uint8_t (*op)(uint8_t, uint8_t), const uint8_t *img_data,
                                const uint8_t *reversed_mask, const uint32_t seed, const uint32_t num_pixels,
                                uint8_t &op_code, uint32_t &max_op_sim, uint8_t curr_op_code, uint8_t curr_n_bits)
{
    /**
     * @brief Evalúa múltiples desplazamientos o rotaciones sobre una máscara y determina la mejor configuración.
     *
     * Esta función aplica una operación bit a bit (`op`) sobre los datos de la máscara invertida (`reversed_mask`)
     * para cada posible cantidad de bits (de 0 a 8). Luego compara los resultados con los datos originales de imagen
     * (`img_data`) utilizando la función `validate_rotate_shift_process` y calcula la similitud (por ejemplo, mediante
     * la distancia de Hamming).
     *
     * Si una configuración proporciona una mejor similitud (menor distancia), se actualizan los parámetros de salida
     * correspondientes: `max_op_sim`, `op_code` y el número de bits óptimo (`op_n`).
     *
     * @param op Puntero a una función que realiza una operación bit a bit entre dos valores uint8_t (por ejemplo, rotación o desplazamiento).
     * @param img_data Puntero al arreglo con los datos de la imagen original.
     * @param reversed_mask Puntero al arreglo con la máscara invertida que será procesada.
     * @param seed Índice base en `img_data` desde el cual se iniciará la comparación.
     * @param num_pixels Número de píxeles (sin contar canales RGB) a procesar.
     * @param op_code Referencia a una variable donde se almacenará el código de la operación si se encuentra una mejor.
     * @param max_op_sim Referencia a la variable que contiene la mejor similitud encontrada hasta el momento (valor mínimo).
     * @param curr_op_code Código de la operación actual que se está evaluando.
     * @param curr_n_bits Número de bits inicial para la operación actual.
     * @return El número de bits (`op_n`) que produce la mayor similitud entre los datos procesados y los datos originales.
     */

    uint8_t op_n = curr_n_bits;

    for (uint8_t i=0; i <= BITS_ON_BYTE; i++) {
        uint32_t op_sim = validate_rotate_shift_process(op, img_data, reversed_mask, seed, num_pixels, i);
        if ((op_sim == MAX_SIMILARITY) || (op_sim < max_op_sim)) {
            max_op_sim = op_sim;
            op_code = curr_op_code;
            op_n = i;
        }
    }

    return op_n;
}

static uint8_t apply_ops(const int32_t op, const uint8_t *img_data, const uint8_t *img_noisy, const uint8_t *reversed_mask,
                                    const uint32_t seed, const uint32_t num_pixels, uint8_t &op_code)
{
    /**
     * @brief Determina qué operación bit a bit aplicada a una máscara invertida genera la mayor similitud con una imagen ruidosa.
     *
     * Esta función prueba diferentes operaciones bit a bit (XOR, rotaciones y desplazamientos) sobre una máscara invertida
     * para encontrar la que produce mayor similitud con una imagen ruidosa (`img_noisy`) en comparación con la imagen original (`img_data`).
     *
     * La similitud se calcula usando funciones como `validate_xor` y `validate_ro_sh`, y se mide mediante una métrica como la
     * distancia de Hamming. Si se encuentra una coincidencia perfecta (`MAX_SIMILARITY`), se retorna inmediatamente.
     *
     * @param op Índice o identificador de la operación que se está evaluando (usado solo para propósitos de impresión).
     * @param img_data Puntero al arreglo que contiene los datos originales de la imagen limpia.
     * @param img_noisy Puntero al arreglo con la imagen ruidosa que se quiere comparar.
     * @param reversed_mask Puntero a la máscara invertida que se usará para probar las operaciones.
     * @param seed Índice desde el cual se empezará a comparar en la imagen.
     * @param num_pixels Número de píxeles a evaluar (sin contar canales RGB).
     * @param op_code Referencia a una variable donde se almacenará el código de la operación con mayor similitud.
     * @return El número de bits usados en la operación que dio mayor similitud (para XOR retorna un valor dummy `DUMMY_N`).
     */

    uint32_t max_op_sim = 0;
    uint8_t op_n = 0;

    //Aplicar test para XOR
    max_op_sim = validate_xor(img_data, img_noisy, reversed_mask, seed, num_pixels);

    if (max_op_sim == MAX_SIMILARITY) {
        cout << "La operación #" << op << " fue: " << "XOR" << endl;
        op_code = XOR_OP;
        return DUMMY_N;
    }

    //Aplicar test para ROR
    op_n = validate_ro_sh(rotate_right_byte, img_data, reversed_mask, seed, num_pixels, op_code, max_op_sim, ROR_OP, op_n);
    if (max_op_sim == MAX_SIMILARITY) {
        cout << "La operación #" << op << " fue: " << "rotación a la derecha de " << (uint32_t)op_n << " bits" << endl;
        return op_n;
    }

    //Aplicar test para ROL
    op_n = validate_ro_sh(rotate_left_byte, img_data, reversed_mask, seed, num_pixels, op_code, max_op_sim, ROL_OP, op_n);
    if (max_op_sim == MAX_SIMILARITY) {
        cout << "La operación #" << op << " fue: " << "rotación a la izquierda de " << (uint32_t)op_n << " bits" << endl;
        return op_n;
    }

    //Aplicar test para SHL
    op_n = validate_ro_sh(shift_left_byte, img_data, reversed_mask, seed, num_pixels, op_code, max_op_sim, SHL_OP, op_n);
    if (max_op_sim == MAX_SIMILARITY) {
        cout << "La operación #" << op << " fue: " << "desplazamiento a la izquierda de " << (uint32_t)op_n << " bits" << endl;
        return op_n;
    }

    //Aplicar test para SHR
    op_n = validate_ro_sh(shift_right_byte, img_data, reversed_mask, seed, num_pixels, op_code, max_op_sim, SHR_OP, op_n);
    if (max_op_sim == MAX_SIMILARITY) {
        cout << "La operación #" << op << " fue: " << "desplazamiento a la derecha de " << (uint32_t)op_n << " bits" << endl;
        return op_n;
    }

    //Si no se alcanzó similitud exacta
    switch(op_code) {
    case XOR_OP:
        cout << "Operación #" << op << " de máxima similitud: " << "XOR" << endl;
        break;
    case ROL_OP:
        cout << "Operación #" << op << " de máxima similitud: " << "Rotacion a izquierda de: " << op_n << " bits" << endl;
        break;
    case ROR_OP:
        cout << "Operación #" << op << " de máxima similitud: " << "Rotacion a derecha de: " << op_n << " bits" << endl;
        break;
    case SHL_OP:
        cout << "Operación #" << op <<" de máxima similitud: " << "Desplazamiento a izquierda de: " << op_n << " bits" << endl;
        break;
    case SHR_OP:
        cout << "Operación #" << op << " de máxima similitud: " << "Desplazamiento a derecha de: " << op_n << " bits" << endl;
        break;
    default:
        cout << "No se detectó ninguna operación" << endl;
        break;
    }
    return op_n;
}

static uint8_t *get_reversed_mask(const char *path_masking_data, const uint8_t *mask_data, uint32_t &seed, uint32_t &n_pixels)
{
    /**
     * @brief Recupera la máscara original aplicada sobre una imagen usando una semilla y una secuencia de enmascaramiento.
     *
     * Esta función carga los datos de enmascaramiento desde un archivo de texto (que contiene la semilla y la secuencia de píxeles a modificar),
     * y luego utiliza esa información para revertir el proceso de enmascaramiento aplicado sobre una imagen de máscara.
     *
     * @param path_masking_data Ruta del archivo que contiene la semilla y los datos de enmascaramiento.
     * @param mask_data Ruta de la imagen de máscara (formato BMP u otro compatible).
     * @param seed Referencia a una variable donde se almacenará la semilla leída desde el archivo de enmascaramiento.
     * @param n_pixels Referencia a una variable donde se almacenará la cantidad de píxeles afectados por la máscara.
     * @return Puntero a un arreglo de uint8_t que representa la máscara original (recuperada). Debe ser liberado por el llamador con delete[].
     *         Retorna nullptr si ocurre algún error durante la lectura de archivos o en el proceso de reversión.
     */

    unsigned int *masking_data = loadSeedMasking(path_masking_data, seed, n_pixels);

    if (masking_data == nullptr) {
        cout << "Error leyendo el archivo de mascaras" << endl;
        return nullptr;
    }

    uint8_t *reversed_mask = reverse_mask(masking_data, mask_data, n_pixels);

    delete[] masking_data;

    if (reversed_mask == nullptr) {
        cout << "Ocurió un error recuperando la máscara" << endl;
        return nullptr;
    }

    return reversed_mask;
}

static uint8_t *reverse_mask(const uint32_t *bytes_masked, const uint8_t *mask, const uint32_t size)
{
    /**
     * @brief Invierte el enmascaramiento aplicado a una imagen RGB.
     *
     * Esta función toma un arreglo de bytes enmascarados (`bytes_masked`) y una máscara (`mask`),
     * ambos de tamaño `size * RGB_CHANNELS`, y calcula el arreglo original antes del enmascaramiento.
     * El enmascaramiento asumido es del tipo: s(k) = ID(k+s) + M(k),
     * por lo que esta función aplica la operación inversa: ID(k+s) = s(k) - M(k)
     *
     * @param bytes_masked Puntero a los datos de la imagen enmascarada (RGB intercalado).
     * @param mask Puntero a los datos de la máscara aplicada (RGB intercalado).
     * @param size Cantidad de píxeles de la imagen.
     * @return Puntero a un nuevo arreglo dinámico con los valores desenmascarados.
     *         Es responsabilidad del llamador liberar esta memoria usando `delete[]`.
     *
     * @note El arreglo retornado tiene un tamaño de `size * RGB_CHANNELS` bytes.
     * @warning Esta función usa memoria dinámica. No olvides liberar el resultado cuando ya no se necesite.
     */
    uint8_t *reversed_mask = new uint8_t[size*RGB_CHANNELS];

    if (reversed_mask == nullptr)
        return nullptr;

    for (uint32_t i = 0; i < size*RGB_CHANNELS; i++)
        reversed_mask[i] = bytes_masked[i] - mask[i];

    return reversed_mask;
}

unsigned char* loadPixels(QString input, uint16_t &width, uint16_t &height)
{
    /*
     * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
     *
     * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
     * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
     * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
     * de cada píxel de la imagen, sin rellenos (padding).
     *
     * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
     * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
     * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
     * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
     *         Devuelve nullptr si la imagen no pudo cargarse.
     *
     * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando `delete[]`.
     */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);              // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;     // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                    // Copia los píxeles RGB de esa línea (sin padding)
    }

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, uint16_t width, uint16_t height, QString archivoSalida)
{
    /*
     * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
     *
     * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
     * `pixelData`, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
     * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
     * en formato BMP en la ruta especificada.
     *
     * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
     *                  El tamaño debe ser igual a width * height * 3 bytes.
     * @param width Ancho de la imagen en píxeles.
     * @param height Alto de la imagen en píxeles.
     * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
     *
     * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
     *
     * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
     */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }

}

unsigned int* loadSeedMasking(const char* nombreArchivo, uint32_t &seed, uint32_t &n_pixels)
{
    /*
     * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
     *
     * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
     * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
     * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
     * y finalmente carga los valores en un arreglo de enteros.
     *
     * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
     * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
     * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
     *                 (equivalente al número de líneas después de la semilla).
     *
     * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
     *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
     *
     * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
     */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        // Verificar si el archivo pudo abrirse correctamente
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla desde la primera línea del archivo
    archivo >> seed;

    int r, g, b;

    // Contar cuántos grupos de valores RGB hay en el archivo
    // Se asume que cada línea después de la semilla tiene tres valores (r, g, b)
    while (archivo >> r >> g >> b) {
        n_pixels++;  // Contamos la cantidad de píxeles
    }

    // Cerrar el archivo para volver a abrirlo desde el inicio
    archivo.close();
    archivo.open(nombreArchivo);

    // Verificar que se pudo reabrir el archivo correctamente
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }

    // Reservar memoria dinámica para guardar todos los valores RGB
    // Cada píxel tiene 3 componentes: R, G y B
    unsigned int* RGB = new unsigned int[n_pixels * 3];

    // Leer nuevamente la semilla desde el archivo (se descarta su valor porque ya se cargó antes)
    archivo >> seed;

    // Leer y almacenar los valores RGB uno por uno en el arreglo dinámico
    for (uint32_t i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    // Cerrar el archivo después de terminar la lectura
    archivo.close();

    // Mostrar información de control en consola
    /*
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;
    */
    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}


