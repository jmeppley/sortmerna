#!/bin/bash

# usage: run generate_databases.sh on cleaned rRNA databases (in gzipped FASTA format)
#  to cluster and genrate fasta file of cluster representatives
#
# Requirements:
#   vsearch (v2.10.4)


rootdir=/home/jmeppley/lus/sortmerna/nexus
indir=${rootdir}/fasta-clean
outdir=${rootdir}/fasta-final
mkdir -p ${outdir}

vsearch_threads=20

reference_db=(bacteria_16S_132 archaea_16S_132 eukarya_18S_132 bacteria_23S_132 archaea_23S_132 eukarya_28S_132 RF00001 RF00002)
id=(0.90 0.95 0.95 0.98 0.98 0.98 0.98 0.98)

for ((i=0;i<${#reference_db[@]};i++));
do
    db=${reference_db[i]}
    fasta_gz=${indir}/${db}_clean.fasta.gz
    reps_fasta=${outdir}/${db}.id${id[i]}.fasta
    cmd="vsearch --cluster_fast ${fasta_gz} --gzip_decompress \
                 --id ${id[i]} --iddef 2 --strand plus \
                 --no_progress --threads ${vsearch_threads} \
                 --centroids ${reps_fasta}"
    echo ${cmd}
    `$cmd`

    #echo "sumaclust -l -p 40 -t ${id[i]} -F $rootdir/step4-c/${reference_db[i]} $rootdir/${reference_db[i]}" | qsub -l nodes=1:ppn=40 -q mem4gbq -k oe -N ${db}_sumaclust; sleep 2
done
