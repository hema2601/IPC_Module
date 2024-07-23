#!/bin/bash

intf=${1:-ens10f1}


ethtool -S ${intf} | grep packets > before_pkt.txt
cat /proc/softirqs | grep NET_ > before_soft_irq.txt 
./scripts/print_irq_cnt.sh $intf > tmp.txt
cat /proc/interrupts | grep -f tmp.txt > before_irq.txt
