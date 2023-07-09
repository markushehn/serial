/**
 * @file main.cpp
 * @brief main source file
 * @author Markus Hehn
 * @date 09.07.2022
 */


#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <chrono>
#include <thread>
#include "serial.hpp"


void serial_test(void)
{
    /* object generation test */
    std::cout << "Serial object generation" << std::endl;
    
    auto serial_obj = serial::Serial("/dev/ttyUSB0",115200,1.0);
    //serial::Serial serial_obj2(serial_obj);                       // if this line is uncommented, a compiler error is generated
    
    try
    {
        serial_obj.open();
        std::cout << serial_obj << std::endl;
    }
    catch(serial::SerialError &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    serial_obj.timeout(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    
    
    /* control line test */
    std::cout << std::endl << "Serial control line test" << std::endl;
    serial_obj.rts(false);
    std::cout << "RTS = " << serial_obj.rts() << std::endl;
    std::cout << "CTS = " << serial_obj.cts() << std::endl;
    
    
    
    /* write and read test */
    std::cout << std::endl << "Serial write and read test" << std::endl;
    
    std::string write_string = "test string\n";
    
    std::vector<uint8_t> write_vector_1{0 , 1 , 2, 3, 4, 5, 6, 7, 8, '\n'};
    /*for(int i = 0; i < 9; i++)
        write_vector_1[i] = i;
    write_vector_1[9] = '\n';*/
    
    std::vector<uint8_t> write_vector_2{10 , 11 , 12, 13, 14, 15, 16, 17, 18, '\n'};
    /*for(int i = 0; i < 9; i++)
        write_vector_2[i] = i + 20;
    write_vector_2[9] = '\n';*/
    
    std::cout << "write num: " << serial_obj.write(write_string) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    serial_obj.reset_input_buffer();
    std::cout << "write num: " << serial_obj.write(write_vector_1) << std::endl;
    
    try
    {
        std::vector<uint8_t> read_vector = serial_obj.read(10);
        std::cout << "read 1: ";
        for(int i = 0; i < 10; i++)
            std::cout << (int)read_vector[i] << " ";
        std::cout << std::endl;
    }
    catch(serial::SerialTimeoutException &e)
    {
        std::cout << "SerialTimeoutException: " << e.what() << std::endl;
    }
    
    std::cout << "write num: " << serial_obj.write(write_vector_2) << std::endl;
    
    try
    {
        std::vector<uint8_t> read_vector = serial_obj.read(10);
        std::cout << "read 2: ";
        for(int i = 0; i < 10; i++)
            std::cout << (int)read_vector[i] << " ";
        std::cout << std::endl;
    }
    catch(serial::SerialTimeoutException &e)
    {
        std::cout << "SerialTimeoutException: " << e.what() << std::endl;
    }
    
    
    
    /* readline test */
    std::cout << std::endl << "readline test" << std::endl;
    
    std::cout << "write num: " << serial_obj.write(write_string) << std::endl;
    
    try
    {
        std::string read_string = serial_obj.readline();
        std::cout << read_string.length() << std::endl;
        std::cout << read_string;
        std::cout << "readline executed" << std::endl;
        
        for(uint32_t i = 0; i < read_string.length(); i++)
        {
            std::cout << (uint32_t)read_string.c_str()[i] << " ";
        }
        std::cout << std::endl;
    }
    catch(serial::SerialError &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}



int main(void)
{
    serial_test();
    return 0;
}



