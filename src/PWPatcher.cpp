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

//==============================================================================
// PWPatcher Implementation
//==============================================================================

PWPatcher::PWPatcher(QObject* parent) : QObject(parent)
{
    m_workerThread = new QThread(this);
    m_worker = new PWPatcherWorker(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &PWPatcherWorker::run);
    connect(m_worker, &PWPatcherWorker::finished, m_workerThread, &QThread::quit);

    m_workerThread->start();

    // Wait for initialization
    QThread::msleep(200);
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
}

void PWPatcher::setStereoUpmix(bool upmix)
{
    m_stereoUpmix = upmix;
}

void PWPatcher::registerClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);

    if (!m_worker) {
        std::cerr << "PWPatcher: Worker not initialized, patching disabled" << std::endl;
        return;
    }

    // Timing workaround similar to JACK/PipeWire issue
    QThread::msleep(100);

    // Get all ports for this client
    std::vector<PWPortInfo> ports = m_worker->getPortsForClient(clientName);

    std::vector<PWPortInfo> clientOutPorts;
    std::vector<PWPortInfo> clientInPorts;

    for (const auto& port : ports) {
        if (port.direction == "out" && port.name.find("broadcast") == std::string::npos) {
            clientOutPorts.push_back(port);
        } else if (port.direction == "in") {
            clientInPorts.push_back(port);
        }
    }

    bool clientIsMono = (clientOutPorts.size() == 1);

    // Handle server-to-client mode stereo upmix
    if (m_includeServer && clientIsMono && m_stereoUpmix) {
        // Get system playback ports
        std::vector<PWPortInfo> allPorts = m_worker->getPortsForClient(QString(""));
        std::vector<PWPortInfo> systemInPorts;

        for (const auto& port : allPorts) {
            if (port.direction == "in" &&
                (port.node_name.find("alsa_output") != std::string::npos ||
                 port.node_name.find("Built-in") != std::string::npos)) {
                systemInPorts.push_back(port);
            }
        }

        if (systemInPorts.size() >= 2 && clientOutPorts.size() > 0) {
            // Connect mono out to right speaker (left should already be connected)
            for (const auto& sysPort : systemInPorts) {
                if (sysPort.channel == "FR") {
                    createConnection(clientOutPorts[0].id, sysPort.id);
                }
            }
        }
    }

    // Perform patching based on mode
    patchClient(clientName);

    m_clients.append(clientName);
    if (clientIsMono) {
        m_monoClients.append(clientName);
    }
}

void PWPatcher::patchClient(const QString& clientName)
{
    std::vector<PWPortInfo> clientPorts = m_worker->getPortsForClient(clientName);
    std::vector<PWPortInfo> clientOutPorts;
    std::vector<PWPortInfo> clientInPorts;

    for (const auto& port : clientPorts) {
        if (port.direction == "out" && port.name.find("broadcast") == std::string::npos) {
            clientOutPorts.push_back(port);
        } else if (port.direction == "in") {
            clientInPorts.push_back(port);
        }
    }

    bool clientIsMono = (clientOutPorts.size() == 1);

    if (m_fan || m_loop) {
        // Connect client output ports to other clients/self
        for (const auto& outPort : clientOutPorts) {
            // Get other clients' ports
            for (const QString& otherClient : m_clients) {
                if (!m_fan && otherClient != clientName) {
                    continue;  // Skip if not fan mode and not loopback
                }
                if (!m_loop && otherClient == clientName) {
                    continue;  // Skip if not loop mode and is loopback
                }

                std::vector<PWPortInfo> otherPorts = m_worker->getPortsForClient(otherClient);

                for (const auto& inPort : otherPorts) {
                    if (inPort.direction == "in") {
                        // Match channels
                        if (outPort.channel == inPort.channel) {
                            createConnection(outPort.id, inPort.id);
                        } else if (m_stereoUpmix && clientIsMono && inPort.channel == "FR") {
                            // Stereo upmix: connect mono out to right channel too
                            createConnection(outPort.id, inPort.id);
                        }
                    }
                }
            }
        }

        // Connect other clients' outputs to this client's inputs (fan mode only)
        if (m_fan) {
            for (const auto& inPort : clientInPorts) {
                for (const QString& otherClient : m_clients) {
                    if (otherClient == clientName) {
                        continue;  // Already handled in loop above
                    }

                    bool otherIsMono = m_monoClients.contains(otherClient);
                    std::vector<PWPortInfo> otherPorts = m_worker->getPortsForClient(otherClient);

                    for (const auto& outPort : otherPorts) {
                        if (outPort.direction == "out" &&
                            outPort.name.find("broadcast") == std::string::npos) {
                            // Match channels
                            if (outPort.channel == inPort.channel) {
                                createConnection(outPort.id, inPort.id);
                            } else if (m_stereoUpmix && inPort.channel == "FR" && otherIsMono) {
                                // Stereo upmix from mono source
                                createConnection(outPort.id, inPort.id);
                            }
                        }
                    }
                }
            }
        }
    }
}

void PWPatcher::createConnection(uint32_t outPort, uint32_t inPort)
{
    if (m_worker) {
        m_worker->createLink(outPort, inPort);
    }
}

void PWPatcher::unregisterClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    m_clients.removeAll(clientName);
    m_monoClients.removeAll(clientName);
}

//==============================================================================
// PWPatcherWorker Implementation
//==============================================================================

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

    pw_thread_loop_unlock(m_loop);

    if (pw_thread_loop_start(m_loop) < 0) {
        std::cerr << "PWPatcher: Failed to start thread loop" << std::endl;
        return false;
    }

    m_running = true;
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

void PWPatcherWorker::createLink(uint32_t outPort, uint32_t inPort)
{
    if (!m_core || !m_running) {
        return;
    }

    pw_thread_loop_lock(m_loop);

    struct pw_properties* props = pw_properties_new(
        PW_KEY_LINK_OUTPUT_PORT, std::to_string(outPort).c_str(),
        PW_KEY_LINK_INPUT_PORT, std::to_string(inPort).c_str(),
        nullptr
        );

    struct pw_proxy* link_proxy = static_cast<struct pw_proxy*>(
        pw_core_create_object(m_core, "link-factory",
                              PW_TYPE_INTERFACE_Link, PW_VERSION_LINK,
                              &props->dict, 0)
        );

    pw_properties_free(props);
    pw_thread_loop_unlock(m_loop);

    if (!link_proxy) {
        std::cerr << "PWPatcher: Failed to create link "
                  << outPort << " -> " << inPort << std::endl;
    }
}

std::vector<PWPortInfo> PWPatcherWorker::getPortsForClient(const QString& clientName)
{
    QMutexLocker locker(&m_dataMutex);
    std::vector<PWPortInfo> result;

    std::string clientNameStr = clientName.toStdString();

    for (const auto& portPair : m_ports) {
        const PWPortInfo& port = portPair.second;
        if (clientName.isEmpty() || port.node_name.find(clientNameStr) != std::string::npos) {
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
        }
    }
    else if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
        PWPortInfo portInfo;
        portInfo.id = id;

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
