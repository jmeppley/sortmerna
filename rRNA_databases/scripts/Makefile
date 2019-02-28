#
# Makefile to generate SMR files from latest silva
# 
# from the rRNA_databases folder run:
#  make 
#
# Install Requirements:
#
# make
# bash 4
# curl
# Python 3 (for scikit-bio library, installed using GitHub instructions with miniconda)
# hmmer (For finding partial seas
#   http://eddylab.org/software/hmmer/hmmer-3.1b1.tar.gz)
#
# For finding partial seas
# http://weizhong-lab.ucsd.edu/meta_rna/rRNA_hmm_fs.tar.gz

REL=132
FTP_ROOT=ftp://ftp.arb-silva.de
FTP_BASE:=$(FTP_ROOT)/release_$(subst .,_,$(REL))/Exports
SSU_FASTA=SILVA_$(REL)_SSURef_tax_silva_trunc.fasta
SSU_URL=$(FTP_BASE)/$(SSU_FASTA).gz
LSU_FASTA=SILVA_$(REL)_LSURef_tax_silva_trunc.fasta
LSU_URL=$(FTP_BASE)/$(LSU_FASTA).gz

# 1. download files from SILVA
downloads:
	mkdir -p downloads

downloads/$(SSU_FASTA): | downloads
	@echo downloading SSU from silva
	curl $(SSU_URL) | gunzip -c > $@

downloads/$(LSU_FASTA): | downloads
	@echo downloading LSU from silva
	curl $(LSU_URL) | gunzip -c > $@

# 2. Run extract_kingdom_arb.py to extract FASTA sequences per kingdom into separate FASTA files:
build:
	mkdir -p build

build/SILVA_132_SSURef_tax_silva_trunc_bacteria.fasta: downloads/$(SSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Bacteria --output-fp $@

build/SILVA_132_SSURef_tax_silva_trunc_archaea.fasta: downloads/$(SSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Archaea --output-fp $@

build/SILVA_132_SSURef_tax_silva_trunc_eukaryota.fasta: downloads/$(SSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Eukaryota --output-fp $@

build/SILVA_132_LSURef_tax_silva_trunc_bacteria.fasta: downloads/$(LSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Bacteria --output-fp $@

build/SILVA_132_LSURef_tax_silva_trunc_archaea.fasta: downloads/$(LSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Archaea --output-fp $@

build/SILVA_132_LSURef_tax_silva_trunc_eukaryota.fasta: downloads/$(LSU_FASTA) | build
	python scripts/extract_kingdom_arb.py --silva-fasta-fp $^ --kingdom Eukaryota --output-fp $@

# 3. Download latest RFAM databases for 5S and 5.8S:
downloads/RF00001.fasta: | downloads
	curl ftp://ftp.ebi.ac.uk/pub/databases/Rfam/CURRENT/fasta_files/RF00001.fa.gz | gunzip -c > $@

downloads/RF00002.fasta: | downloads
	curl ftp://ftp.ebi.ac.uk/pub/databases/Rfam/CURRENT/fasta_files/RF00002.fa.gz | gunzip -c > $@

# 4. Remove partial sequences using HMMER and Meta-RNA HMMs

Set the correct paths for input FASTA files in hmmer_clean.sh and run:

$ bash ./hmmer_clean.sh

5. TODO: Cluster cleaned databases using VSEARCH (it is better supported than SUMACLUST)

NOTE: See generate_database.sh for the old code to do this with SUMACLUST

https://github.com/biocore/sortmerna/blob/master/rRNA_databases/scripts/generate_databases.sh

6. TODO MAYBE: Extract only the clusters (this was a step needed to be done in SUMACLUST, not sure about VSEARCH)

awk '/^>/ {printf("\n%s\t",$0);next;} {printf("%s",$0);} END {printf("\n");}' sumaclust_output_bacteria_16S_119-T.fasta \
           | egrep -v '^$' \
           | grep "cluster_center=True;" \
           | tr "\t" "\n" \
           > silva-bac-16s-id90.fasta 

In any case, the clustered sequences should be distributed at SMR databases.






