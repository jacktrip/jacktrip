#include <oscpp/client.hpp>
#include <oscpp/print.hpp>
#include <oscpp/server.hpp>

#include <autocheck/autocheck.hpp>
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace OSCPP { namespace AST {
class Value
{
public:
    virtual ~Value()
    {}
    virtual void print(std::ostream& out) const = 0;
    virtual void put(OSCPP::Client::Packet& packet) const = 0;
};

template <class T> using List = std::list<std::shared_ptr<T>>;

template <class T> bool equalList(const List<T>& list1, const List<T>& list2)
{
    if (list1.size() != list2.size())
        return false;
    auto it1 = list1.begin();
    auto it2 = list2.begin();
    while ((it1 != list1.end()) && (it2 != list2.end()))
    {
        if (**it1 == **it2)
        {
            it1++;
            it2++;
        }
        else
        {
            return false;
        }
    }
    return true;
}

template <class T> void printList(std::ostream& out, const List<T>& list)
{
    const size_t n = list.size();
    size_t       i = 1;
    out << '[';
    for (auto x : list)
    {
        x->print(out);
        if (i != n)
        {
            out << ',';
        }
        i++;
    }
    out << ']';
}

class Argument : public Value
{
public:
    enum Type
    {
        kInt32,
        kFloat32,
        kString,
        kBlob,
        kArray,
    };
    static constexpr size_t kNumTypes = kArray + 1;

    Argument(Type type)
    : m_type(type)
    {}

    Type type() const
    {
        return m_type;
    }

    virtual size_t numTags() const
    {
        return 1;
    }

    static size_t numTags(const List<Argument>& args)
    {
        size_t n = 0;
        for (auto x : args)
            n += x->numTags();
        return n;
    }

    bool operator==(const Argument& other)
    {
        return (type() == other.type()) && equals(other);
    }

    virtual size_t size() const = 0;

protected:
    virtual bool equals(const Argument& other) const = 0;

private:
    Type m_type;
};

class Bundle;
class Message;

class Packet : public Value
{
public:
    enum Type
    {
        kMessage,
        kBundle
    };

    Packet(Type type)
    : m_type(type)
    {}

    Type type() const
    {
        return m_type;
    }

    virtual size_t size() const = 0;

    static std::shared_ptr<Packet> parse(const OSCPP::Server::Packet& packet)
    {
        return packet.isBundle() ? parseBundle(packet) : parseMessage(packet);
    }

    bool operator==(const Packet& other) const
    {
        return type() == other.type() && equals(other);
    }

protected:
    virtual bool equals(const Packet& other) const = 0;

private:
    static std::shared_ptr<Packet>
                parseBundle(const OSCPP::Server::Bundle& bdl);
    static void parseArgs(OSCPP::Server::ArgStream& inArgs,
                          List<Argument>&           outArgs);
    static std::shared_ptr<Packet>
    parseMessage(const OSCPP::Server::Message& msg);

    Type m_type;
};

class Bundle : public Packet
{
public:
    Bundle(uint64_t time, List<Packet> packets)
    : Packet(kBundle)
    , m_time(time)
    , m_packets(packets)
    {
        // assert(packets.size() > 0);
    }

    void print(std::ostream& out) const override
    {
        out << "Bundle(" << m_time << ", ";
        printList(out, m_packets);
        out << ')';
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.openBundle(m_time);
        for (auto p : m_packets)
            p->put(packet);
        packet.closeBundle();
    }

    size_t size() const override
    {
        size_t payload = 0;
        for (auto x : m_packets)
            payload += x->size();
        assert(OSCPP::isAligned(payload));
        return OSCPP::Size::bundle(m_packets.size()) + payload;
    }

protected:
    bool equals(const Packet& other) const override
    {
        const auto& otherBundle = dynamic_cast<const Bundle&>(other);
        return m_time == otherBundle.m_time &&
               equalList(m_packets, otherBundle.m_packets);
    }

private:
    uint64_t     m_time;
    List<Packet> m_packets;
};

class Message : public Packet
{
public:
    Message(std::string address, List<Argument> args)
    : Packet(kMessage)
    , m_address(address)
    , m_args(args)
    {}

    void print(std::ostream& out) const override
    {
        out << "Message(" << m_address << ", ";
        printList(out, m_args);
        out << ')';
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.openMessage(m_address.c_str(), Argument::numTags(m_args));
        for (auto x : m_args)
            x->put(packet);
        packet.closeMessage();
    }

    size_t size() const override
    {
        size_t payload = 0;
        for (auto x : m_args)
            payload += x->size();
        assert(OSCPP::isAligned(payload));
        return OSCPP::Size::message(OSCPP::Size::String(m_address.c_str()),
                                    Argument::numTags(m_args)) +
               payload;
    }

protected:
    bool equals(const Packet& other) const override
    {
        const auto& otherMsg = dynamic_cast<const Message&>(other);
        return m_address == otherMsg.m_address &&
               equalList(m_args, otherMsg.m_args);
    }

private:
    std::string    m_address;
    List<Argument> m_args;
};

class Int32 : public Argument
{
public:
    Int32(int32_t value)
    : Argument(kInt32)
    , m_value(value)
    {}

    void print(std::ostream& out) const override
    {
        out << "i:" << m_value;
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.put(m_value);
    }

    size_t size() const override
    {
        return OSCPP::Size::int32();
    }

protected:
    bool equals(const Argument& other) const override
    {
        return dynamic_cast<const Int32&>(other).m_value == m_value;
    }

private:
    int32_t m_value;
};

class Float32 : public Argument
{
public:
    Float32(float value)
    : Argument(kFloat32)
    , m_value(value)
    {}

    void print(std::ostream& out) const override
    {
        out << "f:" << m_value;
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.put(m_value);
    }

    size_t size() const override
    {
        return OSCPP::Size::float32();
    }

protected:
    bool equals(const Argument& other) const override
    {
        return dynamic_cast<const Float32&>(other).m_value == m_value;
    }

private:
    float m_value;
};

class String : public Argument
{
public:
    String(std::string value)
    : Argument(kString)
    , m_value(value)
    {}

    void print(std::ostream& out) const override
    {
        out << "s:" << m_value;
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.put(m_value.c_str());
    }

    size_t size() const override
    {
        return OSCPP::Size::string(OSCPP::Size::String(m_value.c_str()));
    }

protected:
    bool equals(const Argument& other) const override
    {
        return dynamic_cast<const String&>(other).m_value == m_value;
    }

private:
    std::string m_value;
};

class Blob : public Argument
{
public:
    Blob(int32_t size, const void* data = nullptr)
    : Argument(kBlob)
    , m_size(std::max(0, size))
    , m_data(nullptr)
    {
        if (m_size > 0)
        {
            m_data = new char[m_size];
            if (data != nullptr)
                std::memcpy(m_data, data, m_size);
        }
    }

    Blob(OSCPP::Blob b)
    : Blob(static_cast<int32_t>(b.size()), b.data())
    {}

    ~Blob()
    {
        delete[] m_data;
    }

    void print(std::ostream& out) const override
    {
        out << "b:" << m_size;
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.put(OSCPP::Blob(m_data, m_size));
    }

    size_t size() const override
    {
        return OSCPP::Size::blob(m_size);
    }

protected:
    bool equals(const Argument& other) const override
    {
        const Blob& otherBlob = dynamic_cast<const Blob&>(other);
        return otherBlob.m_size == m_size &&
               memcmp(m_data, otherBlob.m_data, m_size) == 0;
    }

private:
    size_t m_size;
    char*  m_data;
};

class Array : public Argument
{
public:
    Array(List<Argument> elems = List<Argument>())
    : Argument(kArray)
    , m_elems(elems)
    {}

    void print(std::ostream& out) const override
    {
        printList(out, m_elems);
    }

    void put(OSCPP::Client::Packet& packet) const override
    {
        packet.openArray();
        for (auto x : m_elems)
            x->put(packet);
        packet.closeArray();
    }

    size_t size() const override
    {
        size_t payload = 0;
        for (auto x : m_elems)
            payload += x->size();
        assert(OSCPP::isAligned(payload));
        return payload;
    }

    size_t numTags() const override
    {
        return OSCPP::Tags::array(Argument::numTags(m_elems));
    }

protected:
    bool equals(const Argument& other) const override
    {
        return equalList(m_elems, dynamic_cast<const Array&>(other).m_elems);
    }

private:
    List<Argument> m_elems;
};

std::shared_ptr<Packet> Packet::parseBundle(const OSCPP::Server::Bundle& bdl)
{
    List<Packet>                outPackets;
    OSCPP::Server::PacketStream inPackets(bdl.packets());
    while (!inPackets.atEnd())
    {
        outPackets.push_back(parse(inPackets.next()));
    }
    return std::make_shared<Bundle>(bdl.time(), std::move(outPackets));
}

void Packet::parseArgs(OSCPP::Server::ArgStream& inArgs,
                       List<Argument>&           outArgs)
{
    while (!inArgs.atEnd())
    {
        switch (inArgs.tag())
        {
            case 'i':
                outArgs.push_back(std::make_shared<Int32>(inArgs.int32()));
                break;
            case 'f':
                outArgs.push_back(std::make_shared<Float32>(inArgs.float32()));
                break;
            case 's':
                outArgs.push_back(std::make_shared<String>(inArgs.string()));
                break;
            case 'b':
                outArgs.push_back(std::make_shared<Blob>(inArgs.blob()));
                break;
            case '[':
            {
                OSCPP::Server::ArgStream inElems(inArgs.array());
                List<Argument>           outElems;
                parseArgs(inElems, outElems);
                outArgs.push_back(std::make_shared<Array>(outElems));
            }
            break;
        }
    }
}

std::shared_ptr<Packet> Packet::parseMessage(const OSCPP::Server::Message& msg)
{
    OSCPP::Server::ArgStream inArgs(msg.args());
    List<Argument>           outArgs;
    parseArgs(inArgs, outArgs);
    return std::make_shared<Message>(msg.address(), outArgs);
}

std::ostream& operator<<(std::ostream& out, const Packet& packet)
{
    packet.print(out);
    return out;
}

std::ostream& operator<<(std::ostream&                  out,
                         const std::shared_ptr<Packet>& packet)
{
    packet->print(out);
    return out;
}
}} // namespace OSCPP::AST

namespace ac = autocheck;

namespace OSCPP { namespace AutoCheck {

struct MessageArgListGen
{
    typedef AST::List<AST::Argument> result_type;
    result_type                      operator()(size_t size) const;
};

struct MessageArgGen
{
    typedef std::shared_ptr<AST::Argument> result_type;
    result_type                            operator()(size_t size) const
    {
        AST::Argument::Type argType = static_cast<AST::Argument::Type>(
            ac::generator<size_t>()(AST::Argument::kNumTypes - 1));
        switch (argType)
        {
            case AST::Argument::kInt32:
                return std::make_shared<AST::Int32>(
                    ac::generator<int32_t>()(size));
            case AST::Argument::kFloat32:
                return std::make_shared<AST::Float32>(
                    ac::generator<float>()(size));
            case AST::Argument::kString:
                return std::make_shared<AST::String>(
                    ac::string<ac::ccPrintable>()(std::max<size_t>(1, size)));
            case AST::Argument::kBlob:
                return std::make_shared<AST::Blob>(
                    ac::generator<int32_t>()(size));
            case AST::Argument::kArray:
                // Exponential size backoff
                return std::make_shared<AST::Array>(
                    MessageArgListGen()(size / 2));
            default:
                throw std::logic_error("Invalid AST::Argument::Type value");
        }
        const bool InvalidArgumentType = false;
        assert(InvalidArgumentType);
    }
};

MessageArgListGen::result_type MessageArgListGen::operator()(size_t size) const
{
    const auto& elems = ac::list_of(MessageArgGen())(size);
    return AST::List<AST::Argument>(elems.begin(), elems.end());
}

struct PacketGen
{
    // ac::generator<std::shared_ptr<oscpp::AST::Packet>> source;
    typedef std::shared_ptr<AST::Packet> result_type;
    result_type                          operator()(size_t size) const
    {
        return ac::generator<bool>()(size) ? gen_bundle(size)
                                           : gen_message(size);
    }

    result_type gen_bundle(size_t size) const
    {
        const auto& packets = ac::list_of(PacketGen())(size / 2);
        return std::make_shared<AST::Bundle>(
            ac::generator<uint64_t>()(size),
            AST::List<AST::Packet>(packets.begin(), packets.end()));
    }

    std::string gen_message_address(size_t size) const
    {
        std::string result(
            ac::string<ac::ccAlphaNumeric>()(std::max<size_t>(2, size)));
        if (result[0] != '/')
            result[0] = '/';
        return result;
    }

    result_type gen_message(size_t size) const
    {
        return std::make_shared<AST::Message>(gen_message_address(size),
                                              MessageArgListGen()(size));
    }
};
}} // namespace OSCPP::AutoCheck

bool prop_identity(const std::shared_ptr<OSCPP::AST::Packet>& packet1)
{
    // packet1->print(std::cerr); std::cerr << "\n";
    const size_t            size = packet1->size();
    std::unique_ptr<char[]> data(new char[size]);
    OSCPP::Client::Packet   clientPacket(data.get(), size);
    packet1->put(clientPacket);
    OSCPP::Server::Packet serverPacket(clientPacket.data(),
                                       clientPacket.size());
    auto                  packet2 = OSCPP::AST::Packet::parse(serverPacket);
    using namespace OSCPP::AST;
    if (!(*packet1 == *packet2))
    {
        std::cerr << packet1 << std::endl;
        std::cerr << packet2 << std::endl;
        return false;
    }
    return true;
}

bool prop_overflow(const std::shared_ptr<OSCPP::AST::Packet>& packet,
                   size_t                                     inBufferSize)
{
    const size_t packetSize = packet->size();
    const size_t bufferSize =
        inBufferSize == 0
            ? 1
            : (inBufferSize < packetSize ? inBufferSize : packetSize - 1);
    std::cerr << "bufferSize " << bufferSize << std::endl;
    std::unique_ptr<char[]> data(new char[bufferSize]);
    OSCPP::Client::Packet   clientPacket(data.get(), bufferSize);
    bool                    result = false;
    try
    {
        packet->put(clientPacket);
    }
    catch (OSCPP::OverflowError&)
    {
        result = true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return result;
}

int main(int argc, char** argv)
{
    using namespace OSCPP::AST;
    using namespace OSCPP::AutoCheck;
    ac::check<std::shared_ptr<Packet>>(prop_identity, 150,
                                       ac::make_arbitrary(PacketGen()));
    // ac::check<std::shared_ptr<Packet>,size_t>(
    //     prop_overflow,
    //     150,
    //     ac::make_arbitrary(PacketGen(), ac::generator<size_t>())
    // );
    return 0;
}
