
//c++ lib headers
#include <iostream>

//third party lib headers

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

int main()
{
    boost::system::error_code ec;

    //create a "context" - a platform specific interface
    boost::asio::io_context context;

    // get the address of somewhere to connect to
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1", ec), 80);

    //create a socket, the context handles the implementation
    boost::asio::ip::tcp::socket socket(context);

    //tell the socket to try and connect
    socket.connect(endpoint, ec);

    if(!ec)
    {
        std::cout << "Connected!" << std::endl;
    } 
    else
    {
        std::cout << "Failed to connect to address: \n" << ec.message() << std::endl;
    }

    if(socket.is_open())
    {

    }

    return 0;
}