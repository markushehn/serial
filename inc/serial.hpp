/**
 * @file serial.hpp
 * @brief Serial library header file
 * @author Markus Hehn
 * @date 28.11.2023
 * 
 * Serial port library for Linux.
 * The functionality of this library is similar to PySerial.
 */


#ifndef SERIAL_HPP
#define SERIAL_HPP


#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>


namespace serial
{
    class SerialError : public std::runtime_error
    {
    public:
        explicit SerialError(const std::string& msg) : std::runtime_error(msg) {}
    };
    
    
    class SerialTimeoutException : public std::runtime_error
    {
    public:
        explicit SerialTimeoutException(const std::string& msg) : std::runtime_error(msg) {}
    };
    
    
    class Serial
    {
    private:
        std::string port_stored;
        uint32_t baudrate_stored;
        float timeout_stored;                           // read timeout in seconds
        bool open_flag;
        int serial_fd;
    public:
        Serial();
        explicit Serial(std::string port, uint32_t baudrate);
        explicit Serial(std::string port, uint32_t baudrate, float timeout);
        Serial(const Serial&) = delete;
        ~Serial();
        
        // serial control
        void open(void);
        void close(void);
        
        // write or read data on serial port
        uint32_t write(std::string data);
        uint32_t write(std::vector<uint8_t> data);
        std::string readline(void);
        std::vector<uint8_t> read(uint32_t size);
        
        // reset serial buffers
        void reset_input_buffer(void);
        void reset_output_buffer(void);
        
        // write or read serial control lines
        void rts(bool state);
        void dtr(bool state);
        bool rts(void);
        bool dtr(void);
        bool cts(void);
        bool dsr(void);
        
        // write or read serial settings
        bool is_open(void);
        std::string port(void);
        void port(std::string new_port);
        uint32_t baudrate(void);
        void baudrate(uint32_t new_baudrate);
        float timeout(void);
        void timeout(float new_timeout);
        
        friend std::ostream& operator<< (std::ostream &out, Serial const& serial_obj);
    };
}


#endif
