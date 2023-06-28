#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <chrono>
#include <thread>


#include "serial.hpp"



int main(void)
{
    auto serial_obj = serial::Serial("/dev/ttyUSB0",115200,1.0);
    //serial::Serial serial_obj2(serial_obj);
    
    
    
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
    std::cout << "test start" << std::endl;
    
    
    
    
    
    
    serial_obj.rts(false);
    std::cout << "CTS = " << serial_obj.cts() << std::endl;
    
    
    
    std::string test_string = "0123456789A\n";
    
    std::vector<uint8_t> test_vector(10);
    for(int i = 0; i < 9; i++)
        test_vector[i] = i;
    test_vector[9] = '\n';
    
    std::cout << "write num: " << serial_obj.write(test_string) << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    for(int i = 0; i < 9; i++)
        test_vector[i] = i + 20;
    
    std::cout << "write num: " << serial_obj.write(test_vector) << std::endl;
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //serial_obj.reset_input_buffer();
    
    
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
    
    
    
    for(int i = 0; i < 9; i++)
        test_vector[i] = i + 20;
    
    std::cout << "write num: " << serial_obj.write(test_vector) << std::endl;
    
    
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
    
    //std::cout << "readline 2: " << serial_obj.readline();
    //std::cout << "readline 3: " << serial_obj.readline();
    
    while(1);
    
    //serial_obj.write(test);
    
    while(1)
    {
        try
        {
            
            std::vector<uint8_t> data_vector;
            data_vector = serial_obj.read(3);
            std::cout << data_vector.size() << std::endl;
            
            for(uint32_t i = 0; i < data_vector.size(); i++)
                std::cout << data_vector[i];
            
            std::cout << "read executed" << std::endl;
            
        }
        catch(serial::SerialError &e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    }
    
    
    /*while(1)
    {
        try
        {
            std::string rx_string = serial_obj.readline();
            std::cout << rx_string.length() << std::endl;
            std::cout << rx_string;
            std::cout << "readline executed" << std::endl;
            
            
            for(uint32_t i = 0; i < rx_string.length(); i++)
            {
                std::cout << (uint32_t)rx_string.c_str()[i] << " ";
            }
            std::cout << std::endl;
        }
        catch(serial::SerialError &e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    }*/
    

    
    
    
    return 0;
}
