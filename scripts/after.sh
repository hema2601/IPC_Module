#!/bin/bash

name=${1}
intf=${2:-ens10f1}

ethtool -S $intf | grep packets > after_pkt.txt
cat /proc/softirqs | grep NET_ > after_soft_irq.txt 
cat /proc/interrupts | grep -f tmp.txt > after_irq.txt 


cat before_pkt.txt after_pkt.txt > data/$name/packet_cnt.json
cat before_soft_irq.txt after_soft_irq.txt > data/$name/softirq.json
cat before_irq.txt after_irq.txt > data/$name/irq.json

rm before_pkt.txt
rm after_pkt.txt
rm before_soft_irq.txt
rm after_soft_irq.txt
rm before_irq.txt
rm after_irq.txt
