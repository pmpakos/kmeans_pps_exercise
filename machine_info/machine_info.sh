#!/bin/bash

dir='machine_info_data'
mkdir -p "$dir"

id &> "$dir"/id.txt

uname -a &> "$dir"/uname_-a.txt

cat /etc/os-release &> "$dir"/etc_os-release.txt

gcc -v &> "$dir"/gcc.txt

lscpu &> "$dir"/lscpu.txt
lscpu --caches &> "$dir"/lscpu_--caches.txt

# lshw &> "$dir"/lshw.txt
# lshw -short &> "$dir"/lshw_-short.txt

# lsmem &> "$dir"/lsmem.txt
# lsmem -a &> "$dir"/lsmem_-a.txt

module avail &> "$dir"/module_avail.txt

numactl -s &> "$dir"/numactl_-s.txt
numactl -H &> "$dir"/numactl_-H.txt

# module load papi
# papi_avail &> "$dir"/papi_avail.txt
# papi_avail -a &> "$dir"/papi_avail_-a.txt
# papi_avail -c &> "$dir"/papi_avail_-c.txt
# papi_native_avail &> "$dir"/papi_native_avail.txt
# papi_native_avail -c &> "$dir"/papi_native_avail_-c.txt

cat /proc/cpuinfo &> "$dir"/proc_cpuinfo.txt

# {
#     for f in /sys/class/hwmon/hwmon*/name; do
#         ls "$f"
#         cat "$f"
#     done
#     for f in /sys/class/hwmon/hwmon*/temp*_label; do
#         ls "$f"
#         cat "$f"
#     done
#     for f in /sys/class/hwmon/hwmon*/temp*_input; do
#         ls "$f"
#         cat "$f"
#     done
# } &> "$dir"/hwmon_temp.txt

# {
#     for f in /sys/class/powercap/intel-rapl:*/name; do
#         ls "$f"
#         cat "$f"
#     done
#     for f in /sys/class/powercap/intel-rapl:*/max_energy_range_uj; do
#         ls "$f"
#         cat "$f"
#     done
# } &> "$dir"/rapl.txt

