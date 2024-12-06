[![Build Status](https://img.shields.io/travis/kaoskorobase/oscpp.svg?style=flat)](https://travis-ci.org/kaoskorobase/oscpp)
[![Build status](https://ci.appveyor.com/api/projects/status/b7qk7t9mmgnc1n1v?svg=true)](https://ci.appveyor.com/project/kaoskorobase/oscpp)

**oscpp** is a header-only C++11 library for constructing and parsing
[OpenSoundControl](http://opensoundcontrol.org) packets. Supported platforms
are MacOS X, iOS, Linux, Android and Windows; the code should be easily
portable to any platform with a C++11 compiler. **oscpp** intends to be a
minimal, high-performance solution for working with OSC data. The library
doesn't perform memory allocation (except when throwing exceptions) or other
system calls and is suitable for use in realtime sensitive contexts such as
audio driver callbacks.

**oscpp** conforms to the [OpenSoundControl 1.0
specification](http://opensoundcontrol.org/spec-1_0). Except for arrays,
non-standard message argument types are currently not supported and there is no
direct support for message address patterns or bundle scheduling; it is up to
the user of the library to implement (a subset of) the semantics according to
the spec.

## Installation

Since **oscpp** only consists of header files, the library doesn't need to be
compiled or installed. Simply put the `include` directory into a location that
is searched by your compiler and you're set.

## Usage

**oscpp** places everything in the `OSCPP` namespace, with the two most
important subnamespaces `Client` for constructing packets and `Server` for
parsing packets.

First let's have a look at how to build OSC packets in memory: Assuming you
have allocated a buffer you can construct a client packet on the stack and
start filling the buffer with data. When all the data has been written, the
`size` method returns the actual size in bytes of the resulting OSC packet.

~~~~cpp
#include <oscpp/client.hpp>

size_t makePacket(void* buffer, size_t size)
{
    // Construct a packet
    OSCPP::Client::Packet packet(buffer, size);
    packet
        // Open a bundle with a timetag
        .openBundle(1234ULL)
            // Add a message with two arguments and an array with 6 elements;
            // for efficiency this needs to be known in advance.
            .openMessage("/s_new", 2 + OSCPP::Tags::array(6))
                // Write the arguments
                .string("sinesweep")
                .int32(2)
                .openArray()
                    .string("start-freq")
                    .float32(330.0f)
                    .string("end-freq")
                    .float32(990.0f)
                    .string("amp")
                    .float32(0.4f)
                .closeArray()
            // Every `open` needs a corresponding `close`
            .closeMessage()
            // Add another message with one argument
            .openMessage("/n_free", 1)
                .int32(1)
            .closeMessage()
            // And nother one
            .openMessage("/n_set", 3)
                .int32(1)
                .string("wobble")
                // Numeric arguments are converted automatically
                // (see below)
                .int32(31)
            .closeMessage()
        .closeBundle();
    return packet.size();
}
~~~~

Now given a suitable packet transport (e.g. a UDP socket or an in-memory FIFO,
see below for a dummy implementation), a packet can be constructed and sent as
follows:

~~~~cpp
class Transport;

size_t send(Transport* t, const void* buffer, size_t size);

void sendPacket(Transport* t, void* buffer, size_t bufferSize)
{
    const size_t packetSize = makePacket(buffer, bufferSize);
    send(t, buffer, packetSize);
}
~~~~

When parsing data from OSC packets you have to handle the two distinct cases of bundles and messages:

~~~~cpp
#include <oscpp/server.hpp>
#include <oscpp/print.hpp>
#include <iostream>

void handlePacket(const OSCPP::Server::Packet& packet)
{
    if (packet.isBundle()) {
        // Convert to bundle
        OSCPP::Server::Bundle bundle(packet);

        // Print the time
        std::cout << "#bundle " << bundle.time() << std::endl;

        // Get packet stream
        OSCPP::Server::PacketStream packets(bundle.packets());

        // Iterate over all the packets and call handlePacket recursively.
        // Cuidado: Might lead to stack overflow!
        while (!packets.atEnd()) {
            handlePacket(packets.next());
        }
    } else {
        // Convert to message
        OSCPP::Server::Message msg(packet);

        // Get argument stream
        OSCPP::Server::ArgStream args(msg.args());

        // Directly compare message address to string with operator==.
        // For handling larger address spaces you could use e.g. a
        // dispatch table based on std::unordered_map.
        if (msg == "/s_new") {
            const char* name = args.string();
            const int32_t id = args.int32();
            std::cout << "/s_new" << " "
                      << name << " "
                      << id << " ";
            // Get the params array as an ArgStream
            OSCPP::Server::ArgStream params(args.array());
            while (!params.atEnd()) {
                const char* param = params.string();
                const float value = params.float32();
                std::cout << param << ":" << value << " ";
            }
            std::cout << std::endl;
        } else if (msg == "/n_set") {
            const int32_t id = args.int32();
            const char* key = args.string();
            // Numeric arguments are converted automatically
            // to float32 (e.g. from int32).
            const float value = args.float32();
            std::cout << "/n_set" << " "
                      << id << " "
                      << key << " "
                      << value << std::endl;
        } else {
            // Simply print unknown messages
            std::cout << "Unknown message: " << msg << std::endl;
        }
    }
}
~~~~

Now we can receive data from a message based transport and pass it to our
packet handling function:

~~~~cpp
#include <array>

const size_t kMaxPacketSize = 8192;

size_t recv(Transport* t, void* buffer, size_t size);

void recvPacket(Transport* t)
{
    std::array<char,kMaxPacketSize> buffer;
    size_t size = recv(t, buffer.data(), buffer.size());
    handlePacket(OSCPP::Server::Packet(buffer.data(), size));
}
~~~~

Here's our code in an example main function:

~~~~cpp
#include <memory>
#include <stdexcept>

Transport* newTransport();

int main(int, char**)
{
    std::unique_ptr<Transport> t(newTransport());
    std::array<char,kMaxPacketSize> sendBuffer;
    try {
        sendPacket(t.get(), sendBuffer.data(), sendBuffer.size());
        recvPacket(t.get());
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
~~~~

Compiling and running the example produces the following output:

~~~~
#bundle 1234
/s_new sinesweep 2 start-freq:330 end-freq:990 amp:0.4
Unknown message: /n_free i:1
/n_set 1 wobble 31
~~~~

## How to run the example

You can build and run the example by executing

~~~~
make README
~~~~

You'll need to install the [Haskell Platform](http://www.haskell.org/platform/)
and the [Pandoc](http://johnmacfarlane.net/pandoc/) library:

~~~~
cabal install pandoc
~~~~

## Appendix: Support code

Here's the code for a trivial transport that has a single packet buffer:

~~~~cpp
#include <cstring>

class Transport
{
public:
    size_t send(const void* buffer, size_t size)
    {
        size_t n = std::min(m_buffer.size(), size);
        std::memcpy(m_buffer.data(), buffer, n);
        m_message = n;
        return n;
    }

    size_t recv(void* buffer, size_t size)
    {
        if (m_message > 0) {
            size_t n = std::min(m_message, size);
            std::memcpy(buffer, m_buffer.data(), n);
            m_message = 0;
            return n;
        }
        return 0;
    }

private:
    std::array<char,kMaxPacketSize> m_buffer;
    size_t m_message;
};

Transport* newTransport()
{
    return new Transport;
}

size_t send(Transport* t, const void* buffer, size_t size)
{
    return t->send(buffer, size);
}

size_t recv(Transport* t, void* buffer, size_t size)
{
    return t->recv(buffer, size);
}
~~~~
