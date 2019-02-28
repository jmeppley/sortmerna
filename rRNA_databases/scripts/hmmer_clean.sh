#!/bin/bash

# usage: run hmmer_clean.sh on rRNA databases (in FASTA format) to remove contaminant rRNA (ex. 23S rRNA found in 16S database)
# purpose: to remove sequences spanning multiple genes (+ITS)
# ex.
#        18S           ITS1     5.8S    ITS2                 28S
# |----------------||--------||-----||---------||----------------------------|
# {*******************************}
#          sequence spans 18s + 5.8S
#
# Requirements:
#   hmmer version 3.1b1 (http://eddylab.org/software/hmmer/hmmer-3.1b1.tar.gz)
#   meta_RNA HMMs version Oct. 21, 2011 (http://weizhong-lab.ucsd.edu/meta_rna/rRNA_prediction.tar.bz2)
#   SILVA SSU and LSU databases, one FASTA file per kingdom
#   remove_partial_seqs.py (included with this bash script)

# Number of threads to use for HMM search
threads=32
# Path to HMM databases
hmmdir=/Users/ekopylova/Desktop/sortmerna-databases/arb/hmms
# Path to FASTA databases (Silva and RFAM)
dbdir=/Users/ekopylova/Desktop/sortmerna-databases/arb/fasta
# Output path for FASTA databases without partial sequences
outdir=/Users/ekopylova/Desktop/sortmerna-databases/arb/fasta-clean
declare -A databases=( ["bacteria_16S_132"]="bac_lsu,bac_tsu" ["archaea_16S_132"]="arc_lsu,arc_tsu" ["bacteria_23S_132"]="bac_ssu,bac_tsu" ["archaea_23S_132"]="arc_ssu,arc_tsu" ["RF00001"]="bac_ssu,bac_lsu,arc_ssu,arc_lsu" ["eukarya_18S_132"]="euk_lsu,euk_tsu" ["eukarya_28S_132"]="euk_ssu,euk_tsu" ["RF00002"]="euk_ssu,euk_lsu" )
# Run HMMER to find partial sequences in all databases (0 = do not run; 1 = run)
findPartialSeqs=0
# Remove partial sequences identified by HMMER from original FASTA databases
removePartialSeqs=1
# Remove partial sequences script path
removePartialSeqsScript=/Users/ekopylova/Desktop/sortmerna-databases/arb/remove_partial_seqs.py

mkdir -p $outdir

for db in "${!databases[@]}"
do
    echo "Database: $db"
    hmms=(${databases["$db"]//,/ })
    for i in "${hmms[@]}"
    do
        if [ $findPartialSeqs -eq 1 ]; then
            echo "HMM: $i"
            # Run HMMER
            echo "nhmmer -o $outdir/${db}_vs_${i}.txt --noali --tblout $outdir/${db}_vs_${i}_table.txt --acc -E 1e-5 --cpu $threads ${hmmdir}/${i}.hmm $dbdir/${db}.fasta"
            nhmmer -o $outdir/${db}_vs_${i}.txt \
                   --noali --tblout $outdir/${db}_vs_${i}_table.txt \
                   --acc -E 1e-5 --cpu $threads \
                   ${hmmdir}/${i}.hmm $dbdir/${db}.fasta
        fi
        # Remove identified partial seqs from original database
        if [ $removePartialSeqs -eq 1 ]; then
            echo "$removePartialSeqsScript --fasta-fp $dbdir/${db}.fasta --tblout-fp $outdir/${db}_vs_${i}_table.txt --output-fp $outdir/${db}_clean.fasta"
            python $removePartialSeqsScript --fasta-fp $dbdir/${db}.fasta \
                                            --tblout-fp $outdir/${db}_vs_${i}_table.txt \
                                            --output-fp $outdir/${db}_clean.fasta
        fi
    done
done
