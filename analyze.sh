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
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-0ms-vary-capacity-parallel.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-parallel.tsv

FILES="logs/2015-02-09_15:37:12-05:00.log
logs/2015-02-09_16:03:30-05:00.log logs/2015-02-09_16:59:09-05:00.log
logs/2015-02-09_17:15:23-05:00.log logs/2015-02-09_17:39:25-05:00.log
logs/2015-02-09_15:39:34-05:00.log logs/2015-02-09_16:06:01-05:00.log
logs/2015-02-09_17:00:40-05:00.log logs/2015-02-09_17:20:07-05:00.log
logs/2015-02-09_17:43:41-05:00.log logs/2015-02-09_15:41:58-05:00.log
logs/2015-02-09_16:36:35-05:00.log logs/2015-02-09_17:01:55-05:00.log
logs/2015-02-09_17:27:40-05:00.log logs/2015-02-09_17:48:57-05:00.log
logs/2015-02-09_15:49:31-05:00.log logs/2015-02-09_16:55:29-05:00.log
logs/2015-02-09_17:13:09-05:00.log logs/2015-02-09_17:37:53-05:00.log"

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-0ms-vary-capacity-parallel2.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-parallel2.tsv

FILES="logs/2015-02-09_19:05:30-05:00.log
logs/2015-02-09_19:10:34-05:00.log logs/2015-02-09_19:16:12-05:00.log
logs/2015-02-09_19:22:38-05:00.log logs/2015-02-09_19:35:43-05:00.log
logs/2015-02-09_19:06:43-05:00.log logs/2015-02-09_19:12:05-05:00.log
logs/2015-02-09_19:17:33-05:00.log logs/2015-02-09_19:23:49-05:00.log
logs/2015-02-09_19:39:20-05:00.log logs/2015-02-09_19:07:57-05:00.log
logs/2015-02-09_19:13:46-05:00.log logs/2015-02-09_19:20:17-05:00.log
logs/2015-02-09_19:41:12-05:00.log logs/2015-02-09_19:09:20-05:00.log
logs/2015-02-09_19:15:00-05:00.log logs/2015-02-09_19:21:27-05:00.log
logs/2015-02-09_19:52:49-05:00.log logs/2015-02-09_19:56:15-05:00.log"

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.000 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-0ms-vary-capacity-parallel-asix10.tsv

paste \
    <(grep smsclan $FILES | cut -d , -f 8) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 6) \
    <(grep -A1 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|cut -d / -f 7) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 5 | cut -d ' ' -f 3) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 6) \
    <(grep -A2 "\& sudo ping -c 10000 -f -i 0.001 10.0.0.3$" $FILES |grep rtt|sed -n 'g;n;p'|cut -d / -f 7) \
    | sort -n | tee results/ping-1ms-vary-capacity-parallel-asix10.tsv
