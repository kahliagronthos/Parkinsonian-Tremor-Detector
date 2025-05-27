#include <array>
#include "mbed.h"

// Gyroscope SPI pins
#define GYRO_MOSI PF_9
#define GYRO_MISO PF_8
#define GYRO_SCLK PF_7
#define GYRO_SSEL PC_1

// Address of first register with gyro data
#define OUT_X_L 0x28

// Register fields(bits): data_rate(2), Bandwidth(2), Power_down(1), Zen(1), Yen(1), Xen(1)
#define CTRL_REG1 0x20
// Configuration: 200Hz ODR,50Hz cutoff, Power on, Z on, Y on, X on
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1

// Register fields(bits): reserved(1), endian-ness(1), Full scale sel(2), reserved(1), self-test(2), SPI mode(1)
#define CTRL_REG4 0x23
// Configuration: reserved, little endian, 500 dps, reserved, disabled, 4-wire mode
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0

// Read/write buffer for SPI
#define BUFFER_SIZE 32
// Conversion to Degrees per second
#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)

#define SPI_FLAG 1
#define DATA_READY_FLAG 2

EventFlags GYRO_FLAGS;
/**
 * Callback function for SPI communication w Gyroscope
 *
 * @param event Integer representing the event id
 *
 * @returns None
 */
void GYRO_SPI_CB(int event) {
    GYRO_FLAGS.set(SPI_FLAG);
}

/**
 * @brief Interfaces with and encapsulates an SPI gyroscope sensor 
 * 
 */
class Gyroscope {
private:
    SPI spi;
    uint8_t write_buf[BUFFER_SIZE]; 
    uint8_t read_buf[BUFFER_SIZE];
    
public:
    /** Default Constructor
     * Initializes the gyroscope sensor.
     *
     * This constructor initializes the gyroscope sensor by configuring the SPI communication,
     * setting the control registers, and waiting for the SPI transfer to complete.
     * The gyroscope sensor is connected using the specified SPI pins and uses the specified GPIO
     * for the slave select (SSEL) signal.
     *
     * @param use_gpio_ssel Determines whether to use GPIO for the slave select (SSEL) signal.
     *
     * @returns None
     */
    Gyroscope() : spi(GYRO_MOSI, GYRO_MISO, GYRO_SCLK, GYRO_SSEL, use_gpio_ssel) {
        // Setup the spi for 8 bit data, high steady state clock,
        // second edge capture, with a 1MHz clock rate
        spi.format(8,3);
        spi.frequency(1'000'000);

        // Configuration of Gyroscope
        //configuration: 200Hz ODR, 50Hz cutoff, Power on, Z on, Y on, X on
        write_buf[0] = CTRL_REG1;
        write_buf[1] = CTRL_REG1_CONFIG;
        spi.transfer(write_buf, 2, read_buf, 2, GYRO_SPI_CB, SPI_EVENT_COMPLETE );
        GYRO_FLAGS.wait_all(SPI_FLAG);

        //configuration: reserved, little endian, 500 dps, reserved, disabled, 4-wire mode
        write_buf[0] = CTRL_REG4;
        write_buf[1] = CTRL_REG4_CONFIG;
        spi.transfer(write_buf, 2, read_buf, 2, GYRO_SPI_CB, SPI_EVENT_COMPLETE );
        GYRO_FLAGS.wait_all(SPI_FLAG); 
    }

    /**
     * Reads the current X, Y, and Z values from the sensor sequentially.
     *
     * @returns An array of floats containing the X, Y, and Z values.
     */
    std::array<float, 3> sequential_read() {
        int16_t raw_x, raw_y, raw_z;
        std::array<float, 3> output_xyz;

        // prepare the write buffer to trigger a sequential read
        write_buf[0]= OUT_X_L | 0x80 | 0x40;

        // start sequential sample reading
        spi.transfer(write_buf, 7, read_buf, 7, GYRO_SPI_CB, SPI_EVENT_COMPLETE);
        GYRO_FLAGS.wait_all(SPI_FLAG);

        //read_buf after transfer: garbage byte, x_low, x_high, y_low, y_high, z_low, z_high
        //Put the high and low bytes in the correct order lowB,HighB -> HighB,LowB
        raw_x =( ((uint16_t)read_buf[2] ) << 8 ) | ((uint16_t)read_buf[1] );
        raw_y =( ((uint16_t)read_buf[4] ) << 8 ) | ((uint16_t)read_buf[3] );
        raw_z =( ((uint16_t)read_buf[6] ) << 8 ) | ((uint16_t)read_buf[5] );

        // Convert to degrees per sec
        output_xyz[0] = ((float)raw_x)*(SCALING_FACTOR);
        output_xyz[1] = ((float)raw_y)*(SCALING_FACTOR);
        output_xyz[2] = ((float)raw_z)*(SCALING_FACTOR);

        spi.clear_transfer_buffer();
        return output_xyz;
    }
    
    // Terminate spi connection early. (Avoid traffic)
    void endSPI() {
        spi.~SPI();
    }
};