NM=arm-none-linux-gnueabi-nm
PROGS=`grep "^PROGS" Makefile | cut -f 2 -d = | cut -f 1 -d \#`
mkdir -p logs

LOGFILE="`date --rfc-3339=seconds | sed -e 's/ /_/g'`.log"
exec > >(tee logs/"$LOGFILE")

trap "rm -f logs/\"$LOGFILE\";exit 1" SIGINT SIGTERM

function is_vmm {
    if [ -z "`$NM terrier.elf | grep vmm_map_region`" ]; then
        echo "USE_VMM=0"
    else
        echo "USE_VMM=1"
    fi
}

function get_parameters {
    declare -a CPU
    CPU[0]=""
    CPU[1]=""
    for p in $PROGS; do
        P=`basename $p`
        C=`grep "^unsigned int _scheduler_capacity" $p/*.cats | cut -f 2 -d = | cut -f 1 -d \;`
        T=`grep "^unsigned int _scheduler_period" $p/*.cats | cut -f 2 -d = | cut -f 1 -d \;`
        A=`grep "^unsigned int _scheduler_affinity" $p/*.cats | cut -f 2 -d = | cut -f 1 -d \;`
        A=$(($A - 1))
        CPU[A]="${CPU[A]} $P, ${C/ <</,}, ${T/ <</,},"
    done
    echo CPU0,${CPU[0]}
    echo CPU1,${CPU[1]}
}

function is_ready {
    ping -w 1 -c 1 10.0.0.$1 2>&1 > /dev/null
}

function run_ping {
    CMD="sudo ping -c 10000 -f -i 0.00$2 10.0.0.$1"
    echo $CMD
    eval $CMD | grep "^rtt"
}

function run_parallel_pings {
    {
    coproc PING1 (
        run_ping 2 $@
    )
    coproc PING2 (
        run_ping 3 $@
    )

    read P1C <&${PING1[0]}
    read P2C <&${PING2[0]}
    read P1R <&${PING1[0]}
    read P2R <&${PING2[0]}

    wait

    echo $P1C \& $P2C
    echo $P1R
    echo $P2R
    } 2>/dev/null
}

echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
echo

is_vmm
get_parameters

while ! is_ready 2; do
    sleep 1
done

while ! is_ready 3; do
    sleep 1
done

echo

run_ping 2 1
run_ping 3 1
run_ping 2 0
run_ping 3 0

run_parallel_pings 1
run_parallel_pings 0
