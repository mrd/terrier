# FILES="`grep 'USE_VMM=1' logs/* | cut -d : -f -4`"

FILES="logs/2015-02-08_18:11:06-05:00.log logs/2015-02-08_18:14:21-05:00.log 
    logs/2015-02-08_18:15:47-05:00.log logs/2015-02-08_18:17:04-05:00.log 
    logs/2015-02-08_18:18:27-05:00.log logs/2015-02-08_18:19:35-05:00.log 
    logs/2015-02-08_18:23:49-05:00.log logs/2015-02-08_18:25:10-05:00.log 
    logs/2015-02-08_18:26:30-05:00.log logs/2015-02-08_18:31:23-05:00.log 
    logs/2015-02-08_18:33:13-05:00.log logs/2015-02-08_18:35:27-05:00.log 
    logs/2015-02-08_18:36:55-05:00.log logs/2015-02-08_18:38:18-05:00.log 
    logs/2015-02-08_18:39:37-05:00.log logs/2015-02-08_19:18:38-05:00.log 
    logs/2015-02-08_19:20:44-05:00.log logs/2015-02-08_19:30:24-05:00.log 
    logs/2015-02-08_19:31:28-05:00.log"


paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.2$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.2$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.2$" $FILES |grep rtt|cut -d / -f 7) \
    | sort -n | tee results/ping-0ms-vary-capacity-nic1.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.2$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.2$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.2$" $FILES |grep rtt|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-nic1.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.2$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.2$" $FILES |grep rtt|cut -d / -f 6) \
    | sort -n | tee results/ping-0ms-1ms-vary-capacity-nic1.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    | sort -n | tee results/ping-0ms-vary-capacity-nic2.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "^sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-nic2.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-parallel.tsv
