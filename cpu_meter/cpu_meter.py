#!/usr/bin/python
#
# cpu_meter.py: reads dstat CPU usage (usr+sys) periodically and send it to cloudbit.
# (requires dstat installed)
#
# Usage: > python cpu_meter.py
#

import subprocess
import re 

# cloudBit params
CBIT_ACCESS_TOKEN='<<PUT YOUR ACCESS TOKEN HERE>>'
CBIT_DEVICE_ID='<<PUT YOUR DEVICE ID HERE>>'

PATTERN_DSTAT_CPU_IDLE = re.compile(r'[^,]+,[^,]+,([\d\.]+),.*')

def exec_dstat():
    p = subprocess.Popen('dstat --output dstat.csv', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

def read_dstat():
    p = subprocess.Popen('tail -n 1 -f dstat.csv', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while True:
        yield p.stdout.readline().strip()

def extract_cpu_usage(line):
    m = PATTERN_DSTAT_CPU_IDLE.match(l)
    if m:
        return str(int(100.0 - float(m.group(1))))
    else:
        return None 

def post_cbit(value):
    cmd = 'curl -i -X POST -H "Authorization: Bearer ' + CBIT_ACCESS_TOKEN + '" https://api-http.littlebitscloud.cc/devices/' + CBIT_DEVICE_ID + '/output -d percent=' + value + ' -d duration_ms=10000' 
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    print 'Executing: ' + cmd

# main
exec_dstat()
for l in read_dstat():
    v = extract_cpu_usage(l)
    if v:
        post_cbit(v)

