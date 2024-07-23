
current_path=/home/hema/IPC_Module/
remote_client_addr=hema@115.145.179.191
server_ip=10.0.0.2

exp_name=${1:-exp}
intf=${2:-ens10f1}
num_queue=${3:-6}
core_start=${4:-0}
core_num=${5:-6}
time=${6:-100000}
conns=${7:-6}


# Remove Module
rmmod IPC_module

set -e
# Disable irqbalancer
service irqbalance stop

# Setup the Queues
ethtool -L $intf rx $num_queue tx $num_queue 

# Set Mappings
$current_path/scripts/set_affinity.sh $intf $core_start

# Make and Insert Module
make -C $current_path/module -f Makefile
insmod $current_path/module/IPC_module.ko

# Build Accessor
gcc accessor.c

# Get "Before" Values
$current_path/scripts/before.sh $intf

# Run iperf3
taskset -c "$core_start-$((core_start + core_num - 1))" iperf3 -s -1 & ssh $remote_client_addr "iperf3 -c ${server_ip} -P ${conns} > /dev/null"&

# Wait a little
sleep 6

# Run accessor
./a.out -t $time -c $core_start-$((core_start + core_num - 1)) $exp_name 

# Wait for iperf3 to exit [Optional]


# Get "After" Values
$current_path/scripts/after.sh $exp_name $intf

# Apply File Transformation
python3 file_formatter.py $exp_name IRQ SOFTIRQ PACKET_CNT

