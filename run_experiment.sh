
current_path=/home/hema/IPC_Module/
remote_client_addr=hema@115.145.178.17
server_ip=10.0.0.3

exp_name=${1:-exp}
intf=${2:-ens4}
num_queue=${3:-6}
core_start=${4:-0}
core_num=${5:-6}
time=${6:-100000}
conns=${7:-3}

IPERF_BIN=iperf3

if command -v iperf3_napi &> /dev/null
then
	IPERF_BIN=iperf3_napi
    echo "Using custom iperf3"
fi



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
taskset -c "$core_start-$((core_start + core_num - 1))" $IPERF_BIN -s -1 -J > $current_path/iperf.json & ssh $remote_client_addr "iperf3 -c ${server_ip} -P ${conns} > /dev/null"&
IPERF_PID=$!

# Wait a little
sleep 4

# Run accessor
./a.out -t $time -c $core_start-$((core_start + core_num - 1)) $exp_name 

# Get "After" Values
$current_path/scripts/after.sh $exp_name $intf

# Wait for iperf3 to exit [Optional]
tail --pid=$IPERF_PID -f /dev/null
echo "Iperf ended"

# mv iperf json to data folder
mv $current_path/iperf.json $current_path/data/$exp_name/

# Apply File Transformation
python3 file_formatter.py $exp_name IRQ SOFTIRQ PACKET_CNT IPERF

