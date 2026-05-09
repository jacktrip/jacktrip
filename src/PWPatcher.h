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
 * \file PWPatcher.h
 * \author Generated for JackTrip PipeWire support
 * \date 2026
 */

#ifndef __PWPATCHER_H__
#define __PWPATCHER_H__

#include "JackTrip.h"
#include <pipewire/pipewire.h>
#include <spa/param/props.h>
#include <QMutex>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QThread>
#include <map>
#include <vector>
#include <string>

struct PWPortInfo {
    uint32_t id;
    std::string name;
    uint32_t node_id;
    std::string node_name;
    std::string direction;  // "in" or "out"
    std::string channel;    // "FL", "FR", etc.
};

struct PWNodeInfo {
    uint32_t id;
    std::string name;
};

class PWPatcherWorker;

class PWPatcher : public QObject
{
    Q_OBJECT

   public:
    PWPatcher(QObject* parent = nullptr);
    virtual ~PWPatcher();

    void setPatchMode(JackTrip::hubConnectionModeT patchMode);
    void setStereoUpmix(bool upmix);

    void registerClient(const QString& clientName);
    void unregisterClient(const QString& clientName);

   private:
    friend class PWPatcherWorker;

    QStringList m_clients;
    QStringList m_monoClients;
    QMap<QString, QString> m_clientNameMap;      // Maps JackTrip name to PipeWire node name
    QMap<QString, QSet<uint32_t>> m_clientNodeIds;  // Maps JackTrip name to node IDs
    QSet<QPair<uint32_t, uint32_t>> m_activeConnections;  // Track active connections to avoid duplicates

    bool m_fan           = false;
    bool m_loop          = false;
    bool m_includeServer = true;
    bool m_stereoUpmix   = false;

    QMutex m_connectionMutex;

    PWPatcherWorker* m_worker = nullptr;
    QThread* m_workerThread = nullptr;

    void createConnection(uint32_t outPort, uint32_t inPort);
    void patchClient(const QString& clientName, const QSet<uint32_t>& clientNodeIds);
};

// Worker class to run PipeWire event loop in separate thread
class PWPatcherWorker : public QObject
{
    Q_OBJECT

   public:
    PWPatcherWorker(PWPatcher* patcher);
    ~PWPatcherWorker();

    bool initialize();
    void refreshPorts();
    void createLink(uint32_t outPort, uint32_t inPort);
    std::vector<PWPortInfo> getPortsForClient(const QString& clientName);

    // Static callbacks must be public for C-style callback structs
    static void registryEventGlobal(void* data, uint32_t id,
                                    uint32_t permissions,
                                    const char* type, uint32_t version,
                                    const struct spa_dict* props);
    static void registryEventGlobalRemove(void* data, uint32_t id);
    static void onCoreDone(void* data, uint32_t id, int seq);
    static void onCoreError(void* data, uint32_t id, int seq, int res, const char* message);

   public slots:
    void run();
    void stop();

   signals:
    void initialized();
    void finished();

   private:
    PWPatcher* m_patcher;
    struct pw_thread_loop* m_loop;
    struct pw_core* m_core;
    struct pw_context* m_context;
    struct pw_registry* m_registry;
    struct spa_hook m_registryListener;
    struct spa_hook m_coreListener;

    std::map<uint32_t, PWNodeInfo> m_nodes;
    std::map<uint32_t, PWPortInfo> m_ports;

    bool m_running;
    QMutex m_dataMutex;
};

#endif  // __PWPATCHER_H__
