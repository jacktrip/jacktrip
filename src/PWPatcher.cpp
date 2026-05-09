//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
SoundWIRE group at CCRMA, Stanford University.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file PWPatcher.cpp
 * \author Generated for JackTrip PipeWire support
 * \date 2026
 */

#include "PWPatcher.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// Registry event callbacks
static const struct pw_registry_events registry_events = {
    .version = PW_VERSION_REGISTRY_EVENTS,
    .global = PWPatcherWorker::registryEventGlobal,
    .global_remove = PWPatcherWorker::registryEventGlobalRemove,
};

static const struct pw_core_events core_events = {
    .version = PW_VERSION_CORE_EVENTS,
    .info = nullptr,
    .done = PWPatcherWorker::onCoreDone,
    .ping = nullptr,
    .error = PWPatcherWorker::onCoreError,
    .remove_id = nullptr,
    .bound_id = nullptr,
    .add_mem = nullptr,
    .remove_mem = nullptr,
    .bound_props = nullptr,
};

static int getChannelFromPortName(const std::string& portName)
{
    // For native PipeWire streams, extract channel number
    // Port names look like: "input_FL", "output_FR", etc.
    size_t underscorePos = portName.find_last_of('_');
    if (underscorePos != std::string::npos && underscorePos + 1 < portName.length()) {
        std::string suffix = portName.substr(underscorePos + 1);

        // Handle standard channel names
        if (suffix == "FL" || suffix == "L") return 1;
        if (suffix == "FR" || suffix == "R") return 2;

        // Handle numeric channels
        try {
            return std::stoi(suffix);
        } catch (...) {
            return -1;
        }
    }
    return -1;
}

static bool portsMatch(const PWPortInfo& port1, const PWPortInfo& port2)
{
    // First try to match by channel property
    if (!port1.channel.empty() && !port2.channel.empty()) {
        return port1.channel == port2.channel;
    }

            // Fall back to channel number from port name
    int chan1 = getChannelFromPortName(port1.name);
    int chan2 = getChannelFromPortName(port2.name);

    if (chan1 > 0 && chan2 > 0) {
        return chan1 == chan2;
    }

    return false;
}

static QString getBaseClientName(const QString& clientName)
{
    // Remove _send or _receive suffix from node names
    QString baseName = clientName;
    if (baseName.endsWith("_send") || baseName.endsWith("_receive")) {
        int underscorePos = baseName.lastIndexOf('_');
        if (underscorePos > 0) {
            baseName = baseName.left(underscorePos);
        }
    }

    // Handle numbered clients (e.g., "JackTrip-1" -> "JackTrip")
    int dashPos = baseName.lastIndexOf('-');
    if (dashPos > 0) {
        bool isNumber;
        baseName.mid(dashPos + 1).toInt(&isNumber);
        if (isNumber) {
            return baseName.left(dashPos);
        }
    }
    return baseName;
}

PWPatcher::PWPatcher(QObject* parent) : QObject(parent)
{
    std::cerr << "PWPatcher: Initializing for native PipeWire JackTrip" << std::endl;

    m_workerThread = new QThread(this);
    m_worker = new PWPatcherWorker(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &PWPatcherWorker::run);
    connect(m_worker, &PWPatcherWorker::finished, m_workerThread, &QThread::quit);

    m_workerThread->start();
    QThread::msleep(500);
}

PWPatcher::~PWPatcher()
{
    if (m_worker) {
        m_worker->stop();
    }
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(1000);
    }
}

void PWPatcher::setPatchMode(JackTrip::hubConnectionModeT patchMode)
{
    QMutexLocker locker(&m_connectionMutex);
    m_fan = patchMode == JackTrip::CLIENTFOFI || patchMode == JackTrip::FULLMIX
            || patchMode == JackTrip::SERVFOFI || patchMode == JackTrip::SERVFULLMIX;
    m_loop = patchMode == JackTrip::CLIENTECHO || patchMode == JackTrip::FULLMIX
             || patchMode == JackTrip::SERVFULLMIX;
    m_includeServer = patchMode == JackTrip::SERVERTOCLIENT
                      || patchMode == JackTrip::SERVFOFI
                      || patchMode == JackTrip::SERVFULLMIX;

    std::cerr << "PWPatcher: Mode set - fan=" << m_fan << " loop=" << m_loop
              << " includeServer=" << m_includeServer << std::endl;
}

void PWPatcher::setStereoUpmix(bool upmix)
{
    m_stereoUpmix = upmix;
    std::cerr << "PWPatcher: Stereo upmix=" << upmix << std::endl;
}

void PWPatcher::registerClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);

    if (!m_worker) {
        std::cerr << "PWPatcher: Worker not initialized" << std::endl;
        return;
    }

    std::cerr << "PWPatcher: Registering client '" << clientName.toStdString() << "'" << std::endl;

            // For native PipeWire, the clientName is the assigned name (e.g., "192.168.1.137")
            // We need to find nodes with names like "192.168.1.137_send" and "192.168.1.137_receive"
    QString baseName = clientName;
    std::vector<PWPortInfo> ports;
    int retries = 0;

    while (retries < 10) {
        m_worker->refreshPorts();
        QThread::msleep(100);

        // Look for nodes matching this client (both _send and _receive)
        ports = m_worker->getPortsForClient(baseName);

        if (!ports.empty()) {
            std::cerr << "PWPatcher: Found " << ports.size() << " ports for '"
                      << baseName.toStdString() << "'" << std::endl;
            break;
        }
        retries++;
    }

    if (ports.empty()) {
        std::cerr << "PWPatcher: No ports found for '" << clientName.toStdString()
        << "' after " << retries << " retries" << std::endl;
        return;
    }

            // Determine which node IDs belong to this client
    QSet<uint32_t> existingNodeIds;
    for (const QString& existingClient : m_clients) {
        existingNodeIds.unite(m_clientNodeIds.value(existingClient));
    }

    std::vector<PWPortInfo> clientOutPorts, clientInPorts;
    QSet<uint32_t> clientNodeIds;

    for (const auto& port : ports) {
        if (!existingNodeIds.contains(port.node_id)) {
            clientNodeIds.insert(port.node_id);
            std::cerr << "  Node ID " << port.node_id << ": " << port.node_name
                      << " port: " << port.name << " dir: " << port.direction << std::endl;
        }

        // Categorize ports
        if (port.direction == "out" && port.name.find("broadcast") == std::string::npos) {
            clientOutPorts.push_back(port);
        } else if (port.direction == "in") {
            clientInPorts.push_back(port);
        }
    }

    if (clientNodeIds.isEmpty()) {
        std::cerr << "PWPatcher: No new nodes for '" << clientName.toStdString() << "'" << std::endl;
        return;
    }

    auto portComparator = [](const PWPortInfo& a, const PWPortInfo& b) {
        return getChannelFromPortName(a.name) < getChannelFromPortName(b.name);
    };
    std::sort(clientOutPorts.begin(), clientOutPorts.end(), portComparator);
    std::sort(clientInPorts.begin(), clientInPorts.end(), portComparator);

    bool clientIsMono = (clientOutPorts.size() == 1);
    std::cerr << "PWPatcher: Client has " << clientOutPorts.size() << " outputs, "
              << clientInPorts.size() << " inputs (mono=" << clientIsMono << ")" << std::endl;

    if (m_includeServer && clientIsMono && m_stereoUpmix) {
        std::vector<PWPortInfo> allPorts = m_worker->getPortsForClient(QString(""));
        for (const auto& port : allPorts) {
            if (port.direction == "in" && port.channel == "FR" &&
                (port.node_name.find("alsa_output") != std::string::npos ||
                 port.node_name.find("Built-in") != std::string::npos)) {
                if (!clientOutPorts.empty()) {
                    std::cerr << "PWPatcher: Connecting to system output (upmix)" << std::endl;
                    createConnection(clientOutPorts[0].id, port.id);
                }
            }
        }
    }

    m_clients.append(clientName);
    m_clientNameMap[clientName] = baseName;
    m_clientNodeIds[clientName] = clientNodeIds;
    if (clientIsMono) {
        m_monoClients.append(clientName);
    }

    std::cerr << "PWPatcher: Client registered, now patching..." << std::endl;
    patchClient(clientName, clientNodeIds);
}

void PWPatcher::patchClient(const QString& clientName, const QSet<uint32_t>& clientNodeIds)
{
    QString clientBaseName = m_clientNameMap.value(clientName, getBaseClientName(clientName));
    std::vector<PWPortInfo> clientPorts = m_worker->getPortsForClient(clientBaseName);
    std::vector<PWPortInfo> clientOutPorts, clientInPorts;

    std::cerr << "PWPatcher: Patching client '" << clientName.toStdString() << "'" << std::endl;

    for (const auto& port : clientPorts) {
        if (!clientNodeIds.contains(port.node_id)) continue;
        if (port.direction == "out" && port.name.find("broadcast") == std::string::npos) {
            clientOutPorts.push_back(port);
        } else if (port.direction == "in") {
            clientInPorts.push_back(port);
        }
    }

    bool clientIsMono = (clientOutPorts.size() == 1);

    int connectionsCreated = 0;

    if (m_fan || m_loop) {
        for (const auto& outPort : clientOutPorts) {
            for (const QString& otherClient : m_clients) {
                QSet<uint32_t> otherNodeIds = m_clientNodeIds.value(otherClient);
                bool isSelfConnection = clientNodeIds.intersects(otherNodeIds);

                if (!m_fan && !isSelfConnection) continue;
                if (!m_loop && isSelfConnection) continue;

                QString otherBaseName = m_clientNameMap.value(otherClient, getBaseClientName(otherClient));
                std::vector<PWPortInfo> otherPorts = m_worker->getPortsForClient(otherBaseName);

                for (const auto& inPort : otherPorts) {
                    if (!otherNodeIds.contains(inPort.node_id)) continue;
                    if (inPort.direction == "in") {
                        if (portsMatch(outPort, inPort)) {
                            createConnection(outPort.id, inPort.id);
                            connectionsCreated++;
                        } else if (m_stereoUpmix && clientIsMono) {
                            int inChan = getChannelFromPortName(inPort.name);
                            if (inChan == 2 || inPort.channel == "FR") {
                                createConnection(outPort.id, inPort.id);
                                connectionsCreated++;
                            }
                        }
                    }
                }
            }
        }

        if (m_fan) {
            for (const auto& inPort : clientInPorts) {
                for (const QString& otherClient : m_clients) {
                    QSet<uint32_t> otherNodeIds = m_clientNodeIds.value(otherClient);
                    if (clientNodeIds.intersects(otherNodeIds)) continue;

                    bool otherIsMono = m_monoClients.contains(otherClient);
                    QString otherBaseName = m_clientNameMap.value(otherClient, getBaseClientName(otherClient));
                    std::vector<PWPortInfo> otherPorts = m_worker->getPortsForClient(otherBaseName);

                    for (const auto& outPort : otherPorts) {
                        if (!otherNodeIds.contains(outPort.node_id)) continue;
                        if (outPort.direction == "out" && outPort.name.find("broadcast") == std::string::npos) {
                            if (portsMatch(outPort, inPort)) {
                                createConnection(outPort.id, inPort.id);
                                connectionsCreated++;
                            } else if (m_stereoUpmix && otherIsMono) {
                                int inChan = getChannelFromPortName(inPort.name);
                                if (inChan == 2 || inPort.channel == "FR") {
                                    createConnection(outPort.id, inPort.id);
                                    connectionsCreated++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::cerr << "PWPatcher: Created " << connectionsCreated << " connections for '"
              << clientName.toStdString() << "'" << std::endl;
}

void PWPatcher::createConnection(uint32_t outPort, uint32_t inPort)
{
    QPair<uint32_t, uint32_t> connection(outPort, inPort);
    if (m_activeConnections.contains(connection)) {
        return;
    }

    if (m_worker) {
        std::cerr << "PWPatcher: Creating link: port " << outPort << " -> " << inPort << std::endl;
        m_worker->createLink(outPort, inPort);
        m_activeConnections.insert(connection);
    }
}

void PWPatcher::unregisterClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    std::cerr << "PWPatcher: Unregistering client '" << clientName.toStdString() << "'" << std::endl;
    m_clients.removeAll(clientName);
    m_clientNameMap.remove(clientName);
    m_clientNodeIds.remove(clientName);
    m_monoClients.removeAll(clientName);
}

PWPatcherWorker::PWPatcherWorker(PWPatcher* patcher)
    : m_patcher(patcher), m_loop(nullptr), m_core(nullptr),
    m_context(nullptr), m_registry(nullptr), m_running(false)
{
}

PWPatcherWorker::~PWPatcherWorker()
{
    stop();
}

bool PWPatcherWorker::initialize()
{
    int argc = 0;
    char** argv = nullptr;
    pw_init(&argc, &argv);

    m_loop = pw_thread_loop_new("pwpatcher", nullptr);
    if (!m_loop) {
        std::cerr << "PWPatcher: Failed to create thread loop" << std::endl;
        return false;
    }

    m_context = pw_context_new(pw_thread_loop_get_loop(m_loop), nullptr, 0);
    if (!m_context) {
        std::cerr << "PWPatcher: Failed to create context" << std::endl;
        pw_thread_loop_destroy(m_loop);
        return false;
    }

    pw_thread_loop_lock(m_loop);

    m_core = pw_context_connect(m_context, nullptr, 0);
    if (!m_core) {
        std::cerr << "PWPatcher: Failed to connect to PipeWire" << std::endl;
        pw_thread_loop_unlock(m_loop);
        pw_context_destroy(m_context);
        pw_thread_loop_destroy(m_loop);
        return false;
    }

    pw_core_add_listener(m_core, &m_coreListener, &core_events, this);
    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(m_registry, &m_registryListener, &registry_events, this);

    pw_core_sync(m_core, 0, 0);

    pw_thread_loop_unlock(m_loop);

    if (pw_thread_loop_start(m_loop) < 0) {
        std::cerr << "PWPatcher: Failed to start thread loop" << std::endl;
        return false;
    }

    m_running = true;
    QThread::msleep(300);

    std::cerr << "PWPatcher: Worker initialized successfully" << std::endl;
    return true;
}

void PWPatcherWorker::run()
{
    if (initialize()) {
        emit initialized();
    }
}

void PWPatcherWorker::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_loop) {
        pw_thread_loop_stop(m_loop);
    }

    if (m_registry) {
        pw_proxy_destroy((struct pw_proxy*)m_registry);
        m_registry = nullptr;
    }

    if (m_core) {
        pw_core_disconnect(m_core);
        m_core = nullptr;
    }

    if (m_context) {
        pw_context_destroy(m_context);
        m_context = nullptr;
    }

    if (m_loop) {
        pw_thread_loop_destroy(m_loop);
        m_loop = nullptr;
    }

    pw_deinit();
    emit finished();
}

void PWPatcherWorker::refreshPorts()
{
    if (!m_core || !m_running) {
        return;
    }

    pw_thread_loop_lock(m_loop);
    pw_core_sync(m_core, 0, 0);
    pw_thread_loop_unlock(m_loop);
}

void PWPatcherWorker::createLink(uint32_t outPort, uint32_t inPort)
{
    if (!m_core || !m_running) {
        return;
    }

    pw_thread_loop_lock(m_loop);

    struct pw_properties* props = pw_properties_new(
        PW_KEY_LINK_OUTPUT_PORT, std::to_string(outPort).c_str(),
        PW_KEY_LINK_INPUT_PORT, std::to_string(inPort).c_str(),
        PW_KEY_LINK_PASSIVE, "true",
        nullptr
        );

    struct pw_proxy* link_proxy = static_cast<struct pw_proxy*>(
        pw_core_create_object(m_core, "link-factory",
                              PW_TYPE_INTERFACE_Link, PW_VERSION_LINK,
                              &props->dict, 0)
        );

    pw_properties_free(props);
    pw_thread_loop_unlock(m_loop);

    (void)link_proxy;
}

std::vector<PWPortInfo> PWPatcherWorker::getPortsForClient(const QString& clientName)
{
    QMutexLocker locker(&m_dataMutex);
    std::vector<PWPortInfo> result;

    std::string clientNameStr = clientName.toStdString();

    for (const auto& portPair : m_ports) {
        const PWPortInfo& port = portPair.second;

        // If empty clientName, return all ports
        if (clientName.isEmpty()) {
            result.push_back(port);
            continue;
        }

        // For native PipeWire JackTrip, match node names that contain the client name
        // Node names will be like "192.168.1.137_send" or "192.168.1.137_receive"
        if (port.node_name.find(clientNameStr) != std::string::npos) {
            result.push_back(port);
        }
    }

    return result;
}

void PWPatcherWorker::registryEventGlobal(void* data, uint32_t id,
                                          uint32_t /*permissions*/,
                                          const char* type, uint32_t /*version*/,
                                          const struct spa_dict* props)
{
    PWPatcherWorker* worker = static_cast<PWPatcherWorker*>(data);
    QMutexLocker locker(&worker->m_dataMutex);

    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        const char* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        if (node_name) {
            PWNodeInfo nodeInfo;
            nodeInfo.id = id;
            nodeInfo.name = node_name;
            worker->m_nodes[id] = nodeInfo;

            // Debug output for JackTrip nodes
            std::string name_str(node_name);
            if (name_str.find("_send") != std::string::npos ||
                name_str.find("_receive") != std::string::npos) {
                std::cerr << "PWPatcher: Found JackTrip node " << id << ": " << node_name << std::endl;
            }
        }
    }
    else if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
        PWPortInfo portInfo;
        portInfo.id = id;
        portInfo.node_id = 0;

        const char* port_name = spa_dict_lookup(props, PW_KEY_PORT_NAME);
        const char* node_id_str = spa_dict_lookup(props, PW_KEY_NODE_ID);
        const char* direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
        const char* channel = spa_dict_lookup(props, PW_KEY_AUDIO_CHANNEL);

        if (port_name) portInfo.name = port_name;
        if (direction) portInfo.direction = direction;
        if (channel) portInfo.channel = channel;

        if (node_id_str) {
            portInfo.node_id = std::stoi(node_id_str);
            if (worker->m_nodes.count(portInfo.node_id)) {
                portInfo.node_name = worker->m_nodes[portInfo.node_id].name;
            }
        }

        worker->m_ports[id] = portInfo;
    }
}

void PWPatcherWorker::registryEventGlobalRemove(void* data, uint32_t id)
{
    PWPatcherWorker* worker = static_cast<PWPatcherWorker*>(data);
    QMutexLocker locker(&worker->m_dataMutex);

    worker->m_nodes.erase(id);
    worker->m_ports.erase(id);
}

void PWPatcherWorker::onCoreDone(void* /*data*/, uint32_t /*id*/, int /*seq*/)
{
   // Core sync completed
}

void PWPatcherWorker::onCoreError(void* /*data*/, uint32_t /*id*/, int /*seq*/, int /*res*/, const char* message)
{
    std::cerr << "PWPatcher core error: " << message << std::endl;
}
