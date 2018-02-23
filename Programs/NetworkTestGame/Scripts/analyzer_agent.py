import sys
import json
import psutil

NET_IO_FILENAME = '/tmp/net_io_counters'

response = {
    'metrics': {},
    'traffic': {
        'send': {'bytes': {}, 'packets': {}},
        'recv': {'bytes': {}, 'packets': {}}
    },
    'system': {
        'mem': [],
        'traffic': {
            'send': [],
            'recv': []
        }
    },
    'snapshotstat': {
        'generalstat': [],
        'componentstat': [],
        'typestat': [],
    },
}

def noexcept(fn):
    def wrapped(*args, **kwargs):
        try:
            fn(*args, **kwargs)
        except Exception:
            pass
    return wrapped

def append_metric(name, value, dest):
    if name in dest:
        dest[name].append(value)
    else:
        dest[name] = [value]

@noexcept
def load_one_ts_metric(ts_metric):
    [name, value] = ts_metric.strip().split('=')
    value = int(value)
    append_metric(name, value, response['metrics'])

@noexcept
def load_ts_metrics(line):
    for ts_metric in line.split(':')[1].split(','):
        load_one_ts_metric(ts_metric)

@noexcept
def load_fps_metrics(line):
    [_, _, name, value] = line.split()
    value = float(value)
    append_metric(name, value, response['metrics'])

@noexcept
def load_sendrecv_metrics(line):
    [_, marker, channel, _, _, bytes_per_sec, bps, packets_per_sec, pps] = line.split()
    dest = response['traffic'][marker.strip('[]')]
    append_metric(channel, int(bytes_per_sec), dest['bytes'])
    append_metric(channel, int(packets_per_sec), dest['packets'])

@noexcept
def load_deltapack_value_stat(line):
    line = line[line.find(':')+1:] # remove first characters until first ':'
    [tag, lt1, bits1_3, bits4_6, bits7_9, bits10_12, bits13_15, gt15] = line.split()
    dest = response['value_stat']
    append_metric(tag.strip(), [int(lt1), int(bits1_3), int(bits4_6), int(bits7_9), int(bits10_12), int(bits13_15), int(gt15)], dest)

def load_system_metrics():
    pids = psutil.pids()
    client_pids = list(filter(lambda pid: 'Client' in psutil.Process(pid).name(), pids))
    for pid in client_pids:
        proc = psutil.Process(pid)
        rss_mem = proc.memory_info().rss
        append_metric('mem', rss_mem, response['system'])
    with open(NET_IO_FILENAME, 'r') as tmp_file:
        start_net_io_counters = json.load(tmp_file)
        stop_net_io_counters = psutil.net_io_counters(pernic=True)['eth0']
        send_bytes = stop_net_io_counters.bytes_sent - start_net_io_counters['bytes_sent']
        append_metric('send', send_bytes / len(client_pids), response['system']['traffic'])
        recv_bytes = stop_net_io_counters.bytes_recv - start_net_io_counters['bytes_recv']
        append_metric('recv', recv_bytes / len(client_pids), response['system']['traffic'])

def save_net_io_counters():
    net_io_counters = psutil.net_io_counters(pernic=True)['eth0']
    with open(NET_IO_FILENAME, 'w') as tmp_file:
        json.dump(net_io_counters._asdict(), tmp_file)

@noexcept
def load_generalstat(line):
    line = line[line.find(':')+1:] # remove first characters until first ':'
    [size, ntotal, nempty, nfull] = line.split(';')
    dest = response['snapshotstat']
    append_metric('generalstat', [int(size), int(ntotal), int(nempty), int(nfull)], dest)

@noexcept
def load_componentstat(line):
    line = line[line.find(':')+1:] # remove first characters until first ':'
    [id, name, count, count_new, bits_total, bits_payload] = line.split(';')
    dest = response['snapshotstat']
    append_metric('componentstat', [int(id), name, int(count), int(count_new), int(bits_total), int(bits_payload)], dest)

@noexcept
def load_typestat(line):
    line = line[line.find(':')+1:] # remove first characters until first ':'
    [id, name, count, count_new, bits] = line.split(';')
    dest = response['snapshotstat']
    append_metric('typestat', [int(id), name, int(count), int(count_new), int(bits)], dest)

if __name__ == '__main__':
    assert len(sys.argv) > 1, 'Usage: $> python analyzer_agent.py start|stop'
    if sys.argv[1] == 'start':
        save_net_io_counters()
    elif sys.argv[1] == 'stop':
        load_system_metrics()
        for line in sys.stdin:
            if 'Timestamps:' in line:
                load_ts_metrics(line)
            if 'FPS ' in line:
                load_fps_metrics(line)
            if '[send]' in line or '[recv]' in line:
                load_sendrecv_metrics(line)
            if 'generalstat:' in line:
                load_generalstat(line)
            if 'componentstat:' in line:
                load_componentstat(line)
            if 'typestat:' in line:
                load_typestat(line)
        print json.dumps(response)
