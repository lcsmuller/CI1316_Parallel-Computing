for i in {1..4}; do
    cd "${i}threads"
    cat slurm-* > ./concat.txt
    echo "Times for ${i} threads:" >> ../"${i}threads.txt"
    grep 'Time taken' ./concat.txt | cut -d ' ' -f3 >> ../"${i}threads.txt"
    echo "Throughput for ${i} threads:" >> ../"${i}threads.txt"
    grep 'Throughput' ./concat.txt | cut -d ' ' -f2,3 >> ../"${i}threads.txt"
    cd ..
done

cat *threads.txt >> report.txt