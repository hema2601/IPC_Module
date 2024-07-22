#!/bin/bash

ethtool -S ens4 | grep packets > before_pkt.txt
cat /proc/softirqs | grep NET_ > before_soft_irq.txt 
./print_irq_cnt.sh > tmp.txt
cat /proc/interrupts | grep -f tmp.txt > before_irq.txt
