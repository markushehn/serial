/**
 * @file serial.cpp
 * @brief Serial library source file
 * @author Markus Hehn
 * @date 07.11.2022
 */


#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <chrono>
#include <thread>

#include <termios.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "serial.hpp"


#define SERIAL_RX_LINE_BUFFER_SIZE              256


namespace serial
{
    Serial::Serial(std::string port, uint32_t baudrate, float timeout)
    {
        this->port_stored = port;
        this->baudrate_stored = baudrate;
        this->timeout_stored = timeout;
        this->open_flag = false;
    }
    
    Serial::Serial() : Serial::Serial("/dev/ttyUSB0", 9600, 1.0) {}
    Serial::Serial(std::string port, uint32_t baudrate) : Serial::Serial(port, baudrate, 1.0) {}
    
    Serial::~Serial()
    {
        try
        {
            this->close();
        }
        catch(...)
        {
        }
    }
    
    
    void Serial::open(void)
    {
        if(this->open_flag == false)
        {
            this->serial_fd = ::open(this->port_stored.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
            
            if(this->serial_fd < 0)
                throw SerialError("Serial open: Unable to open serialport.");
            
            if(flock(this->serial_fd, LOCK_EX | LOCK_NB) < 0)
                throw SerialError("Serial open: Serial port is already locked by another process.");
            
            this->open_flag = true;
            
            
            struct termios port_settings;
            
            if(tcgetattr(this->serial_fd, &port_settings) != 0)                             // read existing settings
                throw SerialError("Serial open: Failed to read existing port settings.");
            
            port_settings.c_cflag = CS8 | CLOCAL | CREAD;
            port_settings.c_iflag = 0;
            port_settings.c_oflag = 0;
            port_settings.c_lflag = 0;
            port_settings.c_cc[VMIN] = 0;
            port_settings.c_cc[VTIME] = 0;
            
            
            speed_t baudrate_termios = B0;
            
            switch(this->baudrate_stored)
            {
                case 9600UL :
                    baudrate_termios = B9600;
                    break;
                case 19200UL :
                    baudrate_termios = B19200;
                    break;
                case 38400UL :
                    baudrate_termios = B38400;
                    break;
                case 57600UL :
                    baudrate_termios = B57600;
                    break;
                case 115200UL :
                    baudrate_termios = B115200;
                    break;
                case 1000000UL :
                    baudrate_termios = B1000000;
                    break;
                default :
                    throw SerialError("Serial open: Baudrate is not supported.");
            }
            
            if(cfsetispeed(&port_settings, baudrate_termios) != 0)
                throw SerialError("Serial open: Failed to set port speed.");
            if(cfsetospeed(&port_settings, baudrate_termios) != 0)
                throw SerialError("Serial open: Failed to set port speed.");
            
            
            if(tcsetattr(this->serial_fd, TCSANOW, &port_settings) != 0)                    // save serial settings
                throw SerialError("Serial open: Failed to set port settings.");
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));                     // wait necessary for buffer flush
            
            if(tcflush(this->serial_fd, TCIOFLUSH) != 0)
                throw SerialError("Serial open: Unable to flush.");
        }
        else
        {
            throw SerialError("Serial open: Serial is already open.");
        }
    }
    
    
    void Serial::close(void)
    {
        if(this->open_flag == true)
        {
            if(this->serial_fd < 0)
                throw SerialError("Serial close: Unable to close serialport.");
            if(flock(this->serial_fd, LOCK_UN) < 0)
                throw SerialError("Serial close: Unable to close serialport.");
            if(::close(this->serial_fd) < 0)
                throw SerialError("Serial close: Unable to close serialport.");
            
            this->open_flag = false;
        }
        else
        {
            throw SerialError("Serial close: Serial is already closed.");
        }
    }
    
    
    uint32_t Serial::write(std::string data)
    {
        uint32_t return_num = 0;
        
        if(this->open_flag == true)
        {
            int num = ::write(this->serial_fd, data.c_str(), data.length());
            
            if(num < 0)
                throw SerialError("Serial write: Unable to write data on serialport.");
            else
                return_num = num;
        }
        else
        {
            throw SerialError("Serial write: Serial is closed.");
        }
        
        return return_num;
    }
    
    
    uint32_t Serial::write(std::vector<uint8_t> data)
    {
        uint32_t return_num = 0;
        
        if(this->open_flag == true)
        {
            int num = ::write(this->serial_fd, &data[0], data.size());
            
            if(num < 0)
                throw SerialError("Serial write: Unable to write data on serialport.");
            else
                return_num = num;
        }
        else
        {
            throw SerialError("Serial write: Serial is closed.");
        }
        
        return return_num;
    }
    
    
    std::string Serial::readline(void)
    {
        std::string empty_string;
        
        if(this->open_flag == true)
        {
            struct termios port_settings;
            if(tcgetattr(this->serial_fd, &port_settings) != 0)                             // read existing settings
                throw SerialError("Serial readline: Failed to read existing port settings.");
            port_settings.c_lflag |= ICANON;                                                // process input when a newline character is received
            if(tcsetattr(this->serial_fd, TCSANOW, &port_settings) != 0)                    // save serial settings
                throw SerialError("Serial readline: Failed to set port settings.");
            
            
            fd_set set;
            FD_ZERO(&set);                                                                  // clear the file descriptor set
            FD_SET(this->serial_fd, &set);                                                  // add the serial file descriptor to the set
            
            
            struct timeval* timeout_ptr;
            struct timeval timeout_struct;
            
            if(this->timeout_stored < 0.0)
                timeout_ptr = NULL;
            else
            {
                timeout_struct.tv_sec = (int)this->timeout_stored;
                timeout_struct.tv_usec = ((int)(this->timeout_stored * 1000000.0) % 1000000);
                timeout_ptr = &timeout_struct;
            }
            
            
            int status = select(this->serial_fd + 1, &set, NULL, NULL, timeout_ptr);
            
            if(status == -1)
                throw SerialError("Serial readline: Select failed.");                       // error occured
            else if(status == 0)
                throw SerialTimeoutException("Serial readline: Timeout occured");           // timeout occured
            else
            {
                char data_buffer[SERIAL_RX_LINE_BUFFER_SIZE + 1];                           // size for received data and plus one for '\0'
                int num = ::read(this->serial_fd, data_buffer, SERIAL_RX_LINE_BUFFER_SIZE);
                
                if(num <= 0)
                    throw SerialError("Serial readline: Unable to read data on serialport.");
                else
                {
                    if(num < SERIAL_RX_LINE_BUFFER_SIZE + 1)
                    {
                        data_buffer[num] = '\0';
                        std::string data(data_buffer);
                        return data;
                    }
                    else
                        throw SerialError("Serial readline: Index out of bounds.");
                }
            }
        }
        else
        {
            throw SerialError("Serial readline: Serial is closed.");
        }
        
        return empty_string;
    }
    
    
    std::vector<uint8_t> Serial::read(uint32_t size)
    {
        std::vector<uint8_t> empty_vector;
        
        if(this->open_flag == true)
        {
            struct termios port_settings;
            if(tcgetattr(this->serial_fd, &port_settings) != 0)                             // read existing settings
                throw SerialError("Serial read: Failed to read existing port settings.");
            port_settings.c_lflag &= ~ICANON;                                               // raw mode
            if(tcsetattr(this->serial_fd, TCSANOW, &port_settings) != 0)                    // save serial settings
                throw SerialError("Serial read: Failed to set port settings.");
            
            
            fd_set set;
            FD_ZERO(&set);                                                                  // clear the file descriptor set
            FD_SET(this->serial_fd, &set);                                                  // add the serial file descriptor to the set
            
            
            struct timeval* timeout_ptr;
            struct timeval timeout_struct;
            
            if(this->timeout_stored < 0.0)
                timeout_ptr = NULL;
            else
            {
                timeout_struct.tv_sec = (int)this->timeout_stored;
                timeout_struct.tv_usec = ((int)(this->timeout_stored * 1000000.0) % 1000000);
                timeout_ptr = &timeout_struct;
            }
            
            
            std::vector<uint8_t> data(size);
            uint32_t num = 0;
            
            while(num < size)
            {
                int status = select(this->serial_fd + 1, &set, NULL, NULL, timeout_ptr);
                
                if(status == -1)
                    throw SerialError("Serial read: Select failed.");                       // error occured
                else if(status == 0)
                    throw SerialTimeoutException("Serial read: Timeout occured");           // timeout occured
                else
                {
                    std::vector<uint8_t> data_buffer_temp(size - num);
                    int num_temp = ::read(this->serial_fd, &data_buffer_temp[0], data_buffer_temp.size());
                    
                    if(num_temp <= 0)
                        throw SerialError("Serial read: Unable to read data on serialport.");
                    else
                    {
                        for(int i = 0; i < num_temp; i++)
                        {
                            if(i + num < size)
                                data[i + num] = data_buffer_temp[i];
                            else
                                break;
                        }
                        
                        num += num_temp;
                    }
                }
            }
            
            return data;
        }
        else
        {
            throw SerialError("Serial read: Serial is closed.");
        }
        
        return empty_vector;
    }
    
    
    void Serial::reset_input_buffer(void)
    {
        if(this->open_flag == true)
        {
            if(tcflush(this->serial_fd, TCIFLUSH) != 0)
                throw SerialError("Serial reset input buffer: Unable to reset buffer.");
        }
        else
        {
            throw SerialError("Serial reset input buffer: Serial is closed.");
        }
    }
    
    
    void Serial::reset_output_buffer(void)
    {
        if(this->open_flag == true)
        {
            if(tcflush(this->serial_fd, TCOFLUSH) != 0)
                throw SerialError("Serial reset output buffer: Unable to reset buffer.");
        }
        else
        {
            throw SerialError("Serial reset output buffer: Serial is closed.");
        }
    }
    
    
    void Serial::rts(bool state)
    {
        if(this->open_flag == true)
        {
            int bit_mask = TIOCM_RTS;
            
            if(state == true)
            {
                if(ioctl(this->serial_fd, TIOCMBIS, &bit_mask) != 0)
                    throw SerialError("Serial RTS: Unable to write RTS.");
            }
            else
            {
                if(ioctl(this->serial_fd, TIOCMBIC, &bit_mask) != 0)
                    throw SerialError("Serial RTS: Unable to write RTS.");
            }
        }
        else
        {
            throw SerialError("Serial RTS: Serial is closed.");
        }
    }
    
    
    void Serial::dtr(bool state)
    {
        if(this->open_flag == true)
        {
            int bit_mask = TIOCM_DTR;
            
            if(state == true)
            {
                if(ioctl(this->serial_fd, TIOCMBIS, &bit_mask) != 0)
                    throw SerialError("Serial DTR: Unable to write DTR.");
            }
            else
            {
                if(ioctl(this->serial_fd, TIOCMBIC, &bit_mask) != 0)
                    throw SerialError("Serial DTR: Unable to write DTR.");
            }
        }
        else
        {
            throw SerialError("Serial DTR: Serial is closed.");
        }
    }
    
    
    bool Serial::rts(void)
    {
        if(this->open_flag == true)
        {
            int bit_mask;
            
            if(ioctl(this->serial_fd, TIOCMGET, &bit_mask) != 0)
                throw SerialError("Serial RTS: Unable to read RTS.");
            
            if(bit_mask & TIOCM_RTS)
                return true;
            else
                return false;
        }
        else
        {
            throw SerialError("Serial RTS: Serial is closed.");
        }
    }
    
    
    bool Serial::dtr(void)
    {
        if(this->open_flag == true)
        {
            int bit_mask;
            
            if(ioctl(this->serial_fd, TIOCMGET, &bit_mask) != 0)
                throw SerialError("Serial DTR: Unable to read DTR.");
            
            if(bit_mask & TIOCM_DTR)
                return true;
            else
                return false;
        }
        else
        {
            throw SerialError("Serial DTR: Serial is closed.");
        }
    }
    
    
    bool Serial::cts(void)
    {
        if(this->open_flag == true)
        {
            int bit_mask;
            
            if(ioctl(this->serial_fd, TIOCMGET, &bit_mask) != 0)
                throw SerialError("Serial CTS: Unable to read CTS.");
            
            if(bit_mask & TIOCM_CTS)
                return true;
            else
                return false;
        }
        else
        {
            throw SerialError("Serial CTS: Serial is closed.");
        }
    }
    
    
    bool Serial::dsr(void)
    {
        if(this->open_flag == true)
        {
            int bit_mask;
            
            if(ioctl(this->serial_fd, TIOCMGET, &bit_mask) != 0)
                throw SerialError("Serial DSR: Unable to read DSR.");
            
            if(bit_mask & TIOCM_LE)
                return true;
            else
                return false;
        }
        else
        {
            throw SerialError("Serial DSR: Serial is closed.");
        }
    }
    
    
    bool Serial::is_open(void)
    {
        return this->open_flag;
    }
    
    
    std::string Serial::port(void)
    {
        return this->port_stored;
    }
    
    
    void Serial::port(std::string new_port)
    {
        this->port_stored = new_port;
    }
    
    
    uint32_t Serial::baudrate(void)
    {
        return this->baudrate_stored;
    }
    
    
    void Serial::baudrate(uint32_t new_baudrate)
    {
        this->baudrate_stored = new_baudrate;
    }
    
    
    float Serial::timeout(void)
    {
        return this->timeout_stored;
    }
    
    
    void Serial::timeout(float new_timeout)
    {
        this->timeout_stored = new_timeout;
    }
    
    
    std::ostream& operator<< (std::ostream &out, Serial const& serial_obj)
    {
        out << "Port name: " << serial_obj.port_stored << std::endl;
        out << "Baudrate: " << serial_obj.baudrate_stored << " Bd" << std::endl;
        
        if(serial_obj.timeout_stored < 0.0)
            out << "Timeout: None" << std::endl;
        else
            out << "Timeout: " << serial_obj.timeout_stored << " s" << std::endl;
        
        if(serial_obj.open_flag == false)
            out << "Status: Closed";
        else
            out << "Status: Open";
        
        return out;
    }
}



