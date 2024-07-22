
current_path=/home/hema/IPC_Module/
remote_client_addr=hema@115.145.178.17

exp_name=${1:-exp}
intf=${2:-ens4}
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
$current_path/set_affinity.sh $intf $core_start

# Make and Insert Module
make -C $current_path/module -f Makefile
insmod $current_path/module/IPC_module.ko

# Build Accessor
gcc accessor.c

# Get "Before" Values
$current_path/before.sh

# Run iperf3
taskset -c "$core_start-$((core_start + core_num - 1))" iperf3 -s -1 & ssh $remote_client_addr "iperf3 -c 10.0.0.3 -P ${conns} > /dev/null"&

# Wait a little
sleep 2
echo "Now!"
# Run accessor
./a.out -t $time -c $core_start-$((core_start + core_num - 1)) $exp_name 

# Wait for iperf3 to exit [Optional]


# Get "After" Values
$current_path/after.sh $exp_name

# Apply File Transformation
python3 file_formatter.py $exp_name IRQ SOFTIRQ PACKET_CNT

