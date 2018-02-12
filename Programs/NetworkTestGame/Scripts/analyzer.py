import sys
import json
import itertools
import argparse

P50 = 0
P75 = 1
P90 = 2
P95 = 3
PERCENTILES = (P50, P75, P90, P95)
REVERSED_METRICS = ('FPS',)

def get_percentiles(values, reverse=False):
    values = sorted(values, reverse=reverse)
    p50 = len(values) / 2
    p75 = len(values) * 3 / 4
    p90 = len(values) * 9 / 10
    p95 = len(values) * 19 / 20
    return values[p50], values[p75], values[p90], values[p95]

def print_metrics(metrics):
    print 'Metrics:'
    for name, values in metrics.iteritems():
        p50, p75, p90, p95 = get_percentiles(values, name in REVERSED_METRICS)
        print '{0}: p50={1}, p75={2}, p90={3}, p95={4}'.format(name, p50, p75, p90, p95)

def print_traffic(traffic):
    print 'Traffic:'
    for direction, stats in traffic.iteritems():
        direction_name = 'Upload' if direction == 'send' else 'Download'
        for metric, channel_stats in stats.iteritems():
            metric_name = 'Bytes/s' if metric == 'bytes' else 'Packets/s'
            total_values = map(sum, list(itertools.izip_longest(*channel_stats.values(), fillvalue=0)))
            percentiles = get_percentiles(total_values)
            print '{0} {1}: p50={2}, p75={3}, p90={4}, p95={5}'.format(
                direction_name, metric_name,
                percentiles[P50], percentiles[P75],
                percentiles[P90], percentiles[P95])

def print_system(system, args):
    print 'System:'
    p50, p75, p90, p95 = [p / (1024 * 1024) for p in get_percentiles(system['mem'])]
    print 'Memory MB: p50={0}, p75={1}, p90={2}, p95={3}'.format(p50, p75, p90, p95)
    recv_bytes = system['traffic']['recv'][0]
    send_bytes = system['traffic']['send'][0]
    total_traffic = recv_bytes + send_bytes
    percentage = total_traffic * 100 / args.max_client_traffic
    print 'Traffic Bytes: {0} ({1}/{2}), {3}% limit: {4}'.format(total_traffic, send_bytes, recv_bytes,
                                                                 percentage, args.max_client_traffic)

def print_snapshot_stat(snapshotstat):
    generalstat = snapshotstat['generalstat']
    componentstat = snapshotstat['componentstat']
    typestat = snapshotstat['typestat']

    g_size=0
    g_ntotal=0
    g_nempty=0
    g_nfull=0
    for size, ntotal, nempty, nfull in generalstat:
        g_size+=size
        g_ntotal+=ntotal
        g_nempty+=nempty
        g_nfull+=nfull

    print 'general stat: byte_size={0} ntotal={1} nempty={2} nfull={3}'.format(g_size, g_ntotal, g_nempty, g_nfull)

    c = {}
    for cid, name, count, count_new, total_bits, payload_bits in componentstat:
        if cid in c:
            c[cid][1]+=count
            c[cid][2]+=count_new
            c[cid][3]+=total_bits
            c[cid][4]+=payload_bits
        else:
            c[cid]=[name, count, count_new, total_bits, payload_bits, {}]

    for cid, type, count, count_new, bits in typestat:
        if type in c[cid][5]:
            c[cid][5][type][1]+=count
            c[cid][5][type][2]+=count_new
            c[cid][5][type][3]+=bits
        else:
            c[cid][5][type]=[type, count, count_new, bits]

    for cid in sorted(c):
        name=c[cid][0]
        count=c[cid][1]
        count_new=c[cid][2]
        total_bits=c[cid][3]
        payload_bits=c[cid][4]
        t=c[cid][5]
        print '{0}.{1}: nadded={2} nchanged={3} total_bits={4} overhead={5} avg_per_item={6:.1f}'.format(
            cid, name, count_new, count - count_new, total_bits, total_bits - payload_bits, float(payload_bits) / count)

        for name in t:
            count=t[name][1]
            count_new=t[name][2]
            bits=t[name][3]
            print '    {0}: nfull={1} ndelta={2} bits={3} avg_per_item={4:.1f}'.format(
                name, count_new, count - count_new, bits, float(bits) / count)

def check_metrics(response, args):
    fails = []
    metrics = response['metrics']
    fps_stats = metrics['FPS']
    frame_durations_ms = [1000.0 / fps for fps in fps_stats]
    percentiles = get_percentiles(frame_durations_ms)
    if percentiles[P95] > args.max_frame_duration:
        fails.append('Frame duration exceeded the limit: {0:0.2f} > {1:0.2f} ms'.format(
                     percentiles[P95], args.max_frame_duration))
    latencies_ms = map(sum, zip(metrics['InputToSend'], metrics['RecvToFrame'],
                                metrics['BufToSend'], metrics['InsideBuf']))
    percentiles = get_percentiles(latencies_ms)
    if percentiles[P95] > args.max_inner_latency:
        fails.append('Inner latency exceeded the limit: {0} > {1} ms'.format(
                     percentiles[P95], args.max_inner_latency))
    percentiles = [p / (1024 * 1024) for p in get_percentiles(response['system']['mem'])]
    if percentiles[P95] > args.max_client_memory:
        fails.append('Client memory usage exceeded the limit: {0} > {1} MB'.format(
                     percentiles[P95], args.max_client_memory))
    system_traffic = response['system']['traffic']
    total_traffic = system_traffic['recv'][0] + system_traffic['send'][0]
    if total_traffic > args.max_client_traffic:
        fails.append('Total traffic exceeded the limit: {0} > {1} bytes'.format(
                     total_traffic, args.max_client_traffic))
    return fails

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--max-frame-duration', type=float, default=16.8,
                            help='max frame duration in millisseconds (default=16.8)')
    arg_parser.add_argument('--max-inner-latency', type=int, default=100,
                            help='max inner latency in millisseconds (default=100)')
    arg_parser.add_argument('--max-client-memory', type=int, default=64,
                            help='max client memory in MB (default=64)')
    arg_parser.add_argument('--max-client-traffic', type=int, default=100*1024*1024,
                            help='max client traffic in bytes (default=100*1024*1024)')
    args = arg_parser.parse_args()
    data = sys.stdin.read()
    response = json.loads(data)
    print_metrics(response['metrics'])
    print_traffic(response['traffic'])
    print_system(response['system'], args)
    print_snapshot_stat(response['snapshotstat'])

    fails = check_metrics(response, args)
    if len(fails) > 0:
        for msg in fails:
            print >>sys.stderr, '[FAIL]: {0}'.format(msg)
        sys.exit(1)
